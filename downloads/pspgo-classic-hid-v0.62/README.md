# PSP Go Classic HID v0.62 — quiesce old input reader on preserved reboot

Hardware test candidate for the v0.61 freeze when exiting a game. A real PSP exit has not been verified with this build. v0.60 remains the fallback: game entry worked, but the controller disconnected on exit. v0.61 has a confirmed hardware freeze/crash report and should not be used.

Replace `ef0:/seplugins/pspgo_hci_transport.prx`, keep your controls text, target file and ARK entries, then fully restart. One plugin, same log, no EBOOT, new configuration or pairing reset. The included controls template is unchanged; preserve your custom file.

The latest v0.61 log shows successful startup recovery (`public_api=0`), native link setup and working GAME input. It does not establish a post-startup public-query fault or identify the exact crashing instruction. The freeze happens on exit after the handover is armed.

The inspected native HID input pump continues reading until it sees empty; native service stop waits for its report thread to finish. Preserving an active connection without stopping the old reader can keep reports flowing through that loop. This build tests that shutdown-starvation mechanism.

Immediately before omitting the guarded reboot-only Sony disconnect request, the plugin marks that exact owned input context quiesced. Its old read wrapper then returns empty without touching the buffer, calling the native read, freeing queues or changing Bluetooth authentication/link state. This lets the native input pump return to its event loop so native service stop can join it. A new plugin generation starts unquiesced. All other input contexts remain unchanged.

Preservation requires a genuinely live selected Sony context, exact native backend, plugin-owned Sony node and installed guarded read wrapper. If quiescing cannot be armed, the original native disconnect request runs once. Quiescing happens only through the existing committed reboot-entry path. Normal USB, suspend and manual disconnect remain native. No firmware flash writes or thread termination are added.

Test entering a game, then HOME → Exit Game with the controller on. Check that VSH returns without freezing and whether the controller stays on/works. If it fails, use the same `ef0:/seplugins/pspgo_hci_transport.log`.

Forced PSP build,35 sanitizer host programs and16 compiled checks pass. New tests cover a continuously reporting native backend, no buffer/queue consumption after quiesce, owned/unowned context isolation, teardown persistence and once-only native fallback when quiesce fails. Compiled quiesce frame16 bytes; read wrapper24 and transition callback32. Hardware success remains unverified.

Own source only; no Sony firmware, private captures, device addresses or keys included.
