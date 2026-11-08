# Remix: Marth sinks into the floor during the up smash (inherited TransN on common moves)

Code: `decomp/src/ft/ftmain.c` (`ftMainDropInheritedHiddenJoints`, called from `ftMainSetStatus`).

## Symptom

Reported from the Switch build with screenshots (Training on Yoshi's Island, 1P against the Fighting Polygon Team on Duel
Zone): "attack + up" with Marth puts him into the ground, only the hair and cape stay above the floor. The move is the up
smash (AttackHi4, status 0xCF); the up tilt (AttackHi3, 0xC7) is fine. Reproduced on macOS on the Duel Zone main floor:
`SSB64_VS_STAGE=14 SHOTS=$(seq -s, 480 2 530) vs.sh 58,8 inputs/usmash_zako_floor.txt usmash58_zako_floor 560`, frames
486 (hair and cape tips above the floor) and 494 (head and shoulders). Roy (fkind 74) too.

## Cause

Same mechanism as `remix_inherited_hidden_joint_sink_2026-11-06.md`: `ftPortCopyMotionArray` merges the parent's
hidden-joint flags into every synth row. Captain Falcon's AttackHi4 row carries `0x40000000` (TransN); Marth's row is 0 and
his up smash animation has no TransN track, so the body height track lands on the hidden TransN joint, which is detached
from the body, and the body is drawn below the floor. The 2026-11-06 fix dropped the inherited bits only for statuses
whose routines are all ported Remix routines; the up smash runs the common `ftCommonAttackHi4*` routines, so its flags
were kept. The up tilt has no inherited bits (Captain's AttackHi3 row is 0), which is why it never sank.

## Fix

`ftMainDropInheritedHiddenJoints` also drops the inherited bits for every status served by the common action table
(`status_struct == dFTCommonActionStatusDescs`). Those routines are shared by every fighter and the ones that read
TransN test the flag first (`ftPhysicsApplyGroundFrictionOrTransN`) or are NULL-guarded in the port, so the Remix row's
own flags are the ROM's rule for them, exactly as for the Remix-owned statuses. Special statuses that still run a parent
routine keep the merged flags, as before.

## Verification

- `fix_usmash58_zako`, `fix_usmash58_yoshi`, `fix_usmash74_zako`: Marth and Roy stand on the floor through the up smash.
- `reg_nsp58`, `reg_nsp74` (Dancing Blade, stage 16): unchanged.
- `reg_normals<fkind>` (`inputs/normals_floor.txt`: jab, tilts, smashes on the Duel Zone floor) for Marth, Roy, Wario,
  Mewtwo, Sheik, Dedede, Banjo, Peach, Falco and Conker, and `parallel.sh 6 matrix 29..96` (NSP/DSP/USP of every Remix
  fighter): no crash and no error lines.
