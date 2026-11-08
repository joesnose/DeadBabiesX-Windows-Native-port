# PSP Go / DualShock 4 research — diagnostic v0.1

Target: PSP Go, Sony firmware 6.61, ARK-5 cIPL.

**This package does not add DualShock 4 Bluetooth support yet.** It contains a compiled, read-only module-inventory plugin and a separately tested DS4 report decoder. Pairing, HID channel setup, receiving input, and injecting it into PSP controls remain unimplemented. The decoder is not connected to the diagnostic PRX.

The diagnostic identifies the loaded native Bluetooth modules on the target console and their function identifiers. It does not call unknown Bluetooth functions, patch code, change pairing records, or change cIPL/flash. Its only file write is an appended log on internal storage. It has been compiled but has not run on physical PSP hardware here.

## Run the diagnostic

1. Copy `pspgo_ds4_diag.prx` to `ef0:/seplugins/` on the PSP Go's internal storage.
2. Add this line to your existing `ef0:/SEPLUGINS/PLUGINS.TXT` (keep your other lines):

   `vsh, ef0:/seplugins/pspgo_ds4_diag.prx, on`

3. Restart the VSH. Within the next 90 seconds, open Bluetooth Device Settings and start a device scan. The diagnostic observes loaded modules for 90 seconds, then stops logging; there is no on-screen UI and it does not unlock or pair a DS4.
4. After 90 seconds, copy `ef0:/seplugins/pspgo_ds4_diag.log` back and send it for analysis. Disable/remove just the line added above afterward.

If no log appears, check the path, confirm the plugin is enabled for VSH, and report that result. If the log reports an unsupported model/firmware or module query errors, send it as-is. Model 4 is PSP Go. Both 6.60/6.61 devkit values are accepted because CFW may report 6.60 on a 6.61 installation.

The log contains firmware/model identifiers, module names, memory addresses, and exported function identifiers. It does not capture Bluetooth keys, traffic, or device addresses. It can help confirm the native module layout, but will not by itself establish the undocumented pairing/transport APIs.

## Development

Compile using the official `pspdev/pspdev` container or a local PSP SDK:

```sh
make
```

Toolchain used: GCC 15.2.0; container digest `sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.

Run host-side decoder tests:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/test_ds4.c src/ds4_report.c -o test_ds4
ASAN_OPTIONS=detect_leaks=0 ./test_ds4
```

The test has 105 assertion groups covering button mappings, diagonals, stick/trigger values, simple/full Bluetooth report layouts, an independently generated CRC fixture, corrupted packets, every truncated full-report length, invalid hats and null arguments. The environment's ptrace setup prevents LeakSanitizer operation; leak checking is disabled, while address/undefined-behaviour checking remains enabled. This decoder performs no dynamic allocation. Synthetic packet tests do not verify wireless connectivity.

`src/ds4_report.c` accepts 10-byte `0x01` Bluetooth reports and 78-byte `0x11` reports with validated CRC. The caller must establish the device identity and remove the HIDP transaction header. USB reports, other controllers, and arbitrary transport framing are intentionally rejected. DS4 button mappings use PSP control bit values; mapping PS to HOME is a candidate mapping for a future driver, not an implemented system-menu hook.

See `docs/RESEARCH.md` for the remaining work and source references. All new code in this package is released under the MIT licence.
