# PSP Go HCI stack v0.10 — protocol and discovery corrections

This development build corrects demonstrated software errors. It does not yet
expose the PSP Go radio as standard HCI or provide demonstrated DS4 controls.
Installing it is not needed to verify the offline results described here.

## Proven from saved evidence

CSR SETREQ type 2 normally receives response type 1. The older parser required
an incorrect type 3. Offline replay of the supplied v0.9 log finds four matching
status-zero response headers and printed key/store/value fields. Full copied
response extent is unknown; independent RAM reads failed. Same-value writes do
not establish effective changes. The old zero-ACK summary is superseded.

Run `python tools/replay_suite.py /path/to/pspgo_hci_transport.log` locally to
repeat that audit. User logs are not included. See `docs/PROTOCOL-AUDIT.md` for
source references and limits.

## Build behavior

The production build makes no native radio preflight calls. RAM-write suites,
mode switches, power/reset commands and marker changes are disabled. Historical
suite source remains solely for regression tests and is not linked into the PRX.
The host still waits for actual standard USB HCI attachment; Sony's existing CDC
interface is not treated as HCI. A corrected parser is not a mode-switch recipe.

The discovery correction verifies candidate identity through SDP rather than
requiring extended-inquiry identity from standard inquiry. Controller names
alone do not establish identity. Pairing and usable controls remain untested on
real hardware. No legacy PIN is guessed.

## Installation and verification

If using this development build, replace the existing PRX and retain its enabled
VSH entry and existing marker/state files. Fully restart the PSP to replace the
resident driver. No special pairing or repeated RAM test is requested. The log
remains `ef0:/seplugins/pspgo_hci_transport.log`.

Build: `make` with the pinned PSP SDK image described in the included notices.
Tests: `sh tests/run.sh` with a native C compiler and ASan/UBSan. Host tests and
mock transport results establish software behavior only.

Original adapters MIT. BTstack retains its personal/noncommercial upstream
license and notices. SDK notice included. No Sony firmware, decompiler output,
private logs or controller keys are distributed. Historical documents describe
older experiments and are superseded by this README and the protocol audit.
