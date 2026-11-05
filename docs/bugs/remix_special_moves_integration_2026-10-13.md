# Remix special moves: integration crashes and LP64 field mapping

Failures found after the fan-out port of every Remix fighter's special moves. Code: `decomp/src/ft/ftremix/<fighter>/`,
`ftremixfields.{h,c}`, `ftremixchartables.c`, `ftremixdeschooks.c`, and the vanilla patch sites listed below. Test: the
attract-demo pairs (`SSB64_START_SCENE=61 SSB64_DEMO_FKINDS=a,b`, all 16 fighters in pairs, 1400 frames) and
`special_matrix.sh` (`.claude/skills/remix-smoke-tests/SKILL.md`).

## 1. Mewtwo Shadow Ball: SIGSEGV in wpManagerMakeWeapon

**Symptom:** `portFixupStructU16` faulted inside `wpManagerMakeWeapon`, called from `wpSamusChargeShotMakeWeapon` the first
time Mewtwo charged its neutral special.

**Cause:** Mewtwo reuses Samus's Charge Shot code. Remix's `get_chargeinstructions_struct_` (`samusshared.asm:81`) swaps the
weapon descriptor for Mewtwo, but the port called the vanilla spawn as is. Its descriptor points at Samus's special file, which
is not loaded in a Mewtwo match.

**Fix:** `ftRemixMewtwoShadowBallMakeWeapon` (`mewtwo/ftremixmewtwonsp.c`) is the stage-setting routine with Remix's two Mewtwo
patches applied: the Shadow Ball descriptor on `p_file_special3`, and the Mewtwo charge FGM (`charge_fgm_fix_`).
`MewtwoShadowBallVars` overlays `wpSamusWeaponVarsChargeShot`, which the shared Samus code still writes, so `owner_gobj` is
kept as a pointer and a `_Static_assert` pins the LP64 layout. `ball_position_fix_` and `shoot_velocity_patch_` are applied in
`ftsamusspecialn.c` under `REGION_REMIX`.

## 2. Dedede Inhale: SIGSEGV in lbCommonAddFighterPartsFigatree

**Symptom:** the crash happened when Dedede's inhale caught an opponent (`ftMainProcSearchCatch` ->
`ftKirbySpecialNCatchEatSetStatusParam` -> `ftMainSetStatus`).

**Cause:** Dedede runs Kirby's vanilla SpecialN routines. `DededeSpecial.asm` patches 27 status changes in them so that they set
Dedede's action ids. Without those patches, Dedede was put in Kirby's status 0x110, which has no motion in Dedede's table, so
the figatree was NULL.

**Fix:** `ftRemixDededeMapKirbyInhaleStatus` (`dedede/ftremixdededensp.c`) maps Kirby's SpecialN ids to Dedede's at the top of
`ftMainSetStatus`. That covers every patched site, and the map is a no-op for other fighters.

## 3. Raw N64 FTStruct offsets on LP64

Several ported routines wrote Remix scratch words as `(u8 *)fp + 0x0B18`. The port's FTStruct is laid out for LP64, so those
offsets land on unrelated fields.

**Fix:** `ftremixfields.{h,c}` maps the N64 scratch ranges onto the same relative place in the port struct:

- 0x0ADC-0x0B13 maps to `passive_vars`;
- 0x0B18-0x0B4B maps to `status_vars`;
- 0x0980 maps to `modelpart_status[2].modelpart_id_base` (Kirby's hat).

An unmapped offset logs an actionable error and exits. Bowser, Dedede DSP, Marth and Sonic NSP use these accessors. Ported code
must use named fields wherever one exists. The MIPS32 struct layout dump generated for the asm port is wrong for WPStruct from
0x25C (4 bytes low: the ROM's `wpMainDecLifeCheckExpire` uses 0x268 for `lifetime`, the dump says 0x264).

## Other vanilla patch sites wired in this pass

| Remix patch | Port site |
|---|---|
| `Character.grounded_script` table (0x800DE408) | `mpCommonSetFighterLandingParams` -> `ftRemixRunGroundedScript` |
| `Character.on_hit` table (`OnHit.asm`, 0x800E8244) | `ftParamStopVoiceRunProcDamage` -> `ftRemixRunOnHit` |
| MarthDSP counter (0x800EA248) | `ftParamUpdateDamage` -> `ftRemixMarthDSPCounterDamage` |
| SonicUSP `action_check_patch_` (0x80150F10) | `ftCommonSpecialAirCheckInterruptCommon` |
| Sonic `spin_dash_fgm_patch_` (0x800E81A8) | `ftParamPlayLoopSFX` |
| PeachFloat (gravity, jump/fall/pass interrupts, fall, aerial cancel, item throw, hit) and Peach ledge hooks | `ftphysics.c`, `ftcommon{jump,fall,pass,attackair,fallspecial,cliffclimb}.c`, `ftparam.c`, `mpcommon.c` |
| Crash `dig_ecb_patch_`, `shield_hitlag_patch_` | descriptor callbacks set in `ftremixdeschooks.c` |

Peach's `ledge_patch_` is at 0x800DE680. That is the cliff-catch code in `mpcommon.c` (`ftRemixPeachLandingSetLedgeFlag`), not
the landing call of `mpCommonSetFighterLandingParams`.

## Verification

All 8 demo pairs ran 1400 frames with no crash. They entered most of the fighters' Remix action ids (0xDC+), including Dedede's
pull and swallow, and Mewtwo's charge and shoot.
