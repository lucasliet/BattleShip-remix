# F7 hazard: Casino Night Zone (stage 0x9F, nGRRemixStage_CASINO)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` 4400-4576.

## Ported

| Routine | Status | Where |
|---|---|---|
| `casino_night_setup` (4400) | Ported. Returns TRUE for `REMIX_SYM_Hazards__casino_night_setup`. Spawns the stage bumper (`nITKindGBumper`, item 0x17) at (0, 2600, 0) with zero velocity, only when `grRemixHazardsAreOn()`. | `decomp/src/gr/grremix/grremixhazards_casino.c` |
| `casino_night_side_bumper_bounce` (4436, patch 0xBC5C8 / 0x80141B88) | Ported. Hook in `ftCommonWallDamageSetStatus` under `REGION_REMIX`: scale 1.1 and fgm 0x3D6 when the wall surface is above 0x16 on a casino stage, else 0.8. | `decomp/src/gr/grremix/grremixcasino.c`, `decomp/src/ft/ftcommon/ftcommonwalldamage.c`, `decomp/src/ft/ftremix/ftremixhooks.h` |

## Skipped

- `casino_night_side_bumper_knockback` (4480, patch 0x61440 / 0x800E5C40): skipped. The patch site is inside the vanilla
  function at 0x800E5C30, which the decomp names `ftMainGetGroundHitObstacle` (`decomp/src/ft/ftmain.c:3887`). That body is
  the damage-floor switch, not the wall knockback that consumes the hook's surface value (`t8`, jump to 0x800E5C64). No
  C site is verified, so no hook is applied (a half-applied patch would change vanilla behavior).

## REMIX-TODO

- `stage bumper display list` (`grremixhazards_casino.c`): the asm sets the bumper DObj dl to file 0x9C + 0x2D0. The
  file pointer comes from `add_item_` storing `*(MPGroundData + 0x80)` at 0x801313F0 (Hazards.asm 972-977). No port field
  for that offset. The bumper keeps its default display list.
- `surface bytes` (`grremixcasino.c`): the asm reads the bitfield at 0xCE and surface bytes at 0x11F / 0x133 of the N64
  fighter struct. The port reads `MPCollData` wall masks instead (left when `mask_curr == MAP_FLAG_LWALL`, material from
  `lwall_flags` / `rwall_flags`). This mapping is unverified.

## Lead must test

- Stage id `SSB64_VS_STAGE=159` (0x9F). Expect one bumper item at the centre and the bounce changing near side bumpers.
- Compare the bounce strength against the N64 build. The bumper look differs (see the display-list TODO).
