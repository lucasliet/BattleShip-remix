# Remix 1P: Home-Run Contest, record in the save and random path (CP6e)

2026-10-27. Checkpoint of T6.2 (HRC finished, Remix 1P path R1/R2). Deviations from the Remix asm and facts that are not obvious from the code.

## Home-Run Contest

- The HRC runs on the Link slot of the 1P game (`sc1PRemixHrcIsActive`) and borrows the Bonus 3 wait (`sc1PGameWaitStageBonus3Update`): no 3-2-1 countdown, no fighter entry (`is_skip_entry` for the player and the Sandbag), "Are You Ready?" (FGM 509) and a 66 tic wait before "GO!".
- Timer: `time_limit` is 1 only so the vanilla timer interface exists; `ifCommonTimerUpdateHrc` (`ifcommon.c`) sets `time_remain = 600 - time_passed` and never ends the match at zero. The attempt end is `sc1PRemixHrcRunEndAttempt` (45 tics, then COMPLETE or FAILURE).
- Distance signs: the stage's layer 1 DObj tree holds the signs as siblings (second sign = 4th sibling of the first child, the next ones every 2). Each has three MObjs (00/X00 pair, hundreds, thousands) set through `texture_id_curr` with `anim_wait = AOBJ_ANIM_NULL`. Digit rules are in `sc1premixhrcsignrules.c` (literal port of the asm state machine).
- `0x8C` of the fighter struct in `HRC.asm` is `coll_data.pos_diff.x` (N64 layout: `coll_data` starts at 0x78), not a velocity field.
- Record: `LBBackup1PRecord.hrc_distance` (tenths of a foot). The struct grew, so a save from before this checkpoint fails the checksum and restarts. First distance is saved quietly; a longer one plays FGM 0x1D0 instead of 0x1CB.
- Not ported on purpose: `rest_area_and_hrc_spawning_fix` (a flag against a vanilla spawn crash; no crash seen) and `prevent_volume_reduction_` (the vanilla branch it guards never runs with this timer).
- Not seen in a match: the camera warp is seen only in the log with a temporary teleport (since removed), the Conker/Bowser/Kirby input exceptions, DK Cargo/Orangstand ending, the projectile/player travel past 200 ft.

## Remix 1P path (R1/R2)

- Data is read at run time from the Remix RAM image (`remix_ram_image_read_*`), nothing from the ROM is versioned. The three per-slot tables (`announcer_calls`, `model_scale`, `name_textures`) are unlabeled `insert`s: they follow the 18 stage lines of `MATCH_SETTINGS_PART1`, 14 words each (checked against the ROM locally).
- `NUM_REMIX_FIGHTERS` is 0x12 (18), taken from `build/remixasm/bass.log`; the pool is the first 18 `match_pool` entries.
- Deviation: the asm stage pick (`_stage_id`) loops forever when all three stage options of a fighter are already used by the path. `sc1PRemix1PPickStage` returns -1 in that case and the caller picks another fighter. The distribution over free options is the same as the asm retry loop.
- Tiny Team (slot 9) stores the fighter in bytes 9, A and B of the stage line; byte B is `opponent_behavior` in `SC1PGameStage`, kept as the asm does because the Tiny Team hooks read it as the third fighter (R3).
- The 1P title card still shows the vanilla opponent (R4) and the special slots (team, giant, tiny, Kirby, duo, final bosses) have no Remix logic yet (R3 to R5).
