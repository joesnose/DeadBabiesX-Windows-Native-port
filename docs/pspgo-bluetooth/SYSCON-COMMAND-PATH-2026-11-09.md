# PSP Go Bluetooth: command path and recovery findings
Date: 2026-11-09

Follow-up to [hardware research](HARDWARE-RESEARCH-2026-11-09.md).

## Result
Static comparison of the private official 6.61 Syscon module with published uOFW confirms separate normal-power, override, and reset paths. Sony BSMan coordinates these paths with its service state. Calling a power export and receiving success is insufficient evidence that the radio was restarted.

This research has not demonstrated a successful radio RAM write, standard HCI attachment, Bluetooth authentication, or usable DS4 controls.

## Command boundary
The examined BSMan request path copies a native message ID, length, and payload into its transmit packet. The examined CDC sender submits a USB vendor/interface control request (bmRequestType 0x41, bRequest 1). No explicit BCCMD-type rewrite was found in these examined PSP functions.

This narrows the investigation; it does not establish where the observed GET-style reply is produced. Radio-side handling remains unresolved. Request 0x71 must not be inferred to be a write command merely because the existing read route uses 0x70.

The native wrapper's third argument selects callback-aware waiting; it is not a timeout. Consequently, changing that argument would not provide a recovery deadline.

## Power and reset findings
Offsets below are relative to the examined official 6.61 Syscon module, not universal patch addresses.

| Offset | Finding |
| --- | --- |
| 0x36B4 | Normal Bluetooth power control uses Syscon command 0x53. While the override state is active, it returns zero without sending that command. |
| 0x315C | Override entry chooses a separate path for Baryon version-byte values 0x30 and 0x31. |
| 0x406C | That path coordinates commands 0x29 and 0x2A and maintains override state; it is not equivalent to a simple normal-power call. |
| 0x4150 | Helper constructs command 0x29 and checks the reply length. The physical meaning and units of its parameters remain unassigned. |
| 0x2A88 | ResetDevice encodes a target and mode through command 0x32. |

The published uOFW implementation corroborates these command families. A decompiler omitted an argument at one call site, so its rendered function prototype alone must not be used to implement the sequence.

In the captured BSMan code:
- The examined shutdown path waits for service activity, then uses ResetDevice(0x20, 1) before normal power-off.
- The examined resume callback uses normal power-on followed by ResetDevice(0x20, 0), subject to its state checks.
- A separate worker uses the override entry, a delay, switch override, queued work, event waiting, and cleanup. Its purpose is not sufficiently established to expose it as a general reset API.

Target 0x20 is associated with Bluetooth in these call sites. Its electrical effect, reset polarity, and ability to restore a changed radio configuration have not been established. No such calls were executed during this analysis.

## Consequences for implementation
A future recovery mechanism needs explicit ownership of Sony's Bluetooth service, bounded handling of outstanding USB requests, and independently observed radio disappearance/reappearance and readiness. Cached power state and a zero return value are inadequate confirmation.

The previous v0.9 hardware results remain unchanged: all four attempted writes produced GET-style replies, immediate RAM reads returned outer status 0x0050, and baseline values stayed unchanged. Repeating those write forms is not justified by this analysis.

The next implementation gate is locating a supported radio-side command route or obtaining sufficiently documented debug access. Only after actual standard HCI events are observed can pairing and HID support be validated. Standard HID support cannot be claimed from a successfully built PRX.

## Sources and limits
- [uOFW Syscon implementation](https://github.com/uofw/uofw/blob/master/src/syscon/syscon.c)
- [uOFW Syscon declarations](https://github.com/uofw/uofw/blob/master/include/syscon.h)
- [Hardware identification, FCC sources, and CSR documentation](HARDWARE-RESEARCH-2026-11-09.md)

The official-module comparison and device captures are retained privately. This document contains original analysis and factual identifiers, not firmware, disassembly, or user logs. Static findings are distinguished above from hardware observations.
