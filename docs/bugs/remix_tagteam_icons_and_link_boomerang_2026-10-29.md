# Tag Team: invisible queue icons and Remix Link crash on a recycled FTStruct

Code: `decomp/src/gm/gmremixtagteamicons.c`, `decomp/src/if/ifcommon.c` (`ifCommonPlayerStockPrepareSprite`),
`decomp/src/ft/ftmanager.c` (`ftManagerInitFighter`). Test: `SSB64_VS_TEAMS=1 SSB64_REMIX_MODE=tagteam SSB64_VS_STOCKS=4` with
`vs.sh 0,8,6` (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom 1: queue icons of characters outside the match are invisible

The HUD lists one stock icon per character left in the Tag Team queue. The icons of characters whose files are not loaded by the
match (the 2nd to 6th slots) drew as empty space, while the icons of the fighters in the match drew correctly.

## Cause 1

`ifCommonPlayerStockMultiMakeInterface` prepares the stock sprite only for the fighters of the match: it runs
`ifCommonPortFixupSpriteFull` on the sprite (byte order, bitmap array, texel data) and sets `SP_TEXSHUF | SP_TRANSPARENT`. A queue
character is read from a private copy of its main file (`gmremixtagteamicons.c`), so its stock sprite never went through that
path. Same family as `zako_stock_sprite_unfixed_2026-05-01.md`.

## Fix 1

`ifCommonPlayerStockPrepareSprite` (`if/ifcommon.c`) wraps the two steps and the icon module calls it for every sprite it
resolves. The private copy of the main file is only read for attributes, so the swap still loads the files of the fighter the
usual way (`ftManagerSetupFilesAllKind` is untouched).

## Symptom 2: SIGSEGV in `wpMainDestroyWeapon` when a swapped-in Link variant dies

`ftManagerDestroyFighterWeapons` -> `ftLinkSpecialNDestroyBoomerang` -> `wpMainDestroyWeapon` read a stale
`passive_vars.link.boomerang_gobj` (fault address 0xe8) on the death of Young Link, after a Tag Team swap.

## Cause 2

`ftManagerInitFighter` clears `passive_vars.link.boomerang_gobj` only for `nFTKindLink` / `nFTKindNLink`. The Remix Link variants
(`REMIX_ID_YLINK`, `REMIX_ID_ELINK`, `REMIX_ID_JLINK`) were handled by the destroy switch but not by the init switch. In a normal
match the FTStruct pool is zeroed once, so a variant always started with NULL. Tag Team destroys a fighter and makes another, so
the struct comes from the free list with the previous fighter's union bytes.

## Fix 2

`ftManagerInitFighter` clears the boomerang pointer for the three Remix Link variants (`REGION_REMIX` only; the model-part setup
of vanilla Link is not repeated for them).

## Verification

`vs.sh 0,8,6` with the variables above: the crash is gone (3 fighters, 9500 frames, results screen), the icons of all queue slots
draw in reverse queue order, and the thief shows and plays the stolen character after each steal (log lines
`tagteam: port 0 steals queue slot 3 of port 1` then `swaps fkind 10 for 57`).
