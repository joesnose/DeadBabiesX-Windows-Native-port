# PSP Go / DualShock 4 — native pairing experiment v0.4

Target: PSP Go, Sony firmware 6.61, ARK-5 cIPL. **Experimental, not hardware-tested and not a finished DS4 controller driver.**

The previous v0.3 captures verified the native Bluetooth/HID/input modules and their export tables. This build makes a small, guarded change to the native pairing screen so it can list device classes normally hidden by Sony, and traces the native device-registration function's return code. Pairing and input processing remain Sony's native implementation; the separate DS4 report decoder is not installed in this build.

## Install and test

1. Disable the previous `pspgo_ds4_diag.prx` entry and remove `pspgo_ds4_capture.flag` if it remains. Do not enable btfree alongside this test; their discovery patches overlap.
2. Copy `pspgo_ds4_pairtest.prx` to `ef0:/seplugins/`.
3. Add this line to your existing `ef0:/SEPLUGINS/PLUGINS.TXT`, keeping your other entries:

   `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`

4. Power the Go fully off and start it again. Stay in the XMB for this test. There is no 90-second timer and no capture flag is needed.
5. Disconnect USB from the DS4. With its light off, hold **SHARE + PS** until the light flashes. In the Go's Bluetooth Device Settings, choose Register New Device and scan. If `Wireless Controller` appears, select it and try registration once. If a PIN prompt appears, record what it asks; do not repeatedly guess PINs.
6. Send `ef0:/seplugins/pspgo_ds4_pair.log` and tell me whether the controller appeared, the on-screen error (if any), and whether it connected. If connected, try the D-pad and face buttons in the XMB and report their behaviour. The controller revision (CUH-ZCT1 or CUH-ZCT2 on its label), if readable, is useful.

After the test, disable the new plugin entry and power the Go fully off and on again. This clears its RAM patches. The module deliberately refuses live unload while its hooks are resident, to avoid freeing code still referenced by the native service. No firmware/cIPL installation or flash writes are performed. The log is the only file the PRX creates; the native Bluetooth menu may save its normal pairing records when you choose registration.

## What the experiment does

- Checks PSP Go model and 6.60/6.61 devkit identifiers.
- Finds the native registration export through ARK and validates it lies inside sceBSMan's code region.
- Installs a trace only if the two displaced registration instructions exactly match the verified stack/pointer-setup prologue. Arguments are forwarded unchanged; no device records, PINs or link keys are read or logged.
- For the Bluetooth VSH module, validates both discovery-filter instructions before changing them in RAM. Unknown instructions are left untouched and logged as a mismatch.
- Retains and calls the existing module-start handler.
- Flushes instruction/data caches after code changes.

The filter change applies to discovery classes generally. For this test, select only your DS4. It does not add authentication, HID channel setup, a DS4 report parser or controller injection. A device becoming visible is not proof it can pair or control the Go.

Expected log markers are `REGISTER hook installed`, `DISCOVERY filter lifted`, and, if native registration is invoked, `REGISTER begin` / `REGISTER result=...`. If a patch is skipped, send the log without trying other firmware addresses. Never manually edit the offsets to force a patch.

## Build and checks

Official pspdev/pspdev image digest: `sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`; GCC 15.2.0. Run `make` with the PSP SDK and SystemControl kernel library installed.

The PRX builds with `-Wall -Wextra -Werror`. Host tests check the displaced MIPS instruction semantics, jump destinations, delay slots, address-region/alignment rejection and both discovery instruction guards. Run:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/test_patch_plan.c -o test_patch_plan
ASAN_OPTIONS=detect_leaks=0 ./test_patch_plan
```

The environment cannot run LeakSanitizer under its ptrace setup; the tests use no dynamic allocation. These checks do not test real pairing, live kernel patching or the controller. This release therefore remains a hardware experiment.

Research references: original discovery-filter technique by JJS / Yoti in [btfree](https://github.com/PSP-Archive/btfree), and its [ARK continuation](https://github.com/Hirador/btfree-ark). The filter instruction offsets and old/new opcodes come from that existing technique. The registration prologue, two-argument forwarding ABI and generic-input path were checked locally against the user's privately supplied native driver code. Captures, logs and decompiled Sony code are not redistributed. New plugin code is MIT licensed.
