# PSP Go DS4 trace v0.9.1 — receive-length correction

This fixes a diagnostic parsing mistake in v0.9. It is **not a Bluetooth connection fix** and does not require another hardware test just to verify the same failed connection.

## Finding from the v0.9 log

Both native outgoing requests failed with state 2 and 802F0140. The incoming decision hook installed, but recorded no incoming HID request. This does not establish that the controller never contacted the radio: failure could occur below that observation point.

The event's receive length of ten includes the four-byte header, leaving six payload bytes: slot, state and profile flags. v0.9 incorrectly treated ten as the payload length and logged two extra halfwords from the reusable receive buffer. The observed 0072/0065 values are not valid event fields and must not be interpreted as HCI/authentication error codes. They are consistent with stale UTF-16 text from an earlier record.

v0.9.1 removes those reads entirely, checks the inclusive length and reports packet_length separately from payload_length. The original native event handling/connection behavior is untouched. The native receiver accumulates actual USB transfer bytes and compares them directly to this header length; the consumer reads the three actual payload halfwords.

## Use

No new PC helper or address-writing test is required. Keep this corrected PRX for the next justified hardware test. To install it, replace `ef0:/seplugins/pspgo_ds4_pairtest.prx`, keep one enabled VSH entry in the existing PLUGINS.TXT, and fully power the Go off/on. Keep older diagnostics and btfree disabled. The newest log section identifies 0.9.1.

The existing incoming-decision trace, automatic logger, discovery and guarded generic-HID menu corrections remain. The logger flushes known numeric events every 500 ms without file I/O in the native Bluetooth event worker. If the logger cannot start, opening Manage Bluetooth Devices remains the flush fallback.

Native connection decisions, modes, authentication, keys, device IDs and report decoding remain unchanged. This update does not add DS4 authentication or establish a working controller connection. No flash/cIPL changes occur. Normal menu operations can change saved pairing records. Live unload is refused while resident hooks/logger remain; disable the entry and fully power off/on to clear RAM changes.

## Validation

Strict PSP GCC15.2.0 build passed with -Wall -Wextra -Werror using pinned pspdev image sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b. Host ASan/UBSan checks passed for prior trampoline/relocation guards, 120 interpreted menu routes, incoming call-site guards and corrected inclusive receive-length bounds. Ten-byte SLC packets are checked as six payload bytes. The correction has not been hardware tested here; it does not change the native connection result.

Source/tests/MIT licence and PSP SDK notice included. Discovery technique credited to JJS/Yoti btfree and its ARK continuation. No user logs/captures, Sony firmware or decompiled code redistributed. Earlier validation notes are historical; this correction supersedes v0.9's claimed extra failure fields.
