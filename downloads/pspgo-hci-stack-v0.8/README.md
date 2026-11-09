# PSP Go combined RAM test suite v0.8 — one PRX, one combined log

**Not an HCI mode switch or a working DS4 plugin.** This replaces the separate
same-value RAM experiments with one automatic suite and restart observation.
It tests six bounded combinations, not arbitrary keys/values or every possible
radio command. The Bluetooth host stack remains idle without HCI attachment.

## Test matrix

Both allowlisted mode keys are tested using their existing effective value1 in
transient store0008:

- USB_VM_CONTROL, key03C0
- ONCHIP_HCI_CLIENT, key03CC

For each key, three BCCMD frame forms use the same documented SET operation:

| Form | Native request payload | Declared BCCMD words |
|---|---:|---:|
| Existing padded envelope | 50 bytes | 9 |
| Exact BCCMD packet | 18 bytes | 9 |
| Full declared padded packet | 50 bytes | 25 |

The last two are framing experiments, not proven Sony-supported envelopes.
No other Sony request ID is tried. Every case uses BCCMD varid7003, SETREQ2,
store8, value1. No zero/off value, permanent-store write, reset, power change,
key deletion, address write, link-key operation or guessed command is included.

Each case records the bounded initialized acknowledgment-buffer snapshot,
classifies a matching GET-style reply separately from a valid SETRESP, attempts
independent RAM readback and rechecks both keys in default, PSI and ROM. A
GET-style reply is never accepted as a successful SET acknowledgment.

The supplied v0.7 hardware capture returned GET-style response1 to a submitted
SETREQ2, followed immediately by RAM-read rejection0050. The baseline was
unchanged. This is not evidence of a successful RAM override. The new matrix
examines whether the behavior varies with framing/key; it does not assume that
this Sony management service implements generic BCCMD writes.

## Automatic state and stop rules

Requires the known radio build 6424, transient memory type2 and both keys' exact
baseline (default1, PSI1, ROM0), with RAM-only GET0050 before starting. It must
create/sync/read back `ef0:/seplugins/pspgo_ramsuite_v08.once` before any SET.
Old marker files are kept and do not need editing.

Each matrix case is attempted at most once during the suite. A rejected ACK is
recorded and does not hide readback. A native transport failure, unexpected RAM
observation or changed/unreadable baseline stops further writes. The suite never
resumes/retries a partial or failed run automatically after restart.

A CRC-checked summary is saved as `ef0:/seplugins/pspgo_ramsuite_v08.state`.
Subsequent startups perform observations only and compare any previously seen
RAM values with their current availability. Missing/corrupt summary or marker
cannot trigger another write. Keep these files; deleting them can re-arm the
suite. A crash/power loss after the one-shot marker but before summary prevents
further writes and leaves the log as the available evidence.

If a RAM value was already visible before a later case, its positive readback
cannot prove that later framing independently wrote it. The log labels this
explicitly. The summary records RAM visibility during the suite, not proof that
each variant succeeded. A later disappearance supports retention behavior over
that interval, not HCI activation or universal recovery.

**Bounded means case count, packet sizes and allowed operations.** Native
management calls have no local cancellation deadline. If Sony drops a request
without replying, the suite can stall at that logged operation and may not run
the remaining cases. It does not kill a thread holding native resources or
attempt an unverified reset to force progress. A full restart leaves it in
observation-only mode. No hard runtime deadline or guaranteed recovery claimed.

## Use

1. Replace only `ef0:/seplugins/pspgo_hci_transport.prx` with this version; retain
   its enabled VSH entry and keep older diagnostic probes disabled.
2. Fully power off/on, enable Bluetooth and wait about 60 seconds.
3. Fully power off/on once more, enable Bluetooth and wait about 30 seconds.
4. Send `ef0:/seplugins/pspgo_hci_transport.log` containing both runs.

No controller pairing, PC helper, marker deletion, version swapping or manual
test selection is needed. This is one new suite under its own one-shot marker,
not a re-use/deletion of the earlier per-version markers. Registration can wait
up to 180 seconds for late native initialization.

## Build, validation and licensing

PSP GCC 15.2 build and sanitizer suites cover all six combinations, GET-style and
rejected ACKs, header validation, readback/baseline checks, durable summary and
CRC, simulated restart observations, failure stops and refusal to repeat. Tests
use a mock radio/filesystem; real radio framing support, writes, retention, USB
HCI, pairing and DS4 controls remain unverified.

Build with `make` in the pinned SDK image recorded in the historical docs.
Host tests: `sh tests/run.sh`. `src/preflight_v07.c` is retained only for earlier
regression tests and is not linked into the PRX. `src/preflight.c` runs the suite.

Original adapters MIT. Vendored BTstack retains its personal/noncommercial
license in `vendor/btstack/LICENSE`, notices and pinned commit. SDK notice
included. No Sony firmware/decompiled code, private logs or actual controller
keys are redistributed. Runtime bond files are sensitive; do not upload them.

Protocol reference: https://github.com/bluez/bluez/blob/5.50/tools/csr.c . Native
Sony envelope differences remain experimental. No GPL source copied.
