# PSP Go Classic HID v0.44 — DS4 basic-input test

Separate test build based on v0.43, which the user confirmed working for NES30 in VSH, in games and on return to VSH. v0.44 adds a DualShock 4 basic report path for native Sony identities 054c:05c4 and 054c:09cc. DS4 pairing, input and lifecycle behavior with this build are not yet hardware confirmed. v0.43 remains the working fallback.

## Install and first test

1. Replace ef0:/seplugins/pspgo_hci_transport.prx. Keep existing VSH/GAME entries and saved Bluetooth registrations. Fully shut down and restart the PSP. Leave NES30 and other controllers off for the DS4 test.
2. Enable Bluetooth. If DS4 is already registered, try its saved entry with USB disconnected and press PS. If a new registration is needed, hold SHARE + PS until the controller flashes rapidly and use the PSP's Bluetooth registration menu. This build does not require rewriting the controller's USB paired-host address again.
3. If connection succeeds, test D-pad, face buttons, left stick, L/R, SHARE, OPTIONS and PS in VSH. Wait five seconds after connecting so the existing remembered-controller mechanism can save the selected registration.
4. Only if VSH controls work, launch a game, allow up to eight seconds for reconnect, check controls, then use HOME/Exit Game. Allow up to 30 seconds for native shutdown retries.
5. Send ef0:/seplugins/pspgo_hci_transport.log with the result. If registration or connection fails, record the displayed error/PIN prompt and stop at that stage; the log is still useful. If the PSP hangs or shuts down, send the log after restarting rather than repeating the same failure.

The remembered-target file automatically changes to the controller last connected successfully. Connecting NES30 again selects it for later game reconnects. No manual mode switch or additional descriptor file is needed.

## What this build changes

DS4 basic Bluetooth report01 is ten bytes including its report ID. It fits the existing native64-byte report reader. This test uses an original68-byte host-side descriptor projection describing only that documented report. At the existing guarded HIDDescriptorList0206 caller, native cached PnP identity must be Sony054c:05c4 or054c:09cc, slot1..8, service1124, attribute0206, callback mode0 and sufficient destination capacity. It supplies the79-byte encoded attribute before the native0206 request is sent. Other requests retain their original seven arguments. The NES30 captured-descriptor override remains unchanged.

This DS4 descriptor is a local projection of a known packet format, not a captured full DS4 descriptor, not a renamed controller and not a DS3 impersonation. Native discovery, registration, authentication, link/service establishment, PnP lookup and other HID properties still run. A successful registration alone does not establish an authenticated usable connection. If the earlier802F0140 failure persists before descriptor/input initialization, this input addition cannot by itself resolve that authentication or service-establishment failure.

At initialization, the dedicated decoder requires both a supported native DS4 identity and an exact match to our projected descriptor, plus the existing validated type3 node/zero-resource ownership guards. It sets that node's report route to01 and consumes exact full10-byte or native-ID-stripped9-byte packets. It never publishes controls merely because initialization or connection succeeded. Invalid report IDs, sizes or hat values clear input; generation checks prevent a report racing disconnect from republishing controls.

The mapping follows DS4 face labels directly: Square/Cross/Circle/Triangle map to their matching PSP buttons; L1/L2 map to PSP L and R1/R2 to PSP R; SHARE maps Select, OPTIONS maps Start, PS maps HOME. Left stick drives the PSP analog stick. Right-stick values are decoded but have no PSP control assignment in this version. Stick clicks and touchpad actions have no assignment. Analog trigger pressure, rumble, LEDs, motion and touch coordinates are outside this test.

Full DS4 Bluetooth report11 is78 bytes, and DualSense's full report is also larger than the current reader. They are not enabled or supported by this build. No DS4 feature/output report is sent to request a switch into full-report mode. A controller already delivering only full reports may therefore produce no usable input. The native64-byte reader and generic parser limits are retained.

## Preserved baseline

NES30 mapping, its captured descriptor, generic/Free paths, native DS3 exclusion, controller merging in VSH/GAME/HOME, remembered-target reconnect and v0.43 completion waits remain. The new timeout handling from v0.43 applies to native status-result0275 and HID pending disconnect completion, with original callback mode, error return and resource ownership. No forced unlock, killed thread, radio-firmware write or flash modification is introduced. The withdrawn global event-receiver observer remains absent.

## Diagnostic markers

HID_DESCRIPTOR installed confirms the guarded descriptor caller supports NES30 plus both DS4 IDs. DS4_DESCRIPTOR source=basic_projection confirms that the actual native identity reached the DS4 descriptor path. DS4 BASIC initialized confirms a type3 node was accepted for the basic format; ZEE INPUT FIRST REPORT and report counters distinguish initialization from actual input delivery. Existing PROFILE_SELECT, HID_CONNECT, SLC_EVENT, SDP_QUERY, STATUS_CALL, COMPLETION_CALL and LIFE markers remain. Metadata and input-state logging run on the existing owned worker; native report callbacks do no file I/O. Private raw pairing payloads, addresses and link keys are not logged by the new code.

## Validation

Pinned PSP SDK full cross-build passed with strict warnings. All23 host test programs passed with ASan/UBSan. New tests cover the basic report's face/shoulder/Share/Options/PS/hat combinations, counter neutrality, analog bounds, incorrect IDs/sizes, neutral failure, both model identities, descriptor encoding/capacity, and production generic descriptor parsing. Callback tests exercise full/stripped frames for both DS4 IDs, no synthetic init input, wrong-projection refusal, native DS3 exclusion, malformed input, termination and a disconnect/report race. Generic, NES30 and lifecycle regression tests remain passing. Test reports are synthetic, not captured DS4 packets.

All eight compiled checks passed. Report callback frame56 bytes; DS4 decoder frame0 with no calls. Known DS4 report-path budget with native/helper allowance328/1024 bytes; existing generic path392/1024. Descriptor wrapper frame56 and seven-argument forwarding checks remain passing. Completion wrappers40, IRQ-only metadata helpers16, stackless callback-mode bridge and logger frame480/8192 remain. These checks do not prove every indirect native stack bound or hardware success.

Source/tests and static descriptors are included. No Sony firmware/disassembly, private raw captures, addresses or link keys are distributed.

## Protocol references

- Linux hid-playstation documents DS4 minimal Bluetooth report01/10bytes, full report11/78bytes and button/axis parsing: https://github.com/torvalds/linux/blob/master/drivers/hid/hid-playstation.c
- Bluepad32 supports the same minimal report and documents feature/output initialization that enables full report11 on controllers: https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/parser/uni_hid_parser_ds4.c

These are wire-format references. This package uses original adapter/decoder code and does not include those drivers.
