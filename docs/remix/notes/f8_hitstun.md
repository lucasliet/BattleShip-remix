# F8 notes: Hitstun toggle (Toggles.entry_hitstun)

Source: `.toolchain/pkgs/smashremix-src/src/Hitstun.asm` (the only file reading `entry_hitstun`; `grep -rn entry_hitstun` over the Remix src finds only Toggles.asm:2419 and Hitstun.asm:15).

Toggle: title `Hitstun`, range 0-1, default 0 (off). Values: 0 = normal, 1 = melee (`string_table_hitstun`). Run with `SSB64_REMIX_TOGGLES="Hitstun=1"`.

## Patches

| asm address | vanilla function | status | notes |
|---|---|---|---|
| 0x800EA1B0 (ROM 0x659B0), hook into hitstun_ | ftParamGetHitStun (decomp/src/ft/ftparam.c) | applied | divisor 1.875 (0x3FF00000) becomes 2.5 (0x40200000) when `gmRemixToggleVS(hitstun)` is non-zero |

Port: `ftParamGetHitStun` under `#ifdef REGION_REMIX` reads `gmRemixToggleVS(hitstun)` (the asm uses `Toggles.single_player_guard`, so Bonus and 1P Game ignore the toggle). Include `gm/gmremixtoggle.h` added to the REGION_REMIX include block at the top of ftparam.c.

The function is the single divisor for all callers (ftCommonDamageInitDamageVars, ftCommonWallDamage, the Donkey throw check, ftCommonDamageUpdateDamageColAnim), so they all follow the toggle, as the asm hook does.

## Uncertainties

- None in semantics. The asm `lui at, 0x4020` / `mtc1 at, f4` is exactly 2.5F.

## What the lead should see with Hitstun=1

- Matches in VS (not Bonus or 1P Game): hits stun for less time, knockback unchanged. Hitstun tics are knockback / divisor, so they are 0.75x of normal (1.875 / 2.5).
- Bonus and 1P Game: no change even with the toggle on.
