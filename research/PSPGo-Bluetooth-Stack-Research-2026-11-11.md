# PSP Go Bluetooth replacement-stack research

11 November 2026. Literature, public source, retained project source and capture review. Three independent agent reviews covered hardware/transport, stack choices and native-path alternatives. No new firmware image, radio command, mode write or device test was performed during this research.

## Conclusion

An independent Classic Bluetooth HID host is technically credible, but a software-only takeover of this Go's internal radio is not demonstrated. Existing host-stack code can be reused. The controlling uncertainty is the radio's actual interface, ownership and recovery, not whether Bluetooth protocols can be written in C.

For the devices already available, completing native service-query and lifecycle handling remains the shortest evidenced path. That should be separate from the HCI feasibility work, so a host-stack prototype is never described as a working controller driver before radio communication succeeds.

## Findings and confidence

| Question | Finding | Confidence / boundary |
|---|---|---|
| What radio product? | CSR explicitly names BC5120 in PSP Go. | High for product designation; no exact board pinout or memory topology follows. |
| Can this chip family expose HCI? | CSR describes on-chip/off-chip stack selection and separately VM/standard USB ownership. | Documented family capability; Sony image support/activation unproven. |
| Has another on-chip CSR module been converted? | RN42/BC04EXT was converted from on-chip SPP to off-chip HCI for Linux. | Firsthand precedent on different hardware, not a Go recipe. |
| Do we have a host stack? | An earlier PSP BTstack prototype compiled but never obtained standard HCI attachment. | Software build evidence only. |
| Does Sony support only DS3 at the hardware level? | Native generic descriptor code and actual Zeemote/GameStick inputs contradict that broad claim. | Own runtime/source evidence, not universal compatibility. |
| Is NES30 missing a parser? | Its captured Bluetooth descriptor passes current parser and adapter tests; PSP stalls before custom descriptor delivery. | Real descriptor with synthetic report tests; no NES30 PSP controls demonstrated. |
| Can a new library add BLE? | Shipped Go is Bluetooth2.0+EDR; BLE needs suitable controller hardware. | Official specification and Android architecture documentation. |
| Was raw HCI takeover found in prior art? | No independently demonstrated Go internal-radio takeover found in targeted searches. | Scoped negative search result, not proof of impossibility. |

## 1. Better hardware identification

CSR's2009 annual report identifies the PSP Go design with BC5120; its October2009 SEC filing independently repeats the designation. This improves on inferring a product solely from a package mark. Sony FCC photography shows an ALPS QZ7-001A module with CSR BC5CE/700CFU/817AA markings; product and package markings describe different layers and need not conflict. Neither identifies every retail revision or establishes a debug-pad map.

Sources: [CSR annual report, printed p12/PDF p15](https://media.corporate-ir.net/media_files/IROL/17/177117/CSR-plc-annual-report.pdf), [CSR SEC filing](https://www.sec.gov/Archives/edgar/data/1368358/000095014209001737/form6k_102909.htm), [Sony FCC internal photographs](https://fccid.io/AK8PN1001A1/Internal-Photos/Internal-Photos-1123145), [iFixit original teardown](https://www.ifixit.com/Teardown/PSP+Go+Teardown/1124).

## 2. A credible HCI lead, with two separate settings

CSR's BlueSuite guide describes unified firmware builds offering different host/on-chip stack arrangements through ONCHIP_HCI_CLIENT. Its USB design note separately describes USB_VM_CONTROL selecting VM-owned USB versus standard HCI USB enumeration. These are separate requirements: advertising an HCI-looking USB descriptor does not prove commands or ACL data will be processed correctly.

The actual Sony firmware may add bootmode overrides, VM behavior, build-time exclusions or activation rules. A manufacturer bootmode note was located, but its direct PDF fetch timed out; bootmode-specific override details should remain a research lead until the text and Sony behavior are verified. No configuration change is justified by the existence of key names alone.

Sources: [CSR BlueSuite guide, section5.3](https://fccid.io/PIWBSMAN/User-Manual/User-Guide-1135316.pdf), [CSR Bluetooth and USB Design Considerations, p35](https://hacker.instanet.net/Bluetooth/CS-101412-AN-4.pdf), [located bootmode note, fetch incomplete](https://www.protonbasic.co.uk/dev/Crownhill%20QuickBlue/bluetooth_project/bluecore/Understanding%20and%20Using%20Bootmodes%20%28bcore-an-019Pd%29.pdf).

A Cambridge/lowRISC firsthand project converted an RN42/BC04EXT from its on-chip SPP application to external HCI/BlueZ, using SPI configuration and BCSP transport. It establishes that this architectural transition is real on a related CSR design. Its chip, wiring, flash layout and settings must not be transplanted to the Go.

Source: [Configure the PMOD-BT](https://www.cl.cam.ac.uk/~jrrk2/docs/pmod-bt/).

## 3. Correction to previous radio-write interpretation

The old RAM-suite parser required a response type that the protocol does not require. SETREQ type2 can legitimately receive GETRESP type1. BlueZ's upstream parser recognizes request0, response1 and set-request2; its write implementation uses command2 through its response path. Consequently the old response-type check cannot establish conversion into a read or a blocked write interface.

The retained responses have protocol-consistent success headers, but independent changed RAM values were not demonstrated. Writes used the existing value; independent RAM readback failed with a Sony outer status whose meaning is unresolved. Default values remaining unchanged after same-value writes cannot settle whether an override was stored.

PSRAM selector8 was correct. Selector zero is not an interchangeable guaranteed-volatile write. Activation and recovery also differ: transient settings may need a warm restart, while cold power loss can erase them. This correction supports careful protocol review, not repeating old writes or changing the radio mode without a recovery design.

Sources: [BlueZ CSR response parser](https://github.com/bluez/bluez/blob/5.50/tools/parser/csr.c), [BlueZ HCI CSR commands](https://github.com/bluez/bluez/blob/5.50/tools/csr_hci.c), [BlueZ PS-store client](https://github.com/bluez/bluez/blob/5.50/tools/bccmd.c), [CSR command manual mirror](https://manualzz.com/doc/24213785/csr-bluecore-electronics-bccmd-commands-user-manual). Local independent audits retain the detailed response and store analysis.

## 4. Reuse existing host and gamepad code

| Candidate | Assessment for this project |
|---|---|
| BTstack | Strongest low-level choice: embedded C, single-thread/run-loop integration, Classic L2CAP/SDP/HID and standard HCI transports. |
| Bluepad32 on BTstack | Strongest gamepad-layer lead: controller setup and parser support for many families, plus a custom-platform interface. Needs a PSP port and real transport. |
| BlueZ | Useful reference and Linux test host; its Linux kernel/userspace integration makes it a poor first PRX port. |
| Android Fluoride | Depends on Android services and a hardware-specific Bluetooth HAL. Importing its code does not supply the missing Go radio interface. |
| Zephyr | Has current Classic/HID-host support, marked experimental; brings additional OS infrastructure. |
| NimBLE | BLE-focused; unsuitable for the Classic HID goal here. |

[BTstack](https://github.com/bluekitchen/btstack/blob/master/README.md) is configurable for small embedded systems. [Bluepad32](https://github.com/ricardoquesada/bluepad32) adds gamepad-specific handling and supports several platforms; its [controller list](https://bluepad32.readthedocs.io/en/latest/supported_gamepads/) distinguishes Classic and BLE protocols. Existing support there is not proof of support through Sony's interface. Bluepad32's own Apache2 license and BTstack's separate noncommercial/commercial terms must both be respected; the combined port must not be relabeled MIT.

Additional primary references: [AOSP Bluetooth architecture](https://source.android.com/docs/core/connect/bluetooth), [BlueZ project](https://bluez.github.io/), [Zephyr Bluetooth architecture](https://docs.zephyrproject.org/latest/services/connectivity/bluetooth/bluetooth-arch.html), [Zephyr Classic configuration](https://github.com/zephyrproject-rtos/zephyr/blob/main/subsys/bluetooth/host/classic/Kconfig), [NimBLE features](https://cwiki.apache.org/confluence/spaces/MYNEWT/pages/144511392/Apache+NimBLE+supported+features), [BTstack license](https://github.com/bluekitchen/btstack/blob/master/LICENSE), [Bluepad32 license](https://github.com/ricardoquesada/bluepad32/blob/main/LICENSE).

The existing v0.11 PSP prototype includes host-stack and PSP adapter code, but was DS4-focused and remained idle awaiting HCI attachment. Its retained ELF measures text109504,data85,bss17360,total126949 bytes, about124KiB. This excludes thread stacks, dynamic allocations and simultaneous game memory needs. It suggests code size is not the demonstrated first blocker; it does not prove the final port fits or works.

## 5. Native alternative and the NES30 evidence

Own source analysis identifies Sony management messages and indexed CDC profile channels. Those are not established arbitrary HCI/ACL or L2CAP PSM operations. An independent PSP-side HID/session layer could reuse framed native HID while retaining radio-side pairing and service establishment, but would still inherit their limitations.

The existing native route already delivers controls for Zeemote and GameStick. NES30 on Linux advertises HID/PnP and supplies an86-byte gamepad descriptor accepted by our parser. On PSP v0.40 the corrected0402 route connects, several service-data receives complete, and one times out before descriptor delivery. Receive count is not attribute count: native service queries loop over fragments. It is not established whether the incomplete transfer is descriptor0206 or another attribute. Subsequent USB threads are useful evidence of continued scheduling, not proof of a successful file transfer or every teardown stage.

The next native-path evidence is service UUID/attribute, fragment status, continuation and length, correlated numerically without raw payloads. Blind retries are inappropriate: the native reply selector identifies connection/event rather than a complete attribute transaction, so late replies could contaminate later queries.

## 6. Prior art and hardware limits

[btfree](https://github.com/PSP-Archive/btfree) removes an early device-class registration filter; it does not supply an independent host stack. [PSP-Bluetooth](https://github.com/PSP-Bluetooth/.github/blob/main/README.md) uses an external ESP32/serial-AV bridge for PSP2000/3000, rather than taking over the Go's internal radio. Neither is a ready internal-radio replacement.

Sony's [official specification](https://sonyinteractive.com/en/press-releases/psp-go-psp-playstationportable-evolves-to-match-the-digital-lifestyle/) lists Bluetooth2.0+EDR. Android itself [documents](https://source.android.com/docs/core/connect/bluetooth) that BLE is not backward-compatible with older chipsets. A new host library cannot turn that hardware into a BLE controller. Broad Classic gamepad coverage is a reasonable goal; all current Bluetooth devices is not.

No validated QZ7 pin map, exact matching radio firmware/restore procedure or independently demonstrated internal Go HCI takeover was found in the sources searched. These are scoped missing evidence, not assertions of impossibility. Manufacturer-family SPI/DFU capabilities alone are not a Go recovery method.

## 7. Recommended work and measurable acceptance criteria

1. Correct the retained protocol interpretation and keep the known native driver baseline separate from transport experiments.
2. Investigate exact Sony boot configuration, effective overrides and command handling through evidenced read interfaces; establish exclusive ownership and a recovery mechanism before any mode transition.
3. Prove a matching standard HCI Read Local Version Information command/completion. Prefer read-only identification before Reset. USB driver registration or changed descriptors alone does not pass this criterion.
4. Prove bidirectional ACL traffic, packet boundaries, credits and completion flow control separately.
5. Reuse the existing BTstack PSP work, evaluate Bluepad32's Classic gamepad layer, and demonstrate authenticated pairing plus HID control/interrupt channels independently of Sony's profile manager.
6. Validate descriptor fragmentation and input, then disconnect, Bluetooth disable/USB entry, game entry/exit, HOME and return to VSH. A replacement stack still needs correct PSP lifecycle integration.

Native SDP investigation can proceed in parallel and may solve NES30 sooner. No new PRX, flash modification or user hardware test is requested by this report. No compatibility or completion date is promised before the transport boundary is demonstrated.
