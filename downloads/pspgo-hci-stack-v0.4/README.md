# PSP Go RAM-override capability test v0.4

**Not an HCI mode switch or DS4 connection fix.** This is the next prerequisite
for a temporary HCI transition: establish whether the radio accepts a RAM-only
override and whether that override disappears across a full power-off/startup.
The compiled pairing/HID stack is retained but remains idle without HCI attachment.

## What this version does

Previous hardware runs showed both mode keys at 1 in PSI/default, 0 in ROM, and
rejected their RAM-only reads with outer status 0050. That did not distinguish
an empty RAM store from unsupported access.

This test first requires all of the following:

- Radio build 1918 hex (6424), with the same native management ABI/code guards.
- PS_MEMORY_TYPE GET for selector 0008 returns type 2, documented as transient RAM.
- USB_VM_CONTROL is 1 in default and PSI, 0 in ROM; ONCHIP_HCI_CLIENT defaults to 1.
- USB_VM_CONTROL RAM-only GET returns outer 0050, as in the supplied baseline log.
- No previous attempt/verification marker, and a one-shot attempt marker can be
  exclusively created, synchronized and read back before sending a SET.

Only then can it send exactly one configuration write:

```
BCCMD SET PSKEY_USB_VM_CONTROL (03C0), store 0008, value 0001
```

That value matches the current effective value; it does not request HCI mode.
No caller-selected key/store/value, persistent-store SET, mode-off value, key
clear/delete, radio reset, power change, address or link-key write is included.
The transient-store identification itself remains a radio-reported fact to be
verified; no universal recovery guarantee is claimed for Sony's firmware.

The test requires a valid SET acknowledgment and RAM-only readback of 1, then
rechecks the original effective/PSI/ROM/on-chip values. On verified success it
saves a second marker. Future starts perform observations only, never another
SET. If the verified RAM value later becomes unavailable with 0050, the combined
log provides evidence that this particular override disappeared during the
restart interval. It does not alone prove that changing modes will enumerate
HCI or that every failure can be recovered.

## Installation and test — two full startups

1. Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this version.
2. Enable its existing VSH entry. Keep older pairing/radio probes disabled.
3. Fully power off/on, enable Bluetooth and wait about 30 seconds. Do not pair
   the DS4 or use the controller-address helper.
4. Fully power off/on once more, enable Bluetooth and wait about 30 seconds.
5. Send `ef0:/seplugins/pspgo_hci_transport.log`, containing both runs.

Registration can wait up to 180 seconds for the native USB host. A prerequisite
rejection means the test stops without its SET; the second run will document
that rejection again. No additional manual recovery procedure is assumed.

Existing ARK entry, if needed:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
```

One-shot markers (not controller keys):
`ef0:/seplugins/pspgo_ramtest_v04.once` and
`ef0:/seplugins/pspgo_ramtest_v04.verified`.
Keep these files: deleting them can re-arm the experiment. Failed or ambiguous
attempts are not automatically retried. Malformed/unreadable markers stop SETs.
The markers do not encode a boot identifier; the user must actually perform the
two full power cycles for the log to be a restart observation. No persistence
claim is made from an ordinary plugin reload or a single run.

A stalled native management transaction cannot be locally canceled; each
operation is logged before submission. Keep the combined log if a call stalls.

## Status, code and licensing

The driver registered on hardware in v0.2/v0.3. The host stack has software tests
only. HCI exposure, native HCI transfers, actual Bluetooth pairing, PSP input
hooks and usable DS4 controls remain unverified. No HCI transition is enabled by
this package. The retained host-stack implementation and limitations are in
`docs/HOST-STACK-v0.3.md` (historical v0.3 documentation); this README supersedes
its read-only test instructions and no-SET description for the new v0.4 test.

Original adapters: MIT. Vendored BTstack: personal/noncommercial upstream terms,
not MIT; see `vendor/btstack/LICENSE`, retained copyright notices and pinned
`UPSTREAM_COMMIT`. SDK notice: `notices/PSPSDK-LICENSE.txt`. No Sony firmware,
decompiled code, private user logs or proprietary PC pairing software included.
Do not upload runtime bond files (`pspgo_ds4_bond.bin`), which contain keys.

Build using `make` with the pinned SDK image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` for sanitizer-backed software tests. See
`docs/VALIDATION.md` and `docs/TRANSITION.md`. Resident driver/hook storage is
removed by disabling the entry and fully restarting, not live unload.
