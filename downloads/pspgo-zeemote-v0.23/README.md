# PSP Go Zeemote v0.23 candidate — HOME menu and automatic return to XMB

The user confirms v0.22 works in PSP games and VSH. Remaining issues were no
remote navigation in the HOME/quit-game menu and a controller power cycle to
reconnect after returning to XMB. v0.23 targets both; these new behaviors remain
unverified on hardware.

## Install

Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX. Keep the existing
registration and Gamepad mode. Keep these two ARK plugin entries enabled:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

They load the same PRX automatically in each context. No manual VSH/GAME mode
switch is required. Merge entries into your existing PLUGINS.TXT; do not replace
other plugin settings. Keep one enabled entry per context and older conflicting
Bluetooth plugins disabled. Full shutdown/restart is needed after replacing
the PRX. Keep a copy of the working v0.22 as rollback.

Leave the controller switched on while launching and quitting PSP games. The
plugin attempts the saved connection two seconds after its worker starts in
either VSH or GAME, unless validated input is already active. A short reconnect
pause may remain during the PSP reboot between contexts. This does not promise
an uninterrupted Bluetooth link or coverage of PS1/POPS/updater/recovery.

## HOME/quit-game menu input

The native sceImpose_Driver imports ordinary two-argument positive peek through
sceCtrl_driver NID2BA616AF. Official reference callsites pass a16-byte sample
and count1. This direct kernel import bypasses the public syscall hooks used
by PSP games.

v0.23 resolves that exact import by library/NID, bounds-checks the stub against
the loaded module, and requires J-to-the-current-Ctrl-peek-export plus a NOP
delay slot. It changes only that J word to the existing positive-peek merge
wrapper. The wrapper calls the original current Ctrl export once, including
ARK's existing entry chain, then merges the validated remote state. No global
controller entry is rewritten and no extended/three-argument API is used.
Installation is worker-only, bounded and refuses unexpected import code.
The existing CTRL diagnostic now includes impose=1 when installed.

## Automatic return to VSH

v0.22 only issued its successful reconnect request in GAME. v0.23 also runs
that same once-per-boot request in VSH, including after quitting a game. It
queries fresh saved records and requires exactly one exact case-insensitive
UTF16 Zeemote: SteelSeries Free registration with valid slot/size/HID flags.
The normal native connection call and mode policy are unchanged. Active input,
missing hooks and missing/ambiguous records cause no connection request.
The new AUTO_RECONNECT log includes the context (100 VSH,200 GAME), match count,
request and original result. No names or addresses are logged.

No re-pairing, PC helper, pairing keys, radio resets, power commands, flash writes
or fake connection state are introduced. The name selects a registration only;
report ownership still requires VID1038/PID1412 and descriptor validation.

## Verification and limits

All13 sanitizer test programs pass. New tests cover the Impose direct import,
menu merge, once-only native call, unchanged Ctrl chain, bad target/delay/bounds
rejection and bounded attempts with absent modules. Existing descriptor,
ownership/lifecycle, GAME/VSH input and reconnect tests remain.
The PSP build and compiled report-thread stack check must also pass; see the
packaged V023 stack evidence. Host tests do not execute the full firmware or
prove physical menu navigation/reconnection. Native connection request success
alone does not establish usable controls.

Use the pinned SDK image
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` and `python tools/check_hid_stack.py pspgo_hci_transport.elf`.
The tiny native report callback still performs no installation, export lookup,
cache flush or file logging. v0.18 remains withdrawn.

If something fails, copy `ef0:/seplugins/pspgo_hci_transport.log` with its newest
route0.23 banner after trying HOME navigation and returning to XMB.
Original source MIT with SDK notice. No Sony firmware, disassembly, private
user logs or controller keys are distributed. Older notes are historical.
