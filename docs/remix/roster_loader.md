# Remix roster loader (`remix/fighters`)

Covers how the port reads the `remix/fighters` blob from the `.o2r` and registers the Remix fighters in the fighter registry: 69 synth fighters (ids
0x1B and 0x1D..0x60) on top of the 27 vanilla ones. Code: `port/remix/remix_roster.{h,cpp}`, `port/remix/remix_synth_ft.h`, `decomp/src/ft/ftport.c`,
`decomp/src/ft/ftremix/ftremixbind.c`. Blob format and ids: "Blob contract" in `docs/remix/torch_factories.md`; roster facts:
`docs/remix/roster_data.md`; registry: `docs/remix/fighters.md` section 6.
Test: `tools/test_remix_blobs.py` for the blob; in the game, a VS match with `SSB64_VS_FKINDS=<fkind>,<fkind>`
(`.claude/skills/remix-smoke-tests/scripts/vs.sh`) and the log line `remix_roster: registered N synth fighters`.
Open gaps: `docs/remix/patch_audit.md`.

## Files

| File | Responsibility |
|---|---|
| `port/remix/remix_roster.{h,cpp}` | Reads the resource, validates the header and the bounds, builds the synth fighters and registers them. Includes no decomp headers. |
| `port/remix/remix_synth_ft.h` | Plain-value C API that creates and frees the `FTData` of a synth. |
| `decomp/src/ft/ftport.c` | Implements `port_fighter_synth_ft_create/destroy`: clones the parent's `FTData` and overwrites the Remix fields. Also `port_fighter_extend_mainmotion`. |
| `decomp/src/ft/ftremix/ftremixbind.c`, `ftremixdeschooks.c` | `ftRemixBindSpecials`, `ftRemixBindKirbyActions`, `ftRemixBindDescriptorHooks`: fill the special statuses and per-fighter callbacks of a descriptor. |
| `port/port.cpp` | Calls `remix_ram_image_load()` and then `remix_roster_load()` right after `port_fighter_seed_vanilla()`, inside `#ifdef REGION_REMIX`. |

Why the `FTData` is assembled in C: `fttypes.h` needs `ssb_types.h`, which is not on the include path of the C++ port, and `CMakeLists.txt` forbids
including `decomp/include` in the port. `ftport.c` is already the link between the decomp and the registry, so the construction lives there. No decomp
header enters C++ code.

## Flow

1. `loadRemixBlob`: `Ship::Context` -> `LoadResource("__OTR__remix/fighters")`. Missing or not a Blob: `warn` and `false`.
2. Header: size >= 16, magic `RMXF`, version 1, `count == std::size(kRemixRoster)` (97), and the 97 records fit in the blob.
3. `patchVanillaAttributeOffsets` (fkinds 13..25, the relocated `o_attributes`) and `patchVanillaMenuZoom` (fkinds 12..26) update the vanilla rows.
4. For every id 0x1B..0x60 except 0x1C, it reads the record (0x80 B, little-endian) and checks `remix_id` against the index and against
   `kRemixRoster[id].remix_id`.
5. Parent: `parent_remix_id` must be vanilla (< 27) and registered. A vanilla parent has fkind = id (`REMIX_ID_TO_PORT_FKIND(id) = id`).
6. Motion arrays: `mainmotion` and `menu` (the submotion). 12 bytes per row; the offset is relative to the start of the motion region
   (`16 + 97 * 0x80`); bounds are checked before reading.
7. All or nothing: every synth is built before any `port_fighter_register`. If one fails, nothing is registered and the vanilla registry stays intact.
   Repeated successful calls do not redo the work.
8. `bindKirbyCopyActions` extends Kirby's main motions with the Remix copy-ability motions (`port_fighter_extend_mainmotion`) and rebinds his specials.

## Field fallback (a synth is a clone of its parent)

`FTData` (`port_fighter_synth_ft_create`):
- Copied from the parent: `particles_script_lo/hi`, `particles_texture_lo/hi`, and every field not listed below. The Remix blob carries no particle data.
- From the Remix: `file_*_id` (9), `file_main_size`, `o_attributes`, `file_anim_size`, `mainmotion`, `submotion` and their counts.
- Own runtime slots (they do not point at the parent): `p_file_*` (9), `p_particle`, `submotion_array_count`. They live inside the block allocated by the C
  side and are released by `port_fighter_synth_ft_destroy`.

`FighterDescriptor`: starts as a full copy of the parent's descriptor (entry appear, opening descs, costume row, skeleton anim base, Yoshi egg, FGMs,
computer attack list, CSS motion/attack1, spotlight, AI routines, default costumes, charge smash, results announce FGM and emblem...). Replaced:
- `ft_data` (the synth's `FTData`), `parent_fkind`, `scale` (the Remix `menu_zoom`), `results_name` (roster name) and the results layout
  (`results_name_lx/scale`, `results_wins_lx`). Without this the results screen would show the parent's name.
- Special statuses, special entry handlers and Kirby copy handlers: `ftRemixBindSpecials` builds them from the Remix action array of the id; a routine
  with no port falls back to the parent's.
- Entry statuses (`entry_appear_status[0..1]`): read from `Character.entry_action.table` in the Remix RAM image (`bindEntryStatuses`).
- Per-fighter callbacks of `ftRemixBindDescriptorHooks` (entry effect for all; ECB override and shield hitlag skip for Crash; custom capture action and
  DK interrupt for Wario and Piano).

## Constraints

- `fp->fkind` is the synth's own (27..96). A `switch (fkind)` in the decomp with no case for it falls to `default`; code that needs the vanilla row
  calls `port_fighter_parent_fkind`.
- Particle scripts and textures are the parent's.
- Format assumptions the loader relies on (written by the Torch factory): motion offsets are relative to the motion region (after the 97 records);
  `menu_*` is the `submotion`; `file_ids` follow the `FTData` order (main, mainmotion, submotion, model, shieldpose, special1..4).
