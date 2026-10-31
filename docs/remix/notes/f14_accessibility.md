# F14 audit: Accessibility.asm (Smash Remix 2.0.1)

Source: `.toolchain/pkgs/smashremix-src/src/Accessibility.asm` (198 lines). Vanilla RAM to decomp: `build/remixasm/refs/vanilla_funcs.txt`.
The patch RAM addresses are from the `OS.patch_start(rom, ram)` calls; ROM = RAM - 0x80084800 for the overlay code.

## Patch table

| # | Asm (line) | RAM | Vanilla function (file:line) | Effect | State | Port location |
|---|-----------|-----|------------------------------|--------|-------|---------------|
| 1 | flash_guard (18-31) | 0x80115E10 | ifScreenFlashMakeInterface (if/ifscreenflash.c:64) | Flash Guard skips the screen flash | Implemented | if/ifscreenflash.c:71, gmRemixToggle(flash_guard) |
| 1b | flash_guard checks in DekuNut.asm and Flashbang.asm (cited at line 15) | - | it/itremix/itremixscreenflash.c:53, :63 | item flashes skipped | Implemented | ITREMIX_FLASH_GUARD_ON |
| 1c | flash_guard in Lighting.asm (line 16) | - | it/itremix/itremixlightning.c:141, :152 | lightning flashes skipped | Implemented | ITREMIX_FLASH_GUARD_ON |
| 2 | ParamMakeEffect save_gobj (36-52) | 0x800EABF4 | ftParamMakeEffect (ft/ftparam.c:2146) | stores the fighter (`lw t7, 0x84(a0)`, FTStruct*) on every call | Missing | new gm/gmremixaccessibility.c |
| 3 | ParamMakeEffect clear_gobj (54-58) | 0x800EB388 | ftParamMakeEffect epilogue | clears the stored fighter on return | Missing | new gm/gmremixaccessibility.c |
| 4 | screenshake_toggle, RTTF branch (75-89) | 0x80100964 | efManagerQuakeMakeEffect (ef/efmanager.c:3899) | off-screen CPU in Remix RTTF does not shake | Missing | new gm/gmremixaccessibility.c |
| 4b | screenshake_toggle, toggle (91-107) | 0x80100964 | efManagerQuakeMakeEffect | DEFAULT / LIGHT (0) / OFF (-1) severity | Implemented | ef/efremixscreenshake.c:17 efRemixQuakeSeverity, call at ef/efmanager.c:3934 |
| 5 | blastzone_explode_gfx check_toggle (118-130) | 0x801021C4 | efManagerDeadExplodeMakeEffect (ef/efmanager.c:4915) | OFF skips the explosion | Implemented | ef/efmanager.c:4927, efRemixBlastZoneGfxIsOff() |
| 6 | blastzone scale_env (140-158) | 0x801022CC | efManagerDeadExplodeMakeEffect | REDUCED: effect DObj scale 0.64 (x, y) | Implemented | ef/efmanager.c:4965, efRemixBlastZoneScaleEffect() |
| 7 | blastzone scale_prts (170-191) | 0x80102218 | efManagerDeadExplodeMakeEffect | REDUCED: star particle scale 0.75 (x, y) | Implemented | ef/efmanager.c:4941, efRemixBlastZoneScaleParticle() |

Note on 5: the asm OFF path jumps to 0x801023C8 (the vanilla epilogue), so the vanilla return value is whatever v0 held. All callers
(ft/ftcommon/ftcommondead.c:321, :363, :404; gm/gmremixsmashketballscore.c:80; ef/efmanager.c:4996) ignore the return value, so the port's
`return NULL` is not observable.

## Missing work: ftParamMakeEffect store and RTTF suppression

Patch 2 and 3 store the fighter as `ParamMakeEffect.gobj`. The stored value is `t7 = gobj->user_data` (FTStruct*, offset 0x84 of the
fighter GObj). It is set on entry to ftParamMakeEffect and cleared on return. Patch 4 reads it during a quake: when the stage is
Remix RTTF and `lh 0x018C` has mask 0x0004, the quake does not shake (severity -1, no shake branch).

The mask 0x0004 is the low byte (FTStruct 0x018D) bit 0x04. Per `build/remixasm/refs/struct_offsets.txt` it is `is_magnify_show`,
the same flag used in ft/ftremix/ftremixaiteching.c:54 (`ftRemixAiIsOffStage`). The stage check uses
`gSCManagerBattleState->gkind == nGRRemixStage_REMIX_RTTF` as in gr/grremix/grremixrttf.c:14.

Files written (new, port only, `#if defined(PORT) && defined(REGION_REMIX)`):
- `decomp/src/gm/gmremixaccessibility.h`
- `decomp/src/gm/gmremixaccessibility.c`

### Hooks for the lead (existing files, not edited here)

1. `decomp/src/ft/ftparam.c`, function `ftParamMakeEffect` (line 2146). Insert after the declarations, before `effect = NULL;` (line 2155):
   `gmRemixAccessibilityParamEffectBegin(fp);` inside `#ifdef REGION_REMIX`, with `#include <gm/gmremixaccessibility.h>` in the file.
2. Same function, insert `gmRemixAccessibilityParamEffectEnd();` before each return:
   - line 2164 (`return NULL;`, Remix new-effect path),
   - line 2169 (`return effect;`, Remix new-effect path),
   - line 2510 (`return effect;`, final).
   The asm clears at the single epilogue. The Remix path at 2164 and 2169 is a port-only early return, so it needs its own clear.
3. `decomp/src/ef/efremixscreenshake.c`, function `efRemixQuakeSeverity` (line 17). Insert as the first statement:
   `if (gmRemixAccessibilityIsQuakeSuppressed()) return -1;` (include `gm/gmremixaccessibility.h`). `-1` is the asm's invalid severity.
   This must come before the toggle switch, because the asm RTTF check runs regardless of the toggle value.
   The call site is `decomp/src/ef/efmanager.c:3934`, under `#ifdef REGION_REMIX`. No change there.

Order matters: the store must run before `efManagerQuakeMakeEffect` is called from inside ftParamMakeEffect. Quakes created outside an
ftParamMakeEffect call see NULL and shake normally, same as the asm.

## Uncertainties

- Patch 3 (clear): the asm clears at the vanilla epilogue, so nested ftParamMakeEffect calls clear the outer value too. The
  hooks above keep that behaviour, since the inner call also clears.
- The asm reads `lh 0x018C`, which is 16 bits. Only the low byte (0x018D) matters for mask 0x0004. The field name comes from the
  generated `struct_offsets.txt`, which is the same mapping ftremixaiteching.c already relies on.
- No build, no game run, no git (per the brief). Syntax check: `scripts/remix/syntax-check.sh decomp/src/gm/gmremixaccessibility.c` prints `syntax-check OK`.
