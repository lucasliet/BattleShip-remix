# Character Data screen: Dedede crash, out-of-bounds filler status and the figatree size check

Code: `decomp/src/mn/mndata/mncharacters.c`, `decomp/src/mn/mndata/mnremixcharacters.c`, `decomp/src/ft/ftmain.c`
(`ftMainSetStatus`). Test: `scene.sh 58` with `inputs/chardata_lap.txt`, and `SSB64_REMIX_CHARDATA_MOTIONS` to force the
motions (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

Run `b12_chardata3` (scene 58, A on Characters, R up to Dedede): SIGSEGV with fault address 0x8 about 140 frames after
Dedede's page opened, in `lbCommonAddFighterPartsFigatree` called from `ftMainSetStatus` from
`mnCharactersFighterProcUpdate`. Dedede had played a victory pose (opening status 0x10001) and was entering his forward aerial
(status 0xD2, motion 185). The faulting read is `parts->is_have_anim`: one DObj of Dedede's joint tree had a NULL `FTParts`
pointer. Later runs with the same input did not crash; the screen picks its motions at random.

## Findings

- **Out-of-bounds filler status (fixed).** Before Phase 5b lot 1, every extended fighter used the common motion rows for its
  special moves. Those rows hold the filler status 0xA2C2A. `ftMainSetStatus` subtracts the Character Data base, sees an
  opening status, and indexed the fighter's opening table at 0x72C2A: a random descriptor (log line
  `status=0x82c2a motion=-65536`), whose callbacks and motion entry are whatever memory lies there. The picker now rejects a
  motion kind whose rows the fighter cannot play (`mnCharactersCheckRemixMotionKindPlayable`), falls back to Wait when the
  second pick still yields the filler, and the extended fighters read their real special and jab rows from the Remix tables
  (`CharacterDataScreen.special_action_pointers`, `jab_action_pointers`).
- **Figatree heap overflow (ruled out).** The Data screen loads every animation into one heap sized
  `gFTManagerFigatreeHeapSize`. `ftMainSetStatus` now logs `figatree overflow` when an animation is larger than the fighter's
  `file_anim_size`. No animation of any fighter exceeded it in the runs below.
- **Not proven.** The `b12_chardata3` log shows no filler status for Dedede himself before the fault, so the exact corruption
  path of that run is not confirmed. The size check stays to make an overflow visible if one ever happens.

## Verification (2026-11-05)

- `l1_lap`: the lap through every fighter, no crash.
- `l1_stress_all`: 30 fighters, 450 frames each, with `SSB64_REMIX_CHARDATA_MOTIONS` cycling all 39 motion kinds; no crash, no
  overflow line.
- `l1_stress_rand`: the same lap with random motions; no crash.
- `l1_stress_dedede`, `l1_dedede_forced`: Dedede alternating every victory pose with the forward aerial, then his specials and
  jab; no crash.

Phase 8 repeats the lap; a new crash on this screen reopens this entry.
