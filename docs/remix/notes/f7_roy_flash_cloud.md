# F7 notes: Roy counter, Flashbang flags, Cloud flash flags

## 1. Roy counter (Remix id 0x4A)

Applied: nothing. The Roy counter is disabled in the shipped Remix source, so there is no active behaviour to port.

- The only counter hit code is `scope detection_patch_` in `.toolchain/pkgs/smashremix-src/src/Marth/MarthSpecial.asm` (lines 1152-1190), patching `ftParamUpdateDamage` (0x800EA248, `ft/ftparam.c`).
- The Roy branch is commented out (MarthSpecial.asm 1168-1176, "comment out the above and uncomment the below to give Roy a counter lmao"). The active code checks `Character.id.MARTH` only.
- The port hook `ftRemixMarthDSPCounterDamage` (`decomp/src/ft/ftremix/marth/ftremixmarth.c:573`) returns `damage` unchanged unless `fp->fkind == REMIX_ID_MARTH`. Roy keeps vanilla damage.
- `Roy.asm` and `RoySpecial.asm` have no counter or hit-reaction logic (grep for `counter` and `hit` finds none outside the Marth file).
- The Roy DSP port (`decomp/src/ft/ftremix/roy/ftremixroydsp.c`) has no REMIX-TODO markers.

Lead decision needed: porting the commented-out Roy branch would enable a behaviour the Remix author disabled. It was not applied. If wanted, the change is one extra `fkind == REMIX_ID_ROY` test in `ftRemixMarthDSPCounterDamage` plus the DSP state check (DSP_Ground/DSP_Air) that Marth already does.

## 2. Flashbang (item 0x4A, `decomp/src/it/itremix/itremixflashbang.c`)

Applied, all in the port file (no vanilla patch):

| Asm | Port | Where |
|---|---|---|
| Flashbang.asm 127-128: `0x0158 \|= 0x10` | `ip->attack_coll.can_rehit_shield = TRUE;` | `itRemixFlashbangSetupState` |
| Flashbang.asm 184: `0x0100 = 0` | `ip->coll_data.cliff_id = 0;` | `itRemixFlashbangSetupState` |
| Flashbang.asm 618-624: first joint `0x0054 = 2` | `DObjGetStruct(item_gobj)->flags = 2;` | `itRemixFlashbangExplode` |

Mapping notes:
- `0x0158` bit 0x10 is `attack_coll.can_rehit_shield` in the big-endian bitfield table (`build/remixasm/refs/struct_offsets.txt`, `0x0158:3-3`). The asm comment calls it "unknown bitflag".
- `0x0100` is the same word as `coll_data.cliff_id` in ITStruct. The asm comment calls it a Bomb character id reference. The write is byte-equivalent, but its meaning in the item is only assumed.
- DObj `0x0054` is `DObj.flags` (`struct_offsets.txt`, DObj section).

Still `REMIX-TODO(GFX)`, not applied:
- Screen flash: Flashbang.asm 598-606 writes `flash_array_` to 0x80131A40 unless `Toggles.entry_flash_guard` is set. No port of the Remix toggle or the blend command exists in `decomp/src`.
- `Item.BlueShell.hit_gfx_` (asm 636). No port.

Uncertainties, unchanged from `docs/remix/notes/item_Flashbang.md`: blast zone return value, `is_allow_pickup` mapping of `0x02CE` bit 0x80, the sin/cos y velocity comment, the 0.9375 drag, `lrintf` versus truncation, and the landing-floor condition in detect collision.

## 3. Cloud flash flags (item 0x49, `decomp/src/it/itprocess.c`)

Applied:
- `itRemixKeepsFlashFlags(kind)` (`decomp/src/it/itremix/itremixcloud.c:52`, TRUE for 0x49 and 0x11A) is now called from `itProcessProcItemMain`, the `else` that clears `item_gobj->flags`.
- The vanilla line is kept under `#else`. The Remix form is:

```c
#ifdef REGION_REMIX
    /* Cloud.asm enable_flash (Cloud.asm 287-315): Cloud and the Bowser bomb keep their GObj flash flags. */
    else if (!itRemixKeepsFlashFlags(ip->kind)) item_gobj->flags = GOBJ_FLAG_NONE;
#else
    else item_gobj->flags = GOBJ_FLAG_NONE;
#endif
```

- `#include "it/itremix/itremixcloud.h"` is added in the existing `#ifdef REGION_REMIX` include block at the top of `itprocess.c`.
- `ip->kind` is ITStruct 0x0C (`struct_offsets.txt`). Cloud.asm 287-315 reads the item id from `0x0C(a2)`.

Uncertainties:
- The patch at RAM 0x8016F5B4 has a `bgezl t8` branch (Cloud.asm 292-305). The C form suppresses the store for kinds 0x49 and 0x11A on every path. That matches the asm, where the Cloud path never executes the store. The meaning of `t8` in the vanilla C was not confirmed against a disassembly, so the `else` placement is assumed.
- The earlier note said `waddledoo.c` consumes the helper. grep finds no consumer outside `itremixcloud.c`, and the helper now has one in `itprocess.c`.
- `itRemixKeepsFlashFlags` is defined under `#ifdef PORT` in `itremixcloud.c`. The Remix include and call are under `#ifdef REGION_REMIX`, following the same pattern as the basketball helper.

## Verification

- `scripts/remix/syntax-check.sh` returns OK for `itremixflashbang.c`, `itprocess.c` and `itremixcloud.c`.
- `itprocess.c` and `itremixflashbang.c` were also checked with `-DREGION_REMIX` (a temporary copy of the script in the session scratchpad). Both pass.
- No build, cmake, game or git commands were run.
