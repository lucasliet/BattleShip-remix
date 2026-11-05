# Remix gameplay subsystems: where they live and what to remember

Map of the Smash Remix gameplay subsystems in the port, with the findings that are not obvious from the code. Per-subsystem
bug write-ups: `remix_gfx_and_gfxroutine_2026-10-16.md`, `remix_kirby_copy_actions_2026-10-16.md`. Test: the special matrix
(`special_matrix.sh`, 80 fighters by NSP, DSP and USP), `stage_sweep.sh` (175 stage ids) and `item_cpu_sweep.sh` (13 standard
items), run with `parallel.sh 6 ...` (`.claude/skills/remix-smoke-tests/SKILL.md`).

| Subsystem | Code |
|---|---|
| GFX, GFXRoutine | `decomp/src/lb/lbremixgfx.c`, `decomp/src/gm/gmremixcolscripts.c` |
| CharEnvColor | `decomp/src/ft/ftremix/ftremixenvcolor.c`, `ftdisplaymain.c`, cloak, Dracula and Reaper colours |
| Reflect | `decomp/src/ft/ftremix/ftremixreflect.c`, hooks in `ftmain.c`, `wpprocess.c`, `itprocess.c`, `wpmain.c`, `itmain.c` |
| Size | `decomp/src/ft/ftremix/ftremixsize*.c` |
| Kirby copy and hats | `ftremixkirbycopy.c`, `ftremixkirbyhats.c`, `ftremixkirbypower.c`, `ftremixbind.c` |
| Knockback, damage, poison, surface | `ftremixknockback.c`, `ftremixdamage.c`, `ftremixpoison.c`, `decomp/src/gr/grremix/grremixsurface.c` |
| Leftover vanilla patches | `ftremixleftovers.c` |

## Findings worth keeping

- The Remix RAM image keeps words in host order: tables read as `u32` have their two halfwords in logical order
  (`FTKirbyCopy` is `(copy_id << 16) | hat`), and byte streams (particle scripts, texture pixels) must be copied back to big
  endian first (`lbRemixGfxCopyBytes`).
- Kirby's copy table is the Remix `Character.kirby_inhale_struct.table`, indexed by Remix character id; the vanilla table of
  the Kirby file only has 27 rows.
- The harness actors (`SSB64_REMIX_GFX_TEST`, `SSB64_REMIX_COLANIM_TEST`, `SSB64_REMIX_KIRBY_COPY_TEST`,
  `SSB64_VS_P2_HUMAN`, `SSB64_MAPS_START_GKIND`) are documented in the `remix-smoke-tests` skill.
- Copying a power in the appear status (frame 1) crashes: the Kirby copy harness waits until frame ~405.
- On macOS, runs can die on the Metal `nextDrawable` flake; a rerun alone passes, and only a reproducible crash is a bug.

Open gaps: `docs/remix/patch_audit.md`, groups g5 and g7.
