# PSP Go DS4 host stack + HCI transition preflight v0.3

**Experimental, incomplete controller support. This does not yet expose HCI on
the Go's radio and does not fix 802F0140.** v0.2 successfully registered its USB
class driver on the user's PSP Go/ARK system. That did not validate USB HCI
attachment, transfers, Bluetooth pairing or usable controls.

## What changed

This build adds a PSP port of upstream BTstack's Classic Bluetooth host stack,
plus original PSP adapters for its transport, scheduler, bond storage and input.
The compiled path includes HCI initialization/command credits/ACL flow control,
SSP pairing, L2CAP, SDP and HID host. The application selects Sony DS4 identities
(USB vendor 054C, products 05C4/09CC), requests Report protocol and feature 02,
checks HIDP input transactions and full-report CRC, and maps decoded controls to
PSP input. This is implemented code, **not hardware-verified DS4 support**.

The stack remains idle until the USB class driver actually attaches a primary
standard HCI interface. It sends no HCI Reset while waiting. On real attachment,
BTstack initializes the controller. With no saved bond it opens a 60-second SSP
pairing window and discovers DS4 PnP identities advertised in extended inquiry
responses. It requires the Sony USB identity; it does not select by generic
"Wireless Controller" name alone. Clones and controllers omitting that identity
are not supported by this first policy. Legacy PIN pairing is rejected.

One peer's bond is stored in `ef0:/seplugins/pspgo_ds4_bond.bin`, tied to the local
radio address, versioned and CRC checked. Keys are never logged. The file is
sensitive and is not encrypted; do not upload it. Persistence failure is logged
and leaves a session-only bond. Save/rename semantics still need PSP hardware
validation. Software CRC detects corruption, not malicious modification.

PSP syscall hooks for four ordinary sceCtrl read/peek positive/negative APIs are
installed only after a valid DS4 report arrives. Native results and physical
buttons are preserved, analog input has a center dead zone, and remote input
expires after 250 ms without a valid report. Disconnect/error clears remote
input. Extended/private controller APIs and every game's control path have not
been covered. Hooks and driver buffers remain resident until full restart.

## Required next test: radio transition preflight

The unresolved gate is switching the built-in radio from Sony's VM-managed CDC
interface to standard HCI while retaining a recovery route. CSR documentation
says USB_VM_CONTROL=false selects HCI enumeration; unified firmware also uses
ONCHIP_HCI_CLIENT. Published CSR tools identify store 0008 as PSRAM and use warm
reset after RAM configuration. That does not establish activation, reset,
retention or power-recovery behavior for Sony build 6424.

This version therefore performs **ten allowlisted GETs only**: keys 03CC and
03C0 across selectors 0000 (default), 0001, 0002, 0004 and 0008. It records
per-store values or errors. This adds store provenance to the previous default
reads; it does not repeat address writes or pairing tests. No generic BCCMD
interface, SET, warm/cold reset, radio power change or firmware write is included.

1. Replace the existing `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX.
2. Keep its existing VSH entry; keep older pairing/radio probes disabled.
3. Fully power off/on, enable Bluetooth and wait about 30 seconds (registration
   can wait up to 180 seconds if the native host initializes late).
4. Send `ef0:/seplugins/pspgo_hci_transport.log`. No DS4 pairing attempt is needed.

Existing ARK configuration line, if needed:

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Expected current behavior is REGISTERED, STORE GET results, PREFLIGHT DONE, then
HOST WAIT. That is not a DS4 connection. A management transaction has no local
cancellation timeout; each GET is logged before submission to identify a stall.
A missing store or rejected selector is evidence to examine, not permission to
write a different store. `docs/TRANSITION.md` records the remaining gates.

## Build, tests and limits

Source and required upstream files are included. Build with `make` in the pinned
PSP SDK container:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Host tests: `sh tests/run.sh`. See `docs/VALIDATION.md`.

Firmware reporting 6.60 or 6.61 is accepted on PSP Go; native module/export-offset
and entry checks remain in place. Those are ABI checks, not full integrity checks.
Native USB transfer/cache/cancellation behavior remains unverified on hardware.
Receive polling uses bounded local waits followed by native cancellation. The
prototype's polling overhead, latency and race behavior require hardware tests.
The timeouts are not hard wall-clock deadlines under scheduling/lock contention.

No live unload, suspend/resume recovery or automatic radio-mode restoration is
implemented. Do not manually change mode keys to activate the stack. Disable the
plugin and fully restart to remove it; no existing plugin configuration is
included or overwritten.

## Licensing

Original adapter code is MIT (`LICENSE`). BTstack is **not MIT**: its upstream
license permits personal/noncommercial use and redistribution subject to its
conditions; commercial use requires an appropriate BlueKitchen license. See
`vendor/btstack/LICENSE` and `vendor/btstack/UPSTREAM_COMMIT` for the pinned source.
Copyright notices in vendored files are retained. Linked SDK notice is in
`notices/PSPSDK-LICENSE.txt`.

No Sony firmware/disassembly/decompiled code, private logs or proprietary pairing
software is included.
