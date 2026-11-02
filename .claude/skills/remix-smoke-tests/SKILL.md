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
3. Every run writes its own log and save (`SSB64_LOG_PATH`, `SSB64_SAVE_PATH`), so game batches run in parallel with
   `parallel.sh`; only validation by the lead counts (workers never run or sign off a batch).
4. Prefer `parallel.sh 6 ...` for sweeps (about 6x faster); the serial scripts stay for one-off runs. Never run two sweeps that write the same run name at once.
5. Long sweeps (stage sweep about 10 s per stage, item sweep about 40 s per item) go in the background
   (`run_in_background`), then wait with Monitor on the `*_DONE` line; do not poll with sleep.

## Scripts (`.claude/skills/remix-smoke-tests/scripts/`)

| Script | What it does |
|---|---|
| `vs.sh <fkinds> <input> <name> [frames]` | VS shortcut (scene 22): P1 human, others CPU, Dream Land unless `SSB64_VS_STAGE` (needs at least two fkinds; one fkind ignores the stage). Prints `rc`, screenshot count and `metal_flake`. |
| `special_matrix.sh <fkind>...` | NSP/DSP/USP every 150 frames vs Kirby for each fighter, then greps the log for abort/assert/fatal/unhandled. |
| `stage_sweep.sh [first] [last] [prefix]` | Every stage id (default 41..215), 480 frames, one screenshot at 450. |
| `hazard_sweep.sh <stage>...` | Three fighters on each stage for 4000 frames, a shot every 150 frames, one 5x5 sheet per stage (`hz_<stage>/sheet.png`). |
| `item_cpu_sweep.sh [kind]...` | Mario idle + Kirby + Yoshi CPUs, every spawned item forced to `<kind>`, 3000 frames, item event counts. |
| `stage_select_sweep.sh [gkind...]` | Stage select opened on each of the 85 selectable stages (`SSB64_MAPS_START_GKIND`), screenshot at frame 300 into `ss_<gkind>/`. |
| `parallel.sh <jobs> matrix\|stages\|items\|hazards\|custom <args>` | The sweeps above with several game instances at once (own log and save per run through `SSB64_LOG_PATH` / `SSB64_SAVE_PATH`). 6 jobs ran 6 matrix runs in 19 s, against 19 s per run serially; 8 jobs gave no further gain on 11 cores. Windows still open in front, so keep the machine free. |
| `parallel.sh <jobs> custom <file>` | One shell command per line of `<file>` (each a `vs.sh` or `css.sh` call with its own env), run `<jobs>` at a time, e.g. the Kirby copy batch. A run with `shots=0 metal_flake=1` is the Metal flake: rerun it alone. |
| `css.sh <input> <name> [frames]` | Character select (scene 16) flow driven by an input script. |
| `sheet.py <name> [--step N] [--max N] [--crop x0,y0,x1,y1]` | Contact sheet `build/agent-shots/vs/<name>/sheet.png` (run with `.toolchain/host/venv/bin/python -I`). |

Outputs: `build/agent-shots/vs/<name>/` with `frame_<n>.png`, `stdout.log`, `ssb64.log`, `save.bin`.

Input scripts (`inputs/`): `soundtest_browse.txt` / `soundtest_wrap.txt` (with `SSB64_START_SCENE=59`, drive the Sound Test), `grab_walk` was dropped (CPUs do not reach grabs reliably), `none.txt` (no input), `nsp_tap.txt`/`dsp_tap.txt`/`usp_tap.txt` (walk right, then the special
every 150 frames from frame 420), `css_pick_four.txt` (four ports pick, START at 200, A at 300; run it with `css.sh` and
`SSB64_REMIX_MODE=<mode>` to play a 4-player match of each Remix mode), `training_oos_platform.txt` (Training with `SSB64_TRAIN_FKINDS=0,8 SSB64_VS_STAGE=6 SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3"`: Mario climbs to the Dream Land left platform where the shielding CPU stands and jabs it; the log shows `training oos`), `training_css_pick.txt` (with `SSB64_START_SCENE=18`: the 1P hand climbs to the Mario portrait, A, START, A on the stage select; the log shows `training port` lines for the picked fighters), `training_tech_platform.txt` (same Training setup with `Tech=<1..5>;ShieldBreak=0;Percent2=30`: Mario climbs to the left platform and fsmashes the CPU right, it lands on the floor and the log shows `training tech ... teched 1 status N`; Roll Left 73, Roll Right 74, In Place 81), `css_pick_mario.txt` (both ports pick Mario, START at 200, A on the stage select at
300), `css_page3_stage.txt` (same, then R 15 times to reach page 3 of the stage select and A at 600). Line format:
`<from> <to> <port> <buttons|-> [stick_x stick_y]`. Do not press A every few frames to pick an item up: the fighter stays
locked in its jab combo; use the CPU item sweep instead.

## Environment overrides

| Variable | Effect |
|---|---|
| `SSB64_START_SCENE=22` / `16` | Boot into the VS battle / the VS character select |
| `SSB64_VS_FKINDS=a,b[,c,d]` | Fighters of the VS shortcut (decimal port fkinds; Remix id = fkind; vanilla 0..26, Remix 0x1D..0x60) |
| `SSB64_MAPS_START_GKIND=<decimal>` | Stage select opens with the cursor on that stage (use with `css.sh` and `inputs/css_pick_wait.txt`; Training CSS is `SSB64_START_SCENE=18`) |
| `SSB64_VS_P2_HUMAN=1` | The second VS fighter becomes human, so an input script can drive port 1 (`inputs/reflect_test*.txt`) |
| `SSB64_REMIX_GFX_TEST=<id>` / `SSB64_REMIX_COLANIM_TEST=<id>` | Spawns the Remix GFX id / applies the Remix GFX routine on the first fighter every 45 frames |
| `SSB64_REMIX_KIRBY_COPY_TEST=<fkind>` | Kirby (P1) copies that power at frame ~405 (not earlier: the appear status crashes). Put the copied fighter in the match (`vs.sh 8,<fkind> inputs/nsp_kirby.txt`): the copy's weapon and item files only load for fighters present. |
| `SSB64_AUDIO_FGM_LOG=1` | Logs `[fgm] play id=<n> table=<count>` for every FGM the game plays (ids >= 695 are Remix sounds); count them per run to prove SFX play |
| `SSB64_VS_STOCKS=<n>` | The VS shortcut becomes a stock match with n stocks (Royal), so a KO ends it and the results scene runs; with `walk_right.txt` P1 falls off the stage |
| `SSB64_AUDIO_DUMP_PATH=<file>` / `SSB64_AUDIO_DUMP_SECONDS=<n>` | WAV dump of the first n seconds (default 16, at most 120); `vs.sh` sets the path to `<run>/audio.wav` and `AUDIO_SECONDS` the length. Summarize with `.toolchain/host/venv/bin/python -I scripts/wavstat.py <run>...` |
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
