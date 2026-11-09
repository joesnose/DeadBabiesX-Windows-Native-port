# PSP Go HCI stack v0.11 — GET-only presence and security audit

This is a focused diagnostic development build, not working DS4 support. It
addresses a specific remaining uncertainty: PS value reads returned outer0050
although write-response headers were successful. It compares reported PS key
sizes and readable PSRAM values in the same session, without writing anything.

## What runs automatically

One fixed batch of at most16 GET transactions runs after guarded native Bluetooth
management readiness. It reads build ID and RAM type, sizes of keys03C0/03CC in
stores0/1/4/8, default security key01FD and initialbootmode03CD, two bounded
next-key metadata queries in store8, and PSRAM values of03C0/03CC. Each uses the
existing native70/170 route and fixed50-byte envelope. No key enumeration loop,
arbitrary query interface, SET, reset, halt, clear, pairing, key import, radio
power change or mode switch is included. Old markers/state files are untouched.

The16-query count is bounded. The native transaction API itself has no local
cancellation deadline; a dropped reply can stall the worker. Native failure,
unexpected outer error or malformed/mismatched response stops further queries.
Outer0050 and valid BCCMD errors after the build/RAM prerequisites are logged
without treating placeholder zeros as values. The copied response extent remains
unknown and is labeled accordingly; parsers check the bounded local buffer.

A cold startup clears transient RAM. This audit describes the CURRENT session;
it cannot retrospectively prove the older writes worked or failed. A key's size
is a distinct observation from its readable value. Security and bootmode reads
are facts to interpret, not permission to disable protections or guess mode bits.

## One installation, one run

1. Replace the existing `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX.
   Keep the existing enabled VSH plugin entry and all marker/state files.
2. Fully power off/on to replace the resident driver, then enable Bluetooth.
3. Allow about60seconds, then copy `ef0:/seplugins/pspgo_hci_transport.log`.

No controller pairing, Linux helper, second startup, marker deletion or manual
case selection is needed. The log first shows the audit, then returns to waiting
for actual HCI attachment. No HCI attachment or DS4 connection is promised.

## Offline findings included

`docs/SONY-MESSAGE-MAP.md` maps the examined Sony initialization/service protocol.
`docs/NATIVE-HID-ROUTE.md` explains the native profile route and transport fragment
markers, generic descriptor path and64-byte report-buffer limit.
`docs/PS-STORE-AUDIT.md` confirms selector8 is PSRAM; SET selector0 is not a safe
substitute because it may select persistent storage.
`docs/PROTOCOL-AUDIT.md` corrects the old expected response type: SETREQ2 normally
receives response1. Saved success headers are not effective-write proof.

The v0.10 discovery correction remains: standard inquiry candidates receive
bounded SDP identity verification before controls/bonds are trusted. Network
SDP and HID connection tests are mocked. Actual radioHCI, authentication and DS4
controls remain unverified. Historical documents describe previous experiments
and are superseded by this README for build behavior.

## Validation and licensing

PSP build uses the pinned SDK image recorded in notices. Host tests run with
`sh tests/run.sh` and ASan/UBSan. Audit tests cover all16 exact GETs, bounds,
response matching, size/next layouts, no write requests, errors and stop policy.
Existing transport, discovery, input and corrected parser regressions remain.
No hardware commands were executed during development of this version.

Original adapters MIT; BTstack retains its personal/noncommercial license and
notices. Source/tests/notices included. No Sony firmware, disassembly, private
user logs or controller keys distributed.
