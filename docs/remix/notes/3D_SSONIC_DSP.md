# 0x3D SSonic DSP notes

Source: `.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm`, scope `SSonicDSP` (lines 3646-3725). Emerald helpers: `.toolchain/pkgs/smashremix-src/src/SSonic/SSonic.asm` lines 426-505.

Output: `decomp/src/ft/ftremix/ssonic/ftremixssonicdsp.c`, table `dFTRemixSSonicDSPProcs` (2 entries, `dFTRemixSSonicDSPProcsNum` = 2). Scope in worklist: 2 routines, both ported.

## 1. Vanilla patches

None. The `SSonicDSP` scope has no `OS.patch_start` blocks. Its entries are referenced only from the action array in `SSonic.asm` (lines 319-323).

## 2. Shared Remix subsystems

- **Emerald effect** (`SSonic.emerald_graphic_init_`, `emerald_graphic_update_`, `emerald_graphic_struct`). Not in the worklist, but both entries call it. Ported as file-local statics: `ftRemixSSonicEmeraldInit`, `ftRemixSSonicEmeraldProcUpdate`, and the `EFDesc sFTRemixSSonicEmeraldDesc`. It uses only vanilla `efManagerMakeEffectForce`, `gcPlayAnimAll`, `gcEjectGObj`, `efManagerSetPrevStructAlloc` and `gcDrawDObjTreeForGObj`. No REMIX-TODO marker was needed.
- **Emerald desc bytes.** The `OS.copy_segment(0xA98F4, 0x8)` bytes decode as `4F 00 00 12 2C 00 00 00` (checked in `baserom.us.z64` and `baserom.remix.z64`). That is transform1 `{0x4F, 0x00, 0x00}` and transform2 `{nGCMatrixKindTra, 0x2C, 0x00}`, the same as `dEFManagerFoxReflectorEffectDesc`.
- **Emerald desc words after the copy.** The render proc is `0x80014038`, which is `gcDrawDObjTreeForGObj` (decomp/src/sys/objdisplay.c:2054). The file offsets are DObj setup `0x930`, mobj sub `0`, anim joint `0x1B08`, matanim `0`, all relative to the SSonic file 7 base (`p_file_special2`).

## 3. Approximations and uncertainties

1. **Return value of `emerald_graphic_init_` (SSonic.asm lines 448-455, call site SonicSpecial.asm 3673/3713).** On the success path the asm calls `0x8000DF34` (`gcPlayAnimAll`) last, so `v0` at the caller's `beqz v0` is whatever that call leaves behind, not the effect pointer. The port returns the effect GObj, and `is_effect_attach` is set when it is non-NULL. This matches the code's intent ("branch if no gfx object created") and differs from the N64 only if `gcPlayAnimAll` leaves a non-zero garbage `v0` after a NULL effect, which is not possible here. Verified by reading: on the NULL path `v0 == 0` from `efManagerMakeEffectForce`.
2. **Gfx pointer at N64 0x0B24.** It is a pointer, so it cannot live in the LP64 `s32` word accessor. It is stored in `sFTRemixSSonicDSPEmeraldGObjs[fp->player]`. Nothing in the SSonic scope reads 0x0B24 back (grep of SonicSpecial.asm and SSonic.asm shows only these two writes). Stale after the emerald is ejected, but unread.
3. **Gravity delay at N64 0x0B28** is written with `ftRemixFighterWord(fp, 0x0B28)`, so it shares storage with the Sonic DSP movement timer (SonicSpecial.asm 467/520/586-589) as in Remix. Not an issue for this scope.
4. **Emerald effect vars.** Remix writes EFStruct words 0x18 (= 0) and 0x1C (= 4). The port writes `effect_vars.reflector.index/status`, which are the same words the Fox reflector effect uses. The emerald update does not read them.
5. **Action ids.** `Action.FOX.ReflectorStart` = `0x0EC` and `ReflectorStartAir` = `0x0F1` (Action.asm lines 738, 743). They are Fox actions, so they are named as file-local enum constants with `nFTRemixSSonicDSP...`.
6. **File 7.** Taken as `fp->data->p_file_special2`, as the brief maps N = 7. The emerald desc's `file_head` is set to that pointer (no offset), as the Remix struct stores `Character.SSONIC_file_7_ptr` directly.
7. **Velocity multiply.** `f2 * 1.25F` (`0x3FA00000`) is written as `*= 1.25F` on `physics.vel_ground.x` (ground) and `physics.vel_air.x` (air). The air entry also zeroes `physics.vel_air.y` (asm `sw r0, 0x004C`).

## 4. Kirby branches

None in this scope.

## 5. Non-static symbols

None. No vanilla hook calls into these routines.
