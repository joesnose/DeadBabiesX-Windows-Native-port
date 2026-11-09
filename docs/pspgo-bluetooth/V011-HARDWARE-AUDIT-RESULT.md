# v0.11 hardware audit result

Date: 2026-11-09. Original summary; raw device log is not redistributed.

All 16 allowlisted GETs completed: 14 parsed values and two native outer errors. No inner BCCMD errors were recorded. The driver then waited for HCI attachment; no attachment or DS4 connection was observed.

| Observation | Returned result |
| --- | --- |
| Build | 0x1918 |
| Store-8 memory type | 2 |
| BCCMD_SECURITY_ACTIVE, default view | 0 |
| INITIAL_BOOTMODE, default view | 1 |
| PS_Size, keys 03C0 and 03CC, requested stores 0/1/4/8 | 1 word in each case |
| PS_Next after 03BF and 03CB, store 8 | No successor (zero) |
| PS value reads of 03C0 and 03CC, store 8 | Native outer 0050; no usable value |

The documented BCCMD security flag is reported off. This is not evidence that every Sony or Bluetooth security policy is disabled. Initial bootmode 1 is recorded without assigning its Sony-specific meaning.

The size results cannot establish RAM entry presence. CSR bcore-sp-005Pe section 10.6 mentions a zero request Length and a Stores parameter, while its figure does not clearly show that parameter. BlueZ 5.50 sends the requested store in the field later read as size, which the audit followed. This documentation/implementation ambiguity must remain explicit; neither a schema-size interpretation nor actual per-store presence has been established.

PS_Next has a documented key/store/next-key layout. Its zero result after 03BF reports no higher key in the requested view, which includes both target mode-key numbers. It does not prove the whole RAM store is empty or establish how Sony's radio-side handler implements that view. Together with the failed direct reads, it provides no confirmation of current target-key RAM entries.

A cold-start observation cannot retrospectively establish earlier volatile write effects. No security-key change, mode switch, guessed command or repeated write is justified by this result alone. The exact outer0050 meaning remains unassigned.

Sources: [CSR manufacturer manual mirror, sections 10.4/10.6](https://manualzz.com/doc/24213785/csr-bluecore-electronics-bccmd-commands-user-manual), [BlueZ 5.50 bccmd implementation](https://github.com/bluez/bluez/blob/5.50/tools/bccmd.c).
