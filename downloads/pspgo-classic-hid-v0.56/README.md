# PSP Go Classic HID v0.56 — basic DualSense and PIN-screen bypass test

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep your ARK entries and fully shut down/restart. One PRX, no EBOOT or Game app. Same single log: `ef0:/seplugins/pspgo_hci_transport.log`.

## Test

Keep your existing DS4 pairing; confirm its controls and reconnect still behave as before. The PS-button HOME-menu handler from v0.55 is retained.

To try a standard DualSense, hold Create + PS until it flashes for pairing, then select it in the PSP Bluetooth registration menu. This build recognizes the exact `DualSense Wireless Controller` and `Wireless Controller` discovery names with the expected gamepad class. Test VSH controls first, then a game and reconnect. Report any remaining passkey screen or error using the same log.

DualSense support and the revised PIN bypass are hardware-unverified. Do not delete a working pairing just to test the PIN UI change.

## DualSense scope

Supports the standard Sony VID/PID054c:0ce6. Its initial Bluetooth report01 is10 bytes with the same basic sticks, D-pad, buttons and PS layout as the DS4. The existing decoder, report projection, selected-target metadata, reconnect setup and HOME-menu path are reused. A completed known non-supported identity is still rejected; missing identity remains unknown and requires the existing fresh preferred-registration/genuine-link/context checks.

This does not add full78-byte report31 handling, adaptive-trigger effects, advanced haptics, audio, motion or DualSense Edge support. The driver does not request full-report mode. Pairing/authentication and whether the PSP radio accepts a DualSense remain hardware questions, not established by report-format tests.

## Revised PIN-entry bypass

v0.55 changed the selected device's private method table. The user still saw a passkey screen and could continue without typing. This build additionally intercepts the guarded virtual selector at the actual PIN-entry decision. It returns the automatic/default path only for a fully validated DS4/DualSense discovery record whose object's registration method is the native PIN function. Other objects invoke their original selector once, preserving its result.

The native constructor, default0000 data and PIN registration/authentication are retained. This is UI automation, not authentication bypass or a different Bluetooth pairing method. Standard firmware errors and any separate radio-side authentication/confirmation remain possible. No synthetic global key press or unrelated-dialog dismissal is added.

A guarded user-memory stub retains the original caller's registers/return behavior. Registration templates now write directly to the output plan, removing nested large stack copies. The code does not alter working analogue thresholds, scaling or native asynchronous queue ownership.

`SONY PAIRING automatic_default_pin=1 entry_screen_hook=1` identifies installation of the new UI path. Some older diagnostic names still say DS4 because the basic-report implementation is shared with DualSense.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep one enabled driver per context and preserve unrelated plugins. Restore v0.54/v0.55 PRX and restart if this candidate regresses a working controller.

## Validation

Forced PSP SDK build,31 ASan/UBSan programs and fourteen compiled checks passed. Tests include the documented DualSense minimal Bluetooth layout, actual decoder/init/report callbacks for054c:0ce6, identity/name/class selection, genuine-link/descriptor/disconnect behavior and preserved DS3/Free paths.

Executable MIPS tests exercise the PIN-screen selector with an original native method table as well as the constructor wrapper, verify native fallback exactly once for Free/non-Sony/non-PIN objects, reject malformed/near-match records, and preserve record84/default PIN32, native constructor return and saved registers. PIN guards, HOME behavior and existing reconnect tests remain passing.

Registration-planning frames were reduced to fit the observed4096-byte module-loader worker. Static accounting checks the known owned path plus a native allowance; it is not a full native stack proof. The native HOME worker and input report paths remain bounded by their existing compiled checks. Hardware pairing, UI flow and controls still need testing.

Own source/tests are separate. No Sony firmware/disassembly, private raw logs, addresses or pairing keys are published.

Report-layout reference: https://github.com/nondebug/dualsense/blob/main/README.md
