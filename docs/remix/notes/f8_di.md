# F8 notes: DI toggle (Toggles.entry_di)

Source: `.toolchain/pkgs/smashremix-src/src/DI.asm` (one patch, `apply_di_modifiers`). Other readers of `entry_di` in the Remix src: none. `Toggles.asm:2420` only links the hitlag entry to it (`entry_hitlag` next pointer), and `Toggles.asm:2421` is the entry itself.

Toggle: title `DI`, range 0-2, string table `string_table_di` (normal, japanese, ultimate). Defaults: CE 0, TE 0, NE 0, JP 1. Values: 0 = normal (vanilla), 1 = Japanese DI (multiplier 1.5), 2 = Ultimate DI (multiplier 0.59765625). Run with `SSB64_REMIX_TOGGLES="DI=1"` or `"DI=2"`.

## Patches

| asm address | vanilla function | status | notes |
|---|---|---|---|
| 0x801408DC (ROM 0xBB31C), `apply_di_modifiers`, replaces `lui at, 0x8019` / `lwc1 f0, 0xC0E0(at)` | ftCommonDamageCommonProcLagUpdate (decomp/src/ft/ftcommon/ftcommondamage.c, range 0x80140878-0x80140934) | applied | the smash DI range multiplier (`FTCOMMON_DAMAGE_SMASH_DI_RANGE_MUL`, 2.1F under REGION_US, which Remix defines) is replaced by `ftRemixDamageSmashDIRangeMul(vanilla)` for both x and y |

Port:
- `decomp/src/ft/ftremix/ftremixdi.c` (new, owned by this scope): `ftRemixDamageSmashDIRangeMul(f32 vanilla_mul)`. `switch (gmRemixToggleVS(di))`: 1 gives `1.5F` (0x3FC00000), 2 gives `0.59765625F` (0x3F190000, the low 16 bits are zero so the decimal is exact), anything else returns `vanilla_mul`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: block appended at the end, before `#endif`.
- `decomp/src/ft/ftcommon/ftcommondamage.c`: the two translate lines in `ftCommonDamageCommonProcLagUpdate` are under `#ifdef REGION_REMIX` (uses a local `di_range_mul`), `#else` keeps the original lines unchanged.

Behaviour in the asm, kept by the port:
- `Toggles.single_player_guard`, so `gmRemixToggleVS` is used: Bonus and 1P Game ignore the toggle. The guard exit (`_single_player_guard_skip`) jumps to `_end` with f0 already holding the vanilla multiplier, which is the `vanilla_mul` return.
- Toggle value 0 also takes the skip path, so normal DI is vanilla.
- A value outside 0-2 keeps the vanilla multiplier (`b _end` in the asm).

## Uncertainties

- The vanilla function is taken from `build/remixasm/refs/vanilla_funcs.txt` (the last entry at or below 0x801408DC). I did not disassemble the ROM to confirm the two replaced instructions sit inside the smash DI `mul.s` path of that function; the logic is the only consumer of the multiplier there.
- Ultimate DI is documented in the asm as "not accurate, it's just weaker (S)DI"; the port keeps the asm value.

## What the lead should see with DI=1 or DI=2

- Matches in VS (not Bonus or 1P Game): during hitlag, a smash DI input moves the fighter further. DI=1 moves about 1.5 per unit of stick range (vanilla US 2.1), so it is shorter than normal. DI=2 moves about 0.598 per unit, so it is much shorter.
- Bonus and 1P Game: no change with the toggle on.
- DI=0: no change.
