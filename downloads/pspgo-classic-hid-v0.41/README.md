# PSP Go Classic HID v0.41 — NES30 captured-descriptor workaround

Experimental NES30 Pro fix candidate. PSP controls and lifecycle success remain unconfirmed. Generic v0.40 behavior is retained, with a deliberately NES30-specific descriptor override and button map. This is not an independent Bluetooth stack or a universal SDP fix.

## Why this change

The NES30 Linux capture advertises Bluetooth HID and supplies an86-byte gamepad descriptor with reportID3 and9-byte payload. The production parser and native-adapter mock accept it. PSP v0.40 now uses native generic0402 mode and connects, but service-data receives eventually time out before custom descriptor delivery. Receive count7 is not proof of attribute7: queries can span multiple fragments. The incomplete attribute is still unknown.

The native HID callback retrieves properties0201/0203 and PnP identity before requesting HIDDescriptorList0206. This build intercepts only that0206 caller. If the native slot identity cache is exactly2dc8:3820, callback mode is0, service is1124, attribute0206, slot1..8 and destination capacity is at least97bytes, it supplies the user's captured86-byte descriptor in a standard encoded SDP AttributeID/HIDDescriptorList value. That lets the original native HID parser continue without issuing the descriptor query that might stall.

The substitution occurs BEFORE sending0206, rather than after a timeout, to avoid intentionally leaving an outstanding descriptor transaction. Authentication, real connection establishment, other HID properties and native identity discovery remain live. If an earlier property stalls or identity is absent/different, the override is not reached/applied. All unmatched calls forward their original seven arguments to the native function. No native connection success, input reports or link keys are fabricated.

The cached descriptor is from this user's Linux Bluetooth capture, not a guessed internet descriptor. It may not describe another firmware/mode sharing the same VID/PID. Use the same mode as the capture. Eight instruction/call/cache-layout guards plus module/branch/data bounds gate the patch. One RAM call-site changes; no global receive entry observer or firmware flashing.

## Input mapping

Original NES30 Pro2dc8:3820 uses a distinct button order. The map retains physical face positions and assigns button3 Guide,7/8 shoulders,11 Select and12 Start. It follows the original device's SDL Bluetooth mapping; Guide activation and physical labels still require PSP hardware testing. Extra buttons remain neutral. Generic and SteelSeries Free mappings stay separate.

Reference: https://github.com/libsdl-org/SDL/blob/main/src/joystick/SDL_gamepad_db.h (original NES30 Pro Bluetooth2dc8:3820; not N30 Pro2 or USB).

## Diagnostic coverage

A second guarded call-site observer records native request78's service and attribute, ACK result and callback mode. The existing SDP receive shim records only validated event0272 header metadata: status, continuation/more and fragment byte count. No radio payloads, descriptors, PINs, link keys or addresses are logged by these observers. A32-entry RAM queue is drained by the existing logger; dropped entries are counted. There is no native-callback file I/O.

SDP_QUERY phase1 is request begin (the result field carries callback mode); phase2 is its native ACK result. SDP_FRAGMENT supplies status/more/bytes. Native service queries hold their native serialization lock; these records are chronological observations, not an added retry mechanism. NES_DESCRIPTOR source=Linux_capture confirms the override was actually used, encoded97bytes/raw86bytes. ZEE INPUT INIT vendor2dc8/product3820 and subsequent report counters establish later input progress.

The generic5-second per-fragment SDP timeout, generic ACK fixes, native-HID-connected diagnostic arming and mixed0622 routing from v0.40 are retained. The withdrawn v0.37 receiver-entry observer is absent. No blanket retries, extra radio mode changes or force-unlocks are added.

## Test

Replace ef0:/seplugins/pspgo_hci_transport.prx, retain VSH/GAME entries and NES30 registration, fully restart, keep other controllers and Linux Bluetooth disconnected. Connect NES30 in the same Android mode used for the Linux capture. Wait5seconds and test D-pad/buttons/sticks in VSH. Send pspgo_hci_transport.log and whether controls work. No additional descriptor file or re-pairing is needed if the saved registration already connects. Check controls before repeating game/USB transitions; previous USB/exit failures are not claimed solved here.

## Verification and limits

Pinned PSP SDK build and20 ASan/UBSan programs pass. Tests exercise the actual descriptor, nested97-byte SDP encoding, exact identity/slot/service/attribute/mode, destination bounds, all65536 vendor/product/attribute alternatives, original NES30 Select/Start/Guide bits, report-ID routing, synthetic full/stripped reports and8 override guards. Fragment metadata rejects malformed headers and excess payload lengths. No actual live reports were captured on Linux; these report tests are synthetic.

Compiled checks: descriptor wrapper56bytes; service-command wrapper32bytes; metadata recorder24bytes. Default seven-argument EABI saves/restores include t0/t1/t2. The original native command executes once per return path, including tail forwarding. Descriptor override intentionally skips only the selected native0206 query; unmatched requests tail-forward. Existing report/ACK/SDP/teardown stack checks pass. These checks do not prove all indirect native/callback stack use or PSP hardware success.

Own MIT source/tests, captured static descriptor metadata and SDK notices included. No Sony firmware/disassembly, private raw device logs, Bluetooth addresses or keys distributed.
