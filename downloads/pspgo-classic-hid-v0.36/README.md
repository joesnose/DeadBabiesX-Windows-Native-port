# PSP Go Classic HID v0.36 — callback-mode exit acknowledgement

Test fix; game exit is not yet confirmed. Keep v0.34 as the user-confirmed launch fallback.

v0.35 still hangs on exit. Its latest GAME capture identifies SceBSMan_Seq held once by sleeping main SceBSMan, with the launch thread waiting. GameStick identity was authorized; no completed main-thread timeout was logged. Other earlier sessions in the accumulated file include a distinct HIDSvc lock wait, so this must not be called a universal lifecycle fix.

Source review finds a concrete gap in v0.35: its main-thread ACK timeout excludes callback mode1, but native main-thread handlers call the ACK receiver in that mode (for example reply016e). The log does not expose which ACK is pending, so this example is not a confirmed command identification.

v0.36 preserves the confirmed worker0185 timeout and extends main-thread ACK0100..01ff timeout eligibility to callback modes0 and1, passing the original mode unchanged. Authorized GameStick0f0d:1011, native thread ID, no existing timeout and all11 instruction/module/branch guards are still required. Worker behavior remains callback mode0 only; unknown callback values, threads, replies and other controllers are unchanged. Native timeout alarm/waiter cleanup/error return still performs the unwind, with no forced mutex release or thread termination.

The wrapper now records a two-slot bounded RAM snapshot before and after each native worker/main ACK wait. The existing8192-byte logger prints STATUS_CALL serial, role1worker/2main, numeric reply, callback mode, target eligibility, prior timeout, bounded, done and result. No packet payloads or pointers are logged. A done0 record observed by the worker identifies an in-flight call, but file logging can stop and fast calls can overwrite snapshots; it is not a complete command history. No filesystem calls occur inside the native wrapper.

Replace ef0:/seplugins/pspgo_hci_transport.prx, retain VSH/GAME entries and saved registration, fully restart, keep NES30 off. Connect GameStick, enter a game and HOME/quit with it on. Allow up to15seconds for native retries. If VSH returns, check input/reconnection. Send pspgo_hci_transport.log with result. No re-pairing. Controller off during transitions remains the known workaround.

Pinned PSP build,17 ASan/UBSan programs and compiled stack/once-only forwarding checks pass. Policy tests cover all65536 reply IDs and32 timeout/callback/identity/role combinations plus invalid callback modes. These tests do not establish hardware success or every indirect native stack bound. Own MIT source/tests and SDK notices included; no Sony firmware/disassembly, private raw logs, Bluetooth addresses or keys distributed.
