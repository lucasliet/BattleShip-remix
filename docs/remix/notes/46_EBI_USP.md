# 0x46 EBI, USP (Ebisumaru up special): port notes

Port file: `decomp/src/ft/ftremix/ebi/ftremixebiusp.c`
Table: `dFTRemixEbiUSPProcs` / `dFTRemixEbiUSPProcsNum` (5 entries: `air_initial_`, `collision_`, `ground_initial_`, `main_`, `physics_`). All 5 worklist routines are ported.
Source: `.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm` lines 698-975 (`EbiUSPRefresh` at 700-704, `EbiUSP` at 712-975).
Action id: `Ebi.Action.USP` = 0xDF (`Ebi.asm:204`), `nFTRemixEbiStatusUSP`.

Not wired: `ftremixported.c` must `extern` the table and add it to `dFTRemixPortedProcTables`. That file is outside this worker's scope.

## 1. Vanilla patches and hooks

### 1a. Refresh hook (`EbiUSPRefresh`, EbiSpecial.asm:700-708)

The asm is installed with `Character.table_patch_start(on_hit, Character.id.EBI, 0x4)` (EbiSpecial.asm:706-708). It runs `sw r0, 0x0ADC(a0)`, which clears the up special bool so the USP is usable again after a hit.

The port exposes it as a non-static function:

```c
void ftRemixEbiUSPRefresh(FTStruct *fp);
```

It is defined in `ftremixebiusp.c` and clears `*ftRemixFighterWord(fp, 0x0ADC)`.

The on-hit table is dispatched by `ftRemixRunOnHit` in `decomp/src/ft/ftremix/ftremixchartables.c`. That is where Sonic's USP refresh is wired (`case REMIX_ID_SONIC:`, the same `passive_vars` word). The C to insert in `ftRemixRunOnHit` under the existing `switch (fp->fkind)`:

```c
    case REMIX_ID_EBI: /* EbiUSPRefresh (EbiSpecial.asm:700): up special is usable again */
        ftRemixEbiUSPRefresh(fp);
        break;
```

Also needed in `ftremixchartables.c`, next to the `ftRemixRunOnHit` definition, or in `ftremix.h`:

```c
void ftRemixEbiUSPRefresh(FTStruct *fp);
```

`REMIX_ID_EBI` is defined in `port/remix/remix_roster_data.h:124`. The hook is a plain `on_hit` entry, so `ftRemixGroundedClearsPassiveWord` does not need to change.

### 1b. Remix on-hit routine (`goemon_on_hit_subroutine_`, EbiSpecial.asm:964-971)

Not a vanilla patch. It is installed into `proc_damage` (FTStruct 0x09EC) by `goemon_on_hit_subroutine_establishment_` (asm 953-960). The asm only loads the player struct and returns, so the port installs a no-op `static void ftRemixEbiUSPOnHit(GObj *)`. Nothing else in the asm changes state on hit.

### 1c. Vanilla calls used by the port

| RAM | Decomp name | Used by |
|---|---|---|
| 0x800E6F24 | ftMainSetStatus | air_initial_ |
| 0x800E0830 | ftMainPlayAnimEventsAll | air_initial_ |
| 0x800DEEC8 | mpCommonSetFighterAir | ground_initial_ |
| 0x800DEE54 | mpCommonSetFighterWaitOrFall | main_ (asm 824, the "transition to idle") |
| 0x800DE724 | mpCommonCheckFighterProject | collision_ |
| 0x800DE798 | mpCommonCheckFighterPassCliff | collision_ |
| 0x80144C24 | ftCommonCliffCatchSetStatus | collision_ |
| 0x80142E3C | ftCommonLandingFallSpecialSetStatus | collision_ |
| 0x800DDF44 | mpCommonSetFighterFallOnEdgeBreak | collision_ |
| 0x8001863C | syUtilsArcTan2 (inferred) | physics_ |
| 0x80035CD0 | __cosf (inferred) | physics_ |
| 0x800303F0 | __sinf (inferred) | physics_ |

The math names are inferred from the same pattern in `ftremixgoemonusp.c` and `ftremixwolf.c:211`, not checked against the decomp symbol table.

### 1d. Copied code (`collision_`, asm 941-949)

`collision_` splices Mario's up special landing check (`ftMarioSpecialHiProcMap`, 0x80156358) from ROM 0xD0D98 (0x78 bytes) and ROM 0xD0E14 (0x44 bytes). The port is the same logic written as C, as in the Goemon and Marina ports. The replaced instruction is `lui a2, LANDING_FSM` (0x3E80) at asm 946. The `ori a2, a2, 0x5C29` at ROM 0xD0E34 is kept, so the lag is 0x3E805C29 (about 0.2507031). The port constant is `EBI_USP_LANDING_LAG = 0.2507031261920929F`.

The landing lag is the same value as Goemon's (`GOEMON_USP_LANDING_LAG`). Marina's is 0x3EC05C29 for the same reason.

## 2. Shared Remix subsystems (REMIX-TODO)

None. The USP spawns no items, weapons or custom GFX. `Size.*` is not referenced.

## 3. Uncertainties

1. **Byte 0x018D, air_initial_ (asm 771-774).** The asm does `ori t6, r0, 7; and v1, v1, t6; sb v1, 0x018D(a0)`. `v1` held the up special bool (0), but it is caller-saved and the calls at asm 752-756 and 758 can clobber it. The stored byte is therefore `v1 & 7`, with an unknown `v1`. The port clears `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall`, and leaves `is_magnify_show`, `is_limit_map_bounds` and `is_invisible` as they are. This matches the Goemon USP port.
2. **Up special guard, air_initial_ (asm 745-746).** When the bool is already set, the asm returns before the status change. `ground_initial_` still runs `mpCommonSetFighterAir` and sets the y velocity to 80 first. The port keeps this order.
3. **Y velocity order, air_initial_ (asm 765, 778).** The y velocity is read after `ftMainSetStatus` and `ftMainPlayAnimEventsAll`, and scaled by 0.375 before being stored back at the end. The port follows the asm.
4. **Frame timer (asm 817-821).** The `sw t6, 0x0B18(v1)` sits in the delay slot of `bnez`, so the timer is incremented on every frame. The port increments first, then compares the old value.
5. **Physics (asm 837-934).** The Ebi physics is line for line the same as `GoemonUSP.physics_` in `ftremixgoemonusp.c`, with identical constants. The port duplicates the helper code in `ftremixebiusp.c` as static functions, because the brief keeps helpers file-local. Consider a shared helper later if a third fighter needs it.
6. **atan2 operand order (asm 906-908, 911-912).** The asm passes `f12 = Y_DIFF` and `f14 = X_DIFF` to `0x8001863C`, and the port calls `syUtilsArcTan2(diff_y, diff_x)`. The function name is inferred.
7. **Stop below minimum speed (asm 857-874).** The `bc1fl` delay-slot behaviour was traced. The port stops only when speed is at most MIN_SPEED (3.0), and otherwise steers toward zero, as the asm does.
8. **Stick deadzone (asm 851-855).** Neutral means `|stick_x| < 11` and `|stick_y| < 11`, using `input.pl.stick_range`.

## 4. Accessors and fields

- `0x0ADC` (up special bool), `0x0B18` (frame timer) and `0x0B20` (cleared word) go through `ftRemixFighterWord` in `ftremixfields.h`. All three are in the mapped `passive_vars` and `status_vars` ranges. No raw FTStruct offsets remain.
- `0x017C`, `0x0180`, `0x0184` are `motion_vars.flags.flag0/1/2`.
- `0x09EC` is `proc_damage`, `0x09C8` is `attr`, `0x0148` is `jumps_used`, `0x018D` is the bitfield byte with `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall`.

## 5. Helpers (static, not in the table)

`ftRemixEbiUSPOnHit`, `ftRemixEbiUSPSetStatus`, `ftRemixEbiUSPOnHitEstablishment`, `ftRemixEbiUSPClearAirFlags`, `ftRemixEbiUSPInitStatusVars`, `ftRemixEbiUSPIsStickNeutral`, `ftRemixEbiUSPStopBelowMinSpeed`, `ftRemixEbiUSPGetStickTarget`, `ftRemixEbiUSPAccelerateToward`, `ftRemixEbiUSPProcMap`.

Non-static (for the hook in section 1a): `ftRemixEbiUSPRefresh`.
