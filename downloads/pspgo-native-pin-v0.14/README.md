# PSP Go native PIN registration v0.14 — class byte-order correction

The v0.12 hardware log proves generic HID mode1/0402 was selected, but both
connections still failed before HID initialization. It also proves registration
used the address-only route. This version targets registration through Sony's
stock PIN path. Working DS4 pairing and input remain unverified on hardware.

## v0.14 correction

The v0.13 log showed the patch installed but registration still used NO_PIN.
Its matcher read the wrong class byte. Sony stores class high/middle/low, so a
gamepad class002508 appears as00 25 08. This build reads the minor class from
byte2, keeps the existing major/name/record guards, and tests the native record
conversion. Reversed-order test fixtures are explicitly rejected. Hardware PIN
registration remains unverified.

## Behavior

For the selected discovery entry matching the DS4-like gamepad class and exact
UTF16 name Wireless Controller, a guarded menu change selects Sony's general
PIN-registration object instead of the address-only object. It preserves the
real84-byte discovery record and its identifier. The stock constructor supplies
UTF16 PIN0000; the complete32-byte native PIN argument is defined. Other entries
retain their stock selection behavior. Class and name are a selected-device
heuristic, not proof of genuine Sony identity.

The v0.12 exact-profile0402 connection-mode correction remains. There are no
BCCMD probes, PS configuration writes, radio resets, HCI mode changes, key
imports or DS3 identity spoofing. Normal pairing operations create/remove saved
Bluetooth registrations. No automatic repeated connection or fallback is added.

## Install and pair afresh

1. Replace `ef0:/seplugins/pspgo_hci_transport.prx`, retaining its single enabled
   VSH entry. Disable old pairtest/diagnostic/btfree entries that patch the same
   functions. Keep RAM-test marker/state files.
2. Fully power off/on, then enable Bluetooth. Delete only the existing DS4 entry
   from Manage Bluetooth Devices, so registration can use the corrected path.
3. Disconnect DS4 USB. Hold SHARE+PS until it flashes rapidly, then register it
   through the PSP's ordinary Bluetooth discovery menu. Use0000 if prompted.
4. Select Connect and test buttons. The log remains
   `ef0:/seplugins/pspgo_hci_transport.log`. A new section identifies0.14;
   PIN_REGISTER indicates the general path was invoked. If it fails, that log
   preserves the real registration and connection outcomes.

No Linux/Windows address-writing helper or DS3 is required for this attempt.
If the class/name or firmware code guards do not match, the new route is not
forced. Hardware success requires actual connection and controls, not just a
registration slot or a successfully installed hook.

## Evidence and limits

Sony's UI already has distinct address-only and general PIN constructors. Its
gamepad class-selection branch can select address-only registration. The latter
matches the supplied log's NO_PIN_REGISTER result. The general constructor uses
the full discovery record and defaults to0000. A primary USB Host Shield DS4
trace shows legacy0000 pairing for an original DS4 revision; newer revisions
and clones may differ. The previous error does not expose a standard HCI reason,
so authentication remains a hypothesis rather than a confirmed diagnosis.

`docs/V012-HARDWARE-RESULT.md` records the new hardware evidence. Source, original
analysis and repeatable tests are included. Software builds/tests cannot prove
PSP authentication or gameplay input. Historical docs are superseded by this
README for current behavior.

Original adapter MIT; SDK notice and discovery credits included. No Sony
firmware, disassembly, private user logs or controller keys redistributed.

Validation: the PSP cross-build and seven ASan/UBSan test executables passed.
The new test interprets the emitted MIPS predicate across65,536 class values,
checks exact name/NUL, record size/identifier, untouched memory/registers and
firmware context guards. Restoring the original predicate fails the new route
regression. The PIN clear length is guarded against the verified120-byte
constructor allocation; it changes16 to32 within the object, preserving the
stock four-character default. No hardware success is inferred from these checks.
