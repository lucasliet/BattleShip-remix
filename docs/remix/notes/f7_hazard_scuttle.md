# F7 hazard: Scuttle Town setup and background fade notes

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 7276-7388 (`scuttle_town_setup_`) and 7389-7470
(`scuttle_town_bg_fade_`). Stage id: `nGRRemixStage_SCUTTLE_TOWN` (0xBF) in `grremixids.h`.

## Files

- `decomp/src/gr/grremix/grremixscuttle.c/.h`: `grRemixScuttleTownSetup()` runs the background fade and the minion factory.
- `decomp/src/gr/grremix/grremixhazards_scuttle.c`: dispatch, calls `grRemixScuttleTownSetup()` and returns TRUE.

## Ported

| Asm | C | Notes |
|---|---|---|
| `scuttle_town_setup_` fade part | `grRemixScuttleFadeSetup` | Fade object (proc prio 3, display prio 6, like Time Twister). Night and day background addresses read from the ground data (+0xC8/+0x48, image +0x34). |
| `scuttle_town_setup_` training check | `grRemixScuttleFadeIsWanted` | Training skips the fade unless the stage background is shown (see REMIX-TODO(toggles)). |
| `scuttle_town_setup_` minion part | `grRemixScuttleMinionSetup` | Header = ground data - 0x14, then `grRemixScuttleMinionFactoryMake`, guarded by `grRemixHazardsAreOn()`. |
| `scuttle_town_bg_fade_` | `grRemixScuttleFadeUpdate`, `grRemixScuttleFadeStart`, `grRemixScuttleFadeStepAlpha` | Alpha step 4, middle 0x80 writes the glass color to ground data +0x4C, max 0xFC reverses, alpha 0 ends the transition. |

## Skipped and why

- The asm draw_rectangle at setup is replaced by the fade object, whose state starts at rgb 0, alpha 0 (the same as the asm
  color 0, alpha 0 rectangle).
- The asm `b _end` at Hazards.asm line 7338 ("TODO: remove when minions stop crashing") disables the whole hazard part
  (the minion factory) in the Remix source. The port runs the factory, as the Scuttle Town minion is enabled for this port.

## REMIX-TODO

- `REMIX-TODO(palette)` in `grRemixScuttleStagePaletteIndex`: the asm walks group 0x2 (RAM 0x800466F8) to the palette index
  (+0x88). It returns 0, so the transition never starts. The fade object exists and the transition code is complete.
- `REMIX-TODO(bg image object)` in `grRemixScuttleFadeSwitchBackground`: the asm writes the background address to the bg image
  object (RAM 0x80046800 -> +0x74, field +0x44). Not written, the background stays the same.
- `REMIX-TODO(toggles)` in `grRemixScuttleTrainingShowsStageBg`: Training.entry_bg + 0x4 has no port accessor. The normal
  training background (no fade) is assumed.

## Uncertainties

- Fade object priorities (3 and 6) are copied from Time Twister, not from the asm draw_rectangle group 0xD.
- The minion header uses ground data - 0x14 (asm `addiu t1, t1, -0x14`), the same as the Peach Castle II and Metallic setups.
- The setup reads the stage file from the ground data pointer directly (asm `lw t0, 0x1300`, offsets +0x48 and +0xC8), while
  the minion uses the header (ground - 0x14). Both are the asm behaviour.

## Lead must test

- Stage id `SSB64_VS_STAGE=191` (0xBF). With the palette TODO, only the minion should show, after 1500 tics, with
  `SSB64_REMIX_ITEM_LOG` set. The fade stays transparent until the palette walk is ported.
