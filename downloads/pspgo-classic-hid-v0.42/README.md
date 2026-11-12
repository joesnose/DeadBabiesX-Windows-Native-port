# PSP Go Classic HID v0.42

NES30 Pro button correction and game reconnect candidate for PSP Go with ARK, based on v0.41. The user confirmed NES30 controls in VSH, with face buttons rotated anticlockwise, and reported games loaded without controller input. Hardware game success with this build still needs testing.

## Install and test

1. Replace ef0:/seplugins/pspgo_hci_transport.prx with this PRX. Keep the existing VSH and GAME plugin entries and saved Bluetooth registration. Fully shut down and restart the PSP.
2. Connect NES30 once from the PSP Bluetooth menu in VSH, then wait five seconds. This automatically remembers the selected registration; no extra file or switch is needed.
3. Test the corrected face buttons, then launch a PSP game with the controller still on. Allow up to eight seconds after loading for reconnect.
4. Test controls and exit back to VSH. If unsuccessful, send pspgo_hci_transport.log and describe which stage failed.

## Changes

NES30-only face outputs rotate clockwise one position to correct the reported anticlockwise layout: previous Cross becomes Square, Square becomes Triangle, Triangle becomes Circle, Circle becomes Cross. Guide, shoulders, Select and Start retain their mappings. Other controllers retain their mappings.

Previously automatic reconnect required exactly one eligible saved HID registration. Multiple registrations could prevent selecting NES30 in game mode. This build remembers the last successfully connected HID registration and validates it against fresh native records before reconnecting. This addresses a reconnect selection problem; it does not establish that this was the only cause of missing game input.

The worker creates ef0:/seplugins/pspgo_hid_target.dat: a checked 24-byte slot/fingerprint record. It contains no raw controller name, address or link key. Fingerprinting ignores mutable state flags and unused name padding. This is registration matching, not authentication; native Bluetooth still authenticates. A stale valid target will not silently select a different controller. To reset it, delete that file and connect the intended controller again.

File access runs only in the logger worker, never in native callbacks. Record readiness checks are bounded to rounds at approximately 2, 4, 6 and 8 seconds, with at most one native connection attempt per context. Without a valid preference, the prior single-candidate policy remains. TARGET and AUTO_RECONNECT logs report selection without raw identity data.

The v0.41 captured-descriptor override for NES30 identity 2dc8:3820 remains, along with generic ACK/SDP timeout handling and native 0422/0622 connection routing. The driver retains generic HID support, with an explicitly NES30-specific descriptor and button map. Native authentication and other device queries remain live. The withdrawn v0.37 global receiver observer is absent. No firmware flashing, mode writes or force-unlock is introduced. GameStick exit hangs and NES30 USB lifecycle issues are not claimed fixed.

## Validation and limits

Pinned PSP SDK cross-build succeeded. All 21 host test programs passed with ASan/UBSan, including preferred-device selection among multiple registrations, stale and corrupt target rejection, mutable flags/padding, invalid addresses, and NES30 face mapping. All six compiled ABI/stack/lifecycle/status/SDP/NES30 checks passed. Logger frame is 464 bytes on its 8192-byte worker stack; report-path known stack budget is 392/1024 bytes. These checks do not prove full native indirect stack bounds or hardware behavior.

Source, tests and static HID descriptor are included. No Sony firmware/disassembly, private raw captures, addresses or keys are included.
