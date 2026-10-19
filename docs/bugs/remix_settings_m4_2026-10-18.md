# Remix settings, milestone M4 (in progress, 2026-10-18)

## Registry (T4.1)

`port/remix/remix_toggles.cpp` builds the Remix settings from the image: the five `Menu.entry` lists
(`Toggles.head_{remix,gameplay,music,stage,pokemon}_settings`, linked by the word at +0x1C) hold 531 toggles; the non-title
nodes sorted by address are exactly the order of `Toggles.profile_defaults_{CE,TE,NE,JP}` (checked: 531 of 531 default values
equal the node values). A node is `type, value, min, max, a_function, string_table, copy_address, next, label, extra,
title...` (`Menu.asm`). Values are CVars `gRemix.Toggle.<index>`; `SSB64_REMIX_TOGGLES="Title=value;..."` and
`SSB64_REMIX_PROFILE=<0..3>` override a run without touching the CVars. The port menu has a **Remix** section generated from
the registry (a sidebar per group, profile buttons).

Decomp code reads a setting with `gmRemixToggle(<label>)` or `gmRemixToggleVS(<label>)` (`gm/gmremixtoggle.h`): the label is the
name in `Toggles.asm` without `entry_`; the VS variant is `Toggles.single_player_guard` (Bonus and 1P Game read 0).

## Gameplay settings (T4.2)

Ported by Haiku workers, one scope each, and validated by the lead: with every setting off the special matrix is unchanged, with
all of them on it runs without errors (6 fighters x NSP, DSP, USP). One setting was also seen acting: Air Dodging shows the new
escape statuses; Hazard Mode = 3 removes the Pirate Land cannon. Per-setting notes: `docs/remix/notes/f8_*.md`.

## Open

Items, camera, display, accessibility and UI settings (see ROADMAP T4.2), the Remix menu drawn in-game (Mode Select Settings
button, L shortcut, T5.3), the Link bomb and boomerang Japanese sounds (the vanilla functions are not mapped), and the Training
and Single Button Mode wiring of Speed (notes in `f8_speed_sbm.md`).
