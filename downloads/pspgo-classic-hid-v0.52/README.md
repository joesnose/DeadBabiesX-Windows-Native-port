# PSP Go Classic HID v0.52 — input timing test

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep existing entries/pairing and fully restart. No EBOOT or Game app. Same single log: `ef0:/seplugins/pspgo_hci_transport.log`.

Connect the DS4 as before, enter the same game and move/tap controls while the delay is noticeable for at least 15 seconds. If the controller disconnects at game entry, reconnect it using the method that worked on v0.50/v0.51. Send the same single log afterward.

## Findings from the supplied v0.51 log

The GAME native legacy reconnect request returns 802F0140, followed by a failed-link event; a later incoming connection succeeds and provides controls. The subsequent game session decodes thousands of report01 packets without invalid, unmatched or busy reports, and the PSP game input hooks merge them. The log has no report-arrival or game-read timing, so these facts do not establish the cause of lag.

## Change

Report updates and game input reads previously called the 64-bit system clock and a software 64-bit division while copying the current controller state. This build uses the low32 microsecond clock and unsigned elapsed-time arithmetic instead. Native read/merge behavior and the 1.5-second stale-state policy remain intact. Wrap and expiry are tested.

One `INPUT_TIMING` summary is appended to the same log about every five seconds while input is being received/read:

- `reports`: report updates in this window.
- `gap_max_us` / `gap_mean_us`: time between reports reaching the host adapter. The maximum can include a long gap between windows; gaps longer than 1.5 seconds are excluded from the mean.
- `samples`: game/VSH reads with a report younger than 1.5 seconds.
- `age_max_us` / `age_mean_us`: time from host report receipt to the controller read. The maximum also includes stale reports; stale reads are excluded from the mean.
- `stale`: reads whose last report is at least 1.5 seconds old.

These are host-side measurements, not physical button-to-screen latency or a measurement of queues inside the controller/radio. The live-session keepalive does not reset the report-receipt timestamp. Counter snapshots reset the window, not the active controller state. No report payloads or addresses are logged by these summaries. Bulk thread profiling remains removed.

## Limits

This is an optimization and timing test, not a confirmed lag fix. The automatic reconnect request still uses v0.51's one native GAME attempt; its observed failure is not fixed by this release. There is no retry storm or fabricated successful-link state. Preventing a disconnect across PSP Bluetooth service reload remains unresolved.

VSH/GAME mappings, native authentication/resource cleanup and basic DS4 report01 decoding are retained. Full report11/DualSense support is not added.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Only one enabled driver per context. Keep unrelated plugin settings and your successful controller registration.

## Validation

Forced PSP SDK build, all 28 ASan/UBSan host programs and twelve compiled-code checks pass. Input tests exercise low32 timer wrap, expiry, keepalive, clearing, exact aggregate read age/gap, stale exclusion and snapshot reset. Compiled checks confirm the report-update and input-merge hot paths use the low32 clock without division/file calls, preserving 16-/40-byte frames. Actual hardware latency improvement is unverified.

Own source/tests are separate. No Sony firmware/disassembly, private raw logs, controller addresses or pairing keys are published.
