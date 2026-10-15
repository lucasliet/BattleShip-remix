# F7 shared_b: Samus grapple, Marina item pull, Dedede jump, Kirby cutter and rock

## 1. Samus grapple descriptor (applied)

- Asm: `src/samusshared.asm` `get_grapple_anim_gfx_struct` (RAM 0x80101E88, lines 321-345) and `dsamus_grapple_anim_gfx_struct` (lines 372-383).
- Vanilla function: `efManagerSamusGrappleBeamGlowMakeEffect` (`decomp/src/ef/efmanager.c`).
- Proc 0x8000DF34 is `gcPlayAnimAll` (`decomp/src/sys/objanim.c:1785`). The earlier blocker (shared_samus2.md) is resolved. Evidence: the vanilla ROM at ROM 0xEB34 (RAM 0x8000DF34, assumed mapping) disassembles to a DObj loop that calls the joint parse and play functions, then an MObj loop, which matches `gcPlayAnimAll` line by line. Confirm in game that the Dark Samus grapple beam draws.
- Remix descriptor = vanilla `dEFManagerSamusGrappleBeamEffectDesc` with file 6 (`fp->data->p_file_special1`) and the four offsets + 0x2970 (vanilla 0x380/0x210/0x410/0x480). Flags, DL link, transform types, proc and render match.
- Port: `ftRemixSamusSharedGrappleBeamDesc` in `decomp/src/ft/ftremix/ftremixsamusshared2.c`, with a file-local static copy of the descriptor (vanilla static never mutated). Only `fkind == REMIX_ID_DSAMUS` changes; a NULL file falls back to the vanilla descriptor.
- Vanilla hook: `decomp/src/ef/efmanager.c`, `efManagerSamusGrappleBeamGlowMakeEffect`, under `#ifdef REGION_REMIX`. The REMIX-TODO(GFX) comment above it was removed because it is applied.
- Declaration: appended to `decomp/src/ft/ftremix/ftremixhooks.h` (block "Samus clones, ftremixsamusshared2.c").
- Still open, not in this scope: `efManagerSamusEntryPointMakeEffect` REMIX-TODO(GFX) (shared_samus2.md, entry glow).

## 2. Marina item pull (already ported, no change)

- Asm: `src/Marina/MarinaSpecial.asm` `pull_main_` (2028-2062) and `create_and_assign_clanpot_item_` (2264-).
- Port: `ftRemixMarinaDSPCreateAndAssignClanpotItem` and `ftRemixMarinaDSPPullProcUpdate` in `decomp/src/ft/ftremix/marina/ftremixmarinadsp.c`. It spawns the item by charge (2 Shuriken, 3 Boomerang, 4 ClanBomb, else Gem), holds it with `itMainSetFighterHold`, and resets the charge.
- `docs/remix/notes/3F_MARINA_DSP.md` section 2 ("the pull creates no item") is stale.
- Uncertainty (asm 2271-2272): the asm sets `Item.skip_item_spawn_gfx_`. The port relies on direct spawns skipping the gfx (comment in the port). Not verified in game.
- Not touched: Marina `prevent_item_despawn_` (item spawner scope).

## 3. Dedede jump multiplier (already applied, no change)

- Asm: `src/Dedede/Dedede.asm` `jump_multiplier_table` (lines 733-741): 0, 0, 68, 58, 52, 0 (indexed by jumps used).
- Port: `ftRemixDededeJumpAerialMultiplier` in `decomp/src/ft/ftremix/ftremixpuffkirbyshared.c` (values match). Called from `decomp/src/ft/ftcommon/ftcommonjumpaerial.c` (~line 264) under `REGION_REMIX`, which is `jump_fix_2` (0x8014004C) in `ftremixpuffkirbyshared.c`. See shared_puff_kirby_mario.md U2 and U3.

## 4. Kirby cutter and rock constants (already applied, no change)

- Cutter: `kirby_special_struct_fix` (0x8016BEA0, asm `jigglypuffkirbyshared.asm` 222-249) is applied as `ftRemixKirbyCutterWeaponDesc`. Called from `decomp/src/wp/wpkirby/wpkirbycutter.c:147` under `REGION_REMIX`.
- Rock: `kirby_rock` (0x80161424), `kirby_rock_2` (0x80161374), `kirby_rock_3` (0x801614E4) are applied as `ftRemixKirbyStoneHealthMax`, `ftRemixKirbyStoneHealthMid` and `ftRemixKirbyStoneReleaseDuration` in `ftremixpuffkirbyshared.c`. Called from `decomp/src/ft/ftchar/ftkirby/ftkirbyspeciallw.c` (lines 25, 58, 98), each guarded by `REGION_REMIX`.
- `src/Kirby/Kirby.asm` has no cutter or rock constants (its patches are magic hat, clone power swap, restore magic hat and the Marth flag). `src/KirbyHats.asm` is the hat system (`use_extended_special_parts_*`, hat select, hat files, power loss). Neither file holds the cutter or rock constants. Not changed.
- Uncertainty: `ftRemixKirbyCutterWeaponDesc` does not check word 2 (kind) of the asm struct (shared_puff_kirby_mario.md U4).

## Verification

`scripts/remix/syntax-check.sh` prints `syntax-check OK` for `ftremixsamusshared2.c`, `efmanager.c`, `ftremixhooks.h`, `marina/ftremixmarinadsp.c` and `dedede/ftremixdedededsp.c`. No build, no game run, no commit.

## REMIX-TODO

None added. None removed except the grapple REMIX-TODO(GFX) above.
