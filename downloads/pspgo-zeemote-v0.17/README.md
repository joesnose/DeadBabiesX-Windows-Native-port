# PSP Go Zeemote SteelSeries Free input fix candidate v0.17

This build targets the SteelSeries Free in HID Gamepad mode. It prepares both
native PIN registration and descriptor-based input translation. A real radio
connection, supported descriptor and valid input report are required before
remote controls activate. The v0.16 hardware log confirms a real Zeemote connection and report traffic.
PSP button operation remains unverified until this build is tested.

## What changes

The selected discovery name must exactly match `Zeemote: SteelSeries Free`,
ignoring ASCII letter case, with a valid84-byte native record and nonzero ID.
No unverified Class-of-Device minor is assumed. The name also exists in Arcade
mode, so this build relies on your confirmed Gamepad mode; it does not establish
mode or genuine identity from the name alone.

The matched entry uses Sony's real PIN-registration object with UTF16 PIN0000
and its genuine discovery identifier. A private copy of its method table keeps
the HID menu category2 instead of inheriting the audio category8, preventing
the known menu rejection/deletion of a0402 HID profile after native registration.
Other entries retain their original routing. Exact saved profile0402 still
selects native generic HID mode1, with one connection call and the real result.

Sony's type3 generic-gamepad report path normally drops input. The new adapter
captures the actual descriptor, authorizes VID1038/PID1412 from native initializer
arguments, and decodes only a supported joystick/gamepad layout. It derives bit
positions and logical ranges from that descriptor; it does not assume a fixed
packet shape. Unsupported or malformed descriptors/reports produce no controls.
DS3 type4 handlers, identity and feature commands remain unchanged.

Accepted Zeemote input merges into four normal PSP controller-read APIs. It
preserves local controls, maps A/B/X/Y to Cross/Circle/Square/Triangle, L/R to
L/R, Start/Select and the D-pad, and maps the left stick to the PSP analog stick.
The right stick is decoded but has no separate PSP analog channel. Remote input
is released on termination or invalid reports. A live-session monitor preserves
held controls even when the controller only sends changes; its1500ms deadman
releases input if that monitor stops. Syscall hooks
are installed only after an authorized valid report. No connected-status flag
is fabricated, and no arbitrary Bluetooth device is treated as a DS3.

## Install and reconnect the existing registration

1. Replace `ef0:/seplugins/pspgo_hci_transport.prx`, retaining one enabled VSH
   entry. Disable older pairtest/diagnostic/btfree plugins that patch the same
   functions. Keep RAM-test markers/state files.
2. Fully power off/on, then enable Bluetooth. Keep the SteelSeries registration;
   the connection stage is already working.
3. After XMB loads, turn on the Zeemote in its existing Gamepad mode and Connect
   using the saved entry. No new pairing, PIN entry or PC helper is needed.
4. Select Connect and test the D-pad, buttons and left stick. If it fails or input
   does not work, copy `ef0:/seplugins/pspgo_hci_transport.log`. It records the
   actual registration/connection outcomes and bounded input-adapter diagnostics.

No extra startup cycle, marker deletion or manual test-case selection is needed.
The resident patches require a full restart to replace or disable. Normal pairing
changes saved Bluetooth records. No radio PS-key writes, HCI-mode switch, reset,
power command, key import or forced connection success is included.

## Evidence and limitations

SteelSeries' manufacturer guide explicitly prescribes0000 for this model's
Gamepad pairing. MissionControl and Chromium corroborate hardware ID1038:1412;
AOSP provides the button/axis mapping. See `docs/STEELSERIES-FREE-PRIMARY-RESEARCH.md`.
Your captured81-byte descriptor parses as reportID1 with7payloadbytes and21
fields. It has been replayed with synthetic neutral, button, hat and axis data.
The log confirms report delivery but shows the adapter never authorized input.
This build removes its dependency on Sony's failing generic input setup: only a
validated1038:1412 node is adapter-owned before native initialization. Foreign
devices retain native initialization. Owned nodes use adapter-owned report and
termination handling; no unallocated native resources are freed. This does not
force Bluetooth success; connection was already observed in the prior build.

The Information pane may remain blank because this adapter does not register a
Sony-native input device. Actual controls, not that pane, are the verification
target. No physical button input has yet been demonstrated with this version.

Source/tests/original research notes are included. Original adapters MIT; SDK
notice and discovery credits included. No Sony firmware, disassembly, private
user logs or controller keys are redistributed. Historical documents describe
older experiments; this README defines current behavior.

## Build and software checks

The strict PSP cross-build passed using pinned image
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
All11 ASan/UBSan test executables passed with warnings treated as errors. Run
`sh tests/run.sh` to repeat them. They include executable emitted MIPS routing
and constructor-wrapper tests, profile selection, synthetic HID descriptors
and reports, native callback mocks, installer guard rejection without writes,
untouched DS3 callbacks, lifecycle races and PSP syscall merge behavior.
These checks replay the actual static descriptor with synthetic reports.
Physical PSP button operation remains to be verified.
