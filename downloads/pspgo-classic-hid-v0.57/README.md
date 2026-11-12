# PSP Go Classic HID v0.57 — DualSense startup handover and PIN guard fix

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep existing ARK entries, controller registrations and target file, then fully shut down/restart. One PRX, no EBOOT or Game app. Same single log: `ef0:/seplugins/pspgo_hci_transport.log`.

Your DualSense is already registered. After restarting, enable Bluetooth, wait five seconds, then press PS alone with controller USB disconnected. Do not hold Create or register it again for this input test. Test VSH controls first, then a game and reconnect. Send the same log if controls remain absent.

## Findings from the supplied v0.56 files

The preferred-target file is valid and selects slot5. The log shows slot5 registration and a successful native connection, followed by PnP/HID metadata timeouts and no reader initialization. It also explicitly reports `automatic_default_pin=0 entry_screen_hook=0`.

Two driver defects were identified:

- A newly selected Sony controller was saved after connection, and target saving cleared the genuine-link authorization bits. During a switch from slot4 DS4 to slot5 DualSense, the local metadata workaround was not armed before native startup.
- One auto-PIN guard expected a no-op where the firmware uses `move v0,zero` in its return delay slot. This incorrectly disabled the PIN-screen bypass. The expected word is corrected, and all13 PIN guards independently match the examined firmware.

## Changes

Fresh Sony profile metadata now prepares the preferred target in RAM after successful registration and before an explicit native HID connection. Native callbacks still perform no profile queries or file work. Selection alone never creates a successful link: the existing real SLC event, cached identity policy and native-context checks remain required.

Only native SLC events now change the scope's link bits. Saving/loading target preferences no longer clears a genuine link. Disk persistence stays on the background worker; explicit target selection invalidates stale in-flight target work, and the existing24-byte target format is unchanged. Other controller preferences retain their existing flow.

The corrected PIN guard permits the v0.56 DS4/DualSense-specific screen selector to install. The native0000 default and native registration/authentication remain in use. No global key injection or blanket dialog dismissal is added. The PIN UI correction remains hardware-unverified; keeping an existing pairing is sufficient to test input.

## Retained behavior and limits

Retains the user-confirmed v0.54 connection setup, v0.55 PS/HOME handler and v0.56 standard DualSense054c:0ce6 basic report01 support. Stick thresholds/scaling are unchanged. Full78-byte report31, DualSense Edge, motion, advanced haptics, adaptive triggers and audio are not added. These fixes address host startup state; working DualSense input on the PSP is not yet confirmed.

Useful markers: `SONY TARGET prepared ... before_native_connect=1`, real `SLC_EVENT ... state=0`, local metadata records, `DS4_READ_START` and report counters. Some names still say DS4 because the basic input path is shared. `SONY PAIRING automatic_default_pin=1 entry_screen_hook=1` confirms the corrected PIN path is enabled.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep one enabled driver per context and preserve unrelated plugins. A previous working PRX can be restored by replacing this file and restarting.

## Validation

Forced PSP SDK build,32 ASan/UBSan programs and fourteen compiled checks passed. The new regression models slot4-to-slot5 handover before metadata startup, unchanged preference persistence, incoming-event ordering, real-link requirements, disconnect/failure and malformed/non-Sony isolation. Existing executable MIPS PIN selector, DS4/DualSense report/callback and HOME tests remain passing.

All13 PIN guard words were compared independently against the examined firmware, in addition to mutation tests. Registration planning retains the bounded loader-stack checks; full firmware-stack and hardware behavior remain unproven. Own source/tests separate; no Sony firmware/disassembly, private logs, addresses or pairing keys are published.
