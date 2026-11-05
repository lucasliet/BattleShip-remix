# I7: image or palette swap per model part (`MODEL_PART_IMAGE_INIT_`)

## How the asm works

`Render.MODEL_PART_IMAGE_INIT_` (`src/Render.asm:68`, RAM `0x800090DC`) is not a Remix routine: it is the vanilla `gcAddMObjForDObj`
(`decomp/src/sys/objman.c:1374`). Remix builds an `MObjSub` ("model part image") in its own memory and attaches it to a DObj. Three
different Remix features write the MObj fields that decide a part's texture, palette and colours:

| Feature | asm | Runs when | Fields written (N64 MObj offset, port field) |
|---------|-----|-----------|----------------------------------------------|
| Extra costumes | `src/Costumes.asm:1310` `check_for_custom_costume_`, hooks `0x800C8D84` and `0x800C8CE0` inside `lbCommonAddMObjForFighterPartsDObj` (`0x800C8CB8`) | every time a fighter part gets its MObjs: model setup, costume change (`ftParamInitAllParts`), special part swap (`ftParamSetModelPartID`), hidden parts | `0x34` `sub.palettes` (palette array), `0x88` `palette_id` (float, extra costume index), `0x58` `sub.primcolor`, `0x68`/`0x6C` `sub.light1color`/`light2color`, `0x0C` `sub.sprites` |
| Yoshi egg colour | `src/Shield.asm:188` `alter_yoshi_egg_color_` (patches `0x801013A8` shield, `0x8010318C` roll, `0x8015EAF4` thrown egg, `0x8014D080` and `0x8014C984` egg lay, `0x80102F5C` entry) | when the egg shield, thrown egg, egg lay egg or entry egg is made | writes `DE000000 0E000000` over the palette `G_SETTIMG` of the egg display list (byte `+0x50`, `+0x58` or `+0x48`), calls `MODEL_PART_IMAGE_INIT_` with a 1 image part whose flag is 4 (palette array, `table_palettes`, 16 entries), then `0x88` = palette index |
| Stage texture | `src/Render.asm:1280` `draw_stage_texture_` | stage backgrounds | not a fighter feature, not part of this item |

The costume tables are Remix data in the RAM image:

- `Costumes.extra_costumes_table[fkind]` (97 words) is the N64 address of a part table; 0 when the character has no extra costume.
  `Costumes.select_.num_costumes[fkind]` is the highest vanilla costume id (0 based).
- Part table (`define_part`, `src/Costumes.asm:27`): byte 0 extra costumes, byte 1 part id and byte 2 first image index of the accessory
  part (the separate FighterParts GObj, id `0x3E9`), byte 3 skipped ids; then 8 bytes per part id (`joint_id - 4`): images array,
  special parts array. Part 0 never has images.
- Image (0x10 bytes): palette array (one palette pointer per extra costume), prim color array (8 bytes per costume: high poly,
  low poly), texture array array, diffuse/ambient array (one pointer per costume to a diffuse/ambient word pair).
- Costume ids: `0..num` vanilla, then `skipped` ids nobody can pick, then the extras. Extra index = `costume - (num + skipped + 1)`
  (`Costumes.asm:1347-1360`). `select_` (`Costumes.asm:619`) cycles C right/left over `0..num + skipped + extra` jumping the skipped ids.
- The MObj of a part is matched to an image by position (`image_index` counts the MObjs of the part; the accessory part starts at
  byte 2 of the table). The special part index (`ftParamSetModelPartID` model part id) picks the special parts array. Samus,
  E Samus and J Samus swap images 0 and 1 of part 2 on the low poly model (`Costumes.asm:1429-1456`).
- The texture array branch (`Costumes.asm:1511-1520`) reads arrays that no Remix costume file fills (`part_type.TEXTURE_ARRAY`
  allocates them, no `set_texture_array` macro exists), so it never overrides anything and is not ported.

## What the port does

| Piece | Where |
|-------|-------|
| Pure index math: extra index, max costume, C right/left with skipped ids, Random legality, prim color pick, accessory image slot, egg palette index | `decomp/src/ft/ftremix/ftremixpartimagerules.{c,h}`, test `tests/remix/test_partimage_rules.c` (54 checks) |
| Big-endian palette copies and token arrays of image palettes | `ftremixpartimagepalette.{c,h}` |
| Extra costume application, costume counts and cycling for the CSS, stock icon LUT | `ftremixpartimage.{c,h}` |
| Yoshi egg shield colour | `ftremixyoshiegg.c` |

Decisions:

- The hook of the asm sits inside `lbCommonAddMObjForFighterPartsDObj`, which has no joint id (the asm recovers it by scanning
  `FTStruct.joints` or from a stack slot). The port applies the same writes to every joint of the fighter from
  `ftDisplayMainProcDisplay` (`ftRemixPartImageApplyFighter`, one guarded call after `ftRemixDKModeResizeHead`). It is idempotent,
  covers every way the MObjs are rebuilt, and returns after three image reads for a vanilla costume.
- Palettes live in the RAM image as native-order words, but Fast3D reads a TLUT as big-endian bytes and only fixes the byte order of
  data inside registered reloc ranges. The image is not one, and fixing it in place would corrupt the tables that sit next to the
  palettes. Each palette is copied (256 colours, because the TLUT load of a CI8 image reads 256) as big-endian bytes into a static
  pool of 1024, registered with `gfxRegisterTrustedLowVARange` (non-PIE builds drop `G_SETTIMG` pointers in the low address range).
  Palette arrays get a slot array of tokens (512 arrays of up to 16 palettes).
- The special part index is the model part id of the joint only when the joint has a model part description (asm: only the special
  part swap of `ftParamSetModelPartID` knows it, every other path uses the normal images array; the difference shows only if a joint
  keeps a special part through `ftParamInitAllParts`, which the CSS never does).
- Yoshi egg: only the shield and the egg escape effect are ported (both draw through `efManagerYoshiShieldProcDisplay` and share one
  display list, which has a single user in the Yoshi model file). `Character.yoshi_egg_color.table` is never written by any Remix
  file (zero fill, `Character.asm:3089`), so the palette index is 0 for every Shield option except Costume (17), which uses
  `Character.costume_shield_color` (`ftRemixShieldGetCostumeColorIndex`); Vanilla (16) is 0. The patch checks that command 10 is the RGBA16
  `G_SETTIMG` and command 12 a `G_LOADTLUT` before writing, so the polygon Yoshi list is left alone.

## Classic Sonic is not an I7 consumer

`Sonic.classic_table` (`src/Sonic/Sonic.asm:1000`) selects a different model file, not part images: `load_classic_model_` (`0x800D7FDC`)
loads `File.CSONIC_MAIN` (0xE38, also 0xE39 character, 0xE3A shield pose) instead of `File.SONIC_MAIN` from the character struct field
at +0x40 (`Character.asm:3737`), the CSS toggles the flag when the costume cycles through 0 (`Costumes.asm select_` `_sonic`), the FGM and
voice patches (`Sonic.asm:545-700`) swap sounds, and the shield colour reads the costume +6 (`Shield.asm color_fix_`). The port keeps the
flag (`ftRemixSonicSetClassic`, nothing sets it yet). Needs its own task: load the classic main file for the synth Sonic when the flag is
set, the CSS toggle, then the `+6` in `ftRemixShieldGetCostumeColorIndex`.

## Hooks for the other owners

1. CSS (`mn/`, other worker). After a fighter model is made or re-costumed on any CSS, call `ftRemixPartImageApplyFighter(fp)`
   (`ft/ftremix/ftremixpartimage.h`), because the CSS model is not drawn by `ftDisplayMainProcDisplay`. Replace
   `mnRemixCostumeGetMaxID`'s use in the cycle by the pair `ftRemixPartImageGetNextCostume(fkind, costume)` and
   `ftRemixPartImageGetPrevCostume(fkind, costume)` (they jump the skipped ids and wrap over the extras), and use
   `ftRemixPartImageIsLegalRandomCostume(fkind, costume)` and `ftRemixPartImageGetMaxCostumeID(fkind)` for the Random pick
   (`get_random_legal_costume_` rerolls ids that are skipped).
2. Match HUD stock icon (`decomp/src/if/ifcommon.c:1154, 1159, 1187, 1192, 1358, 1361, 1450, 1453`): `_luts[fp->costume]` reads past the
   vanilla LUT array for an extra costume. Wrap each token with `ftRemixPartImageGetStockIconLUT(fp, _luts[fp->costume])`
   (returns the vanilla token for every other costume). Not wired because `ifcommon.c` is outside this item's files.
3. Symbols: `tools/gen_remix_actions.py` `EXTRA_SYMBOLS` should list `Shield.alter_yoshi_egg_color_.table_palettes`,
   `Shield.alter_yoshi_egg_color_.table_entry_palettes` and `Costumes.stock_icon_palette_array_table`. Until then
   `ftremixyoshiegg.c` and `ftremixpartimage.c` define the first and the last one as a fallback with the Remix 2.0.1 address (`0x804E0CC4`,
   `0x805289B4`).
4. `ftremixshield.c` still says the Yoshi egg is "Not portable"; that paragraph is stale now.

## Hazards met

- No sprite or texture header is involved: every palette is read as raw RGBA5551 words, so the `width_img` hazard of
  `lbreloc_byteswap.cpp` does not apply. Stock icon palettes are 16 colours (CI4).
- The `ram_image` resource has a 0x44 byte header before the 4 MiB image when read with a plain unzip (the loader strips it).

## Not done

- Thrown egg (`0x8015EAF4`), egg lay egg (`0x8014D080`, `0x8014C984`) and entry egg (`0x80102F5C`, palette table `table_entry_palettes`):
  same technique, other display lists, other creation functions (`wp/`, `ef/`, `ft/`).
- Per-character `Character.costume_shield_color` rows for extra costumes beyond the 8 byte rows already in `ftremixshieldcostume.c`.
- Classic Sonic (above).

## Test recipe (not run: the build and the game were not available to the worker)

Debug override added for it: `SSB64_REMIX_COSTUME_P1..P4=<costume id>` (`ftRemixPartImageDebugCostume`, called from `ftManagerMakeFighter`),
because the VS shortcut always makes costume 0.

- Extra costume: `SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,0 SSB64_REMIX_COSTUME_P1=5 SSB64_REMIX_COSTUME_P2=7 SSB64_SCREENSHOT_FRAMES=520,540`.
  Mario has costumes 0 to 4 and extras 5 to 7 (`Mario.asm` `set_palette_for_part`, `set_prim_color_for_part`); P1 and P2 must not look
  like costume 0 or like each other. Repeat with Falco (fkind 29 in `remix_roster_data.h`), Yoshi (6), a character with skipped ids
  (Pikachu: table header 4 extras, 2 skipped, so extras start at id 6).
- Yoshi egg: `SSB64_START_SCENE=22 SSB64_VS_FKINDS=6,0 SSB64_REMIX_PANEL="Shield.1=17" SSB64_REMIX_COSTUME_P1=1`, and an input script line
  `500 560 0 Z`. Costume 1 maps to Shield index 1 (red) in `Character.costume_shield_color`, so the egg shield must be red, not the
  vanilla green. With `Shield.1=0` or 16 and the same costume the egg stays green. Repeat with `SSB64_REMIX_COSTUME_P1=4` (pink, index 12).
- Log lines to look for: none expected; a `RelocPointerTable invalid/stale token` or `gcDrawMObjForDObj ... palette-null` line means a
  palette slot did not resolve.
