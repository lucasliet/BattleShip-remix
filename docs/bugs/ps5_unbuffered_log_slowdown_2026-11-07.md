# PS5: unbuffered logs to /data slowed the game 20-50x (2026-11-07)

## Symptom

The PS5 title booted and passed the title screen, but every frame took 700-950 ms (single frames up to 54 s), the
menu took minutes to appear and a 250 KB reloc file took 19 s to load.

## Investigation

- A startup micro-benchmark showed the console itself was fast: 100M multiply-adds in 115 ms, 102400 malloc/free
  pairs in 16 ms, 1 ms nanosleeps on time, 16 MiB of `fread` from `/data` in 6.6 ms.
- Phase timing in `PortPushFrame` put the time in `port_resume_service_threads` (game logic); rendering, ImGui and the
  idle present were under 10 ms.
- A SIGUSR2 sampling profiler on the main thread (2 ms interval; the RIP slot of the console's ucontext is word 28,
  found by interrupting a known spin loop) put ~90% of the samples in one libkernel/libc write path, the same chain
  spdlog's `file_helper::flush` uses.

## Root cause

Both logs wrote every line straight to `/data`: `port/port_log.c` set `ssb64.log` to `_IONBF`, and the PS5 spdlog
logger in `libultraship/src/ship/Context.cpp` flushed on every `info` line. Each write to `/data` (ShadowMountPlus
nullfs) costs tens of milliseconds on the console.

## Fix

- `ssb64.log` is fully buffered (64 KiB).
- The PS5 spdlog logger flushes on `err` only.
- Crashes still land in the boot trace (`port/ps5/ps5_crash_trace.cpp`, open/write per line, written only at boot
  steps and on a fatal signal).

Applied together with the Switch's low-pointer rule in `gfx_vtx_addr_is_unresolved`, which now covers `__PS5__`
because `dladdr` is a stub there and heap pointers sit below 4 GB.
