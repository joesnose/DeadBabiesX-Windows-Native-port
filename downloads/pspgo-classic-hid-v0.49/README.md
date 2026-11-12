# PSP Go Classic HID v0.49 — selected DS4 incoming-link test

Experimental build for PSP Go 6.61 with ARK. Hardware input and Bluetooth-menu reconnection are not yet verified on this build.

## Install and test

1. Replace your existing `SEPLUGINS/pspgo_hid_v046.prx` with the supplied file. The installed filename intentionally stays unchanged; the binary is version 0.49. Keep your existing ARK entries, successful DS4 registration and preferred-target file. Run only one copy of the plugin.
2. Fully shut down the PSP, with the controller off. Start the PSP, enable Bluetooth and wait five seconds.
3. Disconnect the controller's USB cable and press PS alone to initiate its incoming connection. Do not hold SHARE or register it again. Automatic outgoing reconnect is deferred for the selected saved Wireless Controller in this test.
4. Test VSH buttons first. If they work, test Bluetooth-menu disconnect/reconnect. Test games and return to VSH afterward.
5. Send `pspgo_hid_v049.log` from the existing plugin log location. The root boot marker is also version 049. HIDCHECK is optional and reports version 0.49.

## Change

The v0.48 log shows an actual incoming link, followed by repeated native PnP/HID metadata timeouts before interrupt-reader startup. This build supplies local basic-report metadata only for the freshly validated, fingerprint-matched preferred saved Wireless Controller after a genuine successful link event. It does not change radio flags, pairing keys or authentication results.

A genuine supported DS4 VID/PID cache is used if present. Missing identity remains 0:0; no Sony identity is invented and the native identity cache is not overwritten. A completed known non-DS4 identity excludes this path. Decoder authorization additionally validates the native HID context, its backend and the selected linked slot.

The adapter supplies HID parser version 1.11, country 0 and the existing report-01 basic descriptor. Native input read registration and report reads are forwarded unchanged. A header-only trace records actual report lengths and IDs before composite routing; it neither generates controls nor changes packets. Full 78-byte DS4 report 0x11 and DualSense support are not added.

Look for `DS4_LOCAL installed`, `DS4_SCOPE` armed, automatic reconnect `deferred=1`, real `SLC_EVENT` state 0, `DS4_LOCAL_ID`, three `DS4_LOCAL_ATTRIBUTE` records, `DS4_READ_START`, `DS4_BASIC` initialization and `DS4_RAW` packet headers. A missing marker identifies where progress stopped; connection alone does not establish working input.

The new path requires guarded native call sites and interface pointers. A mismatch skips it. Other devices retain their existing metadata and reconnect paths, including NES30/Free mappings and native DS3 handling. This is a candidate fix for the startup blockage and reconnect race, not a claim that all menu/disconnect issues are solved.

## Validation and source

Forced PSP SDK plugin and HIDCHECK builds succeeded. All 27 C test programs passed with AddressSanitizer/UndefinedBehaviorSanitizer. Eleven compiled-code checks passed, including wrapper stack/side-effect checks. The conservative report-path stack allowance with the new observers is 456 bytes within the native 1024-byte thread; this is static accounting, not a hardware stack measurement.

Tests cover fresh target selection, real-link gating, identity isolation, encoded metadata/parser integration, all 40 instruction guards, context bounds, unchanged reconnect paths, read tracing and decoder authorization. Own source and tests are included. No Sony firmware/disassembly, private raw logs, controller addresses or pairing keys are included.
