# PSP Go Classic HID v0.35 — GameStick exit acknowledgement wait

Test fix for the remaining game-to-VSH hang. v0.34 game entry is confirmed working by the user; exit success on v0.35 is not yet established.

## Evidence

The v0.34 log records an actual native status timeout802f010e followed by a successful reply, then a GAME boot. During exit it records the launch thread waiting on SceBSMan_Seq, held once by the sleeping main SceBSMan thread. This differs from the launch stall: its owner was SceBSMan_W, holding the mutex twice. No native teardown markers appeared in the failed exit. The log does not identify the exact pending reply, so the second-stage status-event wait must not be assumed to be the cause.

## Change

Preserve v0.34's confirmed worker status0185 fix. Extend the same supported native2-second acknowledgement timeout to the main BSMan thread, only for expected acknowledgement IDs0100..01ff, callback mode0, no existing timeout and an adapter-authorized GameStick identity0f0d:1011. Both thread IDs come from guarded native module state. Other threads, controllers, response IDs, callback-mode calls and existing timeouts retain original behavior. All11 instruction guards and module/branch/bounds checks remain.

The native receiver is called once and its result is preserved. Its existing alarm/waiter cleanup path returns the timeout and its caller releases its own sequence lock. No mutex is force-unlocked, thread killed or success fabricated. No new raw radio packets are sent. The change is in RAM in the PRX; no firmware flashing. This main-thread extension is broader than v0.34's single status reply because the captured main-thread stall has not exposed its exact acknowledgement ID. A timeout may return a native command error; effect on exit/reconnection still requires hardware testing.

Completed bounded waits now log STATUS_WAIT with reply ID and role1(worker)/2(main), count/errors/result and budget. Only the most recent completed wait is summarized on the logger tick; this is not a complete command history. Existing LIFE thread/object/owner and teardown diagnostics remain. No callback writes files.

## Test

Replace ef0:/seplugins/pspgo_hci_transport.prx, retain VSH/GAME entries and saved registration, and fully restart. Keep NES30 off. Connect GameStick, enter a game, then use HOME/quit with the controller still on. Allow up to15seconds for native retries. If VSH returns, check controller input/reconnection. Send pspgo_hci_transport.log with the outcome. No re-pairing. v0.34 remains the confirmed game-entry fallback; controller off during transitions remains a workaround.

## Validation

Pinned PSP cross-build,17 ASan/UBSan test programs and compiled report/observer/once-only-forwarding checks pass. Policy tests cover all65536 reply IDs across32 timeout/callback/identity/role combinations, including unknown roles and exact original arguments/results. Native indirect stack and hardware success are not fully proven. Own MIT source/tests and SDK notices included. No Sony firmware/disassembly, raw private logs, Bluetooth addresses or keys distributed.
