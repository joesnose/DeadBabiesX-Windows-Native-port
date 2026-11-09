# PSP Go combined RAM suite v0.9 — fixed 50-byte envelopes

The v0.8 hardware run attempted only two cases. The standard 50-byte request
returned GET-style response1 and no readable RAM override. The 18-byte request
then returned native 8030000C; subsequent management calls returned 802F0203.
The suite stopped and saved count2/seen0/validACK0 with a valid CRC. After a full
restart, management access and both known baselines were observed again.
The exact firmware meaning/cause of these error codes is not established here.

## Corrections

- Remove the 18-byte native envelope completely from the compiled suite.
- Run four cases: two mode keys x two fixed-50-byte forms (BCCMD declared9words
  and declared25words). The larger declared length remains experimental, not a
  proven Sony-supported form. No other Sony request ID is tried.
- Stop immediately after native transport failure. A latched fault prevents
  subsequent native calls, including followup GETs. Do not retry, reset, kill a
  thread holding native resources or send further probes into an unavailable path.
- Keep independent readback/post-checks when the transport returned normally,
  including GET-style/rejected acknowledgments. Human-readable result names
  distinguish unavailable RAM values from numeric placeholder zeros.

**This is still a diagnostic, not an HCI mode switch or DS4 connection fix.**
The sole SET remains each key's existing value1 in transient store0008; keys are
USB_VM_CONTROL03C0 and ONCHIP_HCI_CLIENT03CC. Known radio build/transient-memory
and default1/PSI1/ROM0 prerequisites remain. No off value, persistent SET, key
clear/delete, reset, power, address or link-key command is included.

## One PRX, automatic sequence

This is one new bounded four-case suite with its own marker
`ef0:/seplugins/pspgo_ramsuite_v09.once` and CRC-checked summary `.state`.
Earlier markers/states are retained untouched. Each new case is attempted at
most once; subsequent starts only observe. A partial/failed suite is not resumed
or repeated automatically. A value visible before a later case is labeled and
cannot independently prove that later variant wrote it.

Replace the existing PRX, retain the enabled VSH entry and all marker/state files.
Fully power off/on, enable Bluetooth and wait about 60 seconds. Fully power off/on
once more, enable Bluetooth and wait about 30 seconds, then send the combined
`ef0:/seplugins/pspgo_hci_transport.log`. No controller pairing/helper, manual
case selection, file deletion or configuration changes are needed.

Bounded means count, sizes and allowed operations. Native calls still have no
local cancellation deadline; a dropped request can stall before remaining cases
complete. Registration can wait up to 180 seconds. No hard deadline, guaranteed
recovery or hardware success is claimed. If every fixed-envelope case returns
GET-style replies without RAM visibility, further repetitions of these same
writes are not justified; a different management route must be understood.

## Validation/licensing

PSP build and ASan/UBSan suites passed. Tests assert every native request is
50bytes, cover four cases/ACK variants/readback/summary/CRC/markers, and assert
that no native query is issued after a simulated transport failure. Restart
observation is simulated, not actual radio validation. Real writes, retention,
HCI, SSP/HID and DS4 input remain unverified. Historical docs are superseded by
this README; unlinked historical source is retained only for regression tests.

Original adapters MIT. BTstack retains its personal/noncommercial upstream
license/notices in `vendor/btstack/`; SDK notice and pinned build metadata
included. Build with `make`; host tests: `sh tests/run.sh`. No Sony firmware,
decompiled code, private logs or actual controller keys are distributed.
