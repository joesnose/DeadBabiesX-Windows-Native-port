# PSP Go Classic HID v0.25 — experimental gamepad/joystick test

This is a separate experimental successor to the confirmed working Zeemote
v0.23. It removes the single-device input allowlist and expands native
registration routing to Bluetooth Classic joystick/gamepad classes. It does
not implement a replacement Bluetooth stack or guarantee every HID device.

## v0.25 mixed-profile route correction

The new hardware log records slot2 saved flags0422, a mode0 request (0202),
native connection acceptance, but no HID descriptor/init/report for that
controller. The PSP Information screen reports SPP. v0.24's exact0402 test
missed this combined value. v0.25 accepts the additional0020 profile bit when
selecting generic HID mode1, so0402 and0422 both request0402. The established
0202/0602 Zeemote behavior is retained. Unknown other bits preserve the old
route. Reconnect eligibility and selection use the same shared policy, so the
manual and automatic paths cannot diverge on0422.

The user also reports a freeze when USB mode disables Bluetooth after this
connection. Its cause is not proven: the log has no native shutdown trace and
this controller never reached an adapter-owned input node. Correcting the
wrong service path may help, but USB shutdown is not claimed fixed. Until
verified, avoid USB mode with this test controller connected. Fully restart
with controllers off before copying the replacement PRX, then restart again.
Select/Connect the existing saved controller once to remember the right target.
No re-registration is required for this routing change. Test inputs first;
disconnect Bluetooth before entering USB mode.

## Install and test

1. Keep a backup of your working v0.23 PRX and plugin settings.
2. Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX. Keep the same
   enabled ARK entries:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

3. Fully shut down/restart. Put the test controller into Bluetooth Classic
   HID/Gamepad pairing mode, then register it through the PSP Bluetooth menu.
   Existing Zeemote registration can stay. BLE-only modes are not supported.
4. Select the new controller and Connect once in XMB. An accepted native
   connection request saves its slot/address as the preferred reconnect target.
5. Test D-pad, left stick, buttons, a PSP game, HOME/quit menu and return to XMB.
   Leave the controller switched on during transitions. A few seconds of
   reconnect delay can remain. Test one active controller at a time.

The same log is `ef0:/seplugins/pspgo_hci_transport.log`, with newest banner
`PSPGo Classic HID experimental route 0.24`. If registration, connection,
controls or mapping fail, retain that log and note the controller model/mode.
An accepted connection request is not proof that a link or usable input exists.
No deletion of the working Zeemote registration or PC helper is required.

## Implemented scope

Discovery registration admits standard Peripheral-major joystick/gamepad
minor classes, using Sony's native PIN registration object with the HID
category. The known Zeemote name fallback remains. Real84-byte discovery
records, nonzero identifiers, exact UI/constructor code guards and native
allocation/PIN bounds are retained. This expands device selection; it does not
add a new authentication method or emulate a DS3.

Only a validated Generic Desktop Gamepad/Joystick application descriptor can
own a native generic type3 report node. The fixed1038:1412 allowlist is removed;
known native DS3 identity/type4 and non-owned native resources remain excluded.
The bounded parser supports a complete report with buttons and either XY axes,
a hat, or all four standard D-pad usages90..93. Pads without analog axes use
neutral PSP analog. The original Zeemote mapping remains for1038:1412.

Generic default button usages:

| HID button | PSP button |
| --- | --- |
| 1,2,3,4 | Cross,Circle,Square,Triangle |
| 5,6 | L,R |
| 7,8 | Select,Start |
| 11 | HOME |

Hat/D-pad usages map to PSP directions; X/Y map to the left analog stick.
Unmapped buttons do nothing. HID button numbers do not standardize physical
face positions, so some pads will require a later mapping profile. Right-stick
axes may decode but have no native PSP analog channel.

Supported limits: descriptor up to512bytes, one complete numbered or unnumbered
gamepad report up to64bytes, bounded field/collection/global stacks, absolute
variable fields, axes up to16bits and buttons1..16. Split gamepad state across
multiple report IDs, vendor protocols, keyboard/consumer-control remotes,
BLE and new SSP pairing support are not implemented. Unsupported/malformed
layouts are refused; native callbacks remain for devices the adapter does not
own. Sony's existing Classic pairing/service support remains a limitation.

## Reconnection and multiple registrations

The most recent accepted menu connection stores16bytes in
`ef0:/seplugins/pspgo_classic_target.dat`: format tag, slot and device address,
not keys or a PIN. Reconnect re-queries current records and requires matching
slot/address and valid saved HID metadata. A reused/stale slot does not silently
select another device. Names/addresses are not written to the diagnostic log.
Without a saved preference, automatic reconnect requires one eligible saved
HID record. With several registrations, Connect the desired controller once
through XMB to select it. The same one-shot reconnect runs in VSH and GAME.
If registration slots change, select/Connect again in XMB.

VSH, game syscall and guarded Impose/HOME routes from v0.23 are retained. No
manual context switch, controller entry overwrites, radio resets, pairing-key
writes or firmware flash. The report-thread stack correction remains.

## Verification and limits

All14 ASan/UBSan programs pass. New cases cover generic hat-only pads,
joysticks without hats, D-pad usages, button mappings, keyboard/consumer
rejection, generic ownership, last-selected slot/address persistence and
131072 emitted-MIPS registration policy cases. Profile selection is also
checked across all65536 flag values, including0422/0622, missing-HID and
unknown-bit refusal; reconnect regressions cover the observed0422 case. Existing VSH/GAME/Impose,
connection/lifecycle, framing and malformed descriptor tests remain.
The pinned PSP cross-build and compiled stack check are recorded in the
packaged V025 evidence. No generic test controller has been verified on a
physical PSP with this candidate. v0.23 remains the hardware-confirmed fallback.

Build image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` and `python tools/check_hid_stack.py pspgo_hci_transport.elf`.
Original source MIT, SDK notice retained. No Sony firmware, disassembly,
private logs or controller keys distributed. Older Zeemote-specific notes are
historical; this README defines v0.25 behavior.
