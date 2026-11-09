# PSP Go RAM-test history-gate fix v0.7

The supplied v0.6 log stopped at the old-marker prerequisite on both starts,
before any GET/SET experiment. That check was an unnecessary dependency on a
previous diagnostic file, not evidence that the current radio was unsafe.

v0.7 records old-marker observations but does not require them. It preserves
exactly the v0.6 one-shot paths/tokens: `pspgo_ramtest_v06.once` and `.verified`.
It cannot repeat a v0.6 attempt already recorded. Invalid/unreadable current
markers still block writing, and their errors/lengths are now logged separately.
No marker is deleted, renamed or automatically repaired.

The actual prerequisites remain unchanged: supported PSP/firmware/module guards,
radio build 6424, store0008 reported as transient RAM(type2), exact known effective,
PSI/ROM/on-chip values, absent RAM override and a durably created one-shot marker.
At most one same-value SET USB_VM_CONTROL=1/store8 is permitted. No HCI mode-off
value, persistent SET, radio reset, power command, address/key operation or
controller pairing is added. SET header/status validation, bounded ACK snapshot,
independent readback and all baseline checks remain as in v0.6.

## Hardware step

Replace only `ef0:/seplugins/pspgo_hci_transport.prx`, keep its existing VSH entry
enabled and all marker files, then fully power off/on, enable Bluetooth and wait
about 30 seconds. Do that full startup once more, then send the combined
`ef0:/seplugins/pspgo_hci_transport.log`. No file deletion or extra configuration.
Registration may wait up to 180 seconds; each native transaction is logged before
submission because it has no local cancellation timeout.

This corrects the test gate; **HCI exposure and real DS4 support remain
unfinished**. A successful same-value write and later disappearance would inform
RAM volatility, not establish mode switching or universal recovery.

PSP build and sanitizer suites include missing/present legacy-marker cases,
ACK variants/rejection, strict current-marker handling and no repeated writes.
Hardware execution of this correction remains unverified. Historical v0.6
instructions in `docs/RAM-TEST-v0.6.md` are superseded by this README's removal of
the old-marker prerequisite; other test limits remain.

Original code MIT. Vendored BTstack retains its personal/noncommercial upstream
license and notices in `vendor/btstack/`; SDK notice included. Source/tests and
pinned build details included; no Sony firmware, decompiled code, user logs or
actual controller keys. Build with `make`; host tests: `sh tests/run.sh`.
