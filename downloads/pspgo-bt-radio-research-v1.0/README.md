# PSP Go radio-identification probe 1.0

This is a **read-only research probe, not a DS4 connection fix**. It identifies the firmware behind Sony's internal Bluetooth interface using a small allowlist of CSR-style metadata queries. No DS3/DS4 controller or PC pairing app is needed for this test.

Target: PSP Go, 6.61, ARK-5 cIPL. No RAM instruction patches or hooks are installed. The probe calls the existing native serialized transaction API after checking its export addresses and instruction context. It reads only build/chip/interface identifiers and four named mode/interface settings. It does not change those settings, switch HCI mode, reset/flash the radio, attempt a connection, or read addresses/link keys/pairing records.

## Run once

1. Disable `pspgo_ds4_pairtest`, older diagnostics and btfree for this run.
2. Copy `seplugins/pspgo_bt_radio_probe.prx` to `ef0:/seplugins/`.
3. Add one line to the existing `ef0:/SEPLUGINS/PLUGINS.TXT`: `vsh, ef0:/seplugins/pspgo_bt_radio_probe.prx, on`.
4. Fully power the Go off/on. In XMB, enable Bluetooth within three minutes. Wait about ten seconds. There is no controller pairing step.
5. Send `ef0:/seplugins/pspgo_bt_radio.log`. Then disable the probe entry and fully power off/on. It deliberately refuses live unload while its worker could still be referenced.

The log starts a new section on each boot and records each query before sending it. A rejected/malformed outer exchange stops the batch. Unsupported inner BCCMD variables are logged without interpreting a value. If a stock synchronous transaction does not return, the log may end at QUERY ... begin; use a full power cycle and send that file. There is no local timeout or safe force-cancellation for the native transaction while it owns the Bluetooth request lock. Hardware behavior remains unverified here.

## Queries

BUILD_ID, CHIP_VERSION, CHIP_REVISION, INTERFACE_VERSION, LOADER_BUILD_ID, ONCHIP_HCI_CLIENT, USB_VM_CONTROL, HOST_INTERFACE and INITIAL_BOOTMODE. All are GET requests. The mode values are reported as raw metadata; they are not automatically treated as proof that generic HCI can be enabled.

## Offline checks

Strict PSP GCC15.2.0 build passes with -Wall -Wextra -Werror using the pinned pspdev image sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b. Host ASan/UBSan tests cover the allowlist, GET framing, refusal of reset/key-store reads, response bounds/echo/status and native API guards. Guards match the real loaded-code capture. The 32-bit native request/response structure sizes are checked at compile time. Hardware query support and native response behavior still need this one identification run.

See REPORT.md in the research bundle for architecture, command map, inventory scope and the remaining gaps. The bundle contains source, tests and metadata only; no firmware/decompiled code, user captures/logs or proprietary PC tools. Protocol-reference links are supplied rather than redistributing reference source. New source is MIT licensed; PSP SDK notice included.
