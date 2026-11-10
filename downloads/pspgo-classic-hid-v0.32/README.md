# PSP Go Classic HID v0.32 — complete thread diagnostics

Diagnostic build, not a confirmed fix. GameStick controls work, but active game entry/exit still hangs.

The v0.31 capture showed continued heartbeats through sample29 and HID/input workers waiting on event flags. No teardown begin/end markers were recorded. Its thread-name shortlist omitted other tasks; it cannot establish the transition wait or rule out a block elsewhere. The last new boot in that log had not yet recorded controller input.

This version keeps v0.31 input and passive teardown observers unchanged and expands read-only status capture to all enumerated threads (bounded128), including module/exit/UI workers regardless of name. States log on change and every10seconds, sampled every2seconds for up to6minutes after first valid controller input. It never suspends or wakes threads. No stack/memory dumps or raw input, PIN, link key or Bluetooth address logging. Cache memory is static, not added to native callback stacks.

Replace ef0:/seplugins/pspgo_hci_transport.prx, keep VSH/GAME entries and saved registration, and fully restart. Keep NES30 off. Connect GameStick, wait5seconds, then attempt game entry with it on. If it hangs, leave10seconds before recovery. Send pspgo_hci_transport.log afterward. No re-pairing required. The known workaround remains controller off for transition, then on after the game loads.

LIFE_EVENT phases1/2=context-removal begin/return;3/4=node termination begin/return; kind1 keyboard,2 mouse,3 gamepad. LIFE_THREAD contains status/wait IDs; LIFE_SEMA/LIFE_FLAG describe readable wait objects. Missing events alone do not prove a function was not entered: file logging or reboot can stop first. Hardware success has not been established.

Own source and tests included under MIT; SDK notices retained. No Sony firmware or private device logs distributed.
