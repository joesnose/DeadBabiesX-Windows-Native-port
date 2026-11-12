# PSP Go Classic HID v0.63 — isolate Sony recovery from NES30 startup

Hardware test candidate for the NES30 Pro game-entry freeze reported on v0.62. This is a regression in recent builds: the NES30 had previously worked. Hardware success with v0.63 is not yet confirmed.

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, retain controls/target/ARK entries, and fully restart. No new configuration, EBOOT or pairing reset is needed. Preserve your customized controls file. Same log: `ef0:/seplugins/pspgo_hci_transport.log`.

The Sony recovery worker used the shared remembered-controller target, which can contain an NES30 registration. It checked the fresh controller name only after issuing its recovery query. That allowed an unnecessary Sony-recovery query to run during a generic controller's native startup. This is an isolation gap and a candidate regression cause, not a proven crash instruction.

Before issuing any recovery query, this build compares the saved registration fingerprint with each supported Sony name plus the native cached current address and slot. An NES30 registration, other generic name or reused slot/address mismatch stops the Sony recovery path without a query. The temporary comparison record is only an internal pre-query hint: it is never supplied to the firmware, used as authentication or used to authorize input. Actual Sony recovery still requires its fresh native record and the existing live-link/backend/race checks afterward.

The earlier v0.43 NES30 descriptor override, button decoder, completion-wait and status-timeout policy files remain byte-identical. The original completion bridge instructions are retained, with later separate Sony bridge code appended. NES30 decoder/mapping and existing shutdown handling are not changed by v0.63. Sony reader quiescing remains restricted to an owned Sony input context in committed reboot teardown.

Test NES30 connected in VSH, launch a game, check controls, then HOME → Exit Game. Test Sony controllers separately if desired. If the PSP still freezes, use the same log; the new gate logs `SONY RESTORE skipped target_not_sony_or_address_changed=1 no_recovery_query=1` when applicable.

PSP build,35 ASan/UBSan host programs and16 compiled checks pass. Tests cover both Sony names, NES30 rejection and stale slot/address rejection before recovery, plus the existing generic mappings and native wait forwarding. Those checks cannot establish hardware success. No Sony firmware, private device logs, addresses or keys are included.
