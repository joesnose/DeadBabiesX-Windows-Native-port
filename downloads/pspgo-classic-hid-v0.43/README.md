# PSP Go Classic HID v0.43

NES30 Pro Square/Circle swap and a targeted game-exit fix candidate for PSP Go with ARK. The user confirmed v0.42 works in game, but Square/Circle need swapping and exit hangs before the PSP shuts down. This build has not yet been tested on the PSP. The exact v0.42 exit stall has not been captured in a new log, so the completion waits below are identified code gaps, not a proven crash diagnosis.

## Install and test

1. Replace ef0:/seplugins/pspgo_hci_transport.prx. Keep the existing VSH/GAME entries, Bluetooth pairing and pspgo_hid_target.dat. Fully shut down and restart.
2. Connect NES30 in VSH and wait five seconds. Check the face buttons, then launch a game and check controls.
3. Use HOME and Exit Game with the controller on. Allow up to 30 seconds for this test because native retries may make several bounded waits. Check that VSH returns and controls reconnect.
4. If exit still hangs or the PSP shuts down, send ef0:/seplugins/pspgo_hci_transport.log after restarting. Do not keep repeating a shutdown failure; the next change needs the failed exit trace.

No re-pairing, additional file to copy or manual mode switch is needed. v0.42 remains the user-confirmed in-game fallback, with the known exit failure.

## Changes

Only the NES30 face map changes: Square and Circle exchange places relative to v0.42. Cross, Triangle and every other NES30 input retain their v0.42 mapping. Other controllers retain their existing maps. Remembered-controller game reconnect and the captured NES30 descriptor workaround remain unchanged.

The earlier ACK timeout covers the immediate command response. It does not cover these later completion stages in native sceBSMan:

- A controller-status request can receive ACK0185 indicating pending, then wait without a timeout for status event0275 while retaining the native sequence mutex. At that one guarded caller, this build supplies a fresh five-second native receive budget only for event0275/count1/capacity16/callback0 with no existing timeout. The original receiver runs once. On timeout the existing native error branch releases its own mutex and returns the error without copying absent status data.
- A disconnect request0069 can receive ACK0169 indicating pending, then wait indefinitely for completion flag2. At that one guarded indirect call, HID-profile requests with a valid event, expected flag/mode/output and no existing timeout receive a fresh five-second native event wait budget. The bridge forwards Sony's actual WaitEventFlag or WaitEventFlagCB selection, preserving callback mode. Non-HID requests and other arguments retain original behavior. On error, the native function returns before deleting the completion event. A timeout is not represented as a successful disconnect; pending native state and late completion stay native-owned.

Both patches require all 31 relocated instruction/context guards and module/branch bounds to match, including the source of the profile bits used by the bridge. They change two call instructions in RAM. They do not replace a global event receiver, force-unlock a mutex, kill a thread, fabricate success, change Bluetooth identity or flash firmware. The status-result call is shared native code; like the retained generic ACK timeout, it is not gated by controller brand. These bounds do not establish that every shutdown wait is covered or that game exit/USB shutdown is fixed.

## Diagnostics

COMPLETION_WAIT installed confirms the two call-site patches matched. COMPLETION_CALL kind1 is the status-result phase; kind2 is HID disconnect completion. bounded1 means a timeout was supplied; done0/done1 indicate the last recorded call's begin/end. The native numeric result is preserved. Metadata is copied to bounded RAM under interrupt protection; formatting and file access remain on the owned logger worker. Fast/concurrent calls can overwrite a snapshot and filesystem teardown can stop logging, so absence of a marker is not conclusive by itself. Existing STATUS_CALL and LIFE diagnostics remain.

## Validation

A full forced cross-build passed with the pinned PSP SDK image and strict warnings. All 22 host programs passed with ASan/UBSan. The new test covers every status event ID across argument combinations, all 65536 service-profile words, fresh budgets, original error/output forwarding, invalid plans and every firmware guard. The NES30 descriptor and callback tests check the requested face map; the remaining controller tests retain existing mappings.

Seven compiled checks passed: report stack, pairing trace, lifecycle, ACK, SDP, NES30 descriptor and completion wrappers. New wrappers each use 40-byte frames; numeric metadata helpers use 16 bytes and call only IRQ functions. The assembly bridge uses no stack and preserves RA and all registers except the extra argument registers t1/t2, conveying original callback target S2 and profile S4. Each C wrapper invokes its original exactly once per terminating path and performs no file logging. Logger frame480 runs on its8192-byte stack. The known report-path budget remains392/1024 bytes. These checks do not prove every indirect native stack bound or real hardware behavior.

Source, tests and static HID descriptor included. No Sony firmware/disassembly, private raw captures, addresses or link keys included.
