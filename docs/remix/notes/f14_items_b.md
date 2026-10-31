# F14 items_b: REMIX-TODO closure (ROADMAP T2.11)

Scope: itremixbasketball, itremixstopwatch, itremixlightning, itremixfranklinbadge, itremixblueshell,
itremixflashbang, itremixcloud, itremixboomerang, itremixspawn, itremixsupermushroom, itremixcloakingdevice
(item-file comment only) and wp/wpmario/wpmarioremixfireball.c. Asm: `.toolchain/pkgs/smashremix-src/src/items/*.asm`,
`fireball.asm`, `Item.asm`, `Poison.asm`.

## Line states

| File:line (before) | State | Notes |
|---|---|---|
| basketball: blast wall (0x398) | implemented | `itRemixBasketballBlastZone` in `ITStruct.proc_dead`; sets `gGMRemixSmashketball.spawn_golden` directly (no setter exists) |
| basketball: smoke gfx | implemented | `efManagerFlameRandgcMakeEffect` every 4 frames; the asm leaves a1 unset, so only the position is passed |
| basketball: display (barrel, respawn) | implemented | `DOBJ_FLAG_HIDDEN` in barrel setup and respawn; `DOBJ_FLAG_NONE` on barrel shoot |
| basketball: palette | skipped | needs the DL palette pointer swap (display list -0x150 / -0x120) and the port's DL palette layout |
| basketball: barrel entry check | skipped | needs the mid-function of `grJungleTaruCannCheckGetDamageKind` (asm 0x8010A018) and the barrel object list 0x80131180 |
| stopwatch: 5 Render lines | skipped | `Render.draw_rectangle` / `DESTROY_OBJECT_` have no port (no rectangle helper in decomp/src or port/remix) |
| lightning: camera colour and mode per frame | implemented | `CObjGetStruct(gGMCameraGObj)` flags and colour; mode guarded by Flash Guard, colour not (asm 0x108 / 0x10C) |
| lightning: camera reset at end | implemented | flags = `COBJ_FLAG_DLBUFFERS` (asm mode 4) |
| lightning: pickup flash | implemented | flags = `COBJ_FLAG_FILLCOLOR`, colour white, skipped with Flash Guard (asm `pickup_lightning_`) |
| lightning: Franklin Badge | implemented | badge-active players are skipped and FGM 0x041B plays (asm `lightning_zap_players_`) |
| lightning: file 0xFB comment | resolved | see "Item file 0xFB" below |
| franklinbadge: reflect dispatch | skipped | needs a Remix reflect table entry (`ftremixreflect`, not in this scope) |
| franklinbadge: reflect_gfx_ | skipped | ledge-grab GFX via 0x800CE870 (no port) |
| blueshell: homing (get_player_in_first_) | skipped | no first-place helper in the port; the asm reads match_info ranking |
| blueshell: pink smoke | skipped | custom particle 0x8A (`custom_smoke_`) is not in the port |
| flashbang: BlueShell hit GFX | skipped | `Item.BlueShell.hit_gfx_` shared routine has no port |
| cloud: Size scale | skipped | Size.asm item scope not ported; cloud animation scale needs the animation interpreter |
| cloud: DObj uncertain line | skipped | asm reads s0 which this routine does not set; left as is |
| cloud: blast zone | skipped | asm `cloud_blast_zone_` is a bare `jr ra`, so the return value (v0) is undefined; the port keeps it in a private field, not `proc_dead` |
| cloud: item info 0x0C | resolved | asm word 0x1B000000 = kind 0x1B = `nGCMatrixKindTraRotRpyR` |
| boomerang: Size multiplier | resolved | `ftRemixSizeGetMultiplier` was already ported |
| spawn: poison clear | skipped | `Poison.clear_poison_` (clears `poisoned_players`) is not called from `itRemixClearActiveItems` |
| supermushroom / cloakingdevice: file comment | resolved | see below |
| wp fireball: GFX id swap | implemented | capsule hits only, see below |

## Item file 0xFB

`ITDesc` uses `&gITManagerCommonData`, which is the file loaded at RAM 0x8018D040 (`decomp/src/it/item.h`,
`itmanager.c`). `llITCommonDataFileID` is 0xFB (`include/reloc_data.us.h`). The asm items load the same pointer
(`dw 0x8018D040 // 0x04 - hard-coded pointer to file 0xFB`). The ITDesc is correct for supermushroom,
cloakingdevice, lightning, franklinbadge and blueshell.

## Fireball GFX (wp/wpmario/wpmarioremixfireball.c)

- `fireball.asm` hit_effect_ `_capsule` writes 0x81 (Dr. Mario) or 0x97 (Dr. Luigi) into byte 0 of the normal-hit GFX table
  at 0x8012DF20 (`dEFManagerDamageNormalLightIDs`), calls 0x800FDC04 (`efManagerDamageNormalLightMakeEffect`), then restores 0x49.
- `_book` does not swap, so book hits keep the vanilla id.
- Dr. Luigi is `char_id == REMIX_ID_DRL`, where char_id is the copied power for Kirby and the fighter otherwise (asm `_drl_check`).
- Open point: the port reads the owner through `wp->owner_gobj`. Kirby copy id is `passive_vars.kirby.copy_id`.

## Unverified details kept as they are

- `itremixbasketball.c` fire gfx calls `efManagerFlameLRMakeEffect` for asm 0x800FE7B4, which is not a named function in
  `vanilla_funcs.txt`. Not changed (outside the TODO list). Check in game.
- `gGMRemixSmashketball.spawn_golden` is written from the item file (`gm/gmremixsmashketballstate.h`). A setter in
  gm would be cleaner; not added because gm is out of scope.
- The camera fields 0x108 / 0x10C are mapped to `CObj.flags` / `CObj.color` by value (2 and 4 are the
  `COBJ_FLAG_FILLCOLOR` and `COBJ_FLAG_DLBUFFERS` values). `struct_offsets.txt` lists CObj flags at 0x80, not 0x108,
  so the mapping is by meaning. Check the lightning flash in game.
