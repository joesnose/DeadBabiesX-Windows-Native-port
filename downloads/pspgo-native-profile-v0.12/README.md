# PSP Go native HID profile fix v0.12

This build changes connection behavior to correct a demonstrated profile-mode
mismatch. It uses Sony's existing radio stack and HID services; it does not wait
for standard HCI or attempt a radio-mode switch. Actual DS4 connectivity and
controls remain unverified until tested on the PSP.

## Corrected behavior

The supplied earlier logs show a saved HID profile0402 but Connect was called
with mode0, causing requestflags0202. Our menu patch had exposed a PS3 action for
a generic HID device while retaining its PS3 mode. The official UI distinguishes
type1 PS3 controller from type2 Bluetooth HID, and the native API maps mode0 to
0202 and nonzero mode to0402. Native HID setup also distinguishes those paths.

Before a mode0 Connect request, this build reads a fresh native80-byte profile
for that slot. Only a successful, correctly sized record whose ID matches the
requested slot and whose flags equal0402 selects mode1. Explicit nonzero modes,
PS3/unknown/mixed flags, invalid records and failed queries preserve the caller's
mode. One native connection call is made; no fallback or automatic retry occurs.
Its real result is returned unchanged. Profiles0402 are generic HID capability,
not sufficient identity proof that a selected device is a genuine Sony DS4.

No BCCMD queries, PS-key writes, radio resets, address changes or key imports are
performed. Normal user pairing/menu operations still affect saved registrations.
The guarded discovery/menu fixes and corrected connection-event tracing remain.

## Install and try the connection

1. Replace `ef0:/seplugins/pspgo_hci_transport.prx` with this PRX. Keep its single
   enabled VSH entry. Disable older `pspgo_ds4_pairtest`, diagnostic and btfree
   entries so two plugins do not patch the same functions. Keep all marker files.
2. Fully power off/on to replace resident code. Enable Bluetooth and disconnect
   the DS4 USB cable. Put the DS4 in pairing/discovery mode with SHARE+PS.
3. Open Manage Bluetooth Devices and select Connect for the already registered
   controller. No Linux helper, host-address rewrite or new registration is
   required for this first attempt. If a PIN is requested, the known legacy DS4
   driver default is0000, with revision-dependent compatibility.
4. Check whether buttons work. If connection still fails, send
   `ef0:/seplugins/pspgo_hci_transport.log`; the new section identifies native HID
   profile0.12 and records selected_mode1 when the0402 policy applies.

If selection preserves mode0 because the profile is different/unavailable, the
log explains why. A mode change alone does not guarantee authentication or
correct report decoding. The prior802F0140 event did not expose an HCI reason,
so it must not be relabeled as a confirmed authentication rejection.

The native PIN-registration ABI and0000 default have been verified in the UI;
no speculative registration redirect is included. Address-only registration
lacks the genuine discovery identifier needed to redirect safely. The prior
USB helper's zero key does not establish a complete bond on the PSP.

## Verification and licensing

PSP cross-build passed with strict warnings using pinned SDK image
pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b.
Six native test executables pass ASan/UBSan, including executable MIPS menu
routing/guards, corrected event lengths and the production profile-query/connect
helper. Every16-bit profile flag combination is checked; failed/stale/mismatched
records preserve mode, and connection errors propagate without retry. A negative
control restoring mode-zero behavior fails the regression as expected. These
software checks are not hardware pairing or gameplay evidence.

Original adapters MIT; discovery technique credited to JJS/Yoti btfree and its
ARK continuation. PSP SDK notice included. Source/tests/original research notes
included; no Sony firmware, disassembly, private user logs or controller keys.
Historical docs describe older builds; this README defines current behavior.
