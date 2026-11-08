# PSP Go DS4 connection-policy trace v0.9

**Focused diagnostic update, not a confirmed DS4 connection fix.** Target: PSP Go / 6.61 / ARK-5 cIPL. No PC helper or further address-writing test is required.

## Why this build

The USB helper confirmed that the DS4's saved host already matched the Go. The previous native Connect trace returned a failed service-link event (state 2) rather than the state-0 initialization path. The DS3 VID/PID handling found in HID initialization is therefore not established to be the cause of that attempt's failure.

Firmware inspection found two additional observation points: the native incoming-service request's allow/reject decision, and four tail bytes in the ten-byte failure event that the current high-level error path does not interpret. v0.9 records both, preserving native decisions, arguments and return values. The tail words are explicitly logged as **opaque**. They are not yet established HCI error codes or authentication reasons.

## One hardware test

1. Replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with the included PRX. Keep one enabled existing entry in `ef0:/SEPLUGINS/PLUGINS.TXT`: `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`.
2. Keep older diagnostics and btfree disabled. Fully power the Go off/on to clear resident hooks. Keep the saved controller registration and the USB pairing target already written.
3. Disconnect DS4 USB, enable Bluetooth on the Go and press PS alone on the DS4. Try the D-pad in XMB after about 10 seconds.
4. If not connected, try the menu's Connect once with the controller awake. Allow at least two seconds after the result so the logger can flush.
5. Send just `ef0:/seplugins/pspgo_ds4_pair.log`. No Linux helper log, host-address copying, firmware capture or repeated pairing is needed. If any buttons worked, say so.
6. Disable the entry and fully power off/on after testing. Live unload is refused while hooks and the logger thread remain resident.

## What it records

- `HID_INCOMING_BEGIN`: native incoming request's slot, HID profile flags and `allow` argument. `allow=0` is a native reject decision; `allow=1` requests acceptance. The plugin does not override it.
- `HID_INCOMING_END`: original helper result. This is not by itself proof of an established connection.
- `SLC_EVENT`: native service-link state/profile/length and whether it occurred during the traced outgoing Connect call.
- `SLC_FAILURE_DETAIL`: two raw 16-bit tail values, only for a state-2 HID failure with exactly ten payload bytes. Their semantics remain unknown.
- `seq` orders the buffered events. Drop counts are reported if a bounded buffer fills.

A dedicated 8 KiB kernel logger thread flushes buffered events every 500 ms. The native Bluetooth event worker only copies small numeric records to fixed RAM buffers; it performs no file I/O. Existing UI-thread drains remain as a fallback. If the log says LOGGER unavailable, reopen Manage Bluetooth Devices to flush events before copying the log.

The incoming hook changes one verified JAL call site and calls the original helper directly exactly once. It requires the original argument setup, JAL target, delay slot, following return jump/delay and original callee prologue to match. Unknown firmware context skips this hook. No trampoline or function-entry rewrite is needed for this additional hook.

## Scope

Existing discovery and guarded generic-HID menu fixes remain. Native connection mode, authentication, pairing keys, profile records and report decoding are not changed by this update. No device identity spoof or forced success is added. The Go's local Bluetooth-address export from v0.8 remains but is not needed for this test. No flash/cIPL changes occur; normal stock menu operations can change saved pairing records.

Logs exclude device names/addresses, PINs and link keys. Only the fixed short failure-event tail described above is added; there is no general packet dump or unknown security-payload capture. The separate local host-address file from v0.8 is not included in any upload.

## Checks and remaining limits

Strict PSP GCC15.2.0 build passes with -Wall -Wextra -Werror and pinned pspdev image sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b.

ASan/UBSan host checks pass for all prior trampoline/address guards, 120 interpreted menu-routing cases, the SLC hook, new incoming call-site guards and exact-length failure-tail parsing. Tests reject mutated call context, wrong jump region/alignment and short failure buffers. The new call-site context and targets were independently checked against the actual loaded-code capture. These are offline checks; hardware behavior and the new logger/hook still need the test above.

Source/tests/licences are included. New code is MIT licensed with the PSP SDK notice. Discovery technique credited to JJS/Yoti btfree and its ARK continuation. No Sony firmware, decompiled code, user logs/captures or PC utilities are redistributed.
