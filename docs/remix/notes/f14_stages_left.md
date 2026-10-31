# F14 stages_left: REMIX-TODO lines of the stage hazards (ROADMAP T2.11)

Scope: `decomp/src/gr/grremix/grremix{scuttle,scuttleminion,bbh,banzai,metallicbeelaser,corneria,spawnedfear,piratecannonball,japes,rainbowchomp,yoshiclouds,dreamland}.c` and `decomp/src/gr/grcommon/grzebes.c`. `grremixtimetwister.c` is not in scope.

Asm source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` (line numbers below refer to it unless stated otherwise).

## Status per TODO

| File:line | TODO | State | Evidence and reason |
| --- | --- | --- | --- |
| grremixscuttleminion.c:106 | object id 0x03F5 passed to 0x8016EA78 | deviation | Hazards.asm 7338 `b _end` skips the whole minion setup, so the factory is never created (GRREMIX_SCUTTLE_MINIONS_ENABLED FALSE). The NULL parent is unreachable. |
| grremixmetallicbeelaser.c:48 | projectile transform word 0x1C000000 | implemented | Top byte of the transform word is the XObjTransformKind. 0x1C = 28 = `nGCMatrixKindTraRotRpyRSca` (sys/objdef.h, counted from `nGCMatrixKindTra = 18`). The laser grows DObj scale.x (robot_bee_laser_main_, 5078..5130), which only renders with the Sca kind. The port used `TraRotRpyR` (27), so the scale was ignored. |
| grremixmetallicbeelaser.c:27 | RAM 0x8019CAA0 max X scale | implemented | Hazards.asm 5108 `lwc1 f2, 0xCAA0(at)` with `lui at, 0x8019`. The float is read at run time with `remix_ram_image_read_word` and a union. The float is not copied into C. If the Remix RAM image is not loaded, the read returns 0 and the laser never grows. |
| grremixmetallicbeelaser.c:195 | whole FTStruct word 0x018C | skipped (open) | Hazards.asm 5188..5192 compares the whole word with 0x04000010 (reflect) and 0x08800110 (psi magnet). The port checks `is_reflect` and `is_absorb`. Equivalence is not proven, because the word may hold other bits. |
| grremixspawnedfear.c:39 | DOOM_ACID colanim applied on next tick | implemented | Hazards.asm 0x0A28 test: if the gfx routine is zero, it is set to GFXRoutine.DOOM_ACID (0x79, same id as ftremixpoison.c). The port uses `gmRemixColScriptsGetOverride` and `gmRemixColScriptsSetOverride` (the poison code uses the same override for 0x0A28), plus `ftParamCheckSetFighterColAnimID`. Applied in the same branch as the asm, right away. |
| grremixspawnedfear.c:46 | KO clears 0x0A20..0x0A30, 0x0A88 | partly implemented, rest skipped | The 0x0A28 gfx clear is done before the KO (`gmRemixColScriptsSetOverride(player, 0)`). The Practice overlay fields 0x0A20, 0x0A24, 0x0A30 and 0x0A88 (Hazards.asm 8554..8558) are not cleared: `ftremixoverlay.c` has no clear function, and that file is outside this scope. The TODO stays open. |
| grremixjapes.c:74 | rushing water location struct | implemented | Hazards.asm 1890..1893 reads `0x0078(v1)` (player + 0x78) as the position pointer and writes x through it. struct_offsets: `coll_data.p_translate` at 0x78 (`Vec3f *`). The port now uses `fp->coll_data.p_translate`, with a NULL guard. |
| grremixjapes.c:24 | f20 in rushing_water | skipped (open), finding | `sub.s f2, f0, f20` at 1950. Nothing in rushing_water (1870..1955) writes f20. The other writes are in other routines (2628, 3150, and a restore at 4358). So f20 is the callee-saved FP register left by the caller. Its value is not proven. The port keeps the base at 0. |
| grzebes.c:268 | camera Y copy 0x8013EB1C to 0x801313FC | skipped (open), finding | Hazards.asm 172..175. 0x801313FC is `GRCommonGroundVarsZebes.acid_level_curr`, so the store would overwrite the acid level. 0x8013EB1C has no decomp camera field, and no Remix reader was found. Not ported. |
| grremixdreamland.c:30 | `Global.current_screen == 1P` mapping | skipped (open), finding | Global.asm 60 defines screen 0x01 as `TITLE_AND_1P` (title, 1P game over, 1P battle, Remix 1P battle). The decomp SCKind index 1 is `nSCKindTitle` (sc/scdef.h 135), so the numbers do not map one to one. The port keeps `nSCKind1PGame`. |
| grremixscuttle.c:69 | stage palette index walk (RAM 0x800466F8) | skipped (open) | The asm walks the group 0x2 object tree to the top joint, first joint, part image palette. The port has no accessor for that tree. |
| grremixscuttle.c:83 | Training menu background value | skipped (open) | Training.asm 3875 uses `entry_bg`. `port/remix/remix_symbols.h` has no `REMIX_SYM_Training__entry_bg`, and `gmRemixToggle` only covers `Toggles.entry_*`. |
| grremixscuttle.c:199 | bg image object position struct write | skipped (open) | Needs the bg image object (RAM 0x80046800, +0x74, field +0x44), which the port does not own. |
| grremixbanzai.c:105 | spawned item id (asm RobotBee 0x3B, port Bullet Bill 0x3D) | skipped (open), finding | Hazards.asm 8133..8300. The factory only runs in training mode (8091..8093 `b _end` for other screens, the port keeps that gate). It spawns item 0x3B through 0x8016EA78, then `banzai_bill_stage_setting_` (8225..) overrides the item: hitbox 150 and 5 damage, angle 0x361, the blast-zone routine at 0x0398 (banzai_bill_blast_zone_), and the animation tracks. The port uses Bullet Bill, which the lead chose earlier. Switching to `itRemixRobotBeeMakeItem` alone would not apply Banzai's stage setting, so this needs the lead's decision. |
| grremixbbh.c:43 | Big Boo countdown 0x80131AD8 | skipped (open) | Hazards.asm 8429 uses the hardcoded bg object word 0x80131AD8. No port owner exists. |
| grremixcorneria.c:61 | header attribute 0x130 | skipped (open), finding | The Great Fox laser projectile struct (Hazards.asm 2156..2160) holds the header pointer 0x80131428 and offset 0x130 (`hitbox added to header`). The asm read at 4598 (`lw t4, 0x0130(t1)`) is in the Robot Bee setup, not Corneria. Reading the Corneria header value needs the stage file's pointer resolved from the Remix ROM. The port's `wpManagerMakeWeapon` descriptor cannot take that data yet. |
| grremixpiratecannonball.c:80 | launch velocity speed 100, angle 270 | skipped (open) | Needs the projectile angle and facing convention, which is not ported. |
| grremixpiratecannonball.c:122 | Samus bomb hit-setup copy (ROM 0xE396C, 0xE39A8) | skipped (open) | Copied vanilla code is not ported. |
| grremixrainbowchomp.c:43 | hitbox slot refresh (offsets 0x214, 0x29C) and chomp sound | skipped (open) | Needs a hitbox-monitor equivalent, which the port does not have. |
| grremixyoshiclouds.c:70 | `Stages.stage_hazard_table` movement check | skipped (open) | Needs the stage hazard table, a separate subsystem. |

## Notes for the lead

- Banzai item id: the factory is only reachable in training mode. Choosing between Bullet Bill and the asm's Robot Bee-with-Banzai-setup is a gameplay decision, not a verification.
- Metallic laser: the transform change is visible in game. Check that the laser now grows and shrinks in X, and that the scale is applied (Sca).
- The Remix RAM image must be loaded for the Metallic laser max scale (`remix/ram_image` in the o2r).
- Spawned Fear: the colanim is now set in the poison branch right away. Check the acid colour on a fighter standing on the doom acid at less than 666%.
