# F7 hazard group h7 (BanzaiFearBoo): notes

Status: NOT PORTED. `decomp/src/gr/grremix/grremixhazards_banzaifearboo.c` is unchanged (returns FALSE). No C file was
edited, so the stages below still run without hazards, as they do today. Same outcome as group h3 (see f7_hazard_h3.md).

## Stage ids and function_table entries (Stages.asm, function_table starts at line 3185)

| Stage | Id | function_table entry | Reachable hazard |
|---|---|---|---|
| Mad Monster Mansion | 0x40 (nGRRemixStage_MADMM) | Hazards.mmm_setup (Stages.asm:3252) | yes, mmm_setup / mmm_main |
| Peach's Castle II | 0x44 (nGRRemixStage_PEACH2) | function.CLONE (Stages.asm:3256) | no: peach_castle_II_setup_ is not referenced by any table |
| Remix RTTF | 0xBD (nGRRemixStage_REMIX_RTTF) | Hazards.remix_rttf_setup_ (Stages.asm:3377) | yes |
| Big Boo's Haunt | 0xC0 (nGRRemixStage_BIG_BOOS_HAUNT) | Hazards.bbh_setup_ (Stages.asm:3380) | yes |
| Spawned Fear | 0xC3 (nGRRemixStage_SPAWNED_FEAR) | Hazards.spawned_fear_acid_setup_ (Stages.asm:3383) | yes |

Open question for the lead: Peach's Castle II maps to CLONE, and no other asm references `peach_castle_II_setup_`, so
Banzai Bill is probably dead code in this Remix build. Confirm against the Remix ROM before porting it.

## Items

- `src/items/BulletBill.asm` (402 lines), Banzai Bill item 0x3D: not ported. Needs `itremixbulletbill.c/.h` with
  `dITRemixBulletBillItemDesc`.
- `src/items/Cacodemon.asm` (54 lines), Cacodemon item 0x3F: not ported. Needs `itremixcacodemon.c/.h` with
  `dITRemixCacodemonItemDesc`.

## Stage asm files

- Present: `src/stages/bbh.asm` (Big Boo's Haunt).
- Missing under the names in the brief: Peach's Castle II, Spawned Fear, Mad Monster Mansion, Remix RTTF. Their geometry
  must come from the stage map file (`Stages.stage_file_table`), and the offsets must be verified before porting.

## Routine ranges (Hazards.asm)

- 2768-2978 remix_rttf_setup_, rttf_bomb_2_ (2916), rttf_bumpers_ (2956). Vanilla hook: line 952 `_load_rttf_files`
  and the call at 2958 (`jal rttf_bumpers_`), which needs a vanilla patch under `#ifdef REGION_REMIX`.
- 8072-8382 peach_castle_II_setup_ and banzai_bill_* (factory 8133, stage_setting 8225, main 8298, destroy 8357, blast_zone 8376).
- 8383-8421 bbh_setup_ (inside 8383 scope), 8422-8448 boo_sound_.
- 8449-8868 spawned_fear_acid_setup_ (8449) and spawned_fear_acid_main_ (REMIX_SYM 0x80490B7C), cacodemon_* (factory label at 8495, cacodemon_coordinates at 8598).
- 8869-9255 cacodemon_thunderball_ and thunderball_* (projectile, collision, hit, stage_setting). Uses raw projectile
  structs at 0x80491400 and 0x804913D0, so it needs a projectile/weapon port first.
- 9656-9772 mmm_setup and mmm_main.
- 9256-9618 are Crash/Peach BTT/BTP bonus-mode routines: skipped (1P bonus modes, out of scope).

## Blockers for a faithful port

1. Items: Banzai Bill and Cacodemon use raw ITStruct offsets and the Remix item vars. Each needs its own item file
   (the f5 item interface) before the hazards can spawn them. Cacodemon is also spawned by Spawned Fear.
2. Thunderball projectile: raw projectile/properties structs (0x80491400, 0x804913D0, 0x804913C4). No port exists.
3. Stage geometry: Peach's Castle II, Spawned Fear, MMM and RTTF stage files are not under the names in the brief.
4. Hazard state lives in fixed Remix RAM (for example 0x80491xxx). It must move to file-local static structs, cleared at setup.
5. `Toggles.entry_hazard_mode` checks go through `grRemixHazardsAreOn()` / `grRemixMovementIsOn()`.

## Suggested split for the lead

- Phase 1: Mad Monster Mansion (mmm_setup, mmm_main), which needs no item. Stage id 0x40.
- Phase 2: Big Boo's Haunt (bbh_setup_, boo_sound_), stage id 0xC0, with `stages/bbh.asm`.
- Phase 3: Banzai Bill item and Peach's Castle II, after confirming the table entry.
- Phase 4: Cacodemon item, Spawned Fear (0xC3), then the thunderball projectile.
- Phase 5: Remix RTTF bombs and bumpers (0xBD), with the vanilla hook at 952 and 2958.
