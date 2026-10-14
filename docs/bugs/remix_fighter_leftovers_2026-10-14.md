# Remix fighter leftovers (Step 2): partial wave (2026-10-14)

Second wave of the REMIX-TODO list. Status: about 30 percent. Build is clean and the special-move matrix has no abort
or crash (Young Link 31, Sonic 59, Dragon King 71 NSP/DSP/USP re-run after the wave; the 11-fighter matrix of the
previous wave also passes, Metal `nextDrawable` flakes aside).

## Landed (uncommitted before this checkpoint, all `#ifdef REGION_REMIX`)

- Link family: Bombchu swap of the status table for a Young Link bomb (`itMainSetStatus`, owner id kept in
  `coll_data.cliff_id` by `ftRemixYoungLinkBombSetOwner`), per-owner bomb file via `ftRemixLinkSharedBombFile`
  (`itLinkBombMakeItem`, bloat scales, attack events), and `cpu_pull_out_bombs_` in `ftcomputer.c` for Link clones,
  Peach and Marina (Marina only when her charge word at 0x0ADC is not 0 or 1).
- Dragon King: electric ball (NSP).
- Sonic: classic table, PSI magnet, spring bounce, USP/DSP changes. WaddleDoo beam star `WPDesc`.
  `itRemixKeepsFlashFlags` for Cloud exists but is not wired into `itprocess.c` yet (inert helper).

## Open

- Dragon King Piano and JKirby (14 routines); Roy counter source; Flashbang GFX; Cloud `enable_flash` wiring.
- Shared vanilla patches: `drain_flash_fix_` (0x8010E9E4), knockback 0x800E44EC, absorb 0x800E33AC, Samus grapple,
  Marina item pull, Dedede jump, Kirby cutter/rock, fireball bounce, bob-omb FGM.
- Subsystem map and order (Size, GFX/GFXRoutine, CharEnvColor, Reflect, PokemonAnnouncer, AI) in
  `docs/remix/implementation_plan.md`.
- Sonic/Tails entry (Tails floats beside Sonic) is tracked in
  `remix_entry_effects_and_polygon_entry_2026-10-14.md`.

## Second wave (same day, built and run)

Landed: Dragon King drain flash (`ifCommonPlayerDamageUpdateDigits`), Piano command grab and absorb (`0x8014C8FC`,
`0x800E33AC`, registered `custom_capture_dk_interrupt`), Kirby `clone_power_action_swap_`, Samus grapple descriptor for Dark
Samus, bob-omb fuse FGM, Flashbang flags, Cloud flash flags in `itProcessProcItemMain`, `JKirby` grounded script, and the
Size subsystem phase (a) (`ftremixsize.c`: tables, top joint and ECB, camera). Regression matrix (Mario, Kirby, Young Link,
Sonic, Dragon King, 18 runs) has no abort.

Still open: Size phases (b) physics/weight and (c) cosmetics, the Kirby hat routines, the Piano `extra_part_visibility_`
patch (0x800E7220), the Remix knockback-angle randomizer (`Knockback.asm`), and the Roy counter (commented out in the
Remix source, intentionally not ported).

Sonic/Tails entry (fixed): `DKShared.asm barrel_alternate` picks the right or left Tails struct by the fighter's facing.
The port always used the left one; `ftRemixEntryMakeDonkeyBarrel` now selects the right row (`0x2A28`/`0x68C8`) when
`entry.lr == +1`.
