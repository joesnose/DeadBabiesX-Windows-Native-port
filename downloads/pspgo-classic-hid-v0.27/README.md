# PSP Go Classic HID v0.27 — clean v0.23-based GameStick test

This restarts from the hardware-confirmed v0.23 Zeemote build. The next target
is the original white GameStick controller, or another Classic HID gamepad.
Keep the NES30 Pro off and out of this round. No GameStick registration or
input success is claimed for this candidate.

## Install and first test

1. Keep the working v0.23 PRX as rollback. After a full shutdown/restart with
   controllers off, replace `ef0:/seplugins/pspgo_hci_transport.prx` with v0.27.
2. Keep the same VSH/GAME entries enabled, then fully restart again:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

3. Put the GameStick controller in its normal Classic pairing mode and use
   PSP Bluetooth registration. Existing Zeemote registration can stay.
4. If registration succeeds, select Connect manually in XMB and test buttons,
   D-pad and left stick. Keep the first test in VSH; do not use the NES30 Pro.
5. Send `ef0:/seplugins/pspgo_hci_transport.log` even if registration fails.
   Newest banner should say `Classic HID clean-baseline route 0.27` and the
   startup log should include `PAIR_TRACE passive numeric event observer`.

Until this controller's shutdown behavior is known, fully shut down/restart
with controllers off before USB file transfer. Do not reproduce the known
NES30 USB freeze just to retrieve a log. A v0.23 success with the Zeemote did
not establish that v0.23 can safely shut down the NES30 connection; the user
later confirmed that controller freezes USB there too.

## Exactly what this build changes

The VSH, GAME and HOME/quit-menu input interception, pairing choice, reconnect
policy and native lifecycle paths come from v0.23. There are no v0.26 setup
wait hooks, no native timeout changes, no v0.24 target-preference file writes,
and no broadened automatic reconnection. Existing `pspgo_classic_target.dat`
from experiments is ignored. Automatic reconnect remains Zeemote-only. Generic
controller auto-reconnect across game launch will be added only after a manual
VSH connection/input session is established.

Generic descriptor input from v0.24 is retained: native type3 gamepad nodes
with a supported Generic Desktop Gamepad/Joystick descriptor can be decoded
without the Free VID/PID allowlist. Existing native resources, native DS3
identity/type4 and code/ownership guards are retained. Free1038:1412 keeps its
established mapping; other devices use default generic mapping:

| HID button | PSP |
| --- | --- |
| 1,2,3,4 | Cross,Circle,Square,Triangle |
| 5,6 | L,R |
| 7,8 | Select,Start |
| 11 | HOME |

X/Y maps to PSP left analog; a hat or standard D-pad usages90..93 maps to
PSP directions. Buttons plus XY axes, a hat, or all four D-pad usages are
supported. Bounds remain512-byte descriptor, one complete numbered/unnumbered
report up to64bytes and bounded absolute variable fields. Layouts outside
those bounds are refused. Keyboard/consumer remotes, BLE, vendor protocols and
split gamepad report IDs are outside this test. Physical button numbering may
need a device-specific profile later.

## Pairing choice and passive diagnostics

Unknown controllers retain Sony's stock pairing selection; this build does not
force every gamepad through the Zeemote's0000 PIN path. The exact Zeemote name
fallback from v0.23 remains. There is no replacement authentication/SSP stack,
so GameStick registration may still fail. That is a pairing result, not a
failed input-decoder test.

A guarded observer forwards the native management receive call's six EABI
arguments and original result unchanged. It copies only bounded numeric event
metadata into a32-entry RAM queue. The plugin worker logs PAIR_EVENT id/length,
slot for known slot-bearing events, and word1 only for262/264. For269 PIN-request
events, no PIN/text bytes are read or logged. No packets are rewritten, native
waits shortened or new radio commands introduced. Event labels/word meanings
are not guessed as an authentication error. Discovery/name/address/key/button
payloads are not logged. Static native type3 gamepad descriptors may be logged
for parser diagnostics; those are metadata, not input reports.

## Verification

All15 ASan/UBSan programs passed. Tests cover native receive forwarding/results,
metadata bounds/no packet mutation, nine call-context guards, existing Zeemote
registration, generic decoder/ownership, lifecycle and VSH/GAME/HOME routes.
The pinned PSP build and compiled stack checks are included in V027 evidence.
These software checks do not prove physical GameStick pairing, input or USB
shutdown. v0.23 remains the Zeemote working fallback.

Build image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` and `python tools/check_hid_stack.py pspgo_hci_transport.elf`.
Original source MIT with SDK notice. No Sony firmware/disassembly, private
user logs, addresses or keys distributed. Historical notes are superseded by
this README for v0.27 behavior.
