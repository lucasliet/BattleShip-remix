---
name: remix-smoke-tests
description: Run headless smoke tests of the Smash Remix port (BattleShip-Remix) and read their results: VS matches with chosen fighters, stages and items, special-move matrices, full stage sweeps, CPU item sweeps, the CSS/stage-select flow, contact sheets of screenshots, crash backtraces and lldb inspection. Use whenever a Remix change needs verification in the running game (ROADMAP tasks, "teste", "smoke", "sweep", "roda o jogo", "confere em partida"), before ticking a task or committing a checkpoint.
---

# Remix smoke tests

Verification by running the real game headless with environment overrides, screenshots and the port log. Every task of
`docs/remix/ROADMAP.md` is accepted only after the test it names passes here.

## Before running

1. Build (absolute paths; a new file under `decomp/src` needs `cmake -S $PWD -B $PWD/build` first):
   - macOS (contained toolchain): `PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j <n>`
   - Linux: `cmake --build <build dir> --target ssb64 -j <n>`. The scripts run the binary and assets from `build/`; when
     the build directory is another one, symlink them into `build/`.
   - Size `-j` from the machine's cores and memory (`nproc` or `sysctl -n hw.ncpu`).

   Run Bash with `dangerouslyDisableSandbox: true` (the game needs the window server).
2. **Never rebuild while a sweep runs**: the sweep keeps launching `build/BattleShip-Remix`.
3. Every run writes its own log and save (`SSB64_LOG_PATH`, `SSB64_SAVE_PATH`), so game batches run in parallel with
   `parallel.sh`; only validation by the lead counts (workers never run or sign off a batch).
4. Prefer `parallel.sh auto ...` for sweeps: `auto` takes the cores minus two, capped at one game per GB of free memory
   (it prints the count). The serial
   scripts stay for one-off runs. Never run two sweeps that write the same run name at once.
   - Linux with a tiling compositor: a game window on a hidden workspace presents about one frame per second and its run times out;
     keep the windows on the visible workspace. Tiled windows get different sizes, so only serial runs (or floating
     windows of one size) are pixel-identical for an A/B diff. There is no lldb: use `gdb -batch -x <cmds>` (delete a
     breakpoint before `call`ing a function that would hit it).
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
| `parallel.sh <jobs|auto> matrix\|stages\|items\|hazards\|custom <args>` | The sweeps above with several game instances at once (own log and save per run through `SSB64_LOG_PATH` / `SSB64_SAVE_PATH`). With `auto`, N runs take about as long as one serial run. Windows still open in front, so keep the machine free. |
| `parallel.sh <jobs|auto> custom <file>` | One shell command per line of `<file>` (each a `vs.sh` or `css.sh` call with its own env), run `<jobs>` at a time, e.g. the Kirby copy batch. A run with `shots=0 metal_flake=1` is the Metal flake: rerun it alone. |
| `css.sh <input> <name> [frames]` | Character select (scene 16) flow driven by an input script. |
| `scene.sh <scene> <input> <name> [frames]` | Same as `css.sh` with any start scene (17 is the 1P CSS: `inputs/css1p_pick_start.txt` picks and starts a 1P run). |
| `lldb_count.sh <name> <fkinds\|-> <input> <frames> <spec>...` | The VS shortcut under lldb; prints the hit count of each probe on one `vs=` line (works with `parallel.sh custom`). Specs: `func`, `func@cond`, `file.c:line`, `ret:func@cond` (every `ret` of the function, the condition sees `$w0`, the return value). The build is Release: a line on a `return` inside an `if` often resolves to a merged line, which the summary marks `(->N)`; probe the pure rules function with `ret:` instead. Quote specs holding `>` `$` `(` `&` in a `custom` file, or the shell eats them. `crash=1` means the process stopped on a signal; a stop in `objc_msgSend` at launch with 0 frames is an lldb flake, rerun it. |
| Bonus stages | The BTT/BTP-class stages and the Rest Area are not VS stages (their ground needs the bonus item file): `parallel.sh stages` skips them. Run one through the 1P path: `SSB64_SPGAME_STAGE=3` (BTT) or `7` (BTP) with `SSB64_SPGAME_FKIND=<owner>` and `scene.sh 52` (Banjo 68 gives BTP_BANJO 197, Marina 63 BTT_MARINA 172, Mewtwo 57 BTP_MTWO 140, Crash 72 BTP_CRASH 206, Sonic 59 BTP_SONIC 161, Peach 73 BTT_PEACH 207). `inputs/pause_camera.txt` exercises the Remix pause camera (C pan, A/B zoom, Z+A fov, stick). |
| `lldb_count.sh <name> <fkinds\|-> <input> <frames> <spec>...` / `lldb_first.sh <name> <fkinds\|-> <input> <frames> <function>[@cond]` | Run the VS shortcut (or `LLDB_START_SCENE`) under lldb: `lldb_count` prints the hit count of each breakpoint spec (`function`, `function@'$x0 == 52'`, `file.c:line`) on the `vs=` line; `lldb_first` prints the argument registers (x0..x2, s0, s1) at the first hit and the return registers after `finish`. Both work inside `parallel.sh custom`. Use them to prove that a rare path runs or returns the Remix value. |
| `ai_recovery.sh [runs] [jobs] [frames]` | Level-9 CPU recovery measurement: `runs` four-CPU matches, 3 stocks, `SSB64_REMIX_AI_LOG`; prints `episodes= ok= ko= selfko= open=` per run and in total (`selfko` = KO episodes where the CPU's percent did not rise, `hit=0` in the log). Run with `bash`. |
| `scene.sh <scene> <input> <name> [frames] [timeout_s]` | Boots any scene of the `SCKind` enum (`decomp/src/sc/scdef.h`): 9 VS mode menu, 16 VS CSS, 17 1P CSS, 18 Training CSS, 19/20 Bonus CSS, 24 VS results, 27 N64 logo, 52 1P game, 53 bonus stage, 54 Training, 55 Congratulations, 56 credits, 58 Data menu, 59 Sound Test, 61 Auto-Demo. Run it with `bash` (the skill folder is not writable from Bash, so no `chmod`). `inputs/chardata_lap.txt` with scene 58: A on Characters, then R through the fighters (Dedede at about frame 700). |
| `sheet.py <name> [--step N] [--max N] [--crop x0,y0,x1,y1]` | Contact sheet `build/agent-shots/vs/<name>/sheet.png` (run with `.toolchain/host/venv/bin/python -I`). |

Outputs: `build/agent-shots/vs/<name>/` with `frame_<n>.png`, `stdout.log`, `ssb64.log`, `save.bin`.

The runs share `build/BattleShip.cfg.json`, and the game writes a Remix setting changed by an input (panel menu,
Training, toggles) to it at once: later runs start with that setting. Copy the file before such a run and put it back
when the next runs must not see the change; keep it when they test that the setting persists.

Input scripts (`inputs/`). Line format: `<from> <to> <port> <buttons|-> [stick_x stick_y]`. Most files start with a header that
says the scene and variables to run them with. Do not press A every few frames to pick an item up: the fighter stays locked in
its jab combo; use the CPU item sweep instead.

| Group | Inputs |
|---|---|
| Match basics (`vs.sh`) | `none.txt` (no input), `walk_right.txt` (P1 walks off the stage; with `SSB64_VS_STOCKS=1` the match ends and the results run), `nsp_tap.txt` / `dsp_tap.txt` / `usp_tap.txt` (walk right, then the special every 150 frames from 420), `dsp_hold_charge.txt` (held down special of growing length), `air_dodge.txt` (air dodge with Z; set the Air Dodging toggle), `pause_start.txt` (pause at 600), `pause_camera.txt` (Remix pause camera controls), `stats_view.txt` (results, Match Stats pages) |
| Fighter checks (`vs.sh`, often `SSB64_VS_P2_HUMAN=1` on stage 16) | `nsp_kirby.txt` and `kirby_charge_air_shot.txt` (Kirby copy, with `SSB64_REMIX_KIRBY_COPY_TEST`), `inhale_walker.txt` (inhale catches P2), `grab_throw.txt` (R grab and throw of P2), `reflect_test.txt` (reflectors), `pipe_mk_left.txt` (Mushroom Kingdom pipe) |
| VS CSS (`css.sh`, scene 16) | `css_pick_mario.txt` (both ports pick Mario, START at 200, A on the stage select at 300), `css_page3_stage.txt` (same, page 3 of the stage select), `css_pick_wait.txt` (picks, then waits on the stage select; with `SSB64_MAPS_START_GKIND`), `css_pick_four.txt` (four ports; with `SSB64_REMIX_MODE=<mode>` plays a 4-player match of each mode), `css_costume_cycle.txt`, `css_l_random.txt`, `css_falco_green_match.txt` (costumes), `css_panelmenu_visibility.txt` (panel menu), `css12cb_match_ko_back.txt` (12-Character Battle) |
| 1P and Bonus (`scene.sh`) | `css1p_pick_start.txt` (1P CSS pick and START), `css1p_start_smash_both.txt` (enemy KOs), `css1p_hold_l_then_start.txt` (practice retry with L), `allstar_a_presses.txt` (All-Star, with `SSB64_SPGAME_AUTOWIN`), `bonus_css_start_hold_l_fall.txt` (Bonus practice retry) |
| Training (`scene.sh` 18 or 54) | `training_css_pick.txt`, `training_oos_platform.txt` (`SSB64_TRAIN_FKINDS=0,8 SSB64_VS_STAGE=6 SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3"`, the log shows `training oos`), `training_tech_platform.txt` (same with `Tech=<1..5>;ShieldBreak=0;Percent2=30`, the log shows `training tech ... teched 1 status N`; Roll Left 73, Roll Right 74, In Place 81), `training_stickswap_advance.txt` and `training_stickless_menu.txt` (D-pad Control) |
| Screens (`scene.sh`) | `chardata_lap.txt` (scene 58, R through every fighter), `data_gallery.txt` (58 to the Gallery and back), `gallery_controls.txt` (55 with `SSB64_START_GALLERY=1`), `soundtest_browse.txt` / `soundtest_wrap.txt` / `soundtest_afk_once.txt` (59; the last is the 7-minute AFK check) |

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
| `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=<mode> SSB64_SPGAME_FKIND=<fkind>` | Boot a Remix 1P mode (`hrc`, `multiman`, `cruel`, `remix1p`, `allstar`, `bonus3`); Multi-Man and Cruel also need `SSB64_SPGAME_STAGE=12` (the Polygon Team stage), else stage 0 (VS Link) runs |
| `SSB64_VS_STAGE=<decimal>` | Stage id (vanilla 0..0x28, Remix 0x29..0xD7; names in `decomp/src/gr/grremix/grremixids.h`). Vanilla: 0 Peach's Castle, 1 Sector Z, 2 Kongo Jungle, 3 Planet Zebes, 4 Hyrule Castle, 5 Yoshi's Island, 6 Dream Land, 7 Saffron City, 8 Mushroom Kingdom, 9 Beta Dream Land, 10 Test Stage, 11 How to Play, 12 Small Yoshi's Island, 13 Meta Crystal, 14 Duel Zone (polygon team), 15 Race to the Finish, 16 Final Destination. Do not count the `nGRKind*` enum in `decomp/src/gr/grdef.h` by hand: its alias entries (`nGRKindBattleStart`, `nGRKindStarterEnd`...) do not take a slot. |
| `SSB64_VS_ITEMS=<1..5>` | Turns every item switch on with that rate (0 means no items) |
| `SSB64_REMIX_FORCE_ITEM=<kind>` | Every stage-spawned item becomes `<kind>`, with a 60-tick spawn wait |
| `SSB64_REMIX_FORCE_ITEM_AT_P1=1` | The forced item drops on player 1 instead of a random item point |
| `SSB64_REMIX_ITEM_LOG=1` | Logs `[remix-item] spawn|hold|consume|touch|drop|throw|bullet kind=0x..` |
| `SSB64_REMIX_FIGHTER_LOG=1` | Logs `[remix-fighter] status|hit|ammo` (status changes, damage with percent and knockback, fire-breath ammo of Bowser and of Kirby with the Bowser hat) and `[remix-weapon] end kind= lifetime=` (frames since the weapon was made); every line has the battle tic `t=` |
| `SSB64_BGM_LOG=1` | Logs `[bgm] play t= track= player=` and `[bgm] stop t= player=` at every BGM start and stop; the track is the one actually played, after the Remix and shuffle substitutions |
| `SSB64_PORT_MENU_OPEN=1` | The port ImGui menu is open from boot, for screenshots of the port menu. With `SSB64_PORT_MENU_PAGE="<header>/<sidebar>"` (e.g. `Remix/Other Screens`) it opens on that page. The open menu saves `gOpenPortMenu` and the page to `build/BattleShip.cfg.json`: run it only through `scene.sh`, which restores the file, and never two such runs at once |
| lldb `expr` on a menu routine | A port-menu button can be tested without a click: break on a per-frame function of the scene (`breakpoint set -n mnDataFuncRun -i 120`), `expr (void)scRemixScreensOpen(4, 5)`, `breakpoint delete 1`, `continue` (run `l5_screens_gallery`) |
| `SSB64_REMIX_CHARDATA_MOTIONS=<k1,k2,...>` | Character Data screen: the demo motions cycle through these `MNCharactersMotionKind` values (decimal: SpecialHi 0, SpecialN 1, SpecialLw 2, Wait 14, Attack1 18, AttackAirF 27, Win1..Win4 33..36) instead of the random pick |
| `SSB64_REMIX_AI_LOG=1` | Logs `[remix-ai] recover_begin t= port= fkind= lvl= x= y=` when a CPU enters the recover objective and `recover_end t= port= result=ok|ko` when it lands or catches a ledge, or is KO'd |
| `SSB64_VS_ALL_CPU=1` | The VS shortcut makes P1 a CPU too (level from `SSB64_VS_CPU_LEVEL`) |
| `VS_TIMEOUT=<s>` | Timeout of one `vs.sh` run (default 300 s) |
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

## Cleanup

- An input made for one investigation (one bug, one audit item) lives in the session scratchpad, not in `inputs/`.
  Add a file to `inputs/` only when it drives a flow other fighters, stages or tasks will reuse, with a header saying
  the scene, env and what to look for.
- When a task is validated and its evidence is recorded, delete its run folders:
  `rm -rf build/agent-shots/vs/<prefix>_*` (sheets, frames, logs, saves, audio dumps). Before the end of a session,
  empty `build/agent-shots/vs/` and `build/debug_traces/` unless a run is still referenced by an open task.
