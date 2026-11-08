# PSP Go / DS4 USB cable-pairing test v0.8

**Experimental workaround attempt, not confirmed DS4 support.** For PSP Go / 6.61 / ARK-5 cIPL and Windows 10/11 with Windows PowerShell 5.1. A controller and the Go are needed to verify the result. No MotioninJoy/Better DS3 installation is needed for this test.

## What changed

The old PS3/PC procedure reads the Go's Bluetooth address, writes it to the controller, reads the controller's address back and registers that address on the Go. Firmware inspection shows its PSP registration step calls the same native no-PIN registration export already used successfully in your trace. The remaining cable-pairing test is on the DS4 side.

This package supplies a Windows USB helper and an updated PRX. The PRX exports the Go's **Bluetooth** address to a separate local file using the native read-only address query. The helper reads DS4 pairing report 0x12 and writes the Go's host address with report 0x13, using the zero-key cable-pairing method in BlueZ. It reads the target address back and reports whether it matches. The controller's own Bluetooth address is never changed.

This does not import a matching link key into the PSP or prove Bluetooth authentication. A saved-address read-back is not proof of a connection. If a matching key or a lower-stack compatibility change is needed, this test may still fail. Input decoding is still native and remains unverified for DS4.

## 1. Get the Go's Bluetooth address

1. Replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with the file in this package's `seplugins` folder. Keep one existing enabled entry in `ef0:/SEPLUGINS/PLUGINS.TXT`: `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`.
2. Keep older diagnostics and btfree disabled. Fully power the Go off/on to replace resident hooks; standby is insufficient.
3. Turn Bluetooth on and open Manage Bluetooth Devices so the saved-device list is read. This should create `ef0:/seplugins/pspgo_ds4_host.txt`. If absent, try Connect once on the registered Wireless Controller, then check again. The log records whether the address file was saved.
4. Copy `pspgo_ds4_host.txt` into this package's `pc-helper` folder on your PC using normal PSP USB storage mode. **Do not substitute the WLAN MAC from System Information.** The helper also offers an Open PSP host file button.

## 2. Program the DS4's pairing target over USB

1. Extract the ZIP fully. Connect just one DS4 to the PC with a data-capable micro-USB cable. Close other controller apps first.
2. Open `pc-helper/Pair-DS4.cmd`. It runs the included readable PowerShell/C# code with the built-in Windows HID driver. It does not install any drivers or require administrator rights.
3. Load the host file if it was not found automatically. Click **Read USB controller**. The helper displays the controller's address and its currently saved host.
4. Click **Write PSP pairing target**. This changes the DS4's saved Bluetooth pairing, including its previous link-key data. To return it to a console/PC later, re-pair it with that device normally. No write occurs merely from opening the helper or reading the controller.
5. Continue only if the helper says **USB read-back confirmed PSP target**. A failed write/read-back can leave a changed pairing state; read again and report the error.

The helper accepts the standard Sony USB IDs 054C:05C4 and 054C:09CC, checks for a physical USB ancestor and a gamepad HID interface, refuses multiple detected controllers, and verifies the controller identity again immediately before writing. Clones and other IDs are not supported by this helper. USB enumeration and I/O still require testing on Windows hardware.

## 3. Try the controller-initiated connection

1. Disconnect the DS4's USB cable. Exit USB storage mode on the Go, turn Bluetooth on and keep its saved Wireless Controller registration.
2. **Press PS alone on the DS4 first, without SHARE and without choosing Connect on the Go.** Allow about 10 seconds and try the D-pad/face buttons in XMB. This tests the DS4 contacting the host programmed by USB.
3. Reopen Manage Bluetooth Devices after the attempt. This flushes buffered HID service-link events to the log, including events outside an outgoing menu request. `during_request=0` identifies events captured outside the traced Connect call; it is not by itself proof of an incoming connection.
4. If it did not connect, try the menu's Connect once with the DS4 awake. Reopen the device list afterward.
5. Send `ef0:/seplugins/pspgo_ds4_pair.log`, the helper's `pc-helper/pairing-helper.log`, and say whether any buttons worked. No firmware capture files are needed. Both logs may remain local until you choose to share them; the helper log includes controller/host addresses, but no link keys.
6. Disable the plugin and fully power off/on after testing. Live unload remains refused while hooks are referenced. Reboot clears RAM patches and temporary stubs, not normal saved pairing records.

If your controller is no longer registered, put it in SHARE+PS mode and register it with the plugin first, then repeat the USB target-write and PS-alone sequence. Do not delete unrelated saved devices.

## Validation and limits

- Strict PSP GCC 15.2.0 build passes with -Wall -Wextra -Werror, pinned pspdev image sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b.
- Host ASan/UBSan checks pass for existing trampolines and relocation guards, 120 interpreted menu routes, SLC call guards, Bluetooth address byte order/output bounds and relocated address-query guards.
- C# compiled successfully; offline executable tests checked report ID, reversed address order, zero key/padding, invalid input rejection and report 0x12 address decoding. PowerShell script syntax checked. Native USB calls/Windows GUI and the actual PSP/DS4 connection have not been verified here.
- The PRX makes at most four attempts to save the Go address, only from UI query/request paths. It does not write controller keys or change firmware storage. Normal menu actions can change saved pairing records.
- Only known numeric HID event fields are buffered (16 entries). No file writes occur in the Bluetooth event worker. Opening the device list or making a Connect request drains the buffer. No unknown event tail, PIN or link-key data is captured.
- The helper uses only the address-read report and pairing-target-write report. It never sends the DS4 own-address-write report. It does not alter the PC Bluetooth adapter or pair to the PC.

## Sources and licensing

- DS4 cable-pairing format/method: https://github.com/bluez/bluez/blob/master/plugins/sixaxis.c
- DS4 host-address/link-key report: https://github.com/dualshock-tools/ds4-tools/blob/master/ds4-tool.py
- Original discovery technique: JJS/Yoti btfree and its ARK continuation.

New code is MIT licensed; PSP SDK notice included. The Windows helper is new source code, not a repackaged MotioninJoy driver. The ZIP includes source and tests, but no user logs/captures, Sony firmware or decompiled code. Private downloaded PC utilities are not redistributed.
