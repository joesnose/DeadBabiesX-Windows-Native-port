# PSP Go Classic HID v0.59 — Sony reboot handover test

This candidate targets DS4/DualSense powering off on VSH↔GAME transitions. It has not been verified on hardware.

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, retain your existing controls text file and ARK entries, then fully restart. No EBOOT is needed. The same `ef0:/seplugins/pspgo_hci_transport.log` is used. The included controls file is the unchanged v0.58 template; keep your custom file.

Test entering a game and HOME → Exit Game with the controller powered on. Check whether the light stays on and controls resume. If it still fails, the same log is sufficient for the next investigation.

The native HID reboot walker sends operation6 (disconnect) to non-DS3 contexts. This build omits that one request only while the actual reboot-entry wrapper is running on its own thread, and only for the selected, genuinely linked Sony context with an adapter-owned input node and exact native backend. Both patch sites are guarded before installation. Suspend uses the same walker but remains native; USB, manual disconnect, native context cleanup and radio shutdown are unchanged. This does not guarantee other firmware transition paths preserve the radio link or that the new module receives restoration events.

The patch does not identify a DS4/DualSense as a DS3. Normal native errors and nonmatching control requests are forwarded once. No storage I/O, allocation or new Bluetooth command is performed by the teardown wrapper. Controls and input decoding are unchanged.

Own source is supplied separately. No Sony firmware, device addresses, keys or private captures are included.
