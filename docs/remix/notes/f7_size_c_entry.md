# F7 Size phase (c): arwing, entry and item (Size.asm 2242-2724)

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm`. Phase (c) is cosmetic: entry effects, the Arwing and item
render. Every ported patch is a no-op at the default multiplier 1.0F.

## Files

- New: `decomp/src/ft/ftremix/ftremixsizeentry.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
- Declarations: block appended before the final `#endif` of `decomp/src/ft/ftremix/ftremixhooks.h` (nothing reordered).
- Vanilla hooks (each under `#ifdef REGION_REMIX`):
  - `decomp/src/ft/ftcommon/ftcommonentry.c` (include + call in `ftCommonAppearProcPhysics`)
  - `decomp/src/ef/efmanager.c` (call in `efManagerFoxEntryArwingProcUpdate`)
- Remix entry: `decomp/src/ft/ftremix/ftremixentry.c` (`ftRemixEntryMakeFoxArwing` binds the owner; REMIX-TODO).
- Item: `decomp/src/it/itremix/itremixcloud.c` (REMIX-TODO only).

## Patches

| Asm (line) | RAM | Vanilla function | Status |
|------------|-----|------------------|--------|
| arwing `update_routine_._update` (2245, data word 0x8012E704) | 0x8012E704 (ptr to 0x80103780) | efManagerFoxEntryArwingProcUpdate (ef/efmanager.c) | applied as a direct call, not a pointer swap: the wing joint scale is multiplied by the owner multiplier after `gcPlayAnimAll` |
| arwing `_initialize` (2250) | 0x801038F0 | efManagerFoxEntryArwingMakeEffect | applied in `ftRemixEntryMakeFoxArwing`: binds the owner fighter GObj in the EFStruct (`ftRemixSizeEntryArwingBind`) |
| entry `animation_` (2310) | 0x8013DB2C | ftCommonAppearProcPhysics (ft/ftcommon/ftcommonentry.c) | applied: the trans joint (`fp->joints[nFTPartsJointTransN]`, offset 0x8EC = joints[1]) translate is scaled for Mario, Luigi, DRM, DRL, J Mario, J Luigi, Metal, MLuigi, Conker, before the top joint reads it |
| entry `gfx_` x7 (2362-2392, 2398) | 0x80103768, 0x8010345C, 0x801034B8, 0x80102B28, 0x80102B84, 0x80102C10, 0x80103670 | efManager{MarioEntryDokan, DonkeyEntryTaru, SamusEntryPoint, LinkEntryWave, LinkEntryBeam, KirbyEntryStar, CaptainEntryCar}MakeEffect | skipped, REMIX-TODO(Size) in ftremixentry.c: the asm picks the joint per effect (`_top_joint`, `_2nd_joint`, `_link_ground_gfx`, `_blue_falcon`) and sets the z offset from the caller; not verified without a build |
| item `apply_size_to_animation_` (2620) | 0x8003DE5C jump table | animation scale setters (vanilla lb/gc, not mapped) | skipped, REMIX-TODO(Size) in itremixcloud.c: needs the animation command interpreter |
| item `render_routine_` (2675) | wraps 0x80171F4C (itDisplayColAnimOPAProcDisplay) | item render wiring of the cloud | skipped, REMIX-TODO(Size) in itremixcloud.c: the render installation is not located in the Remix port |

## Uncertainties (verify with a build and a Remix run)

- U1. The Arwing wing scale is applied in place each frame (`scale *= multiplier`), as the asm does. If the
  AnimJoint resets the scale every frame, the in-place multiply is harmless; if it does not, the scale would
  compound at multiplier != 1.0F. Only the 1.0F default is confirmed.
- U2. The asm scales the Arwing after the original update, including the frame it ejects. The port scales before the
  eject check; the eject only removes the object from the list, so the frame is the same.
- U3. `animation_` assumes the asm offset 0x8EC is `joints[nFTPartsJointTransN]` (joints at 0x8E8, index 1). Checked
  against `build/remixasm/refs/struct_offsets.txt` (`joints`, `DObj *[37]`).
- U4. The Mario-family list uses `REMIX_ID_METAL` for Metal Mario (asm `Character.id.METAL`) and `nFTKindMario`,
  `nFTKindLuigi` for the vanilla Mario and Luigi.

## Wire and test

- Nothing to register: `ftremixsizeentry.c` is picked up by the `decomp/src/ft/*` glob pattern used for ftremix (check the
  CMake glob covers `ft/ftremix/*.c`).
- Test: at multiplier 1.0F nothing changes. A Remix build should match the previous one.
- Not verified: no build, no game run (per brief). `scripts/remix/syntax-check.sh` returned `syntax-check OK` for
  ftremixsizeentry.c, ftcommonentry.c, efmanager.c, ftremixentry.c and itremixcloud.c.
