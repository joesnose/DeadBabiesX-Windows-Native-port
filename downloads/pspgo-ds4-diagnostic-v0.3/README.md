# PSP Go / DualShock 4 research — diagnostic v0.3

Target: PSP Go, Sony firmware 6.61, ARK-5 cIPL.

**This package does not add DualShock 4 Bluetooth support yet.** It contains a compiled module-inventory plugin with opt-in native code capture and a separately tested DS4 report decoder. Pairing, HID channel setup, receiving input, and injecting it into PSP controls remain unimplemented. The decoder is not connected to the diagnostic PRX.

The diagnostic identifies the loaded native Bluetooth modules on the target console and their function identifiers. It does not call unknown Bluetooth functions, patch code, change pairing records, or change cIPL/flash. By default it writes only an appended log on internal storage. An optional flag enables code snapshots of five selected native drivers, up to 64 KiB each, on internal storage. It has been compiled but has not run on physical PSP hardware here.

## Run the diagnostic

1. Copy `pspgo_ds4_diag.prx` to `ef0:/seplugins/` on the PSP Go's internal storage.
2. Add this line to your existing `ef0:/SEPLUGINS/PLUGINS.TXT` (keep your other lines):

   `vsh, ef0:/seplugins/pspgo_ds4_diag.prx, on`

3. Restart the VSH. Within the next 90 seconds, open Bluetooth Device Settings and start a device scan. The diagnostic observes loaded modules for 90 seconds, then stops logging; there is no on-screen UI and it does not unlock or pair a DS4.
4. After 90 seconds, copy `ef0:/seplugins/pspgo_ds4_diag.log` back and send it for analysis. Disable/remove just the line added above afterward.

If no log appears, check the path, confirm the plugin is enabled for VSH, and report that result. If the log reports an unsupported model/firmware or module query errors, send it as-is. Model 4 is PSP Go. Both 6.60/6.61 devkit values are accepted because CFW may report 6.60 on a 6.61 installation.

The log contains firmware/model identifiers, module names, memory addresses, exported function identifiers and, when enabled, code-capture file sizes/checksums. It does not capture Bluetooth keys, traffic, or device addresses. It can help confirm the native module layout, but will not by itself establish the undocumented pairing/transport APIs.

## Optional next step: native driver code capture

The v0.2 log already confirmed the core Bluetooth modules and their exports. Another basic module-list scan is not needed for that. The next stage needs private offline analysis of the native driver instructions.

To opt in to code capture:

1. Replace the old PRX with this v0.3 PRX and enable the same VSH plugin entry.
2. Copy the included `pspgo_ds4_capture.flag` to `ef0:/seplugins/pspgo_ds4_capture.flag`.
3. Restart VSH and wait 90 seconds. The five target modules were already loaded in the v0.2 log; a new pairing attempt is not required for these snapshots.
4. Copy back `pspgo_ds4_code_*.bin` files and `pspgo_ds4_diag.log`. Zip them together and attach them privately in this chat for analysis. The binaries contain Sony driver code, so do not add them to the public GitHub source repository.
5. Remove the capture flag and disable the diagnostic plugin entry afterward.

Targets: sceBSMan, sceHIDSvc, scePadSvc, sceInput_Library, sceUsbBSMCDC_Driver. Only each module's reported code region is copied; data segments, pairing records and general RAM are not dumped. No Bluetooth commands are issued and no flash changes are made. Each snapshot is limited to 64 KiB; incomplete writes are removed and logged. Capture mode is new and has not been tested on hardware. It remains disabled unless the flag file exists. The log supplies load addresses and lengths needed to disassemble the snapshots.

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

## v0.2 fix

The first on-device log confirmed model 4 / devkit 06060010 but every call to the old module-list API returned 8002013A (library not yet linked). v0.2 replaces that unresolved API and the separate count call with sceKernelGetModuleIdListForKernel, whose import NID 05D915DB is mapped by ARK-5 to the 6.60/6.61 export 37E6F41B. It also stops after a module-list failure instead of repeating the same error 45 times. The corrected PRX still needs an on-device test; no DS4 support is claimed.

## v0.3 fix

The v0.2 hardware log successfully captured the core Bluetooth exports, but only the first 64 IDs were populated: the list API expects buffer bytes, not an entry count, despite the current SDK header wording. v0.3 passes sizeof(ids), matching the byte-count contract also shown in uofw module.c, and skips empty/non-positive IDs. This fixes the cause of the repeated uid=00000000 / 8002012E errors. It does not add Bluetooth controller support. The existing v0.2 log is sufficient for the initial core-module analysis; another scan is not required just to reconfirm those exports.
