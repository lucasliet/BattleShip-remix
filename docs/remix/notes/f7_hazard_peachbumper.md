# F7 hazard: Peach's Castle bumper (peach_bumper_, peach_bumper_hitbox_)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 537-602 and 1260-1308.

## Patches

| Asm | RAM | Vanilla function | Status |
|-----|-----|------------------|--------|
| `peach_bumper_` (ROM 0x86CB4) | 0x8010B4B4 | `grCastleMakeGround` (`decomp/src/gr/grcommon/grcastle.c`) | Already applied before this wave: `grRemixStageHasPeachBumper(gkind) && grRemixHazardToggleIsOn()` gates `grCastleInitAll()`. Stage list (0, PCASTLE_DL, PCASTLE_O, PCASTLE_BETA, LMAO_CASTLE) and hazard toggle match the asm. No edit in this wave. |
| `peach_bumper_` `_bowser_stadium` branch | 0x8010B4B4 | `grCastleMakeGround` | Bombs NOT ported (another worker). The `REMIX-TODO(items)` in grcastle.c is kept. |
| `peach_bumper_hitbox_` (ROM 0xF8150) | 0x8017D710 | `itGBumperMakeItem` (`decomp/src/it/itground/itgbumper.c`) | Applied in this wave under `#ifdef REGION_REMIX`. |

## Hitbox port (peach_bumper_hitbox_)

- Classic stages (`grRemixStageHasPeachBumper`): castle knockback (`knockback_weight = ITBUMPER_CASTLE_KNOCKBACK`, `angle = ITBUMPER_CASTLE_ANGLE`). Same values as the vanilla castle block, now also for the Remix clones.
- Casino (`nGRRemixStage_CASINO`): `fgm_id = 0x03D5`, `size = 250.0F` (0x437A0000), low byte of `knockback_scale` = 0x40 (asm `sb t4, 0x0143(v0)`, where v0 is `&ip->attack_coll`; 0x143 is the least significant byte of the u32 at 0x140).
- Any other stage: no change (standard path of the asm).

## Uncertainties

- The asm writes only the byte 0x143, so the other three bytes of `knockback_scale` keep their value. The port masks the low byte with `(knockback_scale & ~0xFF) | 0x40`. This is correct only if the port stores the u32 natively, as the notes of the blanket u32 swap say. Not verified in-game.
- The vanilla castle block in `itGBumperMakeItem` is kept as the `#else` branch; the `#ifdef REGION_REMIX` branch runs only the helper, so the Remix build never executes the old castle check.
- Only the non-`REGION_REMIX` syntax was checked by `syntax-check.sh`; the `REGION_REMIX` branch of `itgbumper.c` still needs a build.

## Files

- `decomp/src/gr/grremix/grremixpeachbumper.c/.h` (new, `#ifdef PORT`): `grRemixPeachBumperSetupHitbox(GObj *)`.
- `decomp/src/it/itground/itgbumper.c`: include under `REGION_REMIX`, call site in `itGBumperMakeItem`.

## Test stage ids (decimal for `SSB64_VS_STAGE`)

- Peach's Castle 0x00 = 0; PCASTLE_DL 0x6B = 107; PCASTLE_O 0x6C = 108; PCASTLE_BETA 0x96 = 150; LMAO_CASTLE 0xC9 = 201 (classic knockback and bumper).
- CASINO 0x9F = 159 (casino hitbox).
- BOWSERB 0x43 = 67 (bombs, not ported here).
