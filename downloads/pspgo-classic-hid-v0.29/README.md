# PSP Go Classic HID v0.29 — clean v0.23-based GameStick test

This restarts from the hardware-confirmed v0.23 Zeemote build. The next target
is the original white GameStick controller, or another Classic HID gamepad.
Keep the NES30 Pro off and out of this round. The user confirmed no-code GameStick registration and connection on v0.27.
Physical button response with this correction remains unverified.

## v0.29 GameStick gamepad-only lifecycle test

The user confirms v0.28 controls work in VSH and in a game when the controller
is switched on after loading. Keeping it on during game entry or exit hangs
the transition; Zeemote enters/exits correctly. This isolates a live GameStick
connection/lifecycle problem, not a missing gamepad decoder.

This candidate suppresses the unused virtual keyboard and mouse interfaces
only for identity0f0d:1011 with the exact232-byte descriptor supplied by this
GameStick. It keeps its validated report3 gamepad. Native keyboard/mouse input
registrations and their input-device teardown are therefore not created for
this matching controller. Those virtual keyboard/mouse functions will not be
available through this plugin; all gamepad controls retain their existing path.
This is a targeted lifecycle isolation test, not a proven cause or hardware fix.

The complete three-node ownership plan is reserved atomically before any
auxiliary initializer is bypassed. Nodes with existing native resources are
not taken over. Nonmatching devices, descriptor variants, insufficient
capacity or pre-existing resources use original native handlers. Sony still
owns descriptor parsing, linked nodes and pool release. The suppressed node
callbacks ignore auxiliary reports without reading their payload and skip
only cleanup for allocations that were never made. Gamepad report ID3 and
existing lifetime checks remain. Termination can occur in any order, and
ownership is independent of descriptor-cache lifetime.

The INIT log should show gamepad_only=1 and aux_mask=3 when both unused
interfaces are suppressed. Report routing should still show route_after=3.
No pairing, PIN, native wait, radio power/reset or forced thread termination
is changed. The Zeemote and generic devices not matching this exact descriptor
retain their native keyboard/mouse behavior.

Install with a full shutdown/restart so the new hooks apply before connecting.
Keep the successful GameStick registration; no re-pairing. Re-enable the GAME
entry if disabled, connect in VSH and test entering and exiting a PSP game
with the controller left on. Keep the NES30 off. If it still hangs, use the
known workaround (controller off during transition, on after loading) to
recover and retrieve the log. Until USB behavior is established, use full
shutdown/restart with controllers off before USB transfer.

All15 sanitizer programs pass, including complete composite ownership,
reservation failure, pre-existing resource/identity/descriptor fallback,
reordered termination/cache eviction and18 additional no-write code guards.
Pinned PSP build and report/observer/auxiliary compiled checks passed.
Aux report wrapper24bytes, outer wrappers0, native passthrough restores the
wrapper frame before tail-calling original handlers. No extra wrapper frame
remains beneath native keyboard/mouse report handling. These are software
checks; physical transition success remains unverified for v0.29.

## Retained composite gamepad report routing

The v0.27 log now establishes successful no-code registration,0402 connection,
identity0f0d:1011 and a supported232-byte HID descriptor. It contains keyboard
report1, mouse report2 and gamepad report3. Our decoder accepts the gamepad's
9-byte payload, but reports=0 shows that no callback reached it.

Sony's native generic gamepad collection handler skips its contents without
assigning node[1] (Report ID). Its composite dispatcher compares the incoming
Report ID with each node[1] before invoking that node's handler. The keyboard
and mouse parsers assign their IDs; the generic gamepad node can remain0.

v0.29 assigns the descriptor-validated report ID only to the adapter-owned,
numbered generic type3 node after the existing ownership/resource checks.
No dispatcher or unrelated keyboard/mouse node is modified. The native
composite dispatcher can then select node3 and pass its ID-stripped payload;
the existing callback validates/restores that ID for decoding. The confirmed
Free1038:1412 route remains unchanged. The INIT log now records route_before
and route_after, so this hypothesis has a direct runtime check.

The actual static descriptor is included as non-secret protocol metadata in
a regression fixture. A synthetic report3 routes past keyboard1/mouse2 into
the owned gamepad and decodes Cross; term/lifetime cleanup remains tested.
This is not a hardware button success claim. The report-ID fix itself adds no pairing or native wait behavior; the
v0.29 auxiliary-interface lifecycle change is described above.

## Install and first test

1. Keep the working v0.23 PRX as rollback. After a full shutdown/restart with
   controllers off, replace `ef0:/seplugins/pspgo_hci_transport.prx` with v0.29.
2. Keep the same VSH/GAME entries enabled, then fully restart again:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

3. Keep the successful GameStick registration and select Connect manually
   after restarting. Do not delete/re-pair it for this input-only change.
   Existing Zeemote registration can stay.
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
The pinned PSP build and compiled stack checks are included in V029 evidence.
The prior log proves GameStick pairing/connection on v0.27, but software checks
do not prove v0.29 physical buttons or USB shutdown. v0.23 remains the Zeemote working fallback.

Build image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` and `python tools/check_hid_stack.py pspgo_hci_transport.elf`.
Original source MIT with SDK notice. No Sony firmware/disassembly, private
user logs, addresses or keys distributed. Historical notes are superseded by
this README for v0.29 behavior.
