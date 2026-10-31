# F14 notes: VsDemo.asm (Smash Remix auto-demo fighters and stages)

Source: `.toolchain/pkgs/smashremix-src/src/VsDemo.asm`. New module: `decomp/src/sc/sccommon/scremixvsdemo.{c,h}`
(under `#if defined(PORT) && defined(REGION_REMIX)`). No existing file was edited. Syntax check: OK. Not built, not run in game.

## Patches and hook sites

`OS.patch_start(ROM, RAM)`: the RAM address is the one used below.

| asm patch | RAM | Decomp function (vanilla_funcs.txt) | Hook | Port function |
|---|---|---|---|---|
| enable_focused_remix_chars_ | 0x80131CDC | mnTitleSetDemoFighterKinds (mntitle.c:308) | Epilogue: insert the call after mntitle.c:351 (the `remix_debug_demo_fkind` lines), before `return 0;` at mntitle.c:353 | `scRemixVsDemoPickFocusFighters()` |
| enable_nonfocused_remix_chars_ | 0x8018D928 | scAutoDemoGetFighterKind (scautodemo.c:527) | For player >= 2, instead of the shuffled pick (scautodemo.c:537-551 branch) | `scRemixVsDemoPickNonFocusedFighter()` |
| enable_remix_stages_ | 0x8018DA20 | scAutoDemoInitDemo (scautodemo.c:562) | Replace the gkind store at scautodemo.c:570 | `scRemixVsDemoPickStage(dSCAutoDemoGroundOrder[...])` |
| name table (`SinglePlayer.name_texture_table`) | 0x8018DBD4, 0x8018DC08 | scAutoDemoInitSObjs (scautodemo.c:613) | Name sprite lookup, scautodemo.c:656-660 | `scRemixVsDemoGetNameTextureOffset(fkind)` (not wired, see gaps) |

Map note: `vanilla_funcs.txt` mixes overlays at the same RAM range. The sc overlay ranges used above come from the next sc function start
(scAutoDemoGetFighterKind to 0x8018D954, scAutoDemoInitDemo to 0x8018DB18, scAutoDemoInitSObjs to 0x8018DCC4). The title range
comes from mnTitleSetDemoFighterKinds (0x80131BC4) to mnTitleInitVars (0x80131CF4). The 0x80131CDC patch is 0x18 bytes before the end,
which matches an epilogue (`lw ra`, `jr ra`).

## Behaviour ported

- Focused demo fighters (slots 0 and 1): two distinct Remix fighters with `variant_type == REMIX_VARIANT_NA`, taken from
  `kRemixRoster` (`remix_roster_data.h`). Range is all `kRemixRoster` entries, as in the asm (`NUM_CHARACTERS`). Unlock masks are ignored, as in the asm.
- Non-focused demo fighters (players 2 and up): NA Remix fighter different from both focused fighters.
- Demo stage: random stage id in `[0, 0xD7)` with class `BATTLE` (0) in `Stages.class_table` (RAM 0x80457E6C), not Rest Area (0x8B).
  The asm range is exclusive, so 0xD7 (HTP_FALL) is never drawn. Matches the asm, not a bug in the port.
- The asm does not touch `demo_mask_prev`, `demo_first_fkind`, `sSCAutoDemoFighterMask`, or `demo_gkind_order`. The port keeps the same.

## Deviations and gaps

1. **Name plates (0x8018DBD4 / 0x8018DC08): not wired.** The asm points the name lookup at `SinglePlayer.name_texture_table`
   (RAM 0x804E96B0, 4-byte entries, index = fkind * 4, plus 0x10). Those offsets index Remix texture data, not the vanilla
   `llCharacterNamesFileID` file, and the port has no loader for that file. Until it exists, the port keeps the parent-name
   fallback (scautodemo.c:656-657). `scRemixVsDemoGetNameTextureOffset` returns the asm value, or -1 when the RAM image is missing.
2. **Non-focused port-3 rule (0x8018D928): skipped.** The asm also rejects `lbu t0, 0x0023(t2)` when the slot is port 3. The register
   `t2` is not set in the patched code and its meaning is unknown, so the extra rule is not ported. Verify with a disassembly of
   0x8018D928 before adding it.
3. **Stage hook site: unverified.** 0x8018DA20 is 0x90 bytes into scAutoDemoInitDemo. The gkind store at scautodemo.c:570 is the
   likely consumer, but the asm epilogue of the hook (restoring t6, t0, t3 with `or t3, v0, r0`) must be checked in the RAM disassembly
   that the vanilla code reads t3 after the hook.
4. **Title order vs `SSB64_DEMO_FKINDS`.** The asm hook runs after mntitle.c:350-351, so the Remix pick overrides the port's
   `remix_debug_demo_fkind` override. Decide the order when wiring. `scmanager.c:976` (SSB64_START_SCENE = Auto-Demo) does not run the
   title function, so the Remix focus pick is not applied on that path.
5. **Debug loop (commented-out test lines).** The asm stage debug path (test counter) is not ported; it is commented out in the source.
6. **RNG.** `syUtilsRandIntRange(n)` returns `[0, n)`, the same as `Global.get_random_int_` with `lli a0, n`. This is assumed from the
   `NUM_CHARACTERS` / `MAX_STAGE_ID` usage in the asm, not measured.

## Files

- `decomp/src/sc/sccommon/scremixvsdemo.h`: four extern declarations.
- `decomp/src/sc/sccommon/scremixvsdemo.c`: helpers `scRemixVsDemoIsEligibleFighter`, `scRemixVsDemoPickEligibleFighter`,
  `scRemixVsDemoIsBattleStage`, plus the four public functions above.
- `docs/remix/notes/f14_vsdemo.md`: this file.

## Lead wiring checklist

- Add `sc/sccommon/scremixvsdemo.c` to the build (PORT + REGION_REMIX only).
- Add the four hook calls at the sites in the table, with the `#include <sc/sccommon/scremixvsdemo.h>` line in each file.
- Test in game: title demo with several rolls (fighters differ and are NA), a CPU-only demo battle (players 2 and up), and a stage roll with no Rest Area.
