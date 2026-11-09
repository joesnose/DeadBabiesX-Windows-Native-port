# PSP Go Bluetooth hardware research — 9 November 2026

## Result and scope

The strongest new finding is Sony's shield-removed FCC photograph: the ALPS
QZ7-001A module contains a CSR-marked IC with visible lines **BC5CE / 700CFU /
817AA**. This is direct physical evidence from the photographed production
prototype, not a marketed-part inference from CHIP_VERSION=5. The exact ordering
code/die configuration, memory topology and pinout have not been matched to a
public datasheet. It does not identify every retail board or the user's unit.

This pass checked original Sony specifications/FCC documents, first-hand
teardowns, manufacturer CSR documentation, original open-source implementations
and independent hardware-tool authors. Search hits from resellers, generic
Bluetooth FAQs and forum speculation were not used as proof of chip capability
or a safe recovery method. No new PRX or radio experiment is introduced here.

## 1. Physical module: substantially better identified

- iFixit's own teardown identifies the shield marking QZ7-001A and ALPS.
- Sony's FCC internal-photo exhibit 1123145, page2 (PDF page index1), shows that
  shield removed and the CSR/BC5CE/700CFU/817AA markings beneath it.
- A separate first-hand teardown labels a QZ7-001B module. At least these two
  published module markings therefore exist; interchangeability is unproven.
- No verified QZ7-001A/B debug-pad map, package ball map, voltage specification
  for the debug interface, or matching complete module datasheet was found in
  this search. Generic BC57E687/HC-05 pinouts must not be substituted.

Sources:
[Sony FCC internal-photo exhibit](https://fccid.io/AK8PN1001A1/Internal-Photos/Internal-Photos-1123145)
([PDF](https://fccid.io/AK8PN1001A1/Internal-Photos/Internal-Photos-1123145.pdf));
[iFixit original teardown](https://www.ifixit.com/Teardown/PSP+Go+Teardown/1124);
[Steward's original teardown](https://steward-fu.github.io/website/handheld/pspgo_teardown.htm).

The photographed 26MHz crystal is consistent with the independent test report;
that agreement does not establish an interchangeable CSR firmware image.

## 2. Official electrical/radio facts and pairing implications

Sony's launch specifications list PSP Go Bluetooth2.0+EDR. Sony's submitted
Bluetooth report 29HE0053-HO-01-B-R2, printed page3, describes a production
prototype with a 26MHz radio clock, internal DC3.2V/DC1.9V supplies, classic
Bluetooth modulation and an inverted-F antenna. These are system/module supply
facts, **not permission to apply those voltages to an unidentified debug pad**.

Sony specifies DS4 CUH-ZCT2 Bluetooth2.1+EDR. That version difference does not
prove incompatibility. It does mean a generic SSP-only implementation cannot be
assumed correct for the Go's actual advertised and firmware-reported features.
The experimental application currently rejects legacy PIN requests; that is an
implementation limitation to revisit after actual controller capabilities are
established, not a reason to invent a PIN or claim firmware pairing is fixed.

Sources:
[Sony PSP Go specification](https://sonyinteractive.com/en/press-releases/psp-go-psp-playstationportable-evolves-to-match-the-digital-lifestyle/);
[Sony/UL Bluetooth test report](https://fccid.io/AK8PN1001A1/Test-Report/Test-Report-1-of-2-1123143)
([PDF](https://fccid.io/AK8PN1001A1/Test-Report/Test-Report-1-of-2-1123143.pdf));
[Sony DS4 specifications](https://www.playstation.com/en-gb/ps4/tech-specs/).

## 3. Why public FCC records do not provide a circuit diagram

Sony's May14,2009 confidentiality request names the schematic, block diagram,
theory of operation and antenna specification for indefinite withholding. The
photos/manuals were requested for temporary confidentiality instead. This
explains why public photographs can identify an IC without supplying its wiring.
It does not prove no schematic exists elsewhere or that the module is impossible
to reverse engineer.

Source:
[Sony confidentiality letter](https://fccid.io/AK8PN1001A1/Letter/Reqeuest-for-confidentiality-letter-1123063)
([scanned PDF](https://fccid.io/AK8PN1001A1/Letter/Reqeuest-for-confidentiality-letter-1123063.pdf)).

## 4. HCI capability is not the same as access to it

CSR's USB design note describes USB_VM_CONTROL=false as standard HCI USB
configuration and true as VM-managed USB. The BlueSuite guide describes unified
firmware offering alternative stack arrangements selected through
ONCHIP_HCI_CLIENT. These are family-level manufacturer facts. Neither document
specifies Sony build6424's write path, overrides, transition activation timing,
USB ownership policy or recovery behavior.

The existing hardware observations showed a GET-style reply to SETREQ and no
observable RAM override across the tested fixed-size frames/keys. Thus the
current Sony request70 route has not been validated as a generic BCCMD writer.
This is consistent with a service translating/handling requests differently,
but does not locate the translation in a particular firmware layer. Examined
PSP-side paths copy request data and response data; there is no actual USB-wire
capture proving where an operation changes. It would be wrong to accept reply1
as reply3 or claim successful writes from zero outer status alone.

Sources:
[CSR Bluetooth and USB Design Considerations, p35](https://hacker.instanet.net/Bluetooth/CS-101412-AN-4.pdf);
[CSR BlueSuite guide, section5.3](https://fccid.io/PIWBSMAN/User-Manual/User-Guide-1135316.pdf);
[reference CSR request implementation](https://github.com/bluez/bluez/blob/5.50/tools/csr.c).
The hardware observations in this paragraph are our own bounded test conclusions,
not an online claim that Sony supports mode switching. No private logs included.

## 5. Recovery lead: genuine Bluetooth power-control code exists

uOFW documents and implements Bluetooth power control/status. The implementation
of sceSysconCtrlBtPower masks a boolean value and normally sends
PSP_SYSCON_CMD_CTRL_BT_POWER (0x53). However, an internal override state can make
it return success before sending that command. Its cached power-control getter
and reported-status getter are separate paths. A return0 or cached control value
alone therefore cannot establish that the radio was actually power-cycled.

This is a concrete lead for a future independently observed recovery design.
It does not prove which QZ7 rails are removed, whether auxiliary retention
survives, or whether Sony's BSM/USB stack can reinitialize after a mode change.
The published uOFW code also needs comparison with the actual loaded firmware
before calling its implementation assumptions confirmed on the user's device.

Sources:
[uOFW Syscon API documentation](https://uofw.github.io/uofw/group__Syscon.html);
[original Syscon implementation](https://github.com/uofw/uofw/blob/master/src/syscon/syscon.c);
[command definitions](https://github.com/uofw/uofw/blob/master/include/syscon.h).

## 6. Hardware debug/dump route: feasible in the family, not mapped for QZ7

The original csr-spi-ftdi project supports CSR programming/debugging through an
FTDI adapter and Linux/Wine or Windows. Its author reports tests on several CSR
parts, including BC57F687A; that is not a test on the Go's photographed BC5CE
marking. The project distinguishes I/O levels across parts. It is a credible
hardware-access lead, conditional on identifying the module's correct signals,
levels, power arrangements and debug enable state.

CSR's own BlueSuite documentation describes flash upload/download and SPI-based
configuration access. It also notes that BlueFlashCmd resets the chip: a backup
operation is not necessarily passive or risk-free. Flash backups do not imply
that every ROM region or custom firmware component is readable/restorable.

A newer Python USB-SPI dump project explicitly targets BlueCore4-External,
BC417143/HC-05 hardware. Its author says other generations use different stubs.
It is **not** a validated Go dumper; its HC-05 pinout, voltage and force option
must not be copied to QZ7. No programmer purchase or wiring is recommended yet.

Sources:
[csr-spi-ftdi original project](https://github.com/lorf/csr-spi-ftdi);
[CSR BlueSuite guide, sections7.2/7.3/7.9/7.10](https://fccid.io/PIWBSMAN/User-Manual/User-Guide-1135316.pdf);
[Python USB-SPI dump project and explicit limitations](https://github.com/brlumen/csr-usbspi-py).

## 7. Existing PSP controller work: useful, but different problems

JJS's btfree removes a discovery/registration class filter. Its original release
scope does not claim a generic Bluetooth host stack or native DS4 connection.
The PSP-Bluetooth project instead describes an ESP32 controller bridge over the
PSP serial/AV interface for 2000/3000 systems. It demonstrates another route to
controller support, not unlocking the Go's existing QZ7 radio. Neither supplies
the missing built-in-radio write/recovery procedure established here.

Sources:
[btfree project](https://github.com/PSP-Archive/btfree);
[PSP-Bluetooth author's architecture](https://github.com/PSP-Bluetooth/.github/blob/main/README.md).

## Recommended next work

1. Stop repeating the fixed-envelope RAM writes: the completed matrix supplied
   the relevant negative evidence for those forms. Preserve logs/markers.
2. Trace the boundary between Sony request70 and the radio's command handling,
   prioritizing static analysis of actual code or an evidenced interface. Do not
   guess request71 or another command merely because its number is adjacent.
3. Compare the loaded Syscon power implementation with uOFW, then design a
   baseline-only, independently observed power recovery test before any HCI-mode
   transition. No power toggling test is included in this research pass.
4. Resolve exact chip/memory/package identity and module revision before selecting
   a dump method. If software access remains unavailable, a spare/donor board
   with physical tracing could establish debug pads without risking the user's
   working unit. This is a potential future hardware step, not a current request.
5. Obtain a trustworthy original dump/backup where technically possible and verify
   the restoration procedure before persistent changes. Review pairing policy
   against actual controller features rather than assuming SSP from a chip label.

Not found: a verified QZ7 pinout, exact matching Sony radio executable, a proven
Go RAM-write/HCI transition recipe, a tested restoration method, or a demonstrated
software-only native DS4 fix in the sources examined. These are scoped search
results, not claims of impossibility. No new test is requested from the user.
