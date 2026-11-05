# Remix settings registry

Covers how the port reads the Smash Remix settings (`Toggles.asm`) and how decomp code uses them. Code:
`port/remix/remix_toggles.{h,cpp}` (registry), `decomp/src/gm/gmremixtoggle.{h,c}` (decomp API and the single-player guard),
`port/gui/PortMenu.cpp` (the **Remix** section of the port menu). Test: `SSB64_REMIX_TOGGLES` and `SSB64_REMIX_PROFILE` in any
`vs.sh` run (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Registry

`remix_toggles.cpp` builds the settings from the Remix RAM image: the five `Menu.entry` lists
(`Toggles.head_{remix,gameplay,music,stage,pokemon}_settings`, linked by the word at +0x1C) hold 531 toggles. The non-title nodes
sorted by address are exactly the order of `Toggles.profile_defaults_{CE,TE,NE,JP}` (531 of 531 default values equal the node
values). A node is `type, value, min, max, a_function, string_table, copy_address, next, label, extra, title...`
(`Menu.asm`). Values are CVars `gRemix.Toggle.<index>`. `SSB64_REMIX_TOGGLES="Title=value;..."` and `SSB64_REMIX_PROFILE=<0..3>`
(0 CE, 1 TE, 2 NE, 3 JP) override a run without touching the CVars. The port menu has a **Remix** section generated from the
registry (a sidebar per group, profile buttons).

Decomp code reads a setting with `gmRemixToggle(<label>)` or `gmRemixToggleVS(<label>)` (`gm/gmremixtoggle.h`). The label is
the name in `Toggles.asm` without `entry_`. The VS variant is `Toggles.single_player_guard`: Bonus and 1P Game read 0
(`gmRemixToggleModeIsGuarded`, which returns FALSE when there is no battle state).

## Gameplay settings

Each gameplay setting is ported where Remix patches the behavior, under `REGION_REMIX`. The regression rule used to accept
them: with every setting off the special matrix is unchanged, and with all of them on it runs without errors (6 fighters x NSP,
DSP, USP). Two settings seen acting in a match: Air Dodging shows the new escape statuses; Hazard Mode = 3 removes the Pirate
Land cannon.

Open gaps: `docs/remix/patch_audit.md`, groups g5 and g9.
