# PSP Go Classic HID v0.54 — reconnect setup test

Replace `ef0:/seplugins/pspgo_hci_transport.prx`. Keep your entries, pairing and preferred-target file, then fully restart. No EBOOT or Game app. Same single log: `ef0:/seplugins/pspgo_hci_transport.log`.

Test the first connection, then disconnect/reconnect the DS4 once in the same game if possible. Compare response before and after reconnect, using D-pad/buttons as well as the stick. Send the same log if it remains laggy. An unchanged first connection and a later reconnect are the useful comparison; do not re-pair.

## Basis

The user reports good input on the first connection and lag after reconnect. The uploaded patched source differs from v0.52 only in two merged-input reset calls. It does not disable file logging or add a packet-drain loop. A merged-state reset does not flush native transport queues.

v0.52 timing logs include gaps of up to about 421 ms between reports reaching the adapter while the game keeps reading controls. Those are host-side measurements, not proof of wireless latency or its cause; idle report suppression can also increase report age. GAME automatic native reconnect still returns 802F0140 in the supplied log. None of these facts establishes a dead-zone problem.

## Experimental change

Sony's native HID startup applies two native setup functions to DS3 identities and skips them for the DS4's unknown identity. This candidate calls those existing functions, in the same order and with their existing DS3 arguments, for the freshly matched selected DS4 after a genuine connection and active input. They send Sony BSM commands 0x79 and 0x24. Their complete radio-side meaning has not been decoded; this is a test of missing connection setup, not a proven low-latency setting.

The candidate:

- Requires the existing preferred-controller fingerprint, fresh native metadata, a genuine successful link and supported/unknown DS4 identity policy. It does not report a false DS3 VID/PID or change the native identity cache.
- Guards 33 relevant native instructions/call contexts before enabling the sequence. A mismatch skips it.
- Runs once per confirmed link generation on the owned background worker. Duplicate connected events do not repeat it. Disconnect/failure invalidates pending work; a changed generation prevents continuation after command79.
- Applies a two-second response timeout only to the owned worker's active 0x17A/0x124 response wait, leaving other callers/explicit timeouts unchanged. Native transaction errors remain errors. Command24's semantic response is not exposed by its existing native wrapper, so the log records only that it was issued.
- Retains the supplied patch's merged-state-reset idea, scoped to fresh confirmed selected-controller links and disconnect/failure events. Failed manual connection attempts no longer clear another live controller's controls.

No asynchronous read is repeated to drain packets. Native queue/read ownership remains unchanged. Analogue thresholds, scaling and button mappings are unchanged from v0.52; the unpublished dead-zone experiment is not included. Existing low32 timing optimization and single-log summaries remain.

`DS4 LINK_SETUP guarded_native_sequence=1` confirms matching guards. A later `DS4 LINK_SETUP slot=... epoch=... command79_result=... command24_issued=...` records each attempted connection setup. It is not a claim of improved latency or completed command24 semantics.

## Limits

This build has not been tested on a PSP. It may not improve reconnect latency, and it does not claim to fix the automatic 802F0140 error or preserve the RF link across service reload. It adds no full DS4 report11 or DualSense support. Keep v0.52 available to restore by replacing the PRX and restarting.

## Installation reference

```
vsh, ef0:/seplugins/pspgo_hci_transport.prx, on
game, ef0:/seplugins/pspgo_hci_transport.prx, on
```

Keep one enabled driver per context and preserve unrelated plugins.

## Validation

Forced PSP SDK build, all 29 ASan/UBSan programs and twelve compiled checks passed. New tests cover once-per-link setup, duplicate events, disconnect/failed-link invalidation, stale-generation continuation refusal, independent slots, all33 native guards and all65536 response IDs plus caller/timeout/callback isolation. Compiled status paths call the original once with a 40-byte wrapper frame; the SLC observer has a 24-byte frame and the owned logger a 544-byte frame on its8192-byte stack. These checks do not prove radio-side behavior.

Own source/tests provided separately. No Sony firmware/disassembly, private raw logs, controller addresses or pairing keys are published.
