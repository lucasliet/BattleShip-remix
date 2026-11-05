# Remix 1P: Home-Run Contest, record in the save and random path

Covers the deviations from the Smash Remix asm in the Home-Run Contest (HRC) and in the Remix 1P random path, plus the facts
that are not obvious from the code. Code: `decomp/src/sc/sc1pmode/sc1premixhrc*.c` (HRC), `sc1premix1p*.c` (random path),
`decomp/src/if/ifcommon.c` (HRC timer). Tests: `cmake --build <build> --target run_remix_mode_tests`
(`tests/remix/test_1p_hrc_*.c`, `test_1p_remix_rules.c`); in-game, `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=hrc` or
`remix1p` (see `.claude/skills/remix-smoke-tests/SKILL.md`).

## Home-Run Contest

- The HRC runs on the Link slot of the 1P game (`sc1PRemixHrcIsActive`) and borrows the Bonus 3 wait
  (`sc1PGameWaitStageBonus3Update`): no 3-2-1 countdown, no fighter entry (`is_skip_entry` for the player and the Sandbag),
  "Are You Ready?" (FGM 509) and a 66-tic wait before "GO!" (`sc1PRemixHrcPlayReady`).
- Timer: `time_limit` is 1 minute only so that the vanilla timer interface exists. `ifCommonTimerUpdateHrc` (`ifcommon.c`)
  sets `time_remain = 600 - time_passed` and never ends the match at zero. The attempt ends in `sc1PRemixHrcRunEndAttempt`
  (`sc1premixhrcrun.c`): 45 tics, then COMPLETE or FAILURE.
- Distance signs: the stage's layer 1 DObj tree holds the signs as siblings (the second sign is the 4th sibling of the first
  child, the next ones every 2). Each has three MObjs (00/X00 pair, hundreds, thousands) set through `texture_id_curr` with
  `anim_wait = AOBJ_ANIM_NULL`. The digit rules are in `sc1premixhrcsignrules.c`, a literal port of the asm state machine.
- `0x8C` of the fighter struct in `HRC.asm` is `coll_data.pos_diff.x` (N64 layout: `coll_data` starts at 0x78), not a
  velocity field.
- Record: `LBBackup1PRecord.hrc_distance` (tenths of a foot, `lbtypes.h`). The save struct grew, so a save written before it
  fails the checksum and restarts. The first distance is saved quietly; a longer one plays FGM 0x1D0 instead of 0x1CB
  (`sc1premixhrcrecord.c`, `sc1premixhrcrun.c`).
- Not ported on purpose: `rest_area_and_hrc_spawning_fix` (`SinglePlayerModes.asm`, a flag against a vanilla spawn crash that
  the port does not have) and `prevent_volume_reduction_` (`HRC.asm`; the vanilla branch it guards never runs with this
  timer).

## Remix 1P random path

- Data is read at run time from the Remix RAM image (`remix_ram_image_read_*`, `sc1premix1pdata.c`); nothing from the ROM is
  versioned. The three per-slot tables (`announcer_calls`, `model_scale`, `name_textures`) are unlabeled `insert`s in the
  asm: they follow the 18 stage lines of `MATCH_SETTINGS_PART1`, 14 words each.
- `NUM_REMIX_FIGHTERS` is 0x12 (18), counted by `Character.asm define_character`.
- Deviation: the asm stage pick (`_stage_id`) loops forever when all three stage options of a fighter are already used by the
  path. `sc1PRemix1PPickStage` (`sc1premix1prules.c`) returns -1 in that case and the caller picks another fighter. The
  distribution over free options is the same as the asm retry loop.
- Tiny Team (slot 9) stores the fighter in bytes 9, A and B of the stage line. Byte B is `opponent_behavior` in
  `SC1PGameStage`, kept as the asm does because the Tiny Team hooks read it as the third fighter (`sc1premix1p.c`).
- The CPU size states of the Giant and Tiny stages are in `remix_1p_size_override_cp6f_2026-10-27.md`.

Open gaps: `docs/remix/patch_audit.md`, group g1.
