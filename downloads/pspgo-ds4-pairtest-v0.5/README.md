# PSP Go / DualShock 4 — expanded native registration trace v0.5

Target: PSP Go / 6.61 / ARK-5 cIPL. **This is an experimental diagnostic build, not working DS4 support.** It does not bypass pairing errors or force unsupported profiles to succeed.

## What the last result established

The v0.4 hardware log confirmed that its registration hook and discovery-filter patch installed. It contained no `REGISTER begin` entry. That only established that the single traced API was not called; it did not establish that Sony's Bluetooth stack was never invoked.

Offline analysis of Sony's official PSP Go 6.61 Bluetooth menu identified another registration path, plus a subsequent profile query. The menu can show its unsupported-device message for registration errors or for an unsupported profile after an operation returns. v0.5 traces both registration APIs and changed profile-query results to distinguish those cases.

## Install and run one test

1. Replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with the v0.5 file. Keep only one enabled entry for it in your existing `ef0:/SEPLUGINS/PLUGINS.TXT`:

   `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`

2. Keep the old `pspgo_ds4_diag.prx` disabled and remove any remaining capture flag. Do not run btfree alongside this experiment because their discovery patches overlap.
3. Power the Go fully off and on again. A VSH-only refresh is not a substitute for clearing the previously resident hooks. Stay in XMB for this test.
4. With USB disconnected from the DS4 and its light off, hold **SHARE + PS** until it flashes. Scan in Bluetooth Device Settings > Register New Device and select `Wireless Controller`. Try registration once. If a PIN prompt appears, record what it asks rather than repeatedly guessing.
5. Send `ef0:/seplugins/pspgo_ds4_pair.log` and the exact on-screen result/error. The file appends: its newest section should start with `native pairing test 0.5`. No capture flag, driver files, or 90-second wait is needed.
6. Disable the entry and power the Go fully off/on after the test. The plugin refuses live unload while its hooks remain referenced; a full restart clears the RAM patches.

The current error may persist: this build's purpose is to expose its source, not conceal it or present failure as success.

## What the PRX changes and records

The discovery filter is the same guarded RAM patch used in v0.4. No new discovery/profile/authentication bypass is added.

Three native export entries are traced, each found by NID and checked inside sceBSMan's code range:

- PIN-style registration (`8AC2F7B8`): forwards its two opaque arguments unchanged.
- Address/no-PIN-style registration (`2D649923`): forwards its one opaque argument unchanged.
- Profile query (`CBD232D1`): forwards both arguments and preserves the result. On successful queries, it checks the returned buffer's RAM range, alignment and 80-byte structure size before reading only its numeric ID and profile flags. No additional Bluetooth queries are issued.

The instruction pairs displaced by each trampoline are verified. The profile prologue guard checks the relocated global address against the verified relative module offset, rather than assuming a fixed load address. Unknown instructions are not patched. The existing module-start handler is chained, and caches are flushed after changes.

Logs contain `PIN_REGISTER`, `NO_PIN_REGISTER`, and `PROFILE` markers. Profile records are deduplicated, capped at 64 changed records per boot, and refreshed after a registration attempt. PINs, keys, Bluetooth addresses and device names are not logged. Calls, arguments, return values and output records are not rewritten.

This is not a DS4 report parser, controller injector, firmware update or cIPL modification. The PRX writes only its internal-storage log; the stock menu may save/remove normal pairing records as part of your chosen operation. The discovery patch exposes classes generally; select only your DS4 for this experiment.

## Verification and build

Compiled with GCC 15.2.0 and `-Wall -Wextra -Werror` using official pspdev/pspdev image digest `sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`. Run `make` with PSP SDK and SystemControl kernel stubs available.

Host tests with AddressSanitizer/UBSan check PIN/no-PIN/profile prologue guards, relocated profile addresses, displaced instructions, return jumps, delay slots, alignment/region rejection and discovery instruction guards:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/test_patch_plan.c -o test_patch_plan
ASAN_OPTIONS=detect_leaks=0 ./test_patch_plan
```

LeakSanitizer cannot run under this environment's ptrace setup; these tests allocate no dynamic memory. Offline tests do not verify live kernel hooks or wireless operation. v0.5 needs hardware testing.

Research provenance and limitations are in `docs/RESEARCH.md`. No Sony firmware files, captured driver code or user logs are included. Discovery-filter technique credited to JJS / Yoti's btfree and its ARK continuation; new tracing code is MIT licensed.
