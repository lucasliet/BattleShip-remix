# Remix HUD text off its panel in widescreen (2026-11-13)

## Symptom

With widescreen on (the desktop and PS5 default) the text of the Remix CSS panel menu (Shield, Player Tag, Visibility and
their values, the page number) was drawn larger than the card and shifted toward the screen centre: on the 1P card it ran
into the 2P card, on the 4P card into the 3P card, while the 2P and 3P cards looked almost right. With widescreen off the
text sat inside the card.

## Root cause

`remix_hud_text` draws through ImGui in N64 units and maps them to window pixels in `GameRect`
(`port/remix/remix_hud_text.cpp`). `GameRect` always used the largest 4:3 rectangle centred in the window. That is where
the game puts its 2D frame with widescreen off, or on with the Center4_3 2D policy (`gEnhancements.Widescreen2DPolicy=1`).
With widescreen on and the default Stretch policy (0) the interpreter spreads the 320-wide 2D frame over the whole window,
so the text kept the 4:3 scale and position while the cards stretched: the error grows with the distance from the centre.

## Fix

`GameRect` maps the N64 frame onto the whole work area when the picture really covers it, else onto the centred 4:3
rectangle. Two things decide it, and each is read where it takes effect:

- the frame of the picture: `port.cpp` latches it at boot from the widescreen CVar (`gAdvancedResolution.Enabled` on for
  the 4:3 pillarbox, `gLowResMode` 1 for the stretched-4:3 Low Resolution Mode), so a widescreen toggle changes it only
  after a restart. `GameImageFillsWorkArea` reads those latched CVars, not the widescreen CVar;
- the 2D policy inside a full-window picture: `port_widescreen_tick` hands it to the interpreter at once, and
  `port_widescreen_stretches_2d()` (`port/widescreen/widescreen.cpp`) reads the interpreter's live anchor mode.

A first version read the widescreen CVar alone: toggling widescreen in the menu moved the text at once while the picture
kept its boot frame until the restart. Every `remix_hud_text` label, bar and rect follows the mapping, not only the panel
menu.

## Verification

`css.sh` with `inputs/css_panelmenu_visibility.txt` and `SSB64_REMIX_TOGGLES="CSS Panel Menu=1"`, shot at frame 300, with
`gEnhancements` unset (widescreen on by default), `Widescreen=0`, `Widescreen=1`, and `Widescreen=1` with
`Widescreen2DPolicy=1`: the labels, values and page number sit inside the 1P card in all four (Linux, and the first three
on macOS in a 16:9 window). Toggling `gEnhancements.Widescreen` mid-session (under gdb, at the 150th CSS frame), from on to
off and from off to on in a 1280x720 window: the text stays in the card until the restart. In a VS match with widescreen
on, the custom player tag of P4 (`SSB64_REMIX_PANEL="PlayerTag.4=2"`, `SSB64_REMIX_TAG_NAMES`) sits over its indicator
near the right edge, and the music title and the pause legend stay in place.
