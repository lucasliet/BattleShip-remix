# F7 Size phase (c), group 2: Link, Pikachu, Ness, Jigglypuff, Dragon King effects

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm`, lines 1791-2241 (scopes `link`, `pikachu`, `ness`, `jigglypuff`,
`dragonking`). Every write is guarded by the owner multiplier: a multiplier of 1.0F is a no-op, so vanilla
and unsourced fighters are unchanged. The per-player multiplier comes from `ftRemixSizeGetMultiplier` (phase a).

## Files

- New: `decomp/src/ft/ftremix/ftremixsizefx2.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
- Appended block: `decomp/src/ft/ftremix/ftremixhooks.h` (Size phase (c) block, before the closing `#endif`).
- Vanilla hooks (under `#ifdef REGION_REMIX`): `decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c`.
- Remix port: `decomp/src/ft/ftremix/dragonking/ftremixdragonkingnsp.c`.
- REMIX-TODO(Size) comments: `decomp/src/ef/efmanager.c` (3), `decomp/src/ft/ftchar/ftness/ftnessspecialn.c` (1).

## Patches

| Asm scope (line) | RAM | Vanilla function (file) | Status |
|------------------|-----|-------------------------|--------|
| link.adjust_usp_projectile_size_ (1795) | 0x80163E20 | ftLinkSpecialHiMakeWeapon (ftlinkspecialhi.c) | applied at spawn: `ftRemixSizeScaleLinkWeapon` sets the weapon top joint scale |
| link.adjust_usp_gfx_size_ (1843) | 0x80163DD8 | ftLinkSpecialHiMakeWeapon (effect call site) | applied at spawn: `ftRemixSizeScaleLinkEffect` sets the 2nd joint scale and y = 300 x multiplier |
| pikachu.adjust_gfx_size_ (1900) | 0x8010146C | efManagerThunderAmpMakeEffect (ef/efmanager.c) | skipped, REMIX-TODO(Size). It writes the gfx scale from the player struct (s1), and this function takes only `pos`. Caller: ftparam.c:2382 (`nEFKindThunderAmp`). |
| ness.nsp.adjust_pkfire_position_ (1928) | 0x801539B4 | ftNessSpecialNProcAccessory (ftnessspecialn.c) | skipped, REMIX-TODO(Size). The asm mixes f4/f10/f12/f18 with the multiplier loaded after the first multiply; without a disassembly the exact offset is unknown. |
| ness.usp: update_routine_._update (1954 table) and _initialize (1959) | 0x8012E494+0x10, 0x80102A3C | efManagerNessPKThunderWaveMakeEffect (efmanager.c) | skipped, REMIX-TODO(Size). The wrapper rescales the animated 2nd and 3rd joints every frame; the port has no per-frame hook for this descriptor. |
| ness.dsp: update_routine_._update (2017 table) and _initialize (2022) | 0x8012E41C+0x10, 0x8010260C | efManagerNessPsychicMagnetMakeEffect (efmanager.c) | skipped, REMIX-TODO(Size), same reason as usp. |
| jigglypuff.update_routine_ (2092) | 0x8012E314+0x10, calls 0x800FD568 | efManagerHaveStructProcUpdate | not applied, not Size logic. It never reads the multiplier: it forces the sing gfx routine index to 0x2E and scale to 1.0 unless the Gameshark code or `Toggles.entry_puff_sing_anim` is set. Needs a GS/toggle owner, not the Size phase. |
| dragonking.nsp.render_routine_ (2148) | 0x8012E1D4+0x14 | Dragon King ball gfx (ftremixdragonkingnsp.c, desc proc_display = gcDrawDObjTreeForGObj) | applied: `ftRemixDragonKingNSPApplyBallScale` (base 0.75F, Kirby copies 0.5F, times the multiplier) runs at spawn and each frame after `gcPlayAnimAll`. |
| dragonking.nsp.render_routine_descend_ (≈2200) | used by Size.asm 1620 (Kirby USP sword descend gfx, `OS.patch_start(0xA9BF4 + 0x14, 0x8012E3F4 + 0x14)`) | Kirby USP sword gfx (not located in the port in this pass) | skipped: the routine is in this scope, but its install site is in the Kirby USP section (outside 1791-2241). It rescales the sword trail translate and scale every frame. Hand to the Kirby USP section owner. |

## Deviations

- D1. Link weapon and effect scales are set once at spawn. The asm rewrites them on every render, so they match only
  while the owner's multiplier does not change. With no size source (multiplier 1.0F) there is no write at all.
- D2. The Link effect 2nd joint translate.y is written at spawn (300 x multiplier). If the effect's own animation
  resets that joint each frame, the value will be overwritten; check with a multiplier other than 1.0F.
- D3. In `ftLinkSpecialHiMakeWeapon`, a new local `GObj *effect_gobj` is declared outside the `#ifdef`. It changes
  the N64 compile of that function (register allocation, not the logic). Matching of this function is not checked here.
- D4. The Dragon King ball scale runs once per frame in the update (after the animation), which is closest to the
  asm's render wrapper. The asm reads the owner multiplier at render time; the port reads it at update time.

## Uncertainties (verify before merging)

- U1. pkfire offsets (Size.asm 1928-1952): the register flow was not checked against a ROM disassembly; no `baserom`
  is in this tree. Check with `tools/remix_disasm_rom.py` at 0x801539B4 once the ROM is present.
- U2. Link effect 2nd joint y (Size.asm 1843-1870): `lui 0x4396` is taken as 300.0F; the port constant is `FTREMIX_SIZE_LINK_EFFECT_Y`.

## Verification

- `scripts/remix/syntax-check.sh` returned `syntax-check OK` for `ftremixsizefx2.c`, `ftlinkspecialhi.c`,
  `ftremixdragonkingnsp.c`, `efmanager.c` and `ftnessspecialn.c`. The build flags include `-DREGION_REMIX=1`, so the Remix paths were checked.
- Not built or run (per brief). Expected change at multiplier 1.0F: none.
