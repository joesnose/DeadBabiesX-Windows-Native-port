# PSP Go corrected ACK / bounded RAM test v0.6

**This does not expose HCI or connect a DS4 yet.** It performs one new, bounded
same-value RAM test to capture the previously missing acknowledgment and verify
storage independently. It preserves the v0.4 markers and uses its own v0.6 marker.

## Evidence and correction

v0.4 sent a SET but rejected its reply before readback. v0.5 retained the old
marker, so its two runs correctly made no new write and could not capture that
old acknowledgment. The old bytes cannot be reconstructed after restart.

The previous parser additionally required SET-response payload fields to echo
the key/store/value. The reference CSR writer checks returned status, without
relying on those payload echoes. This version validates type=SETRESP(3), word
length/buffer bounds, sequence, varid and zero status; it does not interpret the
SET payload as proof of storage. GET replies retain strict key/store/value
parsing. The exact previous Sony reply remains unknown until a capture is made;
this is a correction to an over-strict assumption, not a claimed reconstruction.

The initialized response-buffer snapshot, independent RAM GET, and four baseline
post-checks run even if the new SET acknowledgment is rejected. An unfamiliar
reply is not accepted automatically. Successful acknowledgment and independent
GET=1 with unchanged baseline are required for a verification marker.

## Exactly one new attempt

Requires a valid `pspgo_ramtest_v04.once`, no v0.4 verified marker, the identified
radio build 6424, store0008 identified as transient RAM, exact known mode baseline,
and absent RAM override. It must durably create a new one-shot marker first.
Only this same-effective-value SET can then be sent:

```
USB_VM_CONTROL (03C0) = 1, store0008
```

No off value, persistent-store SET, mode switch, reset, power command, address
write or link-key operation is added. Native management calls have no local
cancellation timeout; each operation is logged before submission.

v0.6 marker files are `ef0:/seplugins/pspgo_ramtest_v06.once` and `.verified`.
The fixed test token is unchanged; the filenames identify the new attempt.
Keep both old and new markers. No marker is deleted or renamed, and later starts
cannot repeat the v0.6 SET. A failed/ambiguous attempt also stays blocked.
An installation without the prior v0.4 attempt marker stops without writing.

## Hardware step

1. Replace the existing `ef0:/seplugins/pspgo_hci_transport.prx` with v0.6 and
   enable its existing VSH entry. Keep previous probes disabled and all markers.
2. Fully power off/on, enable Bluetooth and wait about 30 seconds.
3. Fully power off/on once more, enable Bluetooth and wait about 30 seconds.
4. Send `ef0:/seplugins/pspgo_hci_transport.log` with both runs.

Registration can wait up to 180 seconds if native initialization is late. No
controller pairing, PC helper, manual file deletion or extra configuration is
needed. A proven write followed by an unavailable RAM key on the later startup
would support volatility during that interval; it would not establish HCI mode
activation or complete recovery from a failed transition.

## Validation and limits

PSP build and sanitizer tests cover header-only and non-echo SET acknowledgments,
rejection of wrong types/sequences/varids/status, independent GET/post-checks on
bad acknowledgment, prior-history gates and no repeated v0.6 writes. Retained
transport/host/DS4/input suites are included. Hardware ACK compatibility, writes,
restart retention, native HCI transport, real pairing and controls remain
unverified. The host stack stays idle without standard HCI attachment.

Original adapters MIT. BTstack retains its personal/noncommercial license in
`vendor/btstack/LICENSE`, copyright notices and pinned upstream commit. SDK notice
included. No Sony firmware/decompiler code, private logs or controller keys.
Build: `make` in the pinned SDK image documented in `docs/RAM-TEST-v0.4.md`.
Host tests: `sh tests/run.sh`. Historical instructions are superseded here.

Protocol evidence: BlueZ 5.50 `tools/csr.c`, `csr_write_pskey_complex` (status
check, no payload echo dependency):
https://github.com/bluez/bluez/blob/5.50/tools/csr.c . Wire constants and logic were
independently implemented; GPL source is not copied into this project.
