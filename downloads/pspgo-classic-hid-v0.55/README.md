# PSP Go Classic HID v0.55 — DS4 PS button and automatic PIN entry

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep your existing ARK entries and pairing, then fully shut down and restart. No EBOOT or Game app. Same single log: `ef0:/seplugins/pspgo_hci_transport.log`.

## DS4 PS button

In a PSP game, release PS after connecting, then press it to open the PSP HOME/PS menu. Press again to close it. Existing controller menu navigation remains available.

The report already decoded PS to the HOME bit, but ordinary game button injection did not dispatch the native system-menu action. A dedicated GAME worker now calls guarded native Impose exports to read menu status and request open/close. Native Impose still decides whether the menu is permitted. No Impose code is rewritten.

There is one action per press, not per report. An initial PS hold used for connection is ignored until released. Held repeats, stale taps and disconnected queued presses are rejected. Normal game buttons remain responsive. The raw DS4 HOME bit is consumed while this GAME dispatcher is enabled, avoiding duplicate handling. Other controllers retain their existing behavior. VSH keeps its previous input path.

## DS4 registration without typing a code

For a future new registration of the exact DS4 `Wireless Controller` discovery entry with its expected gamepad class, the PIN-entry screen is skipped and the native object's existing `0000` default is submitted automatically. This retains the registration/authentication method that already worked; it does not switch to a different Bluetooth authentication protocol or report a false DS3 identity.

A DS4-only private UI method table suppresses the entry screen. The native constructor, PIN registration function and default PIN data remain in use. Other devices, including Free/Zeemote, retain their entry flow. Extra guards check the default initialization and UI selector; a mismatch keeps the previous manual-entry path.

Keep your current successful pairing. You do not need to delete it to use this build. Automatic entry only changes a future registration flow.

## Working baseline

The user confirmed v0.54 works well. Its reconnect setup sequence, latency optimization, single log, stick thresholds/scaling and button mappings are retained. The HOME and automatic-PIN changes in this release have not yet been tested on the PSP. Full DS4 report11/DualSense support and the separate automatic802F0140 issue are not newly addressed.

If needed, restore v0.54 by replacing the PRX and restarting. Send the same log for an issue: new markers include `DS4 HOME ready`, `DS4 HOME native toggle dispatched` and `DS4 PAIRING automatic_default_pin=1`.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep one enabled driver per context; preserve unrelated plugin settings.

## Validation

Forced PSP SDK build, 30 ASan/UBSan programs and thirteen compiled checks passed. Tests cover report-to-HOME routing, tap/hold/release, connect-hold suppression, stale/disconnect cancellation, timer wrap, native API guards and all65536 menu-status values. The executable MIPS constructor-wrapper test verifies DS4-only automatic entry, intact record84/default PIN32, preserved ABI/native constructor return, null allocation and non-DS4/Free isolation. All13 new UI guards are mutated individually to verify refusal.

Native menu calls run on a4096-byte owned worker (168-byte compiled frame), not on the1024-byte report thread. The report-update function remains24 bytes and the report callback56 bytes. Own source/tests are separate; no Sony firmware/disassembly, private raw logs, addresses or pairing keys are included.
