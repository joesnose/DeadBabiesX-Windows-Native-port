# PSP Go Zeemote candidate v0.19 — report-thread and ARK correction

**v0.18 is withdrawn because it can crash after connection. Do not use it.**
This version removes its unsafe installation path and global controller-entry
patches. Physical button operation with v0.19 remains unverified.

## Crash correction

Sony's native report thread has1024bytes of stack. v0.18 called an installer
from its report callback, with known live frames totaling at least1168bytes.
This exceeded the stack even when instruction guards refused all patch writes.
Desktop sanitizer tests did not model that native stack limit.

The report callback now uses protected resident scratch storage and never
performs installation, export lookup, cache flushing, filesystem operations or
logging. Reentrant callbacks skip work instead of blocking or sharing buffers.
Installation runs only from the plugin's8192-byte worker after a valid report,
with bounded attempts. Compiled stack accounting is checked separately from
host tests; see `tools/check_hid_stack.py` and `docs/V018-STACK-AUDIT.md`.

## ARK-compatible read path

ARK already hooks the four controller entry points. This build leaves those
entries and their existing chains untouched. It uses public syscall hooks and
one guarded read-positive callsite in the VSH bridge. That wrapper calls the
unchanged import stub, preserving ARK, then merges returned16-byte samples.

The full VSH function context, export, import target, arguments and bounds must
match before any change. Guard mismatch refuses installation; it does not loosen
checks or overwrite ARK. No controller-entry trampoline is used. The separate
three-argument controller API family is not treated as a two-argument API.

## Use the existing working registration

1. Disable v0.18 or replace `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX.
   Keep only one enabled VSH entry and older conflicting plugins disabled.
2. Fully shut down and restart the PSP. Keep the SteelSeries registration and
   existing Gamepad mode. Do not delete/re-pair it or use the PC helper.
3. Once XMB loads, turn on the Zeemote and Connect using the saved entry. Test
   the D-pad, buttons and left stick. If anything fails, copy
   `ef0:/seplugins/pspgo_hci_transport.log`; its newest banner should say0.19.

The prior hardware log already confirms a real Zeemote connection, identity
1038:1412, a supported81-byte descriptor and valid input reports. This version
changes input interception and stack use, not pairing or radio configuration.
The Information pane may remain blank; actual control operation is the target.

The adapter validates the descriptor and report framing, preserves local PSP
controls, and clears remote input on invalid state or disconnect. Its monitor
preserves held change-only controls only while the authorized native session
remains present; a1500ms deadman applies if monitoring stops. This is not an RF
heartbeat. No DS3 identity, feature commands or connected-state flag is spoofed.

## Verification and limits

Run `sh tests/run.sh` for native sanitizer tests. Tests model native interfaces,
execute emitted MIPS routing/VSH wrapper code, check K1/stack preservation,
reject code-context changes without writes, replay the actual static descriptor
with synthetic reports, and check lifecycle races/input merging. They do not
execute the whole PSP firmware or establish physical button operation.

The PSP build uses pinned image
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `python tools/check_hid_stack.py pspgo_hci_transport.elf` after building.
The check reserves space for native helpers and rejects installer calls from the
report callback; indirect calls still require source review.

Original code MIT; SDK notice and discovery credits included. No Sony firmware,
disassembly, private user logs or controller keys redistributed. Historical
notes describe older builds and are superseded by this README.

Build results: pinned PSP cross-build and all12 ASan/UBSan test programs passed.
The compiled report callback uses56bytes; the checked known report path with
a128-byte native-helper allowance totals384 of1024bytes. No direct installer
call remains in that callback. The same check rejects v0.18. These checks fix
the identified resource defect but do not prove physical stability or controls.
