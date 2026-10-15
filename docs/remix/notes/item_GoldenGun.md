# Item GoldenGun (0x35): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm`.
Port: `decomp/src/it/itremix/itremixgoldengun.c`, `decomp/src/it/itremix/itremixgoldengun.h`.
Descriptor: `const ITRemixItemDesc dITRemixGoldenGunItemDesc`, kind 0x35.
Spawn function for fighter or dispatch call sites: `GObj* itRemixGoldenGunMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- `dw 0x8018D040` in `item_info_array` is `gITManagerCommonData` (file 0xFB). No owner file is used, so `ITDesc.p_file = &gITManagerCommonData`.
- `o_attributes = 0xFC0`, the asm `FILE_OFFSET`.

## Routine map

| Remix label | C | Role |
|---|---|---|
| stage_setting_ | itRemixGoldenGunMakeItem | SPAWN_ITEM |
| (inline in stage_setting_) | itRemixGoldenGunRollTilt | RNG advance, tilt store skipped (REMIX-TODO) |
| prepickup, 0x801755FC | itLGunHoldSetStatus | PICKUP_ITEM_INIT (vanilla) |
| pickup_item | itRemixConsumeTomatoTail | PICKUP_ITEM_MAIN, jumps to 0x80145C4C |
| dropped, 0x80175780 | itLGunDroppedSetStatus | DROP_ITEM (vanilla) |
| thrown, 0x801756AC | itLGunThrownSetStatus | THROW_ITEM (vanilla) |
| item_info 0x14, 0x801754F0 | itLGunFallProcUpdate | ITDesc proc_update (vanilla) |
| item_info 0x18, 0x80175550 | itLGunFallProcMap | ITDesc proc_map (vanilla) |
| state 0, 0x80175528 | itLGunWaitProcMap | state table, grounded (vanilla) |
| state 1, 0x801744C0 / 0x80174524 | itTomatoFallProcUpdate / itTomatoFallProcMap | state table, aerial (vanilla) |

The asm has no routines of its own besides `stage_setting_` and `pickup_item`. The projectile and shoot code is not item code; see Vanilla patches.

## Spawn logic (stage_setting_)

1. `itManagerMakeItem(parent, item_info_array, pos, vel, flags)`; return NULL on failure.
2. `ITStruct.multi = AMMO_COUNT (1)`. The asm stores it with `sh` at offset 0x033E, which is `multi`.
3. RNG advance (`jal 0x80018910`), see the tilt REMIX-TODO.
4. `ITStruct.is_unused_item_bool = TRUE`. The asm does `lbu`/`ori 4`/`sb` at 0x02D3, the same bit the other ports set through `is_unused_item_bool` (`itremixclanbomb.c`, `itremixgem.c`).
5. `ITStruct.arrow_gobj = ifCommonItemArrowMakeInterface(ip)`. The asm stores the result at 0x0348, the same as `itremixgem.c`.

## Vanilla patches (not applied)

These hook fighter or weapon code that the port does not own. Each one must be applied in its vanilla function under `#ifdef REGION_REMIX`. The vanilla function name is not in `build/remixasm/refs/vanilla_funcs.txt`, so only the ROM and RAM addresses from the asm are given.

1. **Projectile struct** (`OS.patch_start(0xF04AC, 0x80175A6C)`). When the held item is GoldenGun (id 0x35), the raygun ammo maker uses the Remix `golden_bullet_projectile_struct` instead of the Ray Gun struct. Needs the projectile port (REMIX-TODO(projectile)).
2. **Shoot FGM** (`OS.patch_start(0xC1B74, 0x80147134)`). For GoldenGun, play FGM 0x04C2 instead of 0x003D.
3. **Projectile properties** (`OS.patch_start(0xF04CC, 0x80175A8C)`). For GoldenGun, set the projectile damage to 99 (offset 0x0104), damage type to normal (0x010C = 0), KB1 45 (0x0130), KB2 0 (0x0134), KB3 130 (0x0138), angle 7 (0x012C), hit FGM FGM.item.BAT (0x0146, halfword), and speed 0x43960000 (`lui at, 0x4396`, 300.0F). Raygun keeps its own values.
4. **No-ammo FGM** (`OS.patch_start(0xC1BE0, 0x801471A0)`). For GoldenGun, play FGM 0x04C3 instead of 0x003E.
5. **Grounded shoot** (`OS.patch_start(0xC22C8, 0x80147888)`). For GoldenGun, set the fighter struct word at 0x017C (`s0`) to 0, the frame speed multiplier 0.5 (`lui a3, 0x3F00`), and action 0x8E (`addiu t7, r0, 0x008E`). Otherwise the vanilla path runs.
6. **Aerial shoot** (`OS.patch_start(0xC2370, 0x80147930)`). Same as 5 with action 0x8F.

## REMIX-TODO

- `REMIX-TODO(tilt)`: `itRemixGoldenGunRollTilt`. The asm sets `DObj.rotate.vec.f.y` (offset 0x34) to float 0x8018CD00 or 0x8018CD04, chosen by the low bit of the RNG value. The two float values are not in the port, so the store is skipped and only the RNG advance remains.
- `REMIX-TODO(projectile)`: the golden bullet projectile (`golden_bullet_projectile_struct`, `projectile_info`, display list and image data). Without it the Vanilla patches 1 to 4 cannot take effect. Patches 5 and 6 only change the shoot action and frame speed.

## Uncertainties

- `0x80018910` is assumed to be `syUtilsRandUShort` (`decomp/src/sys/utils.c`). The Remix source calls it "this function advances the rng seed", but the address is not in `vanilla_funcs.txt`, so the identity is not confirmed.
- `ITDesc.transform_types` uses the `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` value used by the Gem and Maxim Tomato ports. The asm word is `0x1B000000` (`0x0C`, "either 0x1B000000 or 0x1C000000"). Not verified.
- The asm labels the stage in `item_state_table` loosely; the table is kept only for parity (see the table comment in the C file).
- ITStruct offsets come from `build/remixasm/refs/struct_offsets.txt`. They were checked against `itremixgem.c` and `ittypes.h` by meaning, not by byte.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PARTIAL — rc=0, no crash, BUT 26x 'ftCommonItemShootSetStatus: unexpected ip->kind=53 type=2 — aborting to avoid UB' in ssb64.log: CPU picks the gun up and the vanilla shoot path rejects the unknown kind. That function is the host of vanilla patches 5/6 (grounded/aerial shoot, RAM 0x80147888/0x80147930); the GoldenGun branch must run before the guard.

## Hooks applied 2026-10-14 (this session)

- Patches 2/4/5/6 applied in decomp/src/ft/ftcommon/ftcommonitemshoot.c: FGM ternaries (0x04C2/0x04C3) in ftCommonLGunShootProcAccessory; case ITREMIX_GOLDENGUN_ID in ftCommonItemShootSetStatus/ftCommonItemShootAirSetStatus routing to nFTCommonStatusLGunShoot(LGunShootAir) with ftMainSetStatus frame speed 0.5F. 0x017C confirmed as fp->motion_vars.flags.flag0 (struct_offsets.txt:126-128), action 0x8E/0x8F are the vanilla nFTCommonStatusLGunShoot(Air) enum values.
- ITREMIX_GOLDENGUN_ID moved to itremixgoldengun.h.
- Patches 1/3 (golden bullet projectile, 0x80175A6C/0x80175A8C): still blocked on REMIX-TODO(projectile).
- The sweep 'ftCommonItemShootSetStatus: unexpected ip->kind=53 — aborting' messages should disappear; re-run the forced-item sweep to confirm.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
