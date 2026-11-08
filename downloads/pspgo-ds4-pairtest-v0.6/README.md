# PSP Go / DualShock 4 — generic HID menu fix v0.6

Target: PSP Go / 6.61 / ARK-5 cIPL. **Experimental hardware build. DS4 connection and usable controls are not confirmed yet.**

The v0.5 hardware trace returned a positive registration slot and a valid profile record reporting HID capability. The unsupported popup occurred after that result. In the official 6.61 menu, the initial-registration action builder omits generic HID type 2, while the existing-device builder already gives types 1 and 2 the same native HID connection helper.

v0.6 adds that missing initial action for type 2 **only when the native record has its HID capability bit set**. It keeps the type-1 path and rejects other types at that decision point. Authentication, native registration/profile results, device IDs and pairing records are not forged. The stock native HID parser is still used; no custom DS4 report decoder is installed.

## Install and test

1. Replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with the v0.6 file. Keep one enabled entry in the existing `ef0:/SEPLUGINS/PLUGINS.TXT`:

   `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`

2. Keep the old diagnostic disabled and remove any capture flag. Do not enable btfree alongside this plugin because the discovery patches overlap.
3. Power the Go fully off and on again to clear the earlier resident hooks. Stay in XMB for the first test.
4. Open Bluetooth Device Settings and check Manage Bluetooth Devices. **If `Wireless Controller` is already listed, turn the DS4 on with PS and try Connect there.** The previous native registration may have saved it despite the popup. There is no need to delete other pairings.
5. If it is not listed, disconnect USB from the DS4, hold SHARE + PS with its light off until it flashes, and use Register New Device. Select `Wireless Controller`, then Connect if the menu offers it. If a PIN prompt appears, record what it asks rather than guessing repeatedly.
6. If connected, try the D-pad, left stick and face buttons in XMB. Send `ef0:/seplugins/pspgo_ds4_pair.log` and report whether registration/connection succeeded and which inputs responded. The newest log section should identify version 0.6. No capture files, flag or 90-second wait are needed.

Disable the entry and fully power off/on after testing. The plugin deliberately refuses live unload while code remains referenced. Full restart clears RAM changes and its small temporary user-memory allocations. The stock Bluetooth menu can save or remove normal pairing records as part of your chosen actions; the PRX does not write flash or change cIPL.

## Guards and limits

The menu correction uses a 56-byte, branch-only user-mode stub. It checks type 1 first, then admits type 2 only with the real HID flag. It preserves the original allocator argument and saved registers. One branch in the menu is redirected only after validating the branch/delay pair, device-type/profile-record context, allocator call target and relocated native helper table address. Unknown code is left untouched. At most four distinct menu load addresses receive private stubs; allocation failure or a limit logs a skip.

The existing guarded discovery patch remains. Native PIN registration, no-PIN registration, profile queries and HID connection requests are traced. All native arguments, return values and query records are preserved, and no extra Bluetooth commands are issued. Logs contain numeric statuses and changed numeric profile metadata, not PINs, keys, device names or Bluetooth addresses.

Expected markers include `HID_MENU initial connection action enabled`, `HID_CONNECT hook installed`, and, when the stock menu requests connection, `HID_CONNECT begin/result`. A zero connection-request return is not by itself proof of working wireless controls; report what the controller actually does.

## Validation

- Strict PSP build: GCC 15.2.0, `-Wall -Wextra -Werror`, official pspdev/pspdev image digest `sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
- 120 host-interpreted MIPS route cases check types, HID flags, branch targets, delay slots and preserved registers. Profile gating rejects non-HID type-2 records and other types.
- Existing trampoline/relocation tests plus the new HID-connect prologue guard pass under AddressSanitizer/UBSan.
- Offline source/ISA checks do not verify live patching, connection, native DS4 report parsing, suspend/resume, games or POPS. v0.6 has not yet run on the Go here.

Build with `make` using PSP SDK and SystemControl kernel stubs. Host tests:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/test_hid_menu.c -o test_hid_menu
ASAN_OPTIONS=detect_leaks=0 ./test_hid_menu
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/test_patch_plan.c -o test_patch_plan
ASAN_OPTIONS=detect_leaks=0 ./test_patch_plan
```

The environment's ptrace setup prevents LeakSanitizer operation; tests use no dynamic allocation. The MIPS test interpreter is a narrow offline check, not PSP hardware emulation.

The ZIP contains plugin/source/instructions only. Private logs, firmware snapshots and Sony binaries/decompiled code are not redistributed. See `docs/RESEARCH.md`. Discovery technique credited to JJS/Yoti's btfree and its ARK continuation; new code is MIT licensed, with PSP SDK notices included.
