# PSP Go Classic HID v0.48 — keep Wireless Controller on generic HID after restart

The v0.46 log confirmed DS4 registration and a successful native generic connection using mode1 while saved flags were0402. The new v0.47 log shows the same saved slot4 after restart with flags0602. Automatic reconnect chose mode0, emitted requestflags0202, and returned802f0140. A later incoming0202 connection did not produce input; PnP/parser service-data waits timed out before the country-code workaround could run. Thus v0.47 did not establish whether the country-code change works on the correct generic route.

v0.48 corrects that demonstrated reconnect selection. DS4 input success with this build still needs hardware confirmation.

## Install and test

1. Replace your enabled PRX with SEPLUGINS/pspgo_hid_v046.prx from this ZIP. The installed filename intentionally stays v046 to preserve the entries just confirmed working; the internal version/banner is0.48.
2. Keep the successful DS4 registration, plugin entries and saved-target file. Fully shut down and restart the PSP. With USB disconnected, connect the saved DS4 entry using PS and the Bluetooth menu. Leave other controllers off for this test.
3. Test VSH buttons/left analog first. If input works, wait five seconds and test game entry, controls and HOME/Exit Game.
4. Send pspgo_hid_v048.log from seplugins or storage root on ef0/ms0. Root boot marker is pspgo_hid_v048.boot. No re-pairing, target-file deletion or USB host-address rewrite is needed.

If your current enabled filename differs, replace that existing file instead of enabling another plugin copy. Optional PSP/GAME/HIDCHECK now reports v0.48 and checks the new log paths. v0.43 remains the user-confirmed NES30 baseline.

## Selection correction

The flag-only fallback intentionally kept legacy0602 at mode0 for working Free/DS3 behavior. That is ambiguous for the DS4 record observed after reboot. This build examines the fresh80-byte native saved record: exact size, valid slot1..8 and matching record ID are required. An exact bounded UTF16 name Wireless Controller, including its NUL terminator, selects generic mode1 when flags are0602. Existing0402/0422/0622 generic selection remains unchanged. Every other0602 name retains legacy mode0.

The saved name is a route-selection hint, not authentication or proof of genuine Sony hardware. The packet decoder still independently requires the supported native Sony PnP identity and projected basic descriptor. Names are not used to bypass authentication or label an arbitrary packet as a DS4 report.

Both automatic saved-target reconnect and manual mode0 Connect use this fresh-record policy. Automatic reconnect selects the mode before clearing each temporary native record, retaining the selected target's mode even when later slots contain other devices. Invalid/mismatched records, unknown flags, failed queries and explicit nonzero caller modes retain previous behavior. Only one native connect call is made; there is no repeated mode fallback or success fabrication. The preferred-target fingerprint format stays compatible and ignores changing state flags as before.

## Expected evidence

MODE_POLICY confirms the new rule is loaded. For the saved Wireless Controller0602 record, HID_CONNECT should show mode1/selected_mode1 and native flags0402. A requested mode1 from automatic reconnect means it already selected the route while inspecting fresh records; PROFILE_SELECT may consequently show no second query. The native result remains authoritative.

The existing HID_PNP, DS4_COUNTRY, DS4_DESCRIPTOR, DS4 BASIC and ZEE INPUT counters then identify progress through metadata, descriptor, initialization and actual reports. The log will also show if an incoming connection still takes another route or a different service query stalls. A corrected outgoing mode alone is not proof of usable input.

## Retained fixes and limits

The v0.47 supported-DS4 country0203 default remains, along with the report01/10-byte descriptor adapter, v0.43 native completion timeouts, generic/Free/NES30 mappings, HOME merging and v0.46 fallback logging. No incoming service flags, radio firmware, pairing results or link keys are rewritten. Full78-byte DS4 reports and DualSense are not added. Unknown/renamed controllers may need a different selection hint; generic compatibility is not universal.

## Validation

Pinned PSP SDK forced release build passed for plugin and checker. All26 host programs passed with ASan/UBSan. The new integration test creates the observed0402-to0602 Wireless Controller transition with a persistent preferred fingerprint and multiple saved devices, then verifies automatic/manual/unique reconnect mode1, unchanged Free/DS3 mode0, NES30 generic mode1, all65536 flag words, bounded exact-name/UTF16/size/ID checks, query failures, explicit modes and one-attempt/native-error preservation. Existing registration, country, decoder and lifecycle tests remain passing.

All ten compiled plugin checks passed. Manual connection trace frame136; logger504 on8192; country/descriptor/report wrappers remain56 bytes and DS4 decoder0. Checker PBP structure and worker160/16384 verified. Synthetic test states and compiled frame/ABI checks do not establish hardware success or full indirect native stack bounds.

Source/tests and SDK notices included. No private raw logs, Bluetooth addresses/keys or Sony firmware/disassembly included.
