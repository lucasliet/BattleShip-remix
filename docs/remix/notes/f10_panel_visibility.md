# F10 panel: Visibility (CSS option 0 Default, 1 Cloaked, 2 None, 3 Dark)

## Edited

- `decomp/src/ft/ftremix/ftremixvisibility.c` (new): `ftRemixVisibilityInitFighter`, `ftRemixVisibilityPlayerTagSetAlpha`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declarations appended.
- `decomp/src/ft/ftmanager.c` `ftManagerInitFighter`: call after `ftRemixSizeInitFighter` (REGION_REMIX block).
- `decomp/src/if/ifcommon.c` `ifCommonPlayerTagProcDisplay`: alpha call before the tag position write (REGION_REMIX block).

Syntax check OK on the three C files.

## Patches

| Asm (Remix) | RAM | Vanilla function | Status |
|---|---|---|---|
| `CharEnvColor.asm` `initialize_override_table_` | 0x800D7CBC | `ftManagerInitFighter` (decomp/src/ft/ftmanager.c:593) | Applied. Sets `state` from the panel, then the override from the state. |
| `CharEnvColor.asm` `update_player_indicator_alpha_` | 0x80111B3C | `ifCommonPlayerTagProcDisplay` (decomp/src/if/ifcommon.c:2016) | Applied. Sets the tag alpha from the override. |
| `CharEnvColor.asm` `override_env_color_` (battle) | 0x800FC9DC | `ftDisplayMain` (ftdisplaymain.c:1286) | Already ported by the existing hook (`ftRemixEnvColorGetOverride`). Not changed. |
| `CharEnvColor.asm` `override_env_color_` (menu) | 0x80390548 | CSS/menu model draw | Skipped: the port does not redraw the CSS. |

## Behaviour

- Panel value to state: 0 Default and 1 Cloaked give `Normal`; 2 None gives `None`; 3 Dark gives `Dark`.
- On the How to Play and Auto-Demo scenes (`nSCKindExplain`, `nSCKindAutoDemo`) the state is `Normal`, as the asm clears the override there (the same scene check as Size). The asm's `0x77` (bonus 3 / multiman) keeps the override; the port keeps it too.
- Default: state Normal, `ftRemixEnvColorRestoreState` writes override 0. This matches the asm `NORMAL` path. Moveset overrides still apply because `ftRemixEnvColorGetOverride` falls back to the moveset table when the override is 0.
- Tag alpha: `ftRemixEnvColorGetOverride(player) & 0xFF` when nonzero, else 0xFF. This is the asm `update_player_indicator_alpha_` logic (override, then moveset, then opaque).
- Tag alpha offset: the asm writes byte `0x2B` of the SObj. On the N64 layout `Sprite` starts at SObj `0x10` and `alpha` is at `Sprite+0x1B` (decomp/include/PR/sp.h), so the byte is `sprite.alpha`.

## Deviations and open items

1. **None value.** The brief says `0xFFFFFF00`. The asm instruction is `addiu t9, r0, -0x00FF`, which is `0xFFFFFF01` (the comment is wrong). The port keeps `0xFFFFFF01` (`FTREMIX_ENVCOLOR_VALUE_NONE`), which is the asm value. Alpha is 0x01 in both cases. The moveset Sheik/Mewtwo/SSonic values use `0xFFFFFF00` (alpha 0), a separate path, not changed.
2. **Cloaked (1).** The asm writes `0xFFFFFF10` through the "I don't know how to do that yet" path. Per the brief, the port does nothing for it (state Normal). Cloak item state (`itRemixCloakingDevice`) is separate and unchanged.
3. **Priority.** An override of Dark or None shadows the moveset overrides (Sheik/Mewtwo/SSonic invisibility). The asm has the same priority (override table before moveset table), so this is intentional, not a regression.
4. **Transparency render.** The asm uses a custom display list (translucent render mode) when the alpha is not 0xFF, which affects None. The port's existing path in `ftDisplayMain` sets the env colour and `sFTDisplayMainSkyFogAlpha` only. Not verified here: the lead must check that a None fighter is really see-through in game.
5. **Build.** Not built, as the brief requires. No REMIX-TODO existed for Visibility, so nothing was deleted.
