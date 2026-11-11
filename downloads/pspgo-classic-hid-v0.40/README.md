# PSP Go Classic HID v0.40 — retain mixed-profile HID reconnect mode

Test candidate; connection, controls and USB success remain unconfirmed.

Latest v0.39 NES30 attempt fails802f0140 before any service-data wait. Its fresh saved record has flags0622; previous policy chose mode0/native0202. An earlier0422 record chose mode1/native0402 and reached service-query ACK0179. This establishes a routing difference, not proof that routing alone caused the connection error.

This version selects mode1 for0402,0422 and0622 records in both manual Connect and unique-saved-HID reconnect. The accumulated0200 bit no longer switches a mixed0422 registration away from its generic route. Legacy0602,0202 and all other flag words retain prior selection; invalid query/slot/mode handling remains unchanged. This is a native flag-based generic policy, without a NES30 name or VID/PID whitelist. All65536 flag words are tested in both selection paths.

The controller was recently paired to Linux for descriptor capture; a changed remembered host/bond is an independent possible cause of802f0140. Disconnect NES30 from Linux before PSP testing. Replace PRX, fully restart and try native Connect. If the same error remains, delete and re-register only the NES30 entry on the PSP in the same controller mode; unrelated saved devices need not be removed. Bond refresh is a hardware pairing step, not something the PRX can assume succeeded.

Send the resulting PSP log. It should show PROFILE_SELECT flags0622 selected1, followed by native flags0402 for this registration. If connection succeeds, check inputs before attempting USB. Earlier USB freeze is still unresolved; the v0.39 bounded service-data wait and pre-input diagnostics remain. The withdrawn v0.37 global receiver observer is absent.

Pinned build,19 ASan/UBSan programs and compiled report/ACK/SDP/observer checks pass. No hardware success or complete indirect stack audit claimed. No private raw logs, Bluetooth addresses, keys or Sony firmware/disassembly published.

## Retained v0.39 service-wait design and test limitations

# PSP Go Classic HID v0.40 — bounded SDP service-data wait

Generic test candidate for NES30 USB shutdown; not a confirmed fix. Based on v0.38; withdrawn v0.37 global event-receive entry observer remains absent.

## Evidence

The new accumulated log includes NES30 slot2 native generic mode selection0422->mode1, successful connection notification and service-query ACK0179 completion, then no NES30 descriptor/init at the custom parser. A later0622 connection selected native mode0 and also stopped after ACK0179. The user reports USB entry crashes. No exact blocked wait was captured in those NES30 sessions because lifecycle snapshots previously armed only after active custom input. Separate earlier sessions contain working GameStick/Free input, and one mismatched descriptor/report case; do not attribute all accumulated entries to NES30.

Linux captured a valid86-byte NES30 HID descriptor, report3/payload9, accepted by production parser/mock tests. This proves a format the parser can handle; it does not prove native PSP service retrieval succeeded.

Native service queries have two stages: command ACK0179, then service data event0272. The latter has a supported optional timeout, but native callers can supply NULL while the service routine owns its reboot mutex. A silent wait there is consistent with the observed pre-parser stall and could obstruct Bluetooth shutdown. The exact USB blocker is still a hypothesis, not proven by the old log.

## Change

Patch one guarded call inside the native service-query routine, not the receiver entry. Only a one-selector event0272 receive with native76-byte fragment capacity, callback mode0/1 and no existing timeout gets a fresh5-second timeout. Existing caller timeouts remain unchanged. All six EABI arguments are forwarded, except the deliberately supplied timeout pointer on the selected path; result and packet buffer handling remain native. The native alarm/waiter cleanup returns an error, and service-query code unwinds its own reboot mutex. No forced unlock, thread termination, fake descriptor, controller impersonation or firmware flashing. No blanket event-loop timeout.

The budget is per service-data receive fragment; it is not a fixed total transaction deadline. This generic change applies to any device using that native SDP call, including PnP/HID requests. A slow reply may now fail with a native timeout rather than wait forever; a late reply and other shutdown waits remain hardware concerns. Existing generic ACK timeouts, profile/reconnect policy and input path are unchanged.

SDP_WAIT records started/finished/error counts, whether bounded, callback mode and last native result. It records metadata only and does not write files inside native callbacks. Counters are snapshots, not a complete per-request history. Lifecycle snapshots now arm on a validated native HID connection event as well as usable custom input, so failed descriptor setup is observable. Sampling remains v0.38's2-second cadence.

## NES30 test

Replace ef0:/seplugins/pspgo_hci_transport.prx, retain VSH/GAME entries and saved registration, and fully restart. Keep other controllers off. Use the same NES30 mode as the Linux capture, connect from the PSP Bluetooth menu, then wait15seconds. Note whether controls work. Try USB entry once and report whether it opens or hangs/crashes, then send pspgo_hci_transport.log after recovery. No re-pairing required if the existing PSP registration still connects. No claim that this supplies missing input support or resolves GameStick game exit.

## Verification

Pinned PSP SDK build and19 ASan/UBSan programs pass. Tests include the actual NES30 static descriptor with synthetic full/stripped reports; all65536 event IDs over32 argument/mode combinations for new SDP policy; timeout-pointer preservation, fresh budget, once-only original arguments/results and10 instruction guards. Compiled SDP wrapper40bytes; metadata helpers32bytes or less and IRQ-only; one original call per terminating path. Six-argument EABI save/restore and the intended fifth-argument replacement were inspected in the generated assembly. Existing report/ACK/teardown checks pass. These checks do not prove hardware success or every indirect native/callback stack bound.

Own MIT source/tests, static descriptor metadata and SDK notices included. No Sony firmware/disassembly, raw private logs, Bluetooth addresses or keys distributed.
