# PSP Go acknowledgment/readback fix v0.5

Fixes the v0.4 control-flow bug that skipped RAM readback when the SET reply was
not recognized. **This is not an HCI mode switch or working DS4 connection fix.**
The radio's unexpected acknowledgment format remains unidentified.

## Changes

- Attempt the independent RAM GET after a returned SET call even if its reply is
  rejected or malformed. Attempt all four post-checks independently as well.
- Record acknowledgment validation, readback and baseline results separately.
  A good readback does not make an unrecognized acknowledgment valid.
- Log only the first nine words of the initialized SET response buffer, along
  with native/outer status and the response-length field. The log explicitly
  distinguishes the buffer capacity from the unknown copied extent; A5A5 is the
  initialization sentinel. Only this fixed nonsecret mode-key operation is dumped.
- Retain exactly the v0.4 attempt/verification marker paths and tokens. Upgrading
  does not re-arm a write. An existing attempt without verified success is not
  silently promoted, even if a later read returns1.
- Add regression tests for malformed ACK and rejected SET: readback and baseline
  checks must still run; a verification marker must not be created; subsequent
  runs must never send another SET.

The same narrow transient-store/build/baseline/durable-marker prerequisites
remain. The sole representable SET is USB_VM_CONTROL=1/store8, matching the
current effective value. No off value, persistent SET, reset or power command.

## Existing user installation

Replace `ef0:/seplugins/pspgo_hci_transport.prx` and retain its enabled VSH entry.
Keep `pspgo_ramtest_v04.once` and `pspgo_ramtest_v04.verified` as they are; do not
delete or rename them. Older pairing/radio probes should remain disabled.
A full restart is needed to replace resident code.

For the supplied v0.4 history, the attempt marker already exists. This build
will therefore make observations only, not capture a new SET acknowledgment or
repeat the write. The old acknowledgment bytes were not logged and cannot be
reconstructed after restart. No additional repeated test or log is requested
just to validate this software fix. A separately designed experiment would be
needed to obtain a new acknowledgment; this update does not initiate one.

The log remains `ef0:/seplugins/pspgo_hci_transport.log`. The compiled host stack
still waits for actual HCI attachment. HCI exposure/recovery and real DS4 pairing
remain unfinished. Historical setup/experiment details: `docs/RAM-TEST-v0.4.md`;
this document supersedes its version and test instructions for this update.

## Verification and licensing

PSP build and ASan/UBSan suites include framing/lifecycle, DS4 decoding, mocked
host initialization/bond storage/input merging, and the new acknowledgment
regressions. These do not replace hardware validation or identify Sony's ACK.

Original adapters MIT. Vendored BTstack retains its personal/noncommercial
upstream license in `vendor/btstack/LICENSE`, copyright notices and pinned commit.
SDK notice included. Build with `make` in the same pinned SDK image recorded in
the historical README; host tests: `sh tests/run.sh`.
No Sony firmware, decompiled code, private logs or real controller keys included.
