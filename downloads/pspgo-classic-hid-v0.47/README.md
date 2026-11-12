# PSP Go Classic HID v0.47 — DS4 HID country-query workaround

The new v0.46 log confirms that the updated plugin loads and writes logs, DS4 registration returns slot4 with HID profile0402, and Connect selects native generic mode1 and returns success. The user reports registration and connection with no input. Startup reaches PnP vendor/product and HID parser-version queries, then the HID0203 query receives an ACK but its service-data result times out802f010e. It never reaches the existing0206 descriptor adapter or HID input initialization.

v0.47 targets that observed stall. Hardware input success with this build remains unconfirmed.

## Install: keep the current entries and pairing

1. Replace your currently enabled PRX with SEPLUGINS/pspgo_hid_v046.prx from this ZIP. The filename intentionally stays v046 as a drop-in for the installation just confirmed by the log. Its internal module/version/banner is0.47; it is not the old binary. Keep only the existing enabled VSH/GAME entries and the successful DS4 registration.
2. Fully shut down and restart the PSP. Leave other controllers off. With USB disconnected, press PS and connect the saved DS4 entry. No new registration or USB address write is needed.
3. Test D-pad, face buttons, left analog, L/R, SHARE, OPTIONS and PS in VSH first. Only if input works, wait five seconds and test game entry, controls and HOME/Exit Game.
4. Send the new pspgo_hid_v047.log. It can be in seplugins or the storage root on ef0/ms0. Root boot marker is pspgo_hid_v047.boot. The optional checker in PSP/GAME/HIDCHECK now reports v0.47 and inspects those new paths.

If your enabled PRX uses a different filename, replace that existing file with the supplied binary rather than enabling a second copy. Existing saved-target selection and controller pairing are retained. v0.43 remains the known-working NES30 baseline.

## Targeted change

HID attribute0203 is HIDCountryCode, encoded as an unsigned byte. The native consumer reads the value from byte4 of the attributed SDP result. For the exact native cached Sony identities054c:05c4 or054c:09cc, this build supplies local country0 metadata as the five-byte encoded attribute09 02 03 08 00 at that one caller, before a request is sent. The basic DS4 gamepad decoder does not use keyboard country mappings.

This is an explicit compatibility default, not a captured controller response. It does not claim that the controller actually reported that country value. Native identity, pairing, authentication and connection remain live. Registration/connection already succeeded in the supplied test; no success is fabricated for those stages.

The override requires slot1..8, service1124, attribute0203, a nonnull destination with at least5-byte capacity, callback mode0 and no supplied timeout. Unknown identities, DS3, DualSense, NES30, other services/attributes/modes, explicit timeouts and insufficient capacity retain the original native request and all seven arguments. The existing NES30/DS4 descriptor206 caller is independent and remains unchanged.

Ten relocated native call/argument/error-branch/consumer instruction guards, module text/data bounds and jump-region checks must match before one call instruction is patched in RAM. The PnP identity read uses the already-bounded native cache. Both local-response markers and vendor/product metadata are copied into bounded RAM for the existing logger; no callback file I/O is added. The skipped country query leaves no deliberately outstanding country transaction.

## What the next log should show

- DS4_COUNTRY installed confirms the guarded caller matched.
- HID_PNP gives numeric vendor/product and slot, allowing confirmation that this controller matches a supported DS4 identity.
- DS4_COUNTRY source=default_zero confirms the0203 native query was skipped.
- DS4_DESCRIPTOR source=basic_projection confirms startup continued to the existing0206 adapter.
- DS4 BASIC initialized and ZEE INPUT report counters distinguish successful initialization from actual input delivery.

An unknown/clone identity will retain the native query and may still time out. Other startup failures or full-report-only input may remain. This build retains the10-byte report01 path; full78-byte DS4 reports and DualSense support are not added. Controller mappings, v0.43 completion handling, v0.46 log fallbacks and startup markers remain.

## Validation

Pinned PSP SDK forced release build succeeded for plugin and optional checker. All25 ASan/UBSan host programs passed. New tests cover all vendor/product/attribute values, exact five-byte SDP encoding, capacity canaries, slot/service/mode/timeout refusal, other-controller isolation and all10 native context guards. Guard fixtures also match the private native capture, which is not distributed.

All ten compiled plugin checks passed. Country wrapper frame56, with all seven original arguments saved/restored and one original tail-call site for unmatched cases. Its direct calls are only IRQ helpers and bounded metadata recording. Existing descriptor wrapper56, report callback56/DS4 decoder0, raw marker120, log352 and logger496/8192 checks remain. Checker PBP structure and worker frame160/16384 are verified. These checks use synthetic input reports and do not prove hardware behavior or complete indirect native stack bounds.

Source/tests and SDK notices included. No private raw device logs, addresses, link keys or Sony firmware/disassembly included.

Protocol reference: BTstack's HID SDP record builder encodes HIDCountryCode0203 as an unsigned8-bit field: https://github.com/bluekitchen/btstack/blob/master/src/classic/hid_device.c
