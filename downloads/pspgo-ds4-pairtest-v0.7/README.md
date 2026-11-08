# PSP Go DS4 connection-event trace v0.7

For PSP Go, 6.61, ARK-5 cIPL. Experimental diagnostic build: connection and controls remain unconfirmed.

v0.6 registered the DS4 but its native HID connection returned 802F0140, including when tested in pairing mode. This is a generic asynchronous service-link failure. v0.7 retains the menu correction and records the associated native event's slot, state, profile flags and payload length. It does not yet identify the low-level Bluetooth reason or fix the connection.

## Install and test

1. Replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with this version. Keep exactly one enabled entry in your existing PLUGINS.TXT: `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`.
2. Keep older diagnostics and btfree disabled. Fully power off/on; a standby/resume does not clear resident hooks.
3. In XMB Bluetooth settings, select the already registered Wireless Controller. With USB disconnected, put the DS4 in pairing mode (SHARE + PS) and try Connect once.
4. Send `ef0:/seplugins/pspgo_ds4_pair.log`. The newest section should say 0.7, with `SLC_TRACE event call installed` and, if observed, `SLC_EVENT` lines. No capture flag or timed wait is required.
5. Disable the plugin and fully power off/on after testing. Live unload is refused while hooks are referenced.

## Scope and checks

Native arguments, connection modes, results and report decoding remain unchanged. No extra Bluetooth commands are sent. The event hook copies only known numeric fields into a fixed 16-entry RAM buffer during an outgoing request. It performs no file I/O in the native event worker; the requesting thread writes the captured events after the connection call returns. Concurrent extra requests are forwarded without opening a second trace. Events are listed in capture order, before the connection result.

The call-site patch requires the expected JAL target, argument delay slot, following load and original handler prologue. A mismatch skips tracing. Existing discovery/menu guards remain. No names, addresses, PINs, link keys, unknown payload bytes or user captures are logged or included. RAM patches clear on full reboot; ordinary menu actions may save pairing records. No flash/cIPL changes.

Strict PSP GCC 15.2.0 build passes with -Wall -Wextra -Werror. Host ASan/UBSan checks pass for call-site mismatch rejection, jump targets and region/alignment checks, existing trampolines and 120 interpreted menu-routing cases. Hardware event tracing has not yet been verified. These checks do not establish usable DS4 support.

Source is in source/, with MIT licence and SDK notice. Build with make and the PSP SDK/SystemControl stubs. Discovery technique credited to JJS/Yoti btfree and its ARK continuation. No Sony firmware or decompiled code is distributed.
