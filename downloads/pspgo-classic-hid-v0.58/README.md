# PSP Go Classic HID v0.58 — editable controls and Sony transition reconnect test

## Install

Copy both files from the ZIP's `seplugins` folder onto internal storage:

- `ef0:/seplugins/pspgo_hci_transport.prx`
- `ef0:/seplugins/pspgo_hid_controls.txt`

Keep the existing ARK entries and controller registrations. Fully shut down/restart after installation or a controls-file edit. Keep your edited controls file when updating the plugin in future.

One PRX, one existing log (`ef0:/seplugins/pspgo_hci_transport.log`), no EBOOT or Game app.

## Default controls

These defaults apply to DS4 and standard DualSense in games, as requested:

| Controller input | PSP action |
|---|---|
| Right stick up | Triangle |
| Right stick left | Square |
| Right stick down | Cross |
| Right stick right | Circle |
| R2 | Cross |
| L2 | Square |

Physical face buttons, D-pad, L1/R1, left stick, Create/Share, Options and PS retain their normal actions. VSH keeps the standard layout, with L2/R2 acting as L/R and no right-stick face-button mapping.

## Edit the text file

The `[vsh]` and `[game]` sections are independent. The shipped file activates `preset = standard` for VSH and `preset = fps` for games. Commented lines list every available override and each preset's defaults. Remove `#` from a binding you want to change. A later override takes precedence over the preset; changing the preset resets bindings at that point in the section.

Example overrides:

```
[game]
preset = fps
r2 = cross
l2 = square
right_up = triangle
right_left = square
right_down = cross
right_right = circle
left_y = -analog_y
r3 = home
```

To use ordinary game controls, set `[game]` to `preset = standard` and leave its override lines commented. This is one shared game profile; it does not automatically detect a particular game's control scheme.

Available physical button sources are `cross`, `circle`, `square`, `triangle`, `l1`, `r1`, `l2`, `r2`, `l3`, `r3`, `create` (or `share`), `options`, `ps`, `touchpad` (click), and `dpad_up/down/left/right`.

Button/direction targets are `none`, `cross` (or `x`), `circle`, `square`, `triangle`, `l`, `r`, `select`, `start`, `home`, `up`, `down`, `left`, `right`, and `analog_up/down/left/right`. Combine targets with `+`, e.g. `r2 = cross+l`. Opposed digital analogue directions cancel to centre.

Stick sources are `left_x`, `left_y`, `right_x`, `right_y`. Map each to `analog_x`, `analog_y` or `none`; prefix an analogue target with `-` to invert it. The eight `left_up/down/left/right` and `right_up/down/left/right` keys additionally map stick directions to button targets. They can coexist with continuous axis routing.

`l2_axis` and `r2_axis` can map continuous trigger travel to `analog_left/right/up/down`, or `none`. These are independent of the digital `l2`/`r2` button bindings. `left_deadzone` and `right_deadzone` accept0..127 (default16/32). The configured source dead zone is applied before routing; no additional Sony-specific merge threshold is added.

PSP has one analogue stick. When multiple continuous sources target an axis, the source furthest from centre wins; ties retain the earlier source. Full digital analogue-direction bindings override continuous input. This cannot create a second native analogue axis in a PSP game; the FPS layout emulates its face buttons with the right stick.

The file is read only at startup, never during input reports. Missing/invalid files use the stated defaults; the single log reports a parse error line. The parser accepts CRLF, UTF8 BOM and `#`/`;` comments, with an8192-byte file limit and159-byte line limit. An invalid file is rejected as a whole. Physical PSP controls and other Bluetooth controller families are unchanged.

Only inputs available in the existing10-byte basic Bluetooth report are exposed. DualSense mute, touchpad coordinates and motion require additional report support and are not included.

## VSH/GAME transition change

The earlier general-controller fix reconnects a remembered controller after PSP services reload. The Sony-specific branch overrode its fresh profile selection with legacy mode0 and deferred all VSH reconnects. This candidate removes that legacy override and makes one automatic saved-profile attempt in both VSH and GAME, using the same route selected for manual connection (mode1 for the demonstrated Sony0402/0602 records).

Fresh preferred-target matching, genuine-link authorization, native errors and existing link setup are retained. An observed connected link or active controls suppresses the extra request. There is no repeated retry loop. The PSP may still drop the RF link while services reload; this change attempts to restore controls automatically and is not hardware-proven to eliminate transition disconnects or802F0140. A powered-off controller still needs PS to wake it.

Test VSH to game and HOME/Exit Game back to VSH with the controller on. Check whether controls return without cycling power. Send the same log if reconnect fails. Keep a working v0.57 PRX available for comparison/restoration.

## Retained baseline and validation

Retains v0.57's DS4/DualSense startup handover, corrected PIN-screen guard, HOME-menu handler and native setup sequence. Pairing need not be repeated. No full report31, adaptive effects/audio or DualSense Edge support is added.

The forced PSP SDK build,33 ASan/UBSan programs and fifteen compiled checks pass. Tests cover the approved FPS mappings, all basic buttons, all four stick-axis routes/inversion, both trigger axes, combinations, direction cancellation, strongest-axis selection, malformed reports, transactional config errors and the shipped file. Report-hook integration uses the real remapper. Transition tests verify VSH/GAME use the saved profile and preserve native errors and real-link gating.

The remapper has a64-byte frame and calls only the basic validator. Config parsing/storage run before report callbacks are installed. Static report-path accounting remains400 bytes including its native allowance on the1024-byte thread; full firmware stacks and actual hardware behavior remain unproven. Own source/tests separate; no Sony firmware, private logs, addresses or pairing keys included.

ARK entries remain:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```
