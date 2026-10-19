# F8 notes: BlastZone Warp (BETA) and BlastZone GFX

Source asm: `.toolchain/pkgs/smashremix-src/src/Blastzone.asm` (warp) and `Accessibility.asm` (`blastzone_explode_gfx`, the GFX
hooks; the setting is ported here although the hooks live outside Blastzone.asm). Helpers:
`decomp/src/ft/ftremix/ftremixblastzone.c` (warp) and `decomp/src/ef/efremixblastzone.c` (GFX). Declared in the appended block
of `decomp/src/ft/ftremix/ftremixhooks.h`. Every vanilla hook is under `#ifdef REGION_REMIX`. `skip_bottom_blast_sfx` (F7) is
not touched.

## Toggles (from `Toggles.asm`)

| Title | Values | Remix default | Read as |
|---|---|---|---|
| `BlastZone Warp (BETA)` (`entry_blastzone_warp`, range 0..3) | 0 OFF, 1 LEFT/RIGHT, 2 TOP/BOTTOM, 3 ALL | 0 | `gmRemixToggle(blastzone_warp)`, bit 1 = LEFT/RIGHT, bit 2 = TOP/BOTTOM |
| `BlastZone GFX` (`entry_blastzone_gfx`, range 0..2) | 0 DEFAULT, 1 OFF, 2 REDUCED | 0 | `gmRemixToggle(blastzone_gfx)` |

Both are read with `gmRemixToggle` (no single player guard), as the asm reads `Toggles.read` directly.

Lead test: `SSB64_REMIX_TOGGLES="BlastZone Warp (BETA)=1"` (LEFT/RIGHT), `=2` (TOP/BOTTOM), `=3` (ALL);
`SSB64_REMIX_TOGGLES="BlastZone GFX=1"` (OFF), `=2` (REDUCED).

## Applied patches

| Asm (patch address) | RAM | Vanilla function (file) | Status |
|---|---|---|---|
| Blastzone.asm `loop_screen_right_` (0xB78DC) | 0x8013CE9C | `ftCommonDeadCheckInterruptCommon` (ftcommon/ftcommondead.c), right bound branch | applied, `ftRemixBlastZoneWarpFromRight` (x = left bound) |
| Blastzone.asm `loop_screen_left_` (0xB790C) | 0x8013CECC | same, left bound branch | applied, `ftRemixBlastZoneWarpFromLeft` (x = right bound) |
| Blastzone.asm `loop_screen_top_1_` (0xB7960) | 0x8013CF20 | same, top branch, `syUtilsRandFloat() < 1/6` (UpFall KO) | applied, `ftRemixBlastZoneWarpFromTop` (y = bottom bound) |
| Blastzone.asm `loop_screen_top_2_` (0xB7970) | 0x8013CF30 | same, top branch, else (UpStar KO) | applied, same helper |
| Blastzone.asm `loop_screen_bottom_` (0xB78A8) | 0x8013CE68 | same, bottom branch (`ftCommonDeadDownSetStatus`) | applied, `ftRemixBlastZoneWarpFromBottom` (y = top bound, stuck-action release) |
| Accessibility.asm `blastzone_explode_gfx` check (0x7D9C4) | 0x801021C4 | `efManagerDeadExplodeMakeEffect` (ef/efmanager.c), before the particle | applied, `efRemixBlastZoneGfxIsOff` (OFF returns NULL) |
| Accessibility.asm scale env (0x7DACC) | 0x801022CC | same, after the effect gobj is made (DObj +0x40/+0x44) | applied, `efRemixBlastZoneScaleEffect` (REDUCED: 0.64F) |
| Accessibility.asm scale prts (0x7DA18) | 0x80102218 | same, particle transform `xf` (+0x1C/+0x20) | applied, `efRemixBlastZoneScaleParticle` (REDUCED: 0.75F) |

Each warp helper returns TRUE when it warps; the vanilla branch then returns TRUE without its KO call. Each returns FALSE while
its bit is off, so vanilla KO runs. The top branch keeps the `syUtilsRandFloat` call before both hooks, as the asm does. The
else branch of that `if` gained braces to hold its hook.

Warp steps, as the asm does them:
- LEFT/RIGHT and TOP (after the bound write): `grab_release_check_` (release a grab or capture, else a ledge hang
  with action 0x54..0x62 falls), then a grounded player (`ga == nMPKineticsGround`) runs `FTStruct.proc_map` to become aerial,
  then `reset_hitbox_interpolation` (active hitboxes stay at 1).
- BOTTOM (after the bound write): stuck actions first. Ground pound drop 0xE3 (Yoshi, Bowser, Giga Bowser), Bowser forward
  throw 0xE6 (Bowser, Giga Bowser) and Kirby forward throw fall 0xE5 (Kirby) release the grab, or, for the drop, Fall (0x1A) after
  250 status tics. Shield break fall 0x9F falls the same way for any fighter. Other actions get the grab/ledge step; then the reset.

## Skipped patches

None. No REMIX-TODO markers in these files.

## Uncertainties (asm line numbers in Blastzone.asm)

- The asm warp exit (`j 0x8013CF44`, Blastzone.asm lines 152, 199, 253, 307, 420) does not set `v0`, so the vanilla return value is whatever the
  reset loop left in `v0`. The port returns TRUE, as the vanilla KO path does. Confirm if a warped fighter stops its interrupts.
- The GFX OFF exit (Accessibility.asm line 129, `j 0x801023C8`) also leaves `v0` undefined. The port returns NULL. Check that no caller reads it.
- The asm treats `a2` as the position struct (`swc1 ..., 0(a2)` and `4(a2)`, `1(a1)`). The port writes `pos`, as the asm's `a1`/`a2`
  point to it, and writes the root DObj translate (`gobj->obj`, asm `lw ..., 0x74(gobj)`).
- Bowser's bomb drop check (Blastzone.asm line 362) compares 0xE3 for Bowser (the ground pound check is line 356): `Bowser.Action.BowserBombDrop` and `Action.YOSHI.GroundPoundDrop`
  are both 0xE3 in the Remix action tables, so the port uses one constant for both.
- The particle hook writes `xf->scale` (LBTransform +0x1C/+0x20). The asm comment says "particle struct"; the offsets match `LBTransform`.
- The effect hook writes the root DObj scale, `scale.vec.f.x/y` (+0x40/+0x44). The 0.64F value is the nearest float to 0.64
  (0x3F23D70A, checked).

## What the lead should see

- `BlastZone Warp (BETA)=1`: a fighter that leaves the left or right edge reappears at the opposite edge, at the same height, with no
  KO star or KO sound. `=2`: leaving the top reappears at the bottom edge; leaving the bottom reappears at the top edge.
  Yoshi ground pound drops and Bowser bomb drops at the bottom edge fall after 250 frames instead of KO. `=3` does both.
- `BlastZone GFX=1`: no explosion at a KO. `=2`: smaller explosion envelope and stars (0.64 and 0.75).

## Build notes

- The new files `ftremixblastzone.c` and `efremixblastzone.c` are picked up by the `file(GLOB_RECURSE)` in CMakeLists.txt, so cmake
  must be re-configured before the build. `build/compile_commands.json` does not list them yet (syntax-checked through the reference flags).
