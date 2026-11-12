# PSP Go Classic HID v0.61 — arm exit handover for native Sony input

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep your controls text, target file and ARK entries, then fully restart. No EBOOT, new configuration or pairing reset is needed. The controls template is unchanged; preserve your custom file. Same log: `ef0:/seplugins/pspgo_hci_transport.log`.

The v0.60 hardware log confirms working GAME input with native Sony identification, but no verified recovery record. Native-known Sony reports can be decoded without the plugin's selected/live metadata scope. The exit-preservation wrapper requires that scope, so working GAME input alone did not enable preservation on exit.

Recovery now checks the existing native live-link/backend snapshots after startup as well as during startup. Once the public device-record API is ready, the owned worker uses that public reader to match the current record to the saved Sony target and cached address. Startup retains its guarded private read. Either path must pass the same fresh fingerprint, name, address, event/backend and race checks before arming the selected/live scope. There are at most3 reads per startup-ready phase; a phase change gets a fresh budget. Existing verified scopes need no recovery.

Reboot preservation remains conditional on the exact native backend, a genuinely live selected Sony context and plugin-owned input node. No blanket disconnect bypass, DS3 impersonation, radio reset or new report-time query is added. Working game-entry behavior and controls are retained. Normal disconnect, USB and suspend behavior remain native.

Test entering a game and HOME → Exit Game with the controller on. Check whether the light remains on and input returns in VSH. Hardware behavior is unverified for this build. The same log adds `public_api=1` to a successful post-startup `SONY RESTORE ... restored_live_verified=1` record.

Forced PSP build,35 sanitizer host programs and16 compiled checks pass. Recovery tests exercise both API-ready phases, exactly one matching native reader, native failures and verified scope activation. Own source only; no Sony firmware, private logs, device addresses or keys included.
