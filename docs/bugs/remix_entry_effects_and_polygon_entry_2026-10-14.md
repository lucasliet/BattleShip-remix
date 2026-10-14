# Remix entry: missing pipe and polygons spawning into the floor (2026-10-14)

Follow-up to [remix_items_and_projectiles_integration](remix_items_and_projectiles_integration_2026-10-13.md).
Two reports from playing a real CSS → VS match: the Mario pipe was not drawn in the battle intro, and
polygon fighters appeared half inside the ground.

## How it was reproduced

`SSB64_VS_FKINDS` on the vanilla fighters showed the pipe on every stage, and the real flow
(`SSB64_START_SCENE=16`, scripted cursor, START, stage A) showed it too, so the report was about a Mario
*clone*. Dr. Mario (0x20), J Mario (0x2A) and the other pipe users entered without the pipe, and
NDrMario (0x51) and the other polygon variants stood crouched into the floor until the countdown ended.
`SSB64_VS_STAGE=<gkind>` (new, same shortcut) was used to check every vanilla stage.

## 1. Clones had no entry effect

**Cause:** a synth fighter copies its parent's `FighterDescriptor`, including `entry_make_effect`
(`ftPortVanillaEntryMakeEffect`). That routine switches on `fp->fkind`, which for a clone is a Remix id,
so it fell into `default` and nothing was made. The entry *status* (the pipe hop, the barrel, the car) was
still the parent's, so the fighter appeared without its prop.

Remix keeps this in `Character.entry_script` (4 bytes per id, `get_entry_script_` 0x8013DC78): a pointer to
one of the vanilla appear routines (0x8013DC8C Mario pipe, 0x8013DC9C Fox Arwing, 0x8013DCAC DK barrel,
0x8013DCBC Samus, 0x8013DCCC Link, 0x8013DCE4 Yoshi egg, 0x8013DCF4 Kirby star, 0x8013DD04 Pokeball,
0x8013DD14 Captain car, 0x8013DD68 none) or to a Remix routine. A clone copies its parent's, every polygon
variant skips it, and Marth, Roy, Marina, Goemon and Ebisumaru run a Remix routine that only sets the
reversed-direction flag when facing left and then jumps into Link's.

**Fix:** `decomp/src/ft/ftremix/ftremixentry.c` holds the effective table for every synth id and is
installed as `entry_make_effect` by `ftRemixBindDescriptorHooks`. The vanilla makers read their model from
the vanilla fighter's file global (`gFTMarioFileSpecial2` and friends), which is not loaded when only a clone
is in the match. Each call therefore points the vanilla `EFDesc` at the entering fighter instead and restores
it afterwards:

- a clone that keeps the parent's effect uses the file slot (special1..4) that holds the same file *id*
  as the vanilla effect's file (Remix gives clones the parent's ids; JSamus, for example, has Samus's
  entry file in `file_6`, not in the vanilla slot);
- a Remix patch that swaps in its own struct is a row of `dFTRemixEntryOverrides`
  (`ftremixentrydata.c`): file slot, optional base effect for procs/transforms, and the four offsets.
  `linkshared.asm` (Young Link, Marth, Roy, Marina, Goemon, Ebisumaru, Banjo), `samusshared.asm` (Dark Samus,
  Crash), `dkshared.asm` (Sonic with Tails, Lanky's Tag Barrel) and `captainshared.asm` (J Falcon, Bowser's
  Clown Car, Conker's Greg's Hand) are entered. The Captain car maker also reads its animation joints from
  hard-coded offsets of the car file, so `efManagerCaptainEntryCarMakeEffect` calls `ftRemixEntryGetCarAnim`
  (`clown_car_animation1/2`, `get_entry_file_ptr_`). Sonic's right struct is unreachable: the asm picks it when
  `lr == 1`, but `ftCommonAppearSetStatus` clears `lr` first;
- when neither exists the effect is skipped with a log line instead of crashing: building a desc from a file
  that lacks the vanilla offsets crashed `gcAddDObj3TransformsKind` (JSamus before the id match).

**Not ported:** Wolf's Arwing (`Wolf.asm` `wolfen_entry*`: flags word 0x070A0000, `Size` update routine, per-fighter
offsets for Wolf and Slippy), and the `Size` entry-scale hooks (Blue Falcon, Kirby star, Yoshi egg). Wolf draws no
Arwing and logs it.

## 2. Polygon variants spawn into the floor

**Cause:** the polygon variants' entry action is the parent's appear action (`0xDD/0xDE` for Mario's family)
edited by Remix to `ftCommonEntryNullProcUpdate` with **no physics proc**, playing the parent's Wait animation
with the appear flag still on (`0x40000000` = `is_use_transn_joint`, the `FTAnimDesc` union; the `FTANIM_FLAG_XROTN_JOINT` macro in `ftdef.h` has the same value and a stale name). In the appear statuses
`ftCommonAppearProcPhysics` consumes that joint; without it the animation's pose is applied raw, so the
polygon is rotated and sunk until the countdown moves it to `Wait`.

**Fix (a known deviation, root cause not found):** `ftMainSetStatus` (REGION_REMIX) clears that flag when the
status runs `ftCommonEntryNullProcUpdate` with no physics proc. Checked with NDrMario, NWario and NWolf. This
overrides the Remix motion row, which really holds `0x40000000`, so it hides a difference between the port and
the N64: the cartridge presumably does not sink. The explanation above (no physics routine consumes the joint) is
a hypothesis; it was not verified. The flag makes `ftMainSetStatus` zero the TransN joint and run the TransN
hidden-part reparent (`ftMainAddHiddenPartID`, TransN is detached so the root translation does not move the model);
if the polygon skeletons' hidden-part table points that bit at a different joint, its track poses a body joint,
which would explain the crouch and is the first thing to check. Follow-up: find what the N64 does for a status
with that flag and no physics (the hidden-part table of the polygon attributes, the figatree parse of that joint) and
replace the guard with the real fix.

## Debug tooling

`SSB64_VS_STAGE=<gkind>` selects the stage of the `SSB64_VS_FKINDS` shortcut (default Dream Land).
The CSS can be driven headless: `SSB64_START_SCENE=16`, then `SSB64_INPUT_SCRIPT` lines with the stick at
`y=80` for 29 frames and `x=80` for 12 frames to put P1's hand on Mario (the hand moves 4 px per frame in
320x240 space), `A`, `START`, and `A` on the stage select.
