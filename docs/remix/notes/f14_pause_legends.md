# F14 pause legends (Pause.asm, Camera.asm unpause_camera_)

| Asm (file:line) | Port | State |
|---|---|---|
| Pause.asm:171 show_legend_helper_ (R legend) | if/ifremixpauselegend.c `ifRemixPauseLegendDraw`, text "R: Legend" | Implemented, glyph approximated (the second R-legend texture 0x19F8 is an image, label text is a guess) |
| Pause.asm:195 check_toggle_hud_press_ (Dpad Left) | `ifRemixPauseLegendToggleHudOnDpadLeft`, only while the toggle is OFF | Implemented |
| Pause.asm:235 check_legend_toggle_press_ (R) | `ifRemixPauseLegendToggleOnRPress` | Implemented; the 12CB Gentlemen's Reset skip (Pause.asm:255) is not ported |
| Pause.asm:73 disable_hud_ (groups 0xE to 0x10 hidden when toggle != 0) | `ifRemixPauseLegendHudVisible` | Implemented |
| Pause.asm:370 render_live_music_string_, group 0xF track title and Next/Random rows | none | Skipped: needs the BGM track title lookup (port/remix/remix_music_title.cpp, not exported) and the alternate music |
| Pause.asm:433 set_alternate_music_ | none | Skipped: reads BGM.current_track / BGM.default_track (hardcoded RAM, no port symbol) and the stage alternate tables; stays REMIX-TODO(BGM) in ifcommon.c |
| Camera.asm:844 unpause_camera_ destroys groups 0xF and 0x10 | none needed | Mapped: the labels are put per pause frame and stop with the pause; the state is reset at the next pause (`ifRemixPauseLegendReset`) |
| Camera.asm:886 unpause_camera_ clears the magnifying glass in a Special Zoom | none | Skipped: the zoom type is static in gm/gmremixspecialzoom.c with no accessor; needs `gmRemixSpecialZoomIsActive()` there |

Positions are estimates from the asm Render.draw_string coordinates (the glyph width is not known to the port).
Pause legends draw only in the default pause kind (`ifCommonBattlePauseUpdateInterface`), where the asm camera control flag (0x80131828) is 1. Bonus and PlayerNA pauses show no legend.
Slots 20 to 23 of remix_hud_text are used.
