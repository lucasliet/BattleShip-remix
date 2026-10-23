# F10 panel items: Start With and Taunt Item

Asm source: `.toolchain/pkgs/smashremix-src/src/Item.asm` lines 1698-2000 (`start_with_table_`, `match_begin_with_item_`,
`start_with_item_`, `respawn_with_item_`, `taunt_item_table_`, `taunt_spawn_item_`), plus `create_and_assign_item_` (2003).
Panel values: 0 None, 1..18 item kinds in table order, 19 Random. Index 0 gives nothing.

## Port

New file `decomp/src/ft/ftremix/ftremixstartitem.c`:

- `sFTRemixStartItemKinds[19]`: index to item kind. Kinds are the Remix ids themselves (vanilla enum for 0x07..0x13, Remix
  defines for 0x30, 0x32, 0x34, 0x35, 0x38).
- Random (19) rolls `syUtilsRandIntRange(19)`; a roll of 0 becomes 1, as `Global.get_random_int_` plus the `beql` in the asm.
- Spawn at the fighter top joint with velocity (0, 30, 0), flags `ITEM_FLAG_PARENT_GROUND` (asm 0x10(sp) = 1). Vanilla
  kinds go through `dITManagerProcMakeList` directly (declared extern here, defined in `it/itmanager.c`), Remix kinds through
  `itRemixMakeItem`. Neither path plays the spawn swirl or appear spin, matching the asm skip-gfx flag.
- Hold through `itMainSetFighterHold` (asm 0x80172CA4). Nothing is given when `fp->item_gobj` is already set (asm
  `create_and_assign_item_` checks the held pointer).
- Hammer: sets `fp->hammer_tics = 0x2D0` and calls `ftHammerSetStatusHammerWait` (asm 0x800F3938).
- Entry points: `ftRemixStartItemMatchBegin`, `ftRemixStartItemRespawn`, `ftRemixTauntItemSpawn` (declared in
  `ftremixhooks.h`, appended block).

## Hooks (all under `#ifdef REGION_REMIX`)

| Asm site | Vanilla function | Port hook |
|---|---|---|
| `Item.asm:1729` `match_begin_with_item_` (ROM 0x8D9F4, GO) | `ifCommonAnnounceGoSetStatus` (`src/if/ifcommon.c`, 0x801121C4) | `ftRemixStartItemMatchBegin` after `ftParamUnlockPlayerControl` |
| `Item.asm:1940` `taunt_spawn_item_` (ROM 0xC918C, in appeal) | `ftCommonAppealSetStatus` (`src/ft/ftcommon/ftcommonappeal.c`, 0x8014E6E0) | `ftRemixTauntItemSpawn` after `ftMainSetStatus` (asm hook is before `flag1 = 0`) |
| `command.asm:138` `respawn_with_item_` (action Revive1) | `ftCommonRebirthStandSetStatus` (`src/ft/ftcommon/ftcommonrebirth.c`, 0x8013D2DC) | `ftRemixStartItemRespawn` after `ftMainSetStatus` |

The panel macro `gmRemixPanel(START_WITH | TAUNT_ITEM, fp->player)` returns 0 for non-human ports, so CPU ports get nothing.
With the default panel (0) all three hooks are no-ops, so vanilla behaviour is unchanged.

## Not closed (for the lead to test or decide)

1. Training mode: `Render.asm:2410` calls `Item.start_with_item_` (non-VS setup). The port hook is only on the GO
   announce. If training does not run `ifCommonAnnounceGoSetStatus`, Start With does nothing there. Not checked in code.
2. Hammer midi (asm 0x800E7AFC, midi 0x2D) not played. Marked `REMIX-TODO(audio)` in the file.
3. Respawn hook assumes Revive1 = `ftCommonRebirthStandSetStatus`. Not verified in game.
4. `fp->hammer_tics` assumed to be the asm `0x0B14(player struct)` field. Not verified against the struct offset.
5. Asm table comment says Pitfall = 0x33, but `Item.asm` `add_item(Pitfall)` gives 0x34. The port uses 0x34.
6. Random uses `syUtilsRandIntRange` assumed to match `Global.get_random_int_` for range 19.
7. Not built and not run (lead builds and tests). Syntax-check OK on the four touched .c files.
   The `ifcommon.c:2407` self-assign warning was already there.

## Test ideas for the lead

- VS, Start With = Beam Sword (1) for P1 at GO: the fighter holds the sword at start. Random (19) gives an item from 1..18.
- Taunt Item = Bob-omb (9), taunt on the ground: a Bob-omb spawns and is held by the taunter.
- Respawn with Start With set: the item is held after the stage-in.
- Default panel (0): no item at start, no item on taunt, no change from vanilla.
