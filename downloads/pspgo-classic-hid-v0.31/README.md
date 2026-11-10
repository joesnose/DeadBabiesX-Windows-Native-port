# PSP Go Classic HID v0.31 — transition diagnosis

This is a diagnostic build, not a claimed game-transition fix. The user reports
v0.30 still hangs. It returns to the v0.28 working GameStick input baseline,
removing both v0.29 auxiliary suppression and v0.30 drain-stop changes. No
native wait/timeout, radio reset, forced cleanup or new pairing behavior.

## One test

1. Keep v0.28 as rollback. With controllers off after a full restart, replace
   `ef0:/seplugins/pspgo_hci_transport.prx`, keep VSH/GAME entries enabled and
   fully shut down/restart again. Keep the existing GameStick registration.
2. Connect GameStick in VSH and wait5seconds for the thread snapshots to arm.
   Keep NES30 out of this test.
3. Try entering a PSP game with GameStick on. If using the known controller-off
   boot workaround, you can instead turn it on in-game and test exit. One
   failing transition is enough.
4. If it hangs, leave it10seconds so any running logger can capture wait states,
   then use the normal full-restart/controller-off recovery and send
   `ef0:/seplugins/pspgo_hci_transport.log`. Until USB behavior is established,
   use a full shutdown/restart with controllers off before USB file transfer.

Newest banner is Classic HID clean-baseline route0.31. Do not delete/re-pair
GameStick. If this diagnostic changes the symptom, report that too; timing
changes are not by themselves a confirmed fix.

## What is recorded

Native HID context-removal and keyboard/mouse termination are wrapped with
numeric begin/end markers, forwarding original arguments and exact result.
The existing adapter-owned gamepad termination is also marked. Native cleanup
is called exactly as before; no allocation/resource state is skipped.

```
LIFE_EVENT phase=1 kind=0 ...   context removal begin
LIFE_EVENT phase=2 kind=0 ...   context removal returned
LIFE_EVENT phase=3 kind=1 ...   keyboard node termination begin
LIFE_EVENT phase=4 kind=1 ...   keyboard node termination returned
```

Kind2 is mouse, kind3 is gamepad. A begin without its return can locate a
suspect call, but not conclusively if logging stopped or entries were dropped.
The queue is32 entries, records drops, and drains only on the owned logger.
No file operations occur inside these native teardown observers.

After first validated input, the8192-byte plugin worker takes a snapshot every
2seconds for up to6minutes. It logs heartbeat, known HID/Bluetooth/input/module
manager thread status, wait type/UID and stack size. Changed states and10-second
refreshes include read-only semaphore/event-object names, counts or patterns
where the kernel API accepts the wait UID. It does not suspend/wake threads,
change semaphore/event state, dump memory/stacks, or log addresses/PIN/keys/raw
buttons. Thread names are restricted to known kernel components.

This should distinguish a native teardown callback from a transport/input
thread waiting elsewhere. Reboot may stop the worker or filesystem before a
final marker flushes, so absence alone is not proof of which call ran.

## Preserved behavior and verification

GameStick composite report3 routing and descriptor decoding, native keyboard/
mouse init/report handling, Zeemote behavior, VSH/GAME/HOME input hooks, stock
pairing selection and Zeemote-only automatic reconnect remain from v0.28.
Generic game-launch reconnect and NES30 shutdown work are not added here.

All16 ASan/UBSan programs pass, including exact native observer calls/args/
results and nine new no-write native context guards. Pinned PSP cross-build,
report-stack, pairing ABI and lifecycle compiled checks passed. Native
context-remove observer frame16, keyboard/mouse observers24 each; they call
only original native target and IRQ metadata helpers. Worker profiler frame864
within its8192-byte stack. Main report callback remains56bytes. These checks
are not complete indirect firmware stack proof or a hardware transition fix.

Build image:
`pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b`.
Run `sh tests/run.sh` and the packaged compiled-check scripts after building.
Original MIT source/tests, SDK notice and factual notes included. No Sony
firmware/disassembly, private raw logs, addresses or keys distributed. Older
notes are historical; this README defines v0.31 diagnostic behavior.
