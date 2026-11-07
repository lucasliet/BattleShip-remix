# Remix: Marth and Roy sink into the floor during Dancing Blade

Code: `decomp/src/ft/ftmain.c` (`ftMainDropInheritedHiddenJoints`, called from `ftMainSetStatus`), `decomp/src/ft/ftport.c`
(`ftPortCollectParentHiddenJointFlags`, `ftPortSynthParentHiddenJointFlags`), `decomp/src/ft/ftremix/ftremixbind.c`
(`ftRemixIsPortedProc`).

## Symptom

Reported from the Switch build: "when I attack with A, Marth's special bugs and he goes underground", Roy too. On the
Switch pad the button labelled A is the N64 B button, so the move is the neutral special, Dancing Blade. Reproduced on macOS:
the first and second hits of the grounded Dancing Blade (statuses 0xE0 NSPG_1 and 0xE1..0xE3 NSPG_2) draw the fighter
with only the head and shoulders above the floor; the third hit (0xE4..0xE6) and every other move are fine. The
fighter's position (TopN) stays on the floor, only the pose sinks.

Repro: `SSB64_VS_STAGE=16 SHOTS=$(seq -s, 470 5 590) vs.sh 58,8 nsp_combo.txt combo58 600` with B tapped at 420/432/444 and
540/552/564 (frames 540..560 show the sunk pose). Roy is fkind 74.

## Cause

`ftPortCopyMotionArray` merges the parent's hidden-joint flags (XRotN/TransN/YRotN, `0xE0000000`) into every synth main-motion
row (`ftPortParentHiddenJointFlags`, added on 2026-10-12 so parent routines such as `ftMarioSpecialHiProcInterrupt` never read a
missing `joints[TransN]`). Marth's and Roy's parent is Captain Falcon, whose rows 0xE0..0xE6 (Falcon Punch and friends) set
`0x40000000` (TransN). Marth.asm sets those rows to flag 0 for NSPG_1/NSPG_2 (`edit_action_parameters(MARTH, Action.NSPG_1, ...,
0)`) and to `0x40000000` only for NSPG_3.

The Dancing Blade 1/2 animations were made without a TransN joint: their first track is the body height (y = 444.5), meant
for the regular root joint. With the merged flag, `ftMainSetStatus` builds the hidden TransN joint, the height track lands on
it, and the TransN joint is detached from the body (the vanilla rule for root-motion joints). The body keeps only the
following tracks (y about -1.5..-123), so it is drawn below the floor. NSPG_3's animation does carry a TransN track (y 0,
z root motion) and its height on the body joint, which is the vanilla layout (Mario, Fox, Link rows measured the same way).

## Fix

`ftport.c` keeps, per synth row, the hidden-joint bits that came only from the parent (`parent_hidden_flags`).
`ftMainSetStatus` drops those bits (`ftMainDropInheritedHiddenJoints`) when the status's update, interrupt and map routines
are all absent or ported Remix routines (`ftRemixIsPortedProc`): such a status runs none of the parent's routines, so the
row's own Remix flags apply. Physics routines are not checked: the vanilla ones that read TransN either test the flag
(`ftPhysicsApplyGroundFrictionOrTransN`, 0x800D8CCC) or are NULL-guarded in the port. Statuses that still run a parent routine
(Wario/Goemon/Crash up-B through `ftMarioSpecialHiProcInterrupt`, Marth's up-B through `ftCaptainSpecialHiProcInterrupt`) keep
the merged flags.

This generalizes the 2026-10-14 polygon entry rule (`remix_entry_effects_and_polygon_entry_2026-10-14.md`), which is kept.

## Verification

- `fix_combo58`, `fix_combo74`: Marth and Roy stand on the floor through all three Dancing Blade hits.
- `parallel.sh 6 matrix 29..96`: NSP/DSP/USP of every Remix fighter, no crash and no error lines.
- With a temporary log in `ftMainDropInheritedHiddenJoints`, the matrix showed the inherited flags dropped for Marth
  (0xDE, 0xE0), Roy (0xDE, 0xE0), Wario 0x21 (0xE1), Piano 0x36 (0xE1), Wolf 0x37 (0xE4, 0xE6), Mewtwo 0x39 (0xDF, 0xE0),
  Sheik 0x3E (0xE4, 0xE9..0xEB), Dedede 0x40 (0xE4), Goemon 0x41 (0xE1), Banjo 0x44 (0xE1, 0xE2) and Peach 0x49 (0xE6).
  Those statuses now follow their Remix rows; only Marth and Roy were checked frame by frame.
- `vs.sh 0,8 none.txt` and an A-move script on vanilla fighters: unchanged.
