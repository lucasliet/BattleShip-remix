# F7 hazard: Scuttle Minion (Scuttle Town) notes

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 7480-7759 (`minion_factory_`, `minion_stage_setting_`,
`minion_main_`, `minion_destroy_`, `minion_blast_zone_`) and `src/items/ScuttleMinion.asm`.

## Files

- `decomp/src/gr/grremix/grremixscuttleminion.c/.h`: `grRemixScuttleMinionFactoryMake(void *stage_header)` creates the
  factory process (`minion_factory_`). `grRemixScuttleMinionStageHeaderRef()` is the p_file address for the item.
- `decomp/src/it/itremix/itremixscuttleminion.c/.h`: `dITRemixScuttleMinionItemDesc` (item id 0x3E,
  `ITREMIX_SCUTTLEMINION_KIND`), spawn `itRemixScuttleMinionMakeItem`.

The dispatch of `Hazards.*` for Scuttle Town (`grremixhazards_scuttle.c`) is not touched; the Scuttle Town setup calls
`grRemixScuttleMinionFactoryMake` with its stage header.

## Ported

| Asm routine | C | Notes |
|---|---|---|
| `minion_factory_` | `grRemixScuttleMinionFactoryProcUpdate` | 1500-tic wait, roll 0x5DC == 0x50, forced spawn at 3000 tics, timer reset at spawn. Active check is `grRemixHazardsMatchHasStarted()`, same as the asm training-mode / match-clock check. |
| `minion_stage_setting_` | `itRemixScuttleMinionMakeItem` | Life timer 0, hitbox size 150, damage 5, angle 0x361, bkb 0x50, kbs 0x25, fgm 0x26, `attack_state` 1. |
| `minion_main_` | `itRemixScuttleMinionMainProcUpdate` | Life counter, destroy when life passes 390 tics. |
| `minion_destroy_` | `itRemixScuttleMinionDestroyProcHurtbox` | Plays FGM 0x3E8, returns TRUE. Wired to the hurtbox slot like the Robot Bee; the asm does not say which state uses it. |
| `minion_blast_zone_` | not wired | REMIX-TODO(blast zone), same as the Robot Bee. |

## Skipped and why

- Animation tracks: the asm `_movement` block (joint track 0x908, material track 0x9B4) is entirely commented out, so the
  minion has no animation. The factory does not draw a facing either.
- The `minion_stage_setting_` sfx timer (0x44 = 0x30) and the laser/sfx block of `minion_main_` are commented out in the asm.
- `minion_stage_setting_` ignores its `a2` argument and spawns at `minion_coordinates` (zero). The factory passes zero, so
  `pos` is used as given.
- The item's own state table in `ScuttleMinion.asm` (lines 64-123) references Waddle Dee routines (`waddle_dee_*`,
  `minion_walk_main`, ...), which look copied from another item. Only the Hazards routines are ported here.
- Object id 0x03F5 passed to `0x8016EA78` (`itManagerMakeItemSetupCommon` in this port) is not a parameter of that function.
  The call uses item kind 0x3E only.

## REMIX-TODO

- `REMIX-TODO(ITStruct)` in `itRemixScuttleMinionSetupHitbox`: unnamed fields 0x158 |= 0x10, 0x2D3 |= 0x08, 0x1D0, 0x1D4, 0x35C.
- `REMIX-TODO(blast zone)` in `itRemixScuttleMinionDestroyProcHurtbox`: ITStruct 0x398 routine table not wired.
- `REMIX-TODO(object id)` in `grRemixScuttleMinionSpawn`.

## Uncertainties

- The item flag: the asm passes 1 to `0x8016E174` as its unknown argument (ScuttleMinion `minion_stage_setting_`,
  Hazards.asm line 7581-7583). The port passes 1 to `itManagerMakeItem`; the proc's `flags` argument is not used.
- The item file offset 0x40 comes from `ScuttleMinion.asm` `item_info_array` (line 60). The Robot Bee uses 0xE0, which is
  a different item footer.
- Lead must test: the Scuttle Town stage id with `SSB64_VS_STAGE=<decimal>` after the stage setup calls
  `grRemixScuttleMinionFactoryMake(stage_header)`, and that the minion appears after 1500 tics with `SSB64_REMIX_ITEM_LOG` set.
