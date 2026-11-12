# PSP Go Classic HID v0.46 — logging and load diagnostics

The user reports no fresh log. The uploaded file ends with v0.43 and contains no v0.44/v0.45 startup, so it cannot establish which newer build actually ran. v0.45 also silently ignored every file-open/write error and logged only to ef0:/seplugins. v0.46 adds fallback logging, raw startup markers and an optional on-screen load checker. PSP hardware behavior is not yet verified.

Registration and input policy are unchanged from v0.45. In particular, the exact Wireless Controller/class rule and the Free exception remain; the broader earlier diagnostic registration selector is not enabled in this logging build. An older capture shows registration success with match_mode=2, which accepted any selected discovery record. Once a fresh log shows the current registration API/result, that evidence can guide the next pairing change. Registration, authenticated connection and usable input are distinct stages.

## Install the versioned plugin

1. Copy SEPLUGINS/pspgo_hid_v046.prx from this ZIP into seplugins on the PSP storage used by your current plugin entry.
2. Change this plugin's existing enabled VSH and GAME entries to the versioned filename. Disable any other enabled entry for older PSPGo HID/DS4 test plugins. Keep unrelated plugins and saved Bluetooth records.

For internal PSP Go storage, the entries are:

```
vsh, ef0:/seplugins/pspgo_hid_v046.prx, on
game, ef0:/seplugins/pspgo_hid_v046.prx, on
```

If your current entry uses ms0: and the PRX is on that memory stick, retain ms0: in both lines. Edit the active ARK configuration you already use; ARK-plugin-entries-v046.txt is a reference, not a replacement for your full configuration. A renamed PRX requires updating the enabled entry, not just copying the file.

3. Fully shut down and restart the PSP; standby does not clear resident hooks. Leave other controllers off, open Bluetooth settings, and wait five seconds.
4. Look for a new pspgo_hid_v046.log in seplugins or at the root of internal storage/memory stick. Also look for pspgo_hid_v046.boot at either storage root. Send whichever new files appear, then test DS4 registration once and send the updated log and exact message.

## New log locations

The plugin tries these locations, preferring the last successful one:

```
ef0:/seplugins/pspgo_hid_v046.log
ef0:/pspgo_hid_v046.log
ms0:/seplugins/pspgo_hid_v046.log
ms0:/pspgo_hid_v046.log
```

It does not overwrite or append to the old pspgo_hci_transport.log. The versioned name avoids confusing an old accumulated log with a fresh test. It does not create missing directories; root fallbacks work without seplugins. Each line tries at most four routes, handles short writes, requires a successful close and retains numeric I/O errors. Failed files can contain a partial or duplicate line; use a complete versioned log where possible.

DIAG version=0.46 records stage, worker activity, selected route0..3 or4=no route, failure count and the last numeric open/write/close errors. Historic error fields remain even after fallback succeeds. File writes use a nonblocking ownership guard; contending lines are skipped and counted rather than overlapping or waiting. This is not a complete event history. Native HID report/event callbacks still do no file I/O.

## Independent startup markers

A small plain-text marker is appended to ef0:/pspgo_hid_v046.boot, falling back to ms0:/pspgo_hid_v046.boot. It does not use the normal log formatter or seplugins directory. Startup attempts and initial worker activity record:

- stage0: module_start entered, before model/firmware validation;
- stage1: supported model/firmware accepted;
- stage2: native hook installation attempts completed;
- stage3: logger thread start attempted;
- stage4: module startup reached READY.

Stages record progress, not proof that every guarded patch matched or that pairing succeeded. Worker1..4 records early logger ticks; f means at least15 ticks if a later error causes another marker. A marker proves code reached the recorded stage. Absence of both logs and markers can still mean either loading failed or neither storage was writable; absence alone is not proof of one cause.

## Optional on-screen checker if no files appear

Copy PSP/GAME/HIDCHECK from the ZIP onto the PSP and run PSPGo HID Load Check v0.46 from Game. It displays the version of a found PSPGoClassicHID/PSPGoNativeDS4 kernel plugin and checks the versioned log/marker file sizes or stat errors on both storage devices. Report the displayed Loaded/Version text, or send a photo if easier. Press START to exit.

Opening the checker starts a GAME session and reloads plugins. Its result covers that GAME session; it does not establish which version was previously running in VSH. The checker itself does not load/start the HID plugin, pair devices, issue Bluetooth commands, modify firmware or write files. The normally enabled GAME plugin still starts as part of entering the app. Old/duplicate plugin entries must still be disabled.

## Retained controller support

v0.43 remains the user-confirmed NES30 VSH/game/exit fallback. v0.46 retains v0.45's DS4 registration selector and v0.44 basic report01 decoder/projection for Sony054c:05c4/09cc, remembered-controller reconnect, generic/Free input and v0.43 completion waits. DS4 basic input, registration and authenticated connection remain hardware-unconfirmed. Full78-byte DS4 reports, DualSense and new radio/HCI functionality are outside this build. No firmware flashing or forced success is introduced.

## Validation

Pinned PSP SDK cross-builds passed for the plugin and checker. All24 host programs passed with ASan/UBSan. New sink tests cover both storage devices/directory/root fallbacks, descriptor0, unavailable storage/recovery, preferred-route failure, short/zero/error/oversized writes, close errors and raw marker bounds. Existing executable registration-rule, DS4 synthetic-packet, NES30, generic and lifecycle tests remain passing.

Nine compiled plugin checks passed. Raw marker frame120 and its direct calls are limited to K1/IRQ/file I/O; it does not call the normal formatter/logger. Log frame352, logger frame496 on its8192-byte stack; DS4 report callback remains56 with its stackless decoder. The checker has a160-byte worker frame on its16384-byte thread; its PBP header/payload were verified and compiled symbols contain no Bluetooth/module-loader or file-write functions. These checks do not prove complete indirect native stack bounds or real hardware behavior.

Source, tests, original static descriptors and SDK notices included. No private raw logs, Bluetooth addresses/keys or Sony firmware/disassembly included.
