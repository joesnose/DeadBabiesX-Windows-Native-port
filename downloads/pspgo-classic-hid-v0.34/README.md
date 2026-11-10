# PSP Go Classic HID v0.34 — bounded GameStick status wait

Test fix for the captured game-entry/exit hang; PSP hardware success is not yet established.

## Evidence and change

v0.33 identifies the launch thread waiting on native mutex SceBSMan_Seq. Its lock count is2 and owner is SceBSMan_W, sleeping, across repeated snapshots. VSH graphics waits for launch completion. The firmware status-query path acquires this mutex, then its command helper acquires it recursively and waits for a radio acknowledgement without a timeout. That path is consistent with the captured lock state; the log does not directly record the pending command number.

v0.34 changes one guarded internal call: when the expected reply is0185 (controller status), no timeout was supplied, callback mode is0, the calling thread is the native BSMan worker, and the HID adapter has an authorized GameStick identity0f0d:1011, supply a fresh2-second timeout to the original native receiver. All other calls retain original arguments and behavior. The native queue already supports an alarm-based timeout, waiter unlink/free and error return; the command and status routines then release their own recursive mutex acquisitions. No mutex is force-unlocked, worker killed, callback skipped or success fabricated. The original receiver is called once and its result preserved.

This is narrow: it does not add timeouts to arbitrary pairing/service requests, change NES30 handling, or assume every controller is a DS3. Existing control routing, pairing, native teardown and diagnostic capture are retained. Firmware word/bounds/branch guards cause installation to be skipped on mismatch. No firmware flashing.

## Test

Replace ef0:/seplugins/pspgo_hci_transport.prx, keep existing VSH/GAME entries and saved registration, and fully restart. Keep NES30 off. Connect GameStick, wait5seconds, then launch a game while it remains on. Allow up to15seconds for native status retries rather than immediately treating a delay as a freeze. If it enters, test controls and HOME/quit back to VSH. Send pspgo_hci_transport.log with the result. No re-pairing required. If it still hangs, leave15seconds before recovery; controller off during transition remains the known workaround.

STATUS_WAIT installed confirms the guarded call was patched. STATUS_WAIT bounded/errors/last counts completed bounded waits and their native result; no completion message can mean the path was never selected or logging stopped, so it is not conclusive alone. Existing LIFE_MUTEX/OWNER/THREAD diagnostics remain. A timeout can cause a native status-query error and retries; late radio replies and hardware lifecycle behavior still require the test.

## Verification and source

Pinned PSP SDK build and17 ASan/UBSan test programs pass. New policy tests cover all65536 reply IDs and16 timeout/callback/identity/worker combinations, fresh timeout budgets, exact original arguments/results and all11 native context guards. Compiled wrapper checks require one original call per return path and no direct file logging. Wrapper frame32bytes; identity check frame8; the native worker has2048bytes. These checks do not prove every indirect native call's maximum stack use or hardware success. Report callback remains56bytes with prior conservative392/1024-byte known budget.

Own MIT source/tests and SDK notices included. Factual offsets are derived from private analysis; no Sony firmware/disassembly, raw private device logs, Bluetooth addresses or keys distributed. This version supersedes diagnostics0.31–0.33 for this specific test.
