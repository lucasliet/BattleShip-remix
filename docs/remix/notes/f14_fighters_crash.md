# F14 notes: fighters_crash (REMIX-TODO closure)

Scope: `decomp/src/ft/ftremix/crash/ftremixcrashdsp.c`, `crash/ftremixcrashnsp.c`, `marina/ftremixmarinansp.c`,
`younglink/ftremixyounglink.c`. Source asm: `.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm`,
`Marina/MarinaSpecial.asm`, `YoungLink/YoungLinkSpecial.asm`, `linkshared.asm`.

## TODO status

| File:line (before) | Asm | State | Notes |
|---|---|---|---|
| crashdsp.c:76 `Size` (dig_gfx_main_) | CrashSpecial.asm 2087-2112 | implemented | Owner root DObj scale = `ftRemixSizeGetMultiplier(port)` each Dig GFX update, after gcPlayAnimAll and before the destroy branch. Guarded by `REGION_REMIX`. |
| crashdsp.c:330 `animation` (DSPWait, 0x8000BD1C) | 1432-1462 | implemented | `gcAddDObjAnimJoint(root->child, track_dig_wait, 0.0F)`. 0x8000BD1C has no decomp name; it is the same call as the klaptrap and cannon helpers. Track is `REMIX_SYM_CrashDSP__track_dig_wait` read by `remix_ram_image_resolve`. |
| crashdsp.c:347 `animation` (DSPTurn) | 1463-1492 | implemented | Same call with `track_dig_turn`, after the status change, only when a Dig GFX exists. |
| crashdsp.c:371 `dig gfx` (GObj +0x4C..+0x54) | 1494-1545 | implemented | The asm copies the player topjoint translation into the Dig GFX GObj. The smoke puff in dig_gfx_main_ reads it, so the port keeps it in `sFTRemixCrashDigEndPos[player]` (per player, `static`). The smoke now stays where the dig ended, as the asm does. |
| crashdsp.c:373 `animation` (DSPEnd) | 1494-1545 | implemented | `track_dig_end` on `root->child`. |
| crashnsp.c:86 `GFX` (create/setup_spin_gfx_) | 119-155, 862-887 | implemented | `efManagerMakeEffectNoForce` with the Spin GFX descriptor (see below). Setup sets is_effect_attach, the primitive and environment colours of both joints from the costume table, and calls `ftRemixSizeScaleLinkEffect`. |
| crashnsp.c:93 `Item` (spin_detect_item_, item_fling_) | 264-329, 891-907 | skipped | Needs the item list head (N64 0x80046700), the item pickup bit at 0x2CE, the fling constants (FLING_HEIGHT_MIDDLE, FLING_WIDTH, FLING_HEIGHT, not extracted), `itMainSetStatus` with the zero `flung_item_state_table`, and the 0x5BD FGM. The port's item manager exposes no list head accessor. The TODO marker is kept. |
| marinansp.c:302 `throw_turn rotation` | MarinaSpecial.asm 652-697 | implemented | Reads the four floats at 0x8018C230..0x8018C23C through `remix_ram_image_read_word`. Right: step C230, target C234. Left: step C238, target C23C. Rotate y = target - (turn_tics / 8.0F) * step, with the timer value after the decrement. |
| younglink.c:78 `ITStruct` (end_fgm_) | YoungLinkSpecial.asm 163-185 | implemented | FGM handle in `item_vars` after the linkbomb block (`ITRemixYoungLinkBombchuVars`). `end_fgm_` clears it and calls `func_80026738_27338`. |
| younglink.c:121 `ITStruct` (begin_moving_ FGM store) | YoungLinkSpecial.asm 220-236 | implemented | `func_800269C0_275C0(0x455)` result stored in the handle slot. |
| younglink.c:464 `ITStruct` (owner FGM clear) | linkshared.asm 352-359 | implemented | `save_character_id_` clears the handle for Young Link owners, the same branch that installs the blast zone. |

## Descriptors and data

- Spin GFX descriptor: `flags 0x06`, `dl_link 0x0F`, transform kinds `{0x50, 0x1A}` and `{0x1C}`, update
  `efManagerHaveStructProcUpdate` (0x800FD568), display `gcDrawDObjTreeForGObj` (0x80014768), offsets
  0x708, 0x838, 0x894, 0x8B4. Same layout as the Dig GFX and Link's Up Special.
- File: Crash and Kirby use the Crash file (Crash file 7 = `p_file_special2`). Kirby and J Kirby read it at
  offset 0xCC4 (`CRASH_NSP_KIRBY_SPIN_FILE_OFFSET`). The port reads the Crash file through
  `port_fighter_data(REMIX_ID_CRASH)`, the same way `ftRemixPowerData` does for Kirby copies.
- Colour tables (`spin_gfx_table`, `kirby_spin_gfx_table`): 7 costumes x 4 words, copied as `static const u32`.

## Uncertainties and approximations (asm line numbers)

1. **Spin GFX Size render routine (CrashSpecial.asm 124-125)**: the asm swaps the render routine to
   `Size.link.adjust_usp_gfx_size_.render_routine_`. The port calls `ftRemixSizeScaleLinkEffect` once at spawn, as
   the header of `ftremixsizefx2.c` describes for Link. If the owner's multiplier changes during the spin, the scale
   is not refreshed.
2. **Kirby spin file when Crash is not loaded**: the asm reads Crash's file global without a check. The port reads
   `*port_fighter_data(REMIX_ID_CRASH)->p_file_special2` without a check either. A NULL base yields a bad pointer, so
   verify that Crash data is loaded when Kirby copies the NSP.
3. **Dig smoke position (dig_gfx_main_ 2110-2130)**: the asm reads the copied GObj position when the Dig GFX
   animation ends. The port reads the captured per-player position. The capture happens in DSPEnd, the same place
   as the asm.
4. **Young Link FGM id (0x1D4)**: not ported as a separate field. The asm always writes the id of the same handle,
   so the `if id != stored id` branch of `end_fgm_` never skips a handle that the port would end.
5. **Marina throw_turn float reads**: the port reads 0x8018C230 and the others from the Remix RAM image. The old
   note said those words were not in the vanilla ROM, which is why the image is needed here.

## Files

- `decomp/src/ft/ftremix/crash/ftremixcrashdsp.c`: Size scale, track helper and calls, end position.
- `decomp/src/ft/ftremix/crash/ftremixcrashnsp.c`: spin GFX descriptor, colour tables, create, setup, spawn.
- `decomp/src/ft/ftremix/marina/ftremixmarinansp.c`: RAM float read, throw turn rotation.
- `decomp/src/ft/ftremix/younglink/ftremixyounglink.c`: Bombchu FGM handle in item_vars, end, owner clear.

## Lead wiring and in-game checks

- Crash DSP at 1x and a size multiplier other than 1.0 (the Size mode): the fighter scales during the dig. Dig ends
  leave the smoke where the dig ended.
- Crash DSP wait, turn and end animation: the joint track runs (wait loops, turn once, end once).
- Crash NSP ground and air, costumes 0 to 6 and Kirby: the spin shows the colours of the costume (not the default).
- Marina throw turn: her facing rotates toward the target over the turn timer.
- Young Link Bombchu: the rolling sound stops on landing, when it is picked up, and at the blast zone.
- Not done: Crash spin item pickup and fling (see the skipped row).
