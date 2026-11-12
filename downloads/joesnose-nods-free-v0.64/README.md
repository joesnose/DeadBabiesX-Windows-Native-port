# joesnose_nods_free v0.64

Logging-free release based on the user-confirmed working v0.63 PSP Go Classic HID build. For PSP Go with6.61 ARK; existing runtime firmware guards also recognize the supported6.60 environment.

## Install

1. Copy `seplugins/joesnose_nods_free.prx` to `ef0:/seplugins/`.
2. In your existing ARK plugin list, replace the old `pspgo_hci_transport.prx` filename with `joesnose_nods_free.prx` in both VSH and GAME entries. Use the included reference:

   ```text
   vsh, ef0:/seplugins/joesnose_nods_free.prx, on
   game, ef0:/seplugins/joesnose_nods_free.prx, on
   ```

3. Disable/remove the old plugin entries so only this PRX loads. Fully power off and restart the PSP.

Keep `ef0:/seplugins/pspgo_hid_controls.txt` and `ef0:/seplugins/pspgo_hid_target.dat`. They retain their existing names so custom controls and the saved target continue working. The included controls file is only a template: do not overwrite your customized file. No re-pairing or EBOOT is needed.

## Release changes

- PRX and internal module name: `joesnose_nods_free`.
- Removed the file-log sink, log pathname and formatter. The compiled sink is inert and cannot call file APIs; no log file is created or updated. An old log can be deleted.
- Kept the owned controller worker because it maintains keepalive, reconnect, target persistence and Sony link setup. Its diagnostic callback is disabled; this is not a disposable logger thread.
- Retained the working v0.63 controller mappings, NES30 regression fix, Sony recovery isolation, pairing, configurable controls and reboot reader quiescing. All other implementation files match v0.63; the unused file-log helper was removed.

## Review and validation

Reviewed the release diff, native patch guards/bounds, Sony/generic isolation, shutdown quiescing, callback stack checks, control configuration and worker lifetime. No additional defect was identified in that review. This does not establish that the code is free of all hardware-specific issues.

Forced PSP SDK build and strict warnings pass.34 remaining ASan/UBSan host programs and16 compiled checks pass; the obsolete file-logger test was removed with its helper. The release check verifies no formatter/log-path/log-writer remains and that the only compiled file write is the existing saved-target persistence operation. This renamed logging-free binary has not independently been tested on hardware.

## Build from source

With PSPSDK and the ARK systemctrl development library available, run `make -B release` from the source project directory. Output: `joesnose_nods_free.prx`. Run host tests with `sh tests/run.sh`; compiled validation tools accept `joesnose_nods_free.elf` as their argument.

Own source only; no Sony firmware, private logs, keys or device identities are included.
