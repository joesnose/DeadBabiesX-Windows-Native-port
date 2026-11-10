# PSP Go Zeemote candidate v0.22 — PSP game input path

v0.20 is user-confirmed working in VSH. The user reports loss of connection
on launching a game. v0.21 addressed two game-mode omissions but the next device log confirms GAME
loaded with HID hooks and no subsequent connection events or reports. v0.22
adds one native saved-target reconnect after those hooks are ready; actual
in-game Bluetooth reconnection and controls remain unverified.

## Install and enable in both contexts

1. Back up the working v0.20 PRX and plugin configuration.
2. Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this build.
3. In ARK's `ef0:/seplugins/PLUGINS.TXT`, ensure these entries are enabled:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

These are entries to merge into your existing file, not a replacement file.
Keep one enabled entry per context and older conflicting Bluetooth plugins
disabled. `ARK-plugin-entries.txt` contains the same two lines. ARK also reads
its installation-folder PLUGINS.TXT; avoid contradictory duplicate entries.
If configuring through ARK's plugin manager, enable this PRX for VSH and GAME.
This candidate targets PSP games and homebrew, not PS1/POPS or the updater.

4. Keep the current Zeemote registration and Gamepad mode. Fully shut down and
restart the PSP. Connect as before in XMB, then launch a PSP game.
5. Wait a few seconds after the game loads, then test D-pad, buttons and left
stick. v0.22 attempts one native reconnect two seconds after its GAME worker
starts. Keep the controller switched on. If it remains disconnected, its log
now records the attempted native connection and result. This build does not claim
seamless link preservation through the PSP's reboot into a game.

If it fails, the existing `ef0:/seplugins/pspgo_hci_transport.log` now records
`route 0.21` and `BOOT context=00000200` when loaded for GAME. A VSH-only log
without a GAME banner means the GAME plugin instance did not start. A GAME
banner without authorized HID reports is a different problem from input
interception. No deletion, re-pairing, PC helper or firmware flash is required.

## v0.22 native reconnect

The v0.21 log proves GAME starts (context=0x200), BSMan and HIDSvc are present,
and HID callbacks install. No GAME SLC event, descriptor, initialization or
report follows. Successful VSH input resumes afterward. This establishes no
post-hook GAME input session; it does not prove an authentication rejection.

Two seconds after its GAME worker starts, v0.22 queries all eight current native
80-byte saved-device records. It requires a valid record size/slot, exact
case-insensitive UTF16 name "Zeemote: SteelSeries Free", supported native HID
flags and exactly one matching registration. It then calls the same guarded
native HID-connect trampoline as the menu, once. Native mode follows the
existing policy (0402 uses mode1, 0202/0602 use mode0). A name selects a saved
registration; report ownership still requires validated 1038:1412 identity and
its HID descriptor. Active validated input, unavailable hooks, no match or
multiple matches cause no connection request. Names/addresses are not logged.

No repeated requests are made after an asynchronous connection result. No
pairing, PIN, link-key, power/reset or flash operation is performed. The
existing log gains GAME_RECONNECT and ordinary HID_CONNECT/SLC diagnostics.

## Existing game input path

v0.20 required sceVshBridge_Driver before installing any input hook, even
though a PSP game uses the public sceCtrl syscalls and need not load that VSH
module. v0.22 explicitly checks the firmware's application type: VSH retains
the confirmed working one-call VSH bridge route; GAME installs only the four
standard positive/negative read/peek syscall routes, without requiring,
reading or modifying VSH code. Current controller exports are called once,
retaining ARK's controller entry hooks. POPS/unknown application types refuse
installation. Model, firmware, export bounds/alias and module-state guards
remain. All installation remains on the owned worker after valid HID input.

The VSH instance is not kept alive across loadexec. ARK loads a fresh kernel
plugin instance for GAME only when its GAME entry is enabled. HID callbacks
are installed for an already present sceHIDSvc or through the module-start
handler when it appears. No radio settings, fake connection state, pairing/key changes or controller
entry detours are added. v0.22 makes one ordinary native saved-target connection
request; it does not change how the native stack authenticates the link. Re-establishing Bluetooth
after loadexec still depends on the native stack accepting the controller.

The v0.18 report-thread overflow correction remains: the callback uses bounded
resident scratch storage and never installs hooks, resolves exports, flushes
caches or writes logs. v0.18 remains withdrawn. v0.20 remains the known working
VSH fallback.

## Verification

All 13 sanitizer test programs pass, including new GAME cases with the VSH
bridge absent and its mock memory deliberately invalid. Tests check zero VSH
writes, four once-only syscall hooks, positive button/axis merge, alias guard
rejection and POPS refusal. New reconnect tests cover fresh aligned queries, exact UTF16/size/slot/HID
validation, duplicate/error refusal, active-input skip, native mode selection
and one original call/result. Existing VSH/ARK guard, descriptor, lifecycle and
stack tests remain. Host tests model firmware interfaces; they do not prove
Bluetooth continuity or physical game controls.

Build with the pinned SDK image documented in the Makefile workflow:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh`, then `python tools/check_hid_stack.py pspgo_hci_transport.elf`
after the PSP cross-build. See packaged build evidence for completed checks.

Original source MIT; SDK notice and credits retained. No Sony firmware,
disassembly, private user logs or controller keys distributed. Older notes
are historical and superseded by this README for v0.22 behavior.
