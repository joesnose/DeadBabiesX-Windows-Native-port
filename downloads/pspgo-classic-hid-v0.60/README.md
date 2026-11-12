# PSP Go Classic HID v0.60 — Sony warm-reboot handover recovery

Targets DS4/DualSense losing input and powering off after VSH↔GAME transitions. This is a hardware test candidate; successful compilation and software tests cannot verify a real controller transition.

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep your existing controls text, target file and ARK entries, then fully restart. No EBOOT or new configuration is needed. The included controls template is unchanged; preserve your customized file. One log: `ef0:/seplugins/pspgo_hci_transport.log`.

Test game entry and HOME → Exit Game with the controller on. Confirm its light stays on and input resumes in both contexts. If it fails, the same log records startup version, preservation installation and restoration eligibility/result. No device addresses, keys or input contents are logged by the new recovery path.

v0.59 preserved the old context's reboot channel, but native startup restores HID before making the public device-record API ready. The new plugin missed the initial link event and could not authorize its local Sony metadata. Native SDP then failed and disconnected the controller. This build adds an owned-worker recovery path during that startup interval.

Recovery requires the remembered target, a live native restored HID entry/event, the matching native HID backend, and a fresh read of the current registration. It converts the native reply to the exported record ABI, checks the exact saved fingerprint/name and current cached address, then rechecks the native link/event/backend and selection for races. Only then does it authorize local metadata and schedule the existing native Sony link setup. Disconnect events still invalidate the link normally. Automatic reconnect is suppressed while this verified restored profile is starting.

The private native registration reader and table layouts have24 instruction/relocation guards. It is read-only, worker-only and limited to3 attempts; no registration/authentication state is written. Failed checks preserve the native behavior. The native command path retains its existing timeout handling. No added file, radio reset, pairing bypass or report callback query is used. Manual disconnect, USB and suspend remain native; v0.59's guarded reboot-only preservation remains.

Forced PSP SDK build,35 sanitizer host programs and compiled checks pass. New tests cover reply/export layout, both Sony names, fingerprint/address/event/backend changes, disconnected state, local metadata eligibility and invalid replies. Hardware transitions remain unverified. Source includes only our code and numeric ABI guards, with no Sony firmware or private captures.
