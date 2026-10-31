# F14 notes: stages scope (REMIX-TODO lines of gr/grremix and gr/grcommon/grzebes.c)

Asm source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm`. Offsets from `build/remixasm/refs/struct_offsets.txt`.

## Implemented

| File | Line | Change | Asm |
|---|---|---|---|
| grremix/grremixcasino.c | wall surface | reads the low byte of `coll_data.lwall_flags` / `rwall_flags` (asm 0x11F / 0x133), selected by `mask_curr` (asm 0xCE), instead of the masked vertex word | Hazards.asm 4450-4455 |
| grremix/grremixbowserbombs.c | ECB | sets `coll_data.map_coll.bottom` (ITStruct 0x78) to -416.0F after the spawn | Hazards.asm 1027-1041, 1357-1368 |
| grremix/grremixthunderball.c | facing | the lr slot receives the bit pattern 0x3F800000 (asm `lui a1, 0x3F80`), not `wp->lr`; the third argument is a2, not set by the asm, 1.0F kept | Hazards.asm 9175-9176 |
| grremix/grremixthunderball.c | reflector / absorb | `wpMarioFireballProcReflector` (0x80168748) and `wpFoxBlasterProcHit` (0x80168964) | Hazards.asm 9242-9243 |
| grremix/grremixmetallicbeelaser.c | absorb | `wpFoxBlasterProcHit` (0x80168964) | Hazards.asm 5295-5296 |
| grremix/grremixpiratecannonball.c | camera | `wp->is_camera_follow = TRUE` (WPStruct 0x26C bit 0x80 = table 0x268:0) | Hazards.asm 4185-4186 |
| grremix/grremixpiratecannonball.c | joint | DObj `rotate.vec.f.y` (0x34) = pi/2 before the barrel animation | Hazards.asm 4190-4193 |
| grremix/grremixmetallic.c | Super Mushroom | `itRemixSuperMushroomDestroyForPlayer(port)` after the shrink | Hazards.asm 5215-5220 |

## Deviations and pending items

| File | Item | State | Why |
|---|---|---|---|
| grcommon/grzebes.c | Zebes camera Y copy (0x8013EB1C -> 0x801313FC) | skipped | 0x801313FC is `GRCommonGroundVarsZebes.acid_level_curr` (offset 0xC) in the decomp layout. No Remix reader of 0x801313FC was found, so porting it would corrupt the acid level. |
| grremix/grremixcorneria.c | header attribute 0x130 | skipped | The Great Fox laser hitbox comes from the Remix Sector Z header at 0x130. The port uses the vanilla Arwing descriptor. Needs the Remix file data. |
| grremix/grremixscuttle.c | palette walk (0x800466F8) | skipped | Needs a joint/part object-tree walk the port does not have. |
| grremix/grremixscuttle.c | training bg toggle | skipped | No accessor for `Training.entry_bg + 4`. |
| grremix/grremixscuttle.c | bg image object | skipped | Needs the background image object (RAM 0x80046800) port. |
| grremix/grremixmetallicbeelaser.c | transform word 0x0C | skipped | Not decoded (0x1C000000). |
| grremix/grremixmetallicbeelaser.c | max X scale (RAM 0x8019CAA0) | skipped | RAM float not read. The Fox blaster clamp is assumed. |
| grremix/grremixmetallicbeelaser.c | FTStruct check | skipped | The asm compares the whole word 0x18C with 0x04000010 / 0x08800110. The port checks the bitfields, so it is an approximation. |
| grremix/grremixmetallic.c | none left | done | |
| grremix/grremixbanzai.c | spawned item id | skipped | The asm spawns Item.RobotBee (0x3B) with the Banzai file animation (track 0x908). The port spawns Bullet Bill with its animation. A id change alone changes the visual and the animation, so it needs an in-game check. |
| grremix/grremixscuttleminion.c | object id 0x3F5 | skipped | The asm passes 0x3F5 in the parent slot. The port passes NULL. Whether the minion reads its parent is not verified. |
| grremix/grremixrainbowchomp.c | hitbox monitor | skipped | Needs the attack_coll record semantics for projectile 0x214 / 0x29C and the fgm 0x401 roll. |
| grremix/grremixjapes.c | f20 base x | skipped | f20 is not set in rushing_water. The value is not known. |
| grremix/grremixjapes.c | fighter location (player + 0x78) | skipped | Equivalence not verified. |
| grremix/grremixspawnedfear.c | colanim on poison | skipped | GFX timing (0x0A28 colanim). |
| grremix/grremixspawnedfear.c | overlay clear (0x0A20..0x0A30, 0x0A88) | skipped | Remix overlay state, not mapped to a port owner. |
| grremix/grremixpiratecannonball.c | launch velocity | skipped | Projectile angle convention not ported. |
| grremix/grremixpiratecannonball.c | Samus bomb setup (ROM 0xE396C / 0xE39A8) | skipped | Copied vanilla code not ported. |
| grremix/grremixdreamland.c | screen id 1P | skipped | Not verified against the Remix screen id table. |
| grremix/grremixbbh.c | bg countdown 0x80131AD8 | skipped | No port owner. |
| grremix/grremixyoshiclouds.c | Stages.stage_hazard_table | skipped | Needs the stage hazard table. |
| grremix/grremixtimetwister.c | T1.5b (62), camera (73), bgm (74), stage header (103) | skipped | Time Twister items; the lead said to leave them. Tags unchanged. |

## Notes for the lead

- The `pi/2` constant of the pirate joint is the asm comment value. The RAM word at 0x8019CA80 was not read.
- `grremixthunderball.c` and `grremixmetallicbeelaser.c` include `<math.h>`. `scripts/remix/syntax-check.sh` fails on it because
  the SDK `MacOSX27.0.sdk` in its flags is missing on this machine. With `-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include`
  both files pass.
