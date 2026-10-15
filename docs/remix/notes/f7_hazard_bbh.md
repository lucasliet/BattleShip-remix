# F7 hazard notes: Big Boo's Haunt (stage id 0xC0)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 8383-8450 (`bbh_setup_`, `boo_sound_`).
Stage file: `.toolchain/pkgs/smashremix-src/src/stages/bbh.asm` (sprite data and the sprite info array only; no hazard code).

## Ported

| Routine | Port | Notes |
|---|---|---|
| `bbh_setup_` | `grRemixBbhSetup` in `decomp/src/gr/grremix/grremixbbh.c` | Skips training mode (`nSCKind1PTrainingMode`, asm screen id 0x36). Creates the GObj with `nGCCommonKindGround` (asm object 0x03F2), link `nGCCommonLinkIDGround` (asm group 1), `GOBJ_PRIORITY_DEFAULT` (asm 0x80000000). Registers `boo_sound_` with `gcAddGObjProcess` as `nGCProcessKindFunc`, priority 4 (asm `0x80008188`). |
| `boo_sound_` | `grRemixBbhBooSoundProcUpdate` | Plays FGM 0x0527 (Boo laugh) via `func_800269C0_275C0` when the countdown is zero. |
| dispatch | `grRemixHazardBbh` in `grremixhazards_bbh.c` | TRUE after `grRemixBbhSetup` when `function == REMIX_SYM_Hazards__bbh_setup_`. |

## Skipped or stubbed

- `REMIX-TODO(bg countdown 0x80131AD8)`: `boo_sound_` reads the word at RAM 0x80131AD8. No port code owns this word, and `bbh.asm` does not call `Stages.add_bg_animation`, so the owner is elsewhere. `grRemixBbhBooCountdownIsZero` returns FALSE, so the laugh never plays until the owner is ported. The surrounding logic is ported.
- `bbh_setup_` clears RAM 0x801313F4 (Boo timer). Nothing in the `bbh_setup_` or `boo_sound_` range reads it, so no state is kept.
- The asm computes the header address from 0x80131300 into `t1` and never uses it. Not ported.
- `sw t6, 0x0010(sp)` stores a stale register into the stack frame. The `CREATE_OBJECT_` call does not read that slot. Not ported.

## Assumptions to confirm

- Stage id: the brief says `nGRRemixStage_BBH`, but `grremixids.h` names it `nGRRemixStage_BIG_BOOS_HAUNT = 0xC0`.
- `Global.current_screen` is taken as `gSCManagerSceneData.scene_curr`, and the asm training id 0x36 is taken as `nSCKind1PTrainingMode`. This assumes the port keeps the vanilla SCKind numbering.
- Asm `Render.CREATE_OBJECT_` (0x80009968) is the port's `gcMakeGObjSPAfter`, and asm object id 0x3F2 equals `nGCCommonKindGround` (1010).
- The FGM 0x0527 id has no named constant in `gm/gmsound.h`, so it is a local define.

## Test

Run the stage with `SSB64_VS_STAGE=192` (0xC0). The Boo laugh stays silent until the REMIX-TODO is resolved, so the test shows only that the setup runs without a crash and the GObj is created. Training mode (scene `nSCKind1PTrainingMode`) should create no object.
