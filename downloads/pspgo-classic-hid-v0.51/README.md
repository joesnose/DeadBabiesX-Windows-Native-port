# PSP Go Classic HID v0.51

One PRX, one log, no EBOOT or Game app.

Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this build. Keep the v0.50 ARK entries, pairing and preferred-target file. Fully restart, connect the DS4 as before, then test input in VSH and a game. Check whether entering the game reconnects without cycling controller power. If needed, press PS alone; do not re-pair.

Send only `ef0:/seplugins/pspgo_hci_transport.log` after the test if lag or reconnect problems remain. This is the same fixed log as v0.50; the startup banner identifies version 0.51.

## Changes

v0.50 was confirmed to provide DS4 controls, including in a game after the controller was restarted. This candidate targets the reported lag and transition reconnect problem:

- Removes the bulk thread/mutex/event-object profiler left over from crash investigations.
- Logs raw-report summaries only for the first packet, format changes or errors, instead of every half second while packets arrive.
- Lowers the background logging worker's priority from 0x30 to 0x60. The native input-report thread's priority and direct input merge are unchanged.
- Allows one native legacy-mode reconnect attempt for the freshly validated preferred DS4 in GAME mode, matching the 0202 route seen on its successful incoming link. VSH retains the working passive incoming path. An already observed incoming link or active controls suppress the extra request. Other controllers retain their reconnect behavior.

This is a hardware-untested candidate. The transition can still drop the physical connection while the PSP reloads its Bluetooth services. Automatic reconnection depends on the controller answering the native request. Only real successful link events authorize input/metadata; connection errors remain errors, with no repeated reconnect loop. The changes do not establish that tracing caused the lag or guarantee reconnection without pressing PS.

Basic DS4 report01 input only; full report11 and DualSense support are not added. Existing mappings and native resource cleanup remain unchanged.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep one enabled Bluetooth-driver entry per context; preserve unrelated plugins. The included entries file is a reference, not your entire configuration.

## Validation

Forced PSP SDK build, 28 ASan/UBSan host test programs and eleven compiled-code checks passed. The transition integration test verifies fresh preferred-device matching, GAME legacy-mode selection, VSH passive behavior, real-link metadata gating, preserved native failures, active/incoming-link suppression and stale-registration isolation. Hardware latency and reconnect remain unverified.

Own source/tests are provided separately. No Sony firmware/disassembly, private raw logs, controller addresses or pairing keys are included.
