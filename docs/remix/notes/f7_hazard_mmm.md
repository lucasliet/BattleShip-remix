# F7 hazard notes: Mmm (Mad Monster Mansion, gkind 0x40 nGRRemixStage_MADMM)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 9656-9772 (mmm_setup, mmm_main).

## Ported

| Routine | Port | Notes |
|---|---|---|
| `Hazards.mmm_setup` | `grRemixMmmMakeGround` in `decomp/src/gr/grremix/grremixmmm.c` | Clears the thank-you flag, creates the routine GObj. |
| `Hazards.mmm_main` | `grRemixMmmProcUpdate` (same file) | Per-frame process, ports 0..3. |

Dispatch: `decomp/src/gr/grremix/grremixhazards_mmm.c`, `REMIX_SYM_Hazards__mmm_setup` -> TRUE.

## Mapping decisions

- `Character.port_to_struct_(port)` -> walk of `gGCCommonLinks[nGCCommonLinkIDFighter]` matching `fp->player`.
- Action id: `FTStruct.status_id` (struct_offsets 0x24) == `Action.Taunt` 0xBD.
- Asm `0xEC` of the FTStruct (named "clipping id") -> `fp->coll_data.floor_line_id` == 2 (struct_offsets 0xEC).
- Fighter x: `DObjGetStruct(fp->fighter_gobj)->translate.vec.f.x` (asm: `gobj->obj`, then `+0x1C`).
- Bounds: 0x44D40000 = 1696.0F (asm comment says "1700ish"), 0x45020000 = 2080.0F. Both inclusive, matching the `c.le.s` + `bc1fl` pair.
- Thank-you sound: `func_800269C0_275C0(0x564)`.
- Flag: asm stores `a0` (a nonzero value after the FGM call); the port stores TRUE. Only zero/nonzero is read.

## Skipped / uncertain

1. `mmm_setup` also does `sw r0, 0x60(0x801313F0)` (clears under-water flags and frenzy turns in `gGRCommonStruct`). MMM has no water or frenzy hazard, and the port keeps water state in its own statics, so no port storage is cleared. If the MMM stage shares state with another hazard, revisit this.
2. `Render.CREATE_OBJECT_` object id 0x3F2 and Remix group 1 have no port equivalent. The routine is a ground-link GObj with a process at priority 3 (`nGCProcessKindFunc`), no display object.
3. Dead loads and stores after the `jal Render.CREATE_OBJECT_` in `mmm_setup` (t7, t8, a1, a2, a3 on the stack) are not ported; no call consumes them.
4. The asm loops over four ports hardcoded (`lli s1, 0x3`); the port uses `GMCOMMON_PLAYERS_MAX` (4).

## Test

Stage id for the lead: `SSB64_VS_STAGE=64` (0x40). Taunt on the marked floor line (floor_line_id 2) with x in [1696, 2080] should play sound 0x564 once per match.
