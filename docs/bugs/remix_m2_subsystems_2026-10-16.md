# Remix gameplay subsystems, milestone M2 (2026-10-16)

Status of ROADMAP M2 at checkpoint CP2a. The GFX and GFXRoutine details are in `remix_gfx_and_gfxroutine_2026-10-16.md`.

| Task | Result | Code |
|---|---|---|
| T2.1 GFX, GFXRoutine | done | `lbremixgfx.c`, `gmremixcolscripts.c` |
| T2.2 CharEnvColor | done (display list fixes open) | `ftremixenvcolor.c`, `ftdisplaymain.c`, cloak, Dracula, Reapers |
| T2.3 Reflect | done (CPU hooks in M7) | `ftremixreflect.c`, hooks in `ftmain.c`, `wpprocess.c`, `itprocess.c`, `wpmain.c`, `itmain.c` |
| T2.4 Size phases b and c | done, sources by the menu open | `ftremixsize*.c` |
| T2.6 Kirby | table and hats done, 3 crashing copies open | `ftremixkirbycopy.c`, `ftremixkirbyhats.c` |
| T2.7 other systems | done, gaps in T2.7b | `ftremixknockback.c`, `ftremixdamage.c`, `ftremixpoison.c`, `grremixsurface.c` ... |
| T2.8 leftovers | done except two sites | `ftremixleftovers.c` |

## Evidence

Lead-run validation after the worker waves, with `parallel.sh` (6 game instances at once): special matrix 240/240 (80 fighters
by NSP, DSP and USP), 175/175 stages, 13/13 standard items, no abort/assert/fatal line in any log. Two runs died on the
macOS Metal `nextDrawable` flake and one lost its screenshot to a stray Esc; all three passed on rerun.

## Findings worth keeping

- The Remix RAM image keeps words in host order: tables read as `u32` have their two halfwords in logical order
  (`FTKirbyCopy` is `(copy_id << 16) | hat`), and byte streams (particle scripts, texture pixels) must be copied back to big
  endian first (`lbRemixGfxCopyBytes`).
- Kirby's copy table is the Remix `Character.kirby_inhale_struct.table`, indexed by Remix character id; the vanilla table of
  the Kirby file only has 27 rows.
- The harness actors (`SSB64_REMIX_GFX_TEST`, `SSB64_REMIX_COLANIM_TEST`, `SSB64_REMIX_KIRBY_COPY_TEST`, `SSB64_VS_P2_HUMAN`,
  `SSB64_MAPS_START_GKIND`) are documented in the `remix-smoke-tests` skill.
- Copying a power in the appear status (frame 1) crashes: the harness waits for frame 330.
