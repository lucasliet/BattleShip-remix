# Smash Remix CSS player panel

Covers the per-port option panel of the Remix character select: 20 options in the Remix (`CharacterSelectDebugMenu.asm`, one
`css/<Item>.asm` per option, match hooks in `src/<Item>.asm`) and 21 in the port (one extra, the Manual stock count). Port code:
registry `port/remix/remix_player_panel.{h,cpp}`, match reader `decomp/src/gm/gmremixpanel.{c,h}`, option modules under
`decomp/src/ft/ftremix/` (plus `gm/`, `if/`, `ef/`), on-screen menu `decomp/src/mn/mnplayers/mnremixcsspanelmenu*.c`. Tests:
`tests/remix/test_css_panel_menu_rules.c`, `test_dpad_control_rules.c`, `test_dpad_map_rules.c`, `test_practice_rules.c`,
`test_stockmode_rules.c` and the recipes of section 5.

Open gaps: `docs/remix/patch_audit.md`, groups g2 (panel items on the CSS) and g5 (gameplay hooks).

---

## 1. Remix: the panel

- A gold button (`CSS_IMAGES` 0xA06 offset 0x1208, 16x16) sits at panel x + 4, y 130 of each CSS panel. It works only when the `CSS Panel
  Menu` setting is on (`Toggles.asm:2381`; on in Community, Netplay and Japanese, off in Tournament) and the panel doors are fully open.
  A press toggles the menu (FGM 163); `set_panel_display_` (`CharacterSelectDebugMenu.asm:317`) hides the fighter model, the series
  logo and name, the spotlight and the HMN/CPU/NA, handicap and team buttons while it is open. The reset button (offset 0x1828, panel
  x + 20) sets every item of the mode's list to its default.
- Layout (`setup_menu_page_` 554, `render_menu_page_` 644): 3 items per page, row height 24; label at (x + 4, y + 18) (colour
  `D0D0D0FF`, scale 0.875, max width W - 8); value centred at (x + W/2, y + 30) (white, max width W - 24); item arrows (`ArrowL` 0xECE8 /
  `ArrowR` 0xEDC8 of file 0x11, scale 0.625, tint `FF0000FF`, palette `303030FF`) at x + 2 and x + W - 5, y + 32 + 24 row; page arrows at
  x + W - 20 and x + W - 5, y + 6; page number centred at x + W - 10.5, y + 4, shown only with more than one page. W is the width of the
  panel image.
- Presses (`press_a_handler_` 1272), in order: panel button, reset button, item arrows, page arrows, each tested with the cursor point
  `(+20, +3)`. A held A repeats an arrow every 6th frame after 15 frames and every frame from 90 (`hold_a_handler_` 1432). A value wraps
  over min..max; a blank Player Tag name is skipped.
- Items are numbered in `add_menu_item` order (`:2720-2739`), the order of the port's `RemixPanelOption`: Shield, Player Tag, Visibility,
  Skeleton, Model, Input HUD, Size, Stock, Knockback, Delay, Handicap, Start With, Taunt Itm., Taunt Btn., Kirby Hat, Dpad map, Dpad ctrl,
  Initial Damage, Poison Dmg, Practice.
- Each item's `APPLIES_TO` bitmask `[vs][1p][training][bonus1][bonus2][allstar]` and `APPLIES_TO_HUMAN_CPU` choose the list of a CSS: all
  six modes for every item except Stock (VS only), Handicap (VS and Training, CPU only), Initial Damage (110110: VS, 1P, Bonus 1 and 2),
  Poison Dmg (110111: those plus All-Star) and Practice (1P only, human only); Taunt Btn., Dpad map and Dpad ctrl are human only. Result:
  VS human 18 items, VS CPU 15, 1P 18, Training human 15 and CPU 12, Bonus 17, All-Star 16.
- Before the 1P and Bonus modes every option clears the CPU ports (`clear_settings_for_1p_`). All values live in per-port tables read by
  the match hooks; the menu applies nothing itself.

---

## 2. Port: registry, reader and menu

- **Registry**: one `RemixPanelOption` per option with min, max, default and the `css/X.asm` value strings (`remix_panel_value_name`). A
  value is the CVar `gRemix.Panel.<Key>.P<1..4>` edited in the port menu (Remix > Player Panel). `SSB64_REMIX_PANEL="Size.2=1;Damage.1=50"`
  overrides values for a run (key case-insensitive, value clamped). The 20 custom tag names are `gRemix.Panel.TagName.<1..20>` (Remix >
  Player Tags) or `SSB64_REMIX_TAG_NAMES="2=afk"`.
- **Reader**: `gmRemixPanel(NAME, player)`. In the Bonus and 1P Game modes only the human port keeps its value, like the asm clearing of
  the CPU ports. `ftremixpanel.c` holds the creation-time reader of Initial Damage.
- **Menu** (`mnremixcsspanelmenu.c` sprites and layout, `...text.c` labels, `...input.c` presses, `...rules.c` pure rules):

| Asm | Port |
|---|---|
| `init_debug_menu_` 27, `update_debug_menu_` 79 (button 0x1208, reset 0x1828) | `MakeSprites`, `PlaceButtons` through `ifRemixSpriteMakeFromFile` (links 22 and 28, like the panel card) |
| `setup_menu_page_`, `render_menu_page_` (texts) | `remix_hud_text_put_fit`, slots 56..83 (7 per panel) |
| `create_item_arrows_` 929, `arrow_state_routine_` 1040, `render_pagination_` 1073 | `PlaceItemArrows`, `PlacePageArrows`; one blink timer per panel (0..19, shown below 11) |
| `press_a_handler_`, `handle_item_arrow_press_` 1761, `handle_reset_button_press_` 1621 | `mnRemixPanelMenuPress`, `mnRemixPanelMenuStepItem`, `mnRemixPanelMenuResetPanel` (union of the human and CPU lists); each CSS calls it in its A-tap chain after the checks the asm also runs first |
| `hold_a_handler_` 1432 | `mnRemixPanelMenuRulesHoldAdvance`, `mnRemixPanelMenuHoldPress` (sound at most every 3 frames) |
| `add_menu_item` 2477, `write_menu_items` 2575, `APPLIES_TO` | `mnremixcsspanelmenurules.c` (the human/CPU split only on VS and Training) |
| `set_panel_display_`, `prevent_player_model_display_` 2135, `prevent_series_logo_and_name_display_` 2190, `prevent_white_circle_display_` 2224, `disable_panel_buttons_` 2082 | each CSS hands its GObjs to `mnRemixPanelMenuSetHideTargets` every frame; the menu controller (GObj link 32, the last, so it runs after the CSS processes) forces them hidden while open and restores the flags at close; the spotlight proc, the HMN/CPU/NA, Team and handicap hit tests, the variant D-pad and the 12CB stock indicator ask `mnRemixPanelMenuIsOpen` |
| gate `Toggles.entry_css_panel_menu` | `gmRemixToggle(css_panel_menu)`: off hides the button and ignores presses |
| `css/StockMode.asm`, `css/Damage.asm` onchange | `mnRemixPanelMenuApplyChangeRules`: outside the default, 12CB, Tug of War and Tag Team modes Stock stays Default (VS); in Stamina above the total HP a right press goes to 0, a left press to the HP |
| `check_high_scores_enabled` 2366 | `mnRemixPanelMenuAreHighScoresEnabled` (`mnremixcsshighscores.c`), refreshed every frame; draws the "high scores disabled" text at (192, 217) |

Per CSS: `mnplayersvs.c` (4 panels, enabled when `door_offset` is 0, none for panels 3 and 4 in the 12CB), `mnplayers1ptraining.c`
(human and CPU panel), `mnplayers1pgame.c` (1P, All-Star when `sc1PRemixModeIs(SC1PREMIX_MODE_ALLSTAR)`), `mnplayers1pbonus.c`. All four
load `REMIX_CSS_IMAGES_FILE_ID` for the button sprites.

---

## 3. Options

Values are from `css/X.asm`; module files are under `decomp/src/ft/ftremix/` unless noted.

| Option | Values | Asm | Port |
|---|---|---|---|
| Shield | 0 Default, 1..15 Red, Orange, Yellow, Lime, Green, Turquoise, Cyan, Azure, Blue, Purple, Magenta, Pink, Brown, Black, White; 16 Vanilla; 17 Costume | `Shield.asm color_fix_` (0x801010E8), `alter_yoshi_egg_color_` | `ftremixshield.c`, `ftremixshieldcostume.c`, `ftremixyoshiegg.c`; hook in `ef/efmanager.c` |
| Player Tag | 0 Default, 1..20 | `PlayerTag.asm` | `if/ifremixplayertag.c` |
| Visibility | 0 Default, 1 Cloaked, 2 None, 3 Dark | `CharEnvColor.asm` | `ftremixvisibility.c`, `ftremixenvcolor.c` |
| Skeleton | 0 Disabled, 1 Enabled | `Skeleton.asm` (0x800F24B0, 0x800F2228) | `ftremixskeleton.c` |
| Model | 0 Default, 1 Hi Poly, 2 Lo Poly | `Costumes.asm force_hi_lo_poly_` | `ftremixhitboxdisplay.c` |
| Input HUD | 0 Disabled, 1 Top, 2 Bottom | `InputDisplay.asm` | `ftremixinputdisplay.c`, `port/remix/remix_input_display.cpp` |
| Size | 0 Default, 1 Giant, 2 Tiny | `Size.asm` | `ftremixsize.c` |
| Stock | 0 Default, 1 Last, 2 Manual | `StockMode.asm` | `gm/gmremixstockmode.c` |
| Knockback | 0 Default, 1 Random | `Knockback.asm` | `ftremixknockback.c` |
| Delay | 0 None, 1..12 frames | `InputDelay.asm` (0x4BF8) | `ftremixinputdelay.c` |
| Handicap | 0 Default, 1..9, 10..39 (five levels each of Yoshi, Kirby, Polygon, G. DK, M. Mario, MH), 40 Samus | `Handicap.asm` (0x800D816C) | `ftremixhandicap.c` |
| Start With / Taunt Itm. | 0 None, 1..18 item, 19 Random | `Item.asm` | `ftremixstartitem.c` |
| Taunt Btn. | 0 L, 1..4 C-Up/Down/Left/Right, 5..8 D-Up/Down/Left/Right | `Joypad.asm` | `ftremixtauntbutton.c` |
| Kirby Hat | 0 None, 1..40 fighter, 41 ???, 42 Random | `KirbyHats.asm` | `ftremixkirbyhats.c` |
| Dpad map | 0 Off, 1 Smash, 2 Tilt, 3 Special | `DpadFunctions.asm` | `ftremixdpadmap.c`, `...rules.c` |
| Dpad ctrl | 0 Off, 1 Stick Swap, 2 Stickless, 3 Stick Swap J, 4 Stickless J | `DpadControl.asm` | `ftremixdpad.c`, `ftremixdpadcontrolrules.c` |
| Initial Damage | 0..999 | `Damage.asm` (0x800D7A14) | `ftremixpanel.c` |
| Poison Dmg | 0 None, 1 Low (+1%), 2 Medium (+2%), 3 High (+4%), 4 Heal (+2 HP) | `PoisonDmg.asm` | `ftremixpoison.c` |
| Practice | 0 OFF, 1..10 Stage N, 11 Final Stage | `Practice_1P.asm` | `gm/gmremixpractice.c` |
| Stock Count (port only) | 0..99, default 3 | `stock_count_table` | `gm/gmremixstockmode.c` |

Fighter names of Kirby Hat 1..40 follow the CSS order (`kFighterNames` in the registry); the Handicap labels are `kHandicapNames`.

**Shield.** Option 0 follows `_teams_check`: the team colour in a VS team battle (red, blue, green, yellow), black for a CPU, otherwise the port
colour (`table_ffa`: red, blue, yellow, green). 1..15 read `table_custom` (literal RGB list, `Shield.asm:219-233`); 16 keeps the vanilla
colour; 17 reads `Character.costume_shield_color`, one index per costume (Classic Sonic adds 6; an index past a row reads the next row, which
`ftremixshieldcostume.c` reproduces by laying the rows out as one block). The shield damage flash always keeps the vanilla colour. Yoshi's egg
shield and roll egg get a model part image with a 16-entry palette array; `ftremixyoshiegg.c` attaches the MObj on the first draw
(`efManagerYoshiShieldProcDisplay`) and sets the palette index every frame. The thrown egg (0x8015EAF4), egg-lay egg (0x8014D080, 0x8014C984)
and entry egg (0x80102F5C) are not wired; `Character.yoshi_egg_color.table` is never filled, so the index is 0 except for Costume.

**Player Tag.** Every value >= 1 swaps the in-match tag for an arrow-only sprite (file 0x26 offset 0x1158, absent from the vanilla 0xEB8-long
table), draws the custom name (`Toggles.entry_player_tags_{N-1} + 0x28`) and skips the `playertag_wait` countdown. Port:
`ifRemixPlayerTagSpriteOffset`, `ifRemixPlayerTagAlwaysShown`, and `ifRemixPlayerTagDrawName` (name through `remix_hud_text` at the sprite
centre every frame, one layer, no black outline copy). Tag number 2 named `afk` also drives the credits speed assist
(`docs/remix/menus.md`) and the 1P away mode (`sc1PRemixAfk*`).

**Visibility.** `initialize_override_table_` (0x800D7CBC) sets the env colour override of the port; `update_player_indicator_alpha_`
(0x80111B3C) gives the tag the override alpha. None is `0xFFFFFF01` (the asm comment says `...00`, the instruction is `addiu t9, r0,
-0x00FF`), Dark `0x000000FF`; Cloaked maps to the Cloaking Device state (override `0xFFFFFF10`), as the asm's fixed value does (GP-5). How to Play and Auto-Demo clear the override. Port: `ftRemixVisibilityInitFighter` (after
`ftRemixSizeInitFighter`) and `ftRemixVisibilityPlayerTagSetAlpha`; the override beats the moveset overrides (Sheik, Mewtwo, Super Sonic), as
in the asm; the existing `ftRemixEnvColorGetOverride` hook in `ftDisplayMain` draws it. The asm uses a translucent render mode below alpha
0xFF; the port only sets the env colour and sky fog alpha.

**Skeleton.** The hooks replace the colanim word at FTStruct 0x0A88 with `0x08000000`, whose bits 3-4 read `colanim.skeleton_id = 1`; nothing is
stored back. Port: `ftRemixSkeletonDisplayId(fp)` returns 1 when the value is on, read by the `FTDISPLAYMAIN_SKELETON_ID(fp)` macro at the three
sites of `ftDisplayMainDrawAll` / `ftDisplayMainDrawSkeleton`; a fighter without skeleton entry 1 keeps its normal draw.

**Model.** The `Model Display` setting is checked first and a non-zero per-port value overrides it (hooks 0x800D8050 and 0x800E91C0, the VS
pause swap). `ftRemixModelDisplayDetail(detail, player)` is called by `ftManagerMakeFighter` and `ftParamSetModelPartDetailAll`; How to Play,
Demo and intro scenes are ignored.

**Input HUD.** The Remix draws an N64 controller from sprites; the port draws an ImGui overlay (`RemixInputDisplayWindow`, a `GuiWindow` on
the foreground list). `ftRemixInputDisplayUpdate` (`ftMainProcUpdateInterrupt`) sends the sample each fighter reads this frame (after Input
Delay and the D-pad transform; CPU samples too) when the port value is not 0. The overlay anchors each port to the top or bottom of the
window (side by side, centred), tints P1 red, P2 blue, P3 yellow, P4 green and lights a shape while its button is held (stick dot clamped
to 80, D-pad arms, A, B, Start, C buttons, L, R, Z). `ftRemixInputDisplayClear` runs in `scManagerRunLoop` at every scene change. R is shown
as R, not as A+Z; the match timer is not moved (`adjust_timer_position_`).

**Size.** `state_table` is copied to `match_state_table`; Giant and Tiny use `multiplier_table`. `ftRemixSizeInitFighter` takes the panel
value (or the state a mode forces, such as the Remix 1P giant and tiny stages) and sets the multiplier (Giant 2.25, Tiny 0.5) except on How
to Play and Auto-Demo; it scales the top joint, ECB, ledge box, camera zoom and offset and the weight (`ftRemixSizeWeight`).

**Stock and Stock Count.** `set_initial_stock_count_`: Default uses the match stocks; Last uses the count the port ended the previous match
with (`previous_stock_count_table`, written at every KO by `update_stocks_remaining_`, at least `min(prev, rules)`); Manual uses
`stock_count_table[port]` when below the rules value (unset -1 means the rules value). The CSS counter is 0-based and the HUD shows
`count + 1`. Tag Team, Tug of War, 12CB, King of the Hill and Smashketball also use the tables. Port: `gmRemixStockModeBeginMatch(player,
match_stocks)` from the `scvsbattle.c` fighter loop stores `initial_stock_count`; Last is `sStockModeLastCount` (reset on a CSS reset and by
the salty runback). `StockCount` is a port-only option edited in the port menu in place of the Manual counter the Remix draws under the panel;
the CSS draws a counter only for the 12CB Manual stocks (`mnremixtwelvecbstockmanual.c`).

**Knockback.** At `ftManagerMakeFighter` each port rolls one angle per action id (0x200) when the scene changed (`randomize_angles_`);
`get_angle_hitbox_` (`ftMainParseMotionEvent`) and `get_angle_throw_` (`ftCommonThrownReleaseThrownUpdateStats`) read it. Port
`ftRemixKnockbackRandomizeAngles` / `ftRemixKnockbackAngle`: 60% a good angle (45-135), 15% a semi-spike or spike (180-360), 15% random
(0-360), 10% the Sakurai angle (361). Re-entering the same scene (Training reset, salty runback) keeps the angles.

**Delay.** The asm writes samples into a per-port buffer at hook 0x4BF8 and reads the one `delay` frames back. Port: a ring of 13
`SYController` per port (`ftRemixInputDelayController`, `ftMainProcUpdateInterrupt` `case nFTPlayerKindMan`, before the D-pad transform as the
asm order), reset in `ftManagerInitFighter`; the ring advances once per call, so the delay counts frames with control enabled. Human fighters.

**Handicap.** For a CPU (`pkind == Com`) with a value other than 0 the panel value is written into the handicap byte of the fighter description
(`sb t9, 0x12(v0)`); `desc->level` is untouched. Values 10..39 are per-character levels that the Remix AI reads from `handicap.table`; the label
names the character, the value alone does not select it. Port: `ftRemixHandicapGetFighterValue` sets `fp->handicap` in `ftManagerInitFighter`.
The byte also indexes the vanilla knockback handicap table (`ftParamGetHandicapIndex`) as in the asm; the Remix AI table is not read. The VS CSS
handicap arrows of a human stop at 9 (`mnRemixHandicapCanRaise` has the 40 rule, `mnRemixCssMakeHandicapIcon` draws the 10..40 icons).

**Start With and Taunt Item.** Table (index 1..18): Beam Sword, Home-Run Bat, Fan, Star Rod, Ray Gun, Fire Flower, Hammer, Motion-Sensor Bomb,
Bob-omb, Bumper, Green Shell, Red Shell, Poke Ball, Spiny Shell, Deku Nut, Pitfall, Golden Gun, Mr. Saturn. Port (`ftremixstartitem.c`): kinds are
the vanilla enum for 0x07..0x13 and Remix ids 0x30, 0x32, 0x34, 0x35, 0x38 (Pitfall is 0x34; the asm comment says 0x33); Random rolls 0..18 and a
roll of 0 becomes 1, as the asm `beql`; the item spawns at the top joint with velocity (0, 30, 0) and `ITEM_FLAG_PARENT_GROUND` without the spawn
swirl, is held with `itMainSetFighterHold`, and is skipped when the fighter already holds an item; the Hammer sets `hammer_tics = 0x2D0` and
`ftHammerSetStatusHammerWait`. Hooks: `ifCommonAnnounceGoSetStatus` (GO), `ftCommonRebirthStandSetStatus` (respawn), `ftCommonAppealSetStatus`
(taunt, after the status change; the asm hook is before `flag1 = 0`). The hammer midi 0x2D is not played. Human ports only.

**Taunt Btn.** `taunt_mask_per_port` / `mask_table` (`Joypad.asm`). `prevent_taunt_jump_` denies a C-button jump only when the pressed C buttons are
exactly the taunt button; Single Button Mode keeps the vanilla mask. Port: `ftRemixTauntButtonMaskFor` where `ftManagerMakeFighter` sets
`fp->input.button_mask_l` (the single source of the button), `ftRemixTauntButtonAllowsJump` in `ftCommonKneeBendCheckButtonTap` (0x8013F450) and
`ftCommonJumpAerialMultiCheckJumpButtonHold` (0x8014012C). With a custom button the shield answers L and Z (`ftRemixDpadControlSetButtonMasks`).

**Kirby Hat.** The value maps to a fighter whose copy hat Kirby wears at spawn; 41 is the magic hat (the None row, hat 0x25), 42 Random. Port
(`ftRemixKirbyHatSpawnInit` in `ftremixkirbyhats.c`, called from `ftManagerInitFighter`): for Kirby and J Kirby the panel fighter replaces the
spawn power, as `kirby_hat_select_` does, and the hat comes from the copy table (Polygon Kirby keeps its own, a deviation); Random rolls once and
shows plain Kirby on the menus, CSS, results and 1P game over. `ftRemixKirbyPowerInitFighter` sets the magic-hat flag (spawn power NONE or panel
41, which also clears the stored hat charges, `ftRemixKirbyMagicHatClearCharge`) and the taunt power loss (panel None only); the magic hat rolls a
random power per neutral special and restores itself (`ftremixmagichat.c`).

**Dpad ctrl.** `ftRemixDpadControlController` (after the input delay, as the asm order 0x4BF8 then 0x4C00) returns a synthetic `SYController` per
port. R becomes A+L instead of A+Z in modes 1..4. Stick Swap (1, 3): the stick becomes D-pad bits (dead zone X 40, Y 30) and the physical D-pad
becomes the stick. Stickless (2, 4): the stick is zeroed and the D-pad becomes the stick (D-pad bits kept). The J modes share the transform; the
stick-jump, run-up and dash-attack checks (`check_jump_type_idle_`, `_running_`, `dash_attack_check_`) act only on modes 1 and 2
(`ftRemixDpadControlDisablesStickJump`, `...HandlesRunStickUp`, `...SkipsDashAttackOf`, hooks in `ftcommonkneebend.c`, `ftcommonrun.c`,
`ftcommonattackdash.c`). Known difference: the asm reads A and B after Single Button Mode filtered them, the port filters after the transform.

**Dpad map.** The D-pad becomes Smash (1), Tilt (2) or Special (3) macros. `ftRemixDpadMapApplyPress` runs after the tap mask (hook 0x800E1330:
swaps the tap, may write the stick, puts A in the hold) and `ftRemixDpadMapHeldButtons` before the hold is stored (0x800E1378: Special holds B,
Smash holds A with Charge Smashes on). The stick is half range (+-40) on a single tap and full (+-80) on a double tap or a held D-pad
(`dpad_move_timer`, `FTREMIX_DPAD_TAP_WINDOW`), with fighter-specific vertical rules (Spacies, Pikachu, Mewtwo, Sheik, ninja). Off with
Stickless, with Single Button Mode (press only) and in Training with "D-pad Controls" other than OFF. Macros read the D-pad generated by Stick Swap.

**Initial Damage.** `char_init_` sets the percent of a new fighter, ignoring respawns; in Stamina the value is limited to the HP. Port
`ftRemixPanelGetInitialDamage` (from `ftManagerInitFighter`): only when the description has no damage and the fighter is not a demo fighter; VS,
1P and Bonus take `panel % 1000`, Training takes the menu percent (Reset Sets Percent).

**Poison Dmg.** `apply_damage` (called by `Poison.asm:51`) runs when the low 7 bits of the match tic are zero (about every 2 s; at game speed 1/3 or
2/3 the low bits may be 1 or 2): the percent rises by 1, 2 or 4 (cap 999) or the fighter heals 2. Super Star and the respawn status skip it; the
cloaking device blocks damage but not Heal (`_anti_venom` is taken first). Port: `ftRemixPoisonPanelTryApply` returns TRUE when it healed or hurt,
which skips the hazard poison for that frame; `ftRemixPoisonUpdate` runs before the heal block of `ftMainProcUpdateInterrupt`, so Heal lands on the
same frame; the timer is `gSCManagerBattleState->time_passed`; game speed is read from the toggle directly; the percent is capped at the Stamina HP
and a defeated Stamina fighter is skipped.

**Practice.** `_1p_practice` maps 1..3 to stages 0..2, 4..6 to 4..6, 7..9 to 8..10, 10 to 12 and 11 to 13 (bonus stages 3, 7, 11 skipped); holding L on
GAME SET or STAGE CLEAR (`hold_l_to_retry_`, `hold_l_to_retry_stage_clear_`) or the pause menu L retries the stage. Port:
`gmRemixPracticeStartStage` from `mnPlayers1PGameSetSceneData` reads the registry directly (the player table is not filled yet and the human guard
could zero it); a retry replays the stage with no continue, level drop or stage preview (the trigger sets a flag, ends the scene and
`sc1PManagerUpdateScene` consumes it through `gmRemixPracticeConsumeRetry`).

---

## 4. Deviations

- The CSS model does not change when Model, Kirby Hat, Visibility or Size is pressed (the asm `onchange_handler`s at 0x800E9198, 0x800E8EAC and
  0x800E8ECC reload it); the value takes effect when the match starts.
- The menu button hides with the doors and when the panel is disabled; the asm creates it once. Hiding is by flags: targets are forced hidden each
  frame and the flags seen before the hide are restored, so a target the CSS legitimately hides while the menu is open can reappear at close until
  the CSS sets it again.
- One blink timer per panel instead of one per arrow; the text is the ImGui overlay (above the game), the sprites are game objects.
- Press order: VS puts the menu press before the 12CB press inside the A-tap block; 1P, Training and Bonus after the puck grab, like the asm hook.
- `SinglePlayer.high_score_enabled` does not exist in the port; the Gameplay/Pokemon toggle checks and the enemy-port check of
  `check_high_scores_enabled` (2396-2417) are not in the refreshed value.
- Stock Count is a port option; Input HUD is an overlay with port colours and layout.
- Visibility Cloaked uses the cloaking state; the per-part render-mode lists of `CharEnvColor.custom_display_lists_*` are open (GP-6); Skeleton needs a skeleton entry 1; the Handicap value reaches only the knockback table.

---

## 5. Testing

Overrides: `SSB64_REMIX_PANEL='Size.2=1;Damage.1=50;InputDisplay.1=1;InputDelay.1=3'`, `SSB64_REMIX_TAG_NAMES='2=afk'`. Match checks run with
`SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,8` through `.claude/skills/remix-smoke-tests/scripts/vs.sh`: Start With 1 holds a Beam Sword at GO, 19
gives an item from 1..18, Taunt Itm. 9 spawns a Bob-omb on a ground taunt, a respawn re-gives Start With; Taunt Btn. 1 (C-Up) taunts without a
jump; Dpad ctrl 1 makes the stick right a D-pad right and the D-pad move, 2 ignores the stick; Delay 6 lags a human fighter only; Poison Dmg 1..3
adds percent about every 2 s and 4 heals; Stock Manual with `StockCount.2=1` shows 2 stocks for P2; Shield 1..15, 16, 17 and Default colours;
`SSB64_START_SCENE=52` with Practice 4 starts on stage 4, 11 on the Boss; Input HUD at the top for P1 and the bottom for P2, nothing left after
the match.

Panel menu on the VS CSS: `SSB64_START_SCENE=16` with `css.sh <input> <name> [frames]`. The card of VS panel `i` is at x = 22 + 69 i, y = 127
(the log line `css panel menu: port N open, card x= y= width=` prints the real values). The cursor hot spot is the cursor sprite position + (20, 3);
hot boxes: panel button x in [x+4, x+20], y in [130, 146]; reset [x+20, x+36] same y; left arrow of row r x in [x+2, x+23], y in
[159+24r, 172+24r]; right arrow x in [x+W-13, x+W+8]; page arrows y in [133, 146], left x in [x+W-20, x+W-4], right [x+W-9, x+W+8]. Port 1
(P2) on panel index 1 (x = 91): `60 68 1 - -80 80` moves the cursor to about (77, 141); `70 71 1 A` opens the menu (rows Shield, Player Tag,
Visibility, page 1); `76 81 1 - 0 -80` then `82 83 1 A` steps Shield 0 to 17 (`Costume`); a long A (`90 160 1 A`) repeats about every 6 frames from
frame 105 (log `css panel menu: port 1 item 0 = N`); the right page arrow shows Skeleton, Model, Input HUD and page 2; reset prints `reset`; the
button closes the menu. With `Size.2=1` the Size row of P2 reads `Giant` on page 3.
