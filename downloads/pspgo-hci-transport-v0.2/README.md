# PSP Go USB HCI transport v0.2 — experimental developer prerelease

Original kernel USB class driver for PSP Go, reporting firmware 6.60 or 6.61, intended for ARK-5.
**This is a transport foundation, not a working DS4 controller plugin. Hardware
loading, attachment and transfers have not been verified. It does not fix 802F0140.**

## Implemented

- Registers an original 24-byte class-driver descriptor with the native USB host.
- Matches the primary standard Bluetooth HCI interface: interface 0, alternate 0,
  class E0/subclass 01/protocol 01, exactly three usable full-speed endpoints.
- Rejects Sony's present CDC interfaces and other USB classes.
- Opens interrupt IN for HCI events and bulk IN/OUT for ACL.
- Sends HCI commands through the standard device-recipient class control request.
- Exposes five kernel-only exports in `PSPGoHciTransport`; header in
  `src/pspgo_hci.h`. Commands/events/ACL omit the UART H4 packet-type byte.
- Reassembles fragmented and coalesced HCI events and ACL packets in bounded
  storage. Supports events up to 257 bytes and ACL packets up to 1028 bytes.
- Uses aligned resident DMA buffers, cache maintenance, per-channel ownership,
  submission/detach serialization, generation checks, and cancellation.
- Retains/quarantines storage if cancellation cannot establish completion. Does
  not allow live unload; disabling the plugin requires a full power cycle.

## What still prevents DS4 support

The Go currently exposes Sony's VM-managed CDC interface and runs an on-chip
Bluetooth stack. This driver waits for a standard USB HCI interface; it does not
make that interface appear. A reversible, tested radio transition/recovery path
is still missing. It writes no radio persistent keys, controller addresses, link
keys or firmware and does not reset the radio.

A Bluetooth host stack (authentication/link keys, HCI command credits and ACL
flow control, L2CAP, SDP and HID), DS4 report handling, and PSP input injection
are also required. Those are not implemented by this transport. A successful
`REGISTERED` log means only USB class-driver registration, not radio attachment,
Bluetooth pairing, a DS4 connection, or usable controls.

The native registration routine may scan unclaimed USB interfaces using its
normal enumeration behavior. The new driver does not patch Sony's driver or
claim its current CDC interfaces. No continuous USB reads or HCI commands are
submitted until a separate kernel caller invokes the transport APIs.

## Current use

No installation or further controller-pairing attempt is needed from the user
at this stage. This release is published for source review and development.
Keep previous pairing/radio probes disabled. Do not change radio mode settings
to try to activate this driver: no recovery procedure has been established.

For a developer-only **registration smoke test** on PSP Go reporting 6.60/6.61 with ARK-5, copy
`pspgo_hci_transport.prx` to `ef0:/seplugins/` and add this line to the existing
ARK plugin configuration (`ef0:/seplugins/PLUGINS.TXT`):

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Fully power off/on, then enable Bluetooth. The registration worker waits up to
180 seconds for the native host to initialize. Log:
`ef0:/seplugins/pspgo_hci_transport.log`. A stock CDC radio is expected to remain
unattached. Disable/remove the entry and fully power off/on to remove the driver.
The package does not overwrite any existing plugin configuration.

## API and lifecycle limits

Kernel threads only, trusted kernel pointers, timeout 1..1000 ms per API call.
Calls may sleep. Each event/ACL channel allows one concurrent caller; competing
calls return busy. Control commands serialize with lifecycle operations.
Scheduling, lock contention and native cancellation can extend wall-clock time;
this is not a hard real-time deadline. Native cancellation's synchronous drain
behavior was recovered from the host implementation but remains unverified on
hardware. Callers must compare positive write results with the requested length.
Receive timeouts preserve already-framed partial input; if a failed transfer may
have consumed unaccounted bytes, or malformed framing is detected, that channel
is quarantined until reboot rather than guessing a packet boundary. This favors
integrity over automatic recovery in this first version.

PSP Go model, reported 6.60/6.61 version, module, export-offset and jump-stub checks precede
registration. These are ABI compatibility checks, not a full firmware-integrity
verification. Patched/incompatible host exports are rejected. Never unload the
native USB host beneath this resident driver.

## Build and tests

Pinned SDK image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`
(GCC 15.2). Mount this source directory at `/project`, use `/project` as workdir,
run `make`. No network is needed inside the SDK container.

Host logic tests: `sh tests/run.sh` (C compiler with AddressSanitizer and UBSan).
See `docs/VALIDATION.md`. Mocks do not establish native ABI correctness or actual
Bluetooth/USB operation. No Sony firmware, disassembly, decompiled code, private
user logs, or proprietary pairing software is distributed.

Original code: MIT. Linked PSPSDK notice is in `notices/PSPSDK-LICENSE.txt`.

## Change in v0.2

The v0.1 hardware log reported devkit `06060010` and stopped before registration.
Accept that 6.60 value as well as `06060110` (6.61). The log alone cannot tell
whether this is actual firmware or ARK version reporting. The native USB-host
module/export-offset and jump-stub checks remain unchanged; a different host
layout still stops without registration. This does not establish general 6.60
compatibility. No radio-mode change or DS4 connection fix is added.

Replace only the existing PRX, keep its existing VSH entry, fully power off/on,
enable Bluetooth, and allow up to 180 seconds for registration. Send the updated
`ef0:/seplugins/pspgo_hci_transport.log`. A registration line alone does not prove
HCI attachment.
