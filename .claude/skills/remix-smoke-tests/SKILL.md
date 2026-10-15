---
name: remix-smoke-tests
description: Run headless smoke tests of the Smash Remix port (BattleShip-Remix) and read their results: VS matches with chosen fighters, stages and items, special-move matrices, full stage sweeps, CPU item sweeps, the CSS/stage-select flow, contact sheets of screenshots, crash backtraces and lldb inspection. Use whenever a Remix change needs verification in the running game (ROADMAP tasks, "teste", "smoke", "sweep", "roda o jogo", "confere em partida"), before ticking a task or committing a checkpoint.
---

# Remix smoke tests

Verification by running the real game headless with environment overrides, screenshots and the port log. Every task of
`docs/remix/ROADMAP.md` is accepted only after the test it names passes here.

## Before running

1. Build: `PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j 4`
   (absolute paths; a new file under `decomp/src` needs `cmake -S $PWD -B $PWD/build` first). Run Bash with
   `dangerouslyDisableSandbox: true` (the game needs the window server).
2. **Never rebuild while a sweep runs**: the sweep keeps launching `build/BattleShip-Remix`.
3. Runs share `~/Library/Application Support/BattleShip/ssb64.log`, so run sweeps one at a time (the scripts copy the log
   into the run folder after each run).
4. Long sweeps (stage sweep about 10 s per stage, item sweep about 40 s per item) go in the background
   (`run_in_background`), then wait with Monitor on the `*_DONE` line; do not poll with sleep.

## Scripts (`.claude/skills/remix-smoke-tests/scripts/`)

| Script | What it does |
|---|---|
| `vs.sh <fkinds> <input> <name> [frames]` | VS shortcut (scene 22): P1 human, others CPU, Dream Land unless `SSB64_VS_STAGE`. Prints `rc`, screenshot count and `metal_flake`. |
| `special_matrix.sh <fkind>...` | NSP/DSP/USP every 150 frames vs Kirby for each fighter, then greps the log for abort/assert/fatal/unhandled. |
| `stage_sweep.sh [first] [last] [prefix]` | Every stage id (default 41..215), 480 frames, one screenshot at 450. |
| `hazard_sweep.sh <stage>...` | Three fighters on each stage for 4000 frames, a shot every 150 frames, one 5x5 sheet per stage (`hz_<stage>/sheet.png`). |
| `item_cpu_sweep.sh [kind]...` | Mario idle + Kirby + Yoshi CPUs, every spawned item forced to `<kind>`, 3000 frames, item event counts. |
| `css.sh <input> <name> [frames]` | Character select (scene 16) flow driven by an input script. |
| `sheet.py <name> [--step N] [--max N] [--crop x0,y0,x1,y1]` | Contact sheet `build/agent-shots/vs/<name>/sheet.png` (run with `.toolchain/host/venv/bin/python -I`). |

Outputs: `build/agent-shots/vs/<name>/` with `frame_<n>.png`, `stdout.log`, `ssb64.log`, `save.bin`.

Input scripts (`inputs/`): `none.txt` (no input), `nsp_tap.txt`/`dsp_tap.txt`/`usp_tap.txt` (walk right, then the special
every 150 frames from frame 420), `css_pick_mario.txt` (both ports pick Mario, START at 200, A on the stage select at
300), `css_page3_stage.txt` (same, then R 15 times to reach page 3 of the stage select and A at 600). Line format:
`<from> <to> <port> <buttons|-> [stick_x stick_y]`. Do not press A every few frames to pick an item up: the fighter stays
locked in its jab combo; use the CPU item sweep instead.

## Environment overrides

| Variable | Effect |
|---|---|
| `SSB64_START_SCENE=22` / `16` | Boot into the VS battle / the VS character select |
| `SSB64_VS_FKINDS=a,b[,c,d]` | Fighters of the VS shortcut (decimal port fkinds; Remix id = fkind; vanilla 0..26, Remix 0x1D..0x60) |
| `SSB64_VS_STAGE=<decimal>` | Stage id (vanilla 0..0x28, Remix 0x29..0xD7; names in `decomp/src/gr/grremix/grremixids.h`) |
| `SSB64_VS_ITEMS=<1..5>` | Turns every item switch on with that rate (0 means no items) |
| `SSB64_REMIX_FORCE_ITEM=<kind>` | Every stage-spawned item becomes `<kind>`, with a 60-tick spawn wait |
| `SSB64_REMIX_FORCE_ITEM_AT_P1=1` | The forced item drops on player 1 instead of a random item point |
| `SSB64_REMIX_ITEM_LOG=1` | Logs `[remix-item] spawn|hold|consume|touch|drop|throw|bullet kind=0x..` |
| `SSB64_INPUT_SCRIPT=<file>` | Controller script (format above) |
| `SSB64_MAX_FRAMES=<n>` | Quit after n frames |
| `SSB64_SCREENSHOT_FRAMES=a,b,...` / `SSB64_SCREENSHOT_DIR` | Screenshots (the scripts set them from `SHOTS`) |
| `SSB64_DUMP_FILE_ID=<id>` | Writes the decompressed reloc file to `build/debug_traces/port_file_<id>.bin` before its byte swap |

## Reading the results

- `rc=0` and the expected screenshot count: the run finished. `rc=139`/`138`: crash. A crash with `metal_flake` > 0
  (`CAMetalLayer nextDrawable` in `stdout.log`) is a macOS flake: rerun it alone; only a reproducible crash is a bug.
- Crash location: `grep -m1 -A8 "main-thread backtrace" build/agent-shots/vs/<name>/ssb64.log`. The log is buffered, so
  the last lines before a crash can be missing; `stdout.log` has the spdlog errors (`chain walk STOPPED`,
  `invalid/stale token`, `has no port`).
- Errors without a crash: `grep -ciE 'abort|assert|fatal|unhandled|unexpected' .../ssb64.log`.
- Look at the screenshots, do not trust `rc` alone: build a sheet and Read it. For a fast event, take dense shots
  (`SHOTS=$(seq -s, 440 4 540)`) and crop (`--crop`).
- Blank-screen check for a stage sweep: PIL `ImageStat` mean/stddev under 12 on a 64x48 thumbnail.

## lldb (system lldb, no install)

Static functions can be inlined: break by file and line, and run the game with the same env as the script:

```
breakpoint set -f itremixbasketball.c -l 851 -i 400      # or -n <exported function>
process handle SIGSEGV -s true -p false                  # to stop at a crash
run
bt 6
p *gMPCollisionGeometry
quit
```
`SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,8 SSB64_MAX_FRAMES=2000 timeout 150 lldb -b -s <cmds.txt> ./BattleShip-Remix`
from `build/`, then filter the output with `sed -n '/stop reason/,$p'`.

## Reporting

Record the evidence next to the task in `docs/remix/ROADMAP.md` (run name, stage id, event counts or what the
screenshot shows). A significant bug fixed along the way gets a `docs/bugs/<topic>_<date>.md` entry and an index row.
