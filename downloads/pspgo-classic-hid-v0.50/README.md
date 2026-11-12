# PSP Go Classic HID v0.50

One plugin: `pspgo_hci_transport.prx`.
One log: `ef0:/seplugins/pspgo_hci_transport.log`.
No EBOOT, Game app, boot markers or versioned log files.

## Install

1. Copy the ZIP's `seplugins/pspgo_hci_transport.prx` onto internal storage at `ef0:/seplugins/pspgo_hci_transport.prx`.
2. In your existing ARK plugin configuration, replace the old Bluetooth-driver entries with:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep only one enabled Bluetooth-driver entry for each context. Do not overwrite unrelated plugin settings. The included ARK-plugin-entries.txt is a reference, not a replacement for your whole configuration.

3. Fully shut down and restart the PSP with the controller off. Enable Bluetooth, wait five seconds, disconnect controller USB and press PS alone. Keep the existing successful registration and preferred-target file; no re-pairing is requested.
4. Test VSH controls first. If the connection still fails, send just `ef0:/seplugins/pspgo_hci_transport.log`.

You can remove the old HIDCHECK app and old versioned diagnostic logs; they are not used by this build. Old driver files may remain disabled, but must not be enabled alongside this one.

## Scope

This release simplifies installation and logging. It retains v0.49 Bluetooth/input behavior: the selected saved Wireless Controller uses a real incoming link and scoped local basic-report metadata, with outgoing automatic reconnect deferred. NES30/Free mappings and native DS3 handling are retained. Missing device identity stays unknown. Full 78-byte DS4 report 0x11 and DualSense support are not added.

The reported 802F0140 and missing-plugin problem are not proven resolved on hardware. Correcting the configured path and confirming a new startup banner in the single log is the next check. A file logger cannot report a binary that firmware never loads. This build does not add another checker or loader app.

The first log line identifies version 0.50 and includes the PSP model/firmware. The file appends across VSH/GAME transitions and restarts. Logging uses one bounded file writer with no alternate destinations or boot markers. Native input callbacks retain deferred RAM traces rather than writing report packets to disk.

Own source and tests are provided separately in the source archive. No Sony firmware/disassembly, private logs, controller addresses or pairing keys are included.
