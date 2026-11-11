# PSP Go Classic HID v0.38 — GENERIC experimental controller build

User-requested generic test for NES30 Pro and other controllers. Based on v0.36, not withdrawn v0.37. No event-receive entry observer and no500ms thread profiler from v0.37. Known GameStick exit hang and earlier NES30 USB freeze are not claimed fixed; hardware behavior on v0.38 is untested.

## Generic fixes

The native2-second ACK timeout no longer requires GameStick VID/PID, a name match, an owned report node or a completed descriptor. It can apply during Bluetooth startup/pairing and to NES30. Native worker response0185 uses callback mode0; native main-thread ACK0100..01ff uses modes0/1. Other threads, replies, unknown callback modes and existing timeout pointers retain original behavior. Native result/error handling, waiter cleanup and lock unwind remain; no forced success, mutex cancellation or firmware flashing. Because identity gating is removed, these shared native-thread requests are no longer isolated to gamepad devices.

Saved-device reconnect no longer requires the Zeemote name. It queries all8 fresh native records and chooses exactly one valid HID-flagged registration with a bounded nonempty UTF16 name. Multiple candidates cause no automatic attempt; select the desired device using the native Bluetooth menu. No registrations are deleted. One reconnect attempt per VSH/GAME context remains.

Profile selection now accepts the additional0020 flag previously observed on NES30 records:0402 and0422 select native generic mode1; other modes and connected-state0200 behavior retain their original policy. Reconnect accepts0202/0402/0602 and the corresponding0222/0422/0622 combinations. Arbitrary unknown flags are not blindly treated as gamepads.

Descriptor-derived input routing, numbered-report repair, control merging in VSH/GAME/HOME and lifecycle invalidation remain generic. Protocol-specific Free button mapping, native DS3 handling and the legacy exact-Free PIN-registration exception remain distinct: applying a device's packet format or pairing protocol to every controller would produce incorrect behavior. Stock pairing choice remains for other names; no generic device is forced to expose a fake DS3 identity.

## Limits

This targets Bluetooth Classic HID. BLE, SPP-only/vendor-specific protocols, unsupported HID descriptors and missing descriptor delivery are not automatically supported. NES30 previously displayed SPP and delivered no usable HID inputs; wider timeout/reconnect eligibility does not prove it will now deliver a descriptor or controls. There is no native radio/USB shutdown wait patch added here. No universal compatibility or USB/game-exit success is claimed.

## NES30 test

Replace ef0:/seplugins/pspgo_hci_transport.prx, retain VSH/GAME entries and current registration, and fully restart. Keep other controllers powered off for this test. Use the same NES30 Android/Windows mode as before and native Connect. If several saved HID registrations exist, automatic reconnect intentionally skips; manual Connect is sufficient. No re-pairing unless the PSP has no saved registration.

Check whether buttons/D-pad work and whether Information still says SPP. If controls work, try game entry and HOME/quit. If testing USB shutdown, note whether a freeze occurs while connected versus after disconnecting. Return pspgo_hci_transport.log plus the outcome. Prior NES30 USB freeze and GameStick exit hang may persist; v0.34 remains the user-confirmed GameStick entry fallback, not an NES30 fix.

STATUS_WAIT GENERIC confirms install. STATUS_CALL target now means request/thread/mode eligibility, not device authorization. Generic timeout logs may occur before any input node exists. Existing pairing/profile/descriptor/CTRL/lifecycle numeric diagnostics remain, at v0.36 rates. Logs stay on PSP; no private raw logs are published.

## Validation

Pinned PSP build and17 ASan/UBSan programs pass. Profile selection and generic reconnect cover all65536 flag words; reconnect also explicitly tests an NES30 Pro0422 registration, any bounded name, invalid records, ambiguity, active/error rejection and exact once-only native connection result. ACK policy tests cover all65536 replies and32 eligibility/mode/role combinations. Compiled checks verify one original call per terminating wrapper path,40-byte ACK wrapper, small IRQ-only metadata helpers and unchanged report callback budget. The v0.37 event observer is absent from ELF. These checks are not hardware proof or a complete indirect stack audit.

Own MIT source/tests and SDK notices included. No Sony firmware/disassembly, private raw device logs, Bluetooth addresses or keys distributed.
