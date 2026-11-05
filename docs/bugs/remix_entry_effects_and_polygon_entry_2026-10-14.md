# Remix entry: missing prop on clone entries and polygon fighters sunk into the floor

Covers the fighter entry (appear) effects of Remix clones and the polygon variants. Code:
`decomp/src/ft/ftremix/ftremixentry.{c,h}` (effect makers), `ftremixentrydata.c` (override rows and Arwing animation rows),
`ftremixdeschooks.c` (`ftRemixBindDescriptorHooks` installs `ftRemixEntryMakeEffect` as `entry_make_effect`),
`decomp/src/ft/ftport.c` (`ftPortVanillaEntryMakeEffect`, `ftPortCopyMotionArray`), `decomp/src/ft/ftmain.c`
(`ftMainSetStatus` polygon rule), `decomp/src/ef/efmanager.c`. Test: `SSB64_START_SCENE=22 SSB64_VS_FKINDS=<a,b>
SSB64_VS_STAGE=<gkind>` and screenshots during the countdown (`.claude/skills/remix-smoke-tests/SKILL.md`); the real CSS flow
is `css.sh` with `inputs/css_pick_mario.txt`.

## 1. Clones had no entry effect

**Symptom.** In a real CSS -> VS match, Dr. Mario (0x20), J Mario (0x2A) and the other pipe users entered without the pipe,
although vanilla Mario showed it on every stage.

**Cause.** A synth fighter copies its parent's `FighterDescriptor`, including `entry_make_effect`
(`ftPortVanillaEntryMakeEffect`). That routine switches on `fp->fkind`, which for a clone is a Remix id, so it fell into
`default` and nothing was made. The entry *status* (the pipe hop, the barrel, the car) was still the parent's, so the fighter
appeared without its prop.

Remix keeps this in `Character.entry_script` (4 bytes per id, `get_entry_script_` 0x8013DC78): a pointer to one of the vanilla
appear routines (0x8013DC8C Mario pipe, 0x8013DC9C Fox Arwing, 0x8013DCAC DK barrel, 0x8013DCBC Samus, 0x8013DCCC Link,
0x8013DCE4 Yoshi egg, 0x8013DCF4 Kirby star, 0x8013DD04 Pokeball, 0x8013DD14 Captain car, 0x8013DD68 none) or to a Remix
routine. A clone copies its parent's, every polygon variant skips it, and Marth, Roy, Marina, Goemon and Ebisumaru run a Remix
routine that only sets the reversed-direction flag when facing left and then jumps into Link's.

**Fix.** `ftremixentry.c` holds the effective table for every synth id. The vanilla makers read their model from the vanilla
fighter's file global (`gFTMarioFileSpecial2` and friends), which is not loaded when only a clone is in the match, so each call
points the vanilla `EFDesc` at the entering fighter and restores it afterwards:

- a clone that keeps the parent's effect uses the file slot (special1..4) that holds the same file *id* as the vanilla
  effect's file (Remix gives clones the parent's ids; JSamus, for example, has Samus's entry file in `file_6`, not in the
  vanilla slot);
- a Remix patch that swaps in its own struct is a row of `dFTRemixEntryOverrides` (`ftremixentrydata.c`): file slot, optional
  base effect for procs and transforms, and the four offsets. `linkshared.asm` (Young Link, Marth, Roy, Marina, Goemon,
  Ebisumaru, Banjo), `samusshared.asm` (Dark Samus, Crash), `dkshared.asm` (Sonic with Tails, Lanky's Tag Barrel) and
  `captainshared.asm` (J Falcon, Bowser's Clown Car, Conker's Greg's Hand) have rows. The Captain car maker also reads its
  animation joints from hard-coded offsets of the car file, so `efManagerCaptainEntryCarMakeEffect` calls
  `ftRemixEntryGetCarAnim` (`clown_car_animation1/2`, `get_entry_file_ptr_`);
- when neither exists the effect is skipped with a log line instead of crashing: building a desc from a file that lacks the
  vanilla offsets crashed `gcAddDObj3TransformsKind` (JSamus before the id match).

**Wolf's Wolfen** (`Wolf.asm wolfen_entry*`). The flags word 0x070A0000 is the vanilla `EFDesc` packed (flags 7, dl_link 10),
so the override row only changes the file (`file_8`, Special3) and the DObj setup offset (0x2610).
`efManagerFoxEntryArwingMakeEffect` also reads its wing joint from the Special3 file (0x2E74) and its body joints from the
Special2 file (0x9E0 right, 0x590 left), so it calls `ftRemixEntryGetArwingAnim` like the Captain car does, and
`dFTRemixEntryArwingAnims` holds Wolf's values (Special3 0x284C; Special2 0xF74 right, 0xB24 left). Falco, Peppy, Slippy and J
Fox draw the Fox Arwing correctly through the id match. Slippy's `wolfen_entry_3` branch only repeats the Fox offsets with his
own copy of the file, so it has no row. The Size scaling of the Arwing, Captain car and Yoshi egg entry effects is applied by
`ftremixsizeentry.c` and `ftremixsizeentrygfx.c`.

**Wolf's entry timing is authored data, not a port bug.** Wolf is visible and crouched while the Wolfen flies in, whereas Fox,
Falco, Peppy and Slippy are hidden until they land. Fox hides his body with a scale-0 track on joint 4 until animation frame 81;
Wolf's `File.WOLF_APPEAR` (0xC90) is a different animation (joint 4 only rotates, the scale-0 hide is on joint 5 and ends near
frame 48, so he lands about 8 frames later). The Wolfen anim joints still follow Fox's timeline (the Arwing passes at frames
~212-216 in both), so Wolf lands about six frames before the Arwing arrives. The `Wolf.asm` comments on
`wolfen_entry_2/_3` say the offsets "need updating with new import", so the mismatch probably exists in Remix itself.

### Sonic and Tails

`DKShared.asm barrel_alternate` picks the right or left Tails struct (`sonic_entry_struct_right/left`) by the fighter's facing.
The port first always used the left one, so a right-facing Sonic got the mirrored Tails animation away from his hands.
`ftRemixEntryMakeDonkeyBarrel` now selects the right row (`0x2A28`/`0x68C8`, effect `nFTRemixEntryEffectDonkeyBarrelRight`)
when `status_vars.common.entry.lr == +1`; `entry.lr` keeps the facing even though `ftCommonAppearSetStatus` clears `fp->lr`.

## 2. Polygon variants spawned into the floor

**Symptom.** NDrMario (0x51) and the other polygon variants stood crouched into the floor until the countdown ended.

**Cause.** The Remix ROM rows of the polygon entry statuses (36 statuses of fighters 0x4D..0x5F, for example NDrMario 0xDD/0xDE)
have flags 0 and no hidden joints, and run `ftCommonEntryNullProcUpdate` with no physics proc. The `0x40000000` flag the
port's rows carried (`is_use_transn_joint` of the `FTAnimDesc` union; `FTANIM_FLAG_XROTN_JOINT` in `ftdef.h` has the same value
and a stale name) is not Remix data: `ftPortCopyMotionArray` merges the parent's `0xE0000000` hidden-joint flags into every
synth row (`ftPortParentHiddenJointFlags`) so the parent's routines never read a missing hidden joint. In an entry status that
runs no parent routine, the merged TransN flag poses a body joint with the animation's root track, which crouches the polygon
into the floor.

**Fix.** `ftMainSetStatus` (REGION_REMIX) clears the XRotN, TransN and YRotN flags when the status runs
`ftCommonEntryNullProcUpdate` with no physics proc, which is the rule of the ROM rows. Checked with NDrMario, NWario and
NWolf. This is not a deviation from Remix: it undoes the port's own merge for the one case where the merge is wrong.

## Notes

- `SSB64_VS_STAGE=<gkind>` selects the stage of the `SSB64_VS_FKINDS` shortcut (default Dream Land); with one fkind the stage
  is ignored.
- Headless CSS: `SSB64_START_SCENE=16`, then `SSB64_INPUT_SCRIPT` lines with the stick at `y=80` for 29 frames and `x=80` for
  12 frames to put P1's hand on Mario (the hand moves 4 px per frame in 320x240 space), `A`, `START`, and `A` on the stage
  select.

Open gaps: `docs/remix/patch_audit.md`, group g7 (`sonic_facing_` in `Sonic.asm`: `ftCommonAppearSetStatus` clears `lr` for
every fighter).
