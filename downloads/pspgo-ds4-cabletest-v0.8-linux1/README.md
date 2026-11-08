# PSP Go / DS4 USB pairing helper for CachyOS

Linux v0.8-linux1, x86-64. Native command-line executable using Linux hidraw, with no Wine, Python, hidapi or MotioninJoy dependency. Requires the glibc already provided by current CachyOS (binary minimum: glibc 2.34). Other architectures can compile the included source.

**Experimental pairing test, not confirmed working DS4 support.** It uses the same zero-key cable-pairing method as the Windows helper. Programming the host address does not prove Bluetooth authentication or functioning controls, and this tool does not provision a matching key in the PSP. The PSP PRX is unchanged from v0.8.

## Extract

Extract the downloaded .tar.gz using your file manager, or:

```sh
tar -xzf PSPGo-DS4-Linux-v0.8-linux1.tar.gz
cd PSPGo-DS4-Linux
```

The archive preserves the executable bit. If your file manager removes it, run `chmod +x ds4pair`.

## Get the Go's Bluetooth address

1. If v0.8 is already installed on the Go, keep it. Otherwise replace `ef0:/seplugins/pspgo_ds4_pairtest.prx` with the included `seplugins/pspgo_ds4_pairtest.prx`. Keep one enabled line in the existing `ef0:/SEPLUGINS/PLUGINS.TXT`: `vsh, ef0:/seplugins/pspgo_ds4_pairtest.prx, on`.
2. Keep earlier diagnostics and btfree disabled. Fully power the Go off/on after replacing the PRX.
3. Enable Bluetooth and open Manage Bluetooth Devices. The PRX should create `ef0:/seplugins/pspgo_ds4_host.txt`. If it does not, try Connect once on the registered Wireless Controller and check the log for HOST_ADDRESS.
4. Use normal PSP USB storage mode to copy that host file into this extracted folder, alongside `ds4pair`. This is the Go's Bluetooth address; do not use its WLAN MAC from System Information.

## Read, then write

Connect one standard DS4 to your CachyOS PC with a USB data cable. Close controller apps. First run the read-only command:

```sh
./ds4pair
```

It should show the controller's address and current paired host. Then program the Go target:

```sh
./ds4pair --host-file ./pspgo_ds4_host.txt --write 2>&1 | tee pairing-helper.log
```

The explicit `--write` changes the DS4's saved Bluetooth host and writes a zero link key. It never changes the controller's own address or your PC Bluetooth configuration. Re-pair the DS4 with its previous console/PC later to restore normal use. Reading it or opening the helper makes no pairing changes.

If access is denied, use the same commands with `sudo` as a one-off, or install the optional narrowly scoped desktop-access rule below. No kernel driver installation is needed.

```sh
sudo ./ds4pair
sudo ./ds4pair --host-file ./pspgo_ds4_host.txt --write 2>&1 | tee pairing-helper.log
```

If more than one controller is attached, unplug the others or select a specific node with `--device /dev/hidrawN`. The chosen node is still checked for a USB bus and supported Sony IDs before any write. Supported IDs: 054C:05C4 and 054C:09CC. Bluetooth-connected controllers and other IDs are rejected. Clones are not established to work even if they reuse these IDs.

Continue only when the helper says **USB read-back confirmed PSP target**. This confirms the stored address only. A failed write/read-back may still leave the pairing state changed; read the controller again and send the error rather than assuming success.

## Try connecting to the Go

1. Disconnect the DS4's USB cable. Exit the Go's USB storage mode and enable Bluetooth. Keep its saved Wireless Controller registration.
2. **Press PS alone on the DS4**, without SHARE and without choosing Connect on the Go initially. Allow about 10 seconds and try the D-pad and face buttons in XMB.
3. Reopen Manage Bluetooth Devices to flush the PRX's buffered event trace. If it did not connect, try menu Connect once with the controller awake, then reopen the list.
4. Send `ef0:/seplugins/pspgo_ds4_pair.log` and the local `pairing-helper.log`, and report whether any buttons worked. The helper output includes controller/host addresses, not keys.
5. Disable the plugin and fully power off/on after testing. Resident hooks cannot be live-unloaded. This clears RAM patches, not normal saved pairing records.

If the controller has no saved registration on the Go, register it in SHARE+PS mode with the plugin first, then repeat the USB write and PS-alone sequence.

## Optional USB access without sudo

For an active desktop session on CachyOS, install the provided rule, reload rules, then unplug/replug the DS4:

```sh
sudo install -m 644 70-pspgo-ds4.rules /etc/udev/rules.d/70-pspgo-ds4.rules
sudo udevadm control --reload-rules
```

The rule grants active-session access only to the two specified Sony USB IDs. It does not grant global access to every hidraw device. To remove it later, remove that one rule file and reload/replug.

## Build and checks

The prebuilt x86-64 program is supplied; building is optional. From the source folder:

```sh
make
make test
```

CachyOS/Arch development dependencies, if building from source: `sudo pacman -S --needed base-devel linux-api-headers`.

Strict native build passed with -Wall -Wextra -Werror. ASan/UBSan tests passed for report 13 encoding, byte order, zero key, buffer bounds, address decoding and invalid inputs. CLI checks passed for malformed/NUL/oversize host files, missing options, non-HID device rejection and absent hardware. The binary ran on the build host. Actual CachyOS desktop permissions, USB feature I/O, controller behavior and PSP connection require hardware testing; they have not been verified here.

The PSP binary is byte-for-byte v0.8 (SHA256 2862a017924f8e6f71a3b481277c583146310346c281f1d2b779d3db52c333d6). Its source and prior validation are included under psp-source. No firmware binaries, user captures or logs are included.

## Licence and references

New helper/PRX source: MIT, with PSP SDK notice included. Uses Linux's existing hidraw API. Pairing method reference: https://github.com/bluez/bluez/blob/master/plugins/sixaxis.c . Discovery technique credited to JJS/Yoti btfree and its ARK continuation.
