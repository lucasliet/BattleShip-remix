#include "remix_toggle_tips.h"
#include "remix_symbols.h"

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace {

struct ToggleTip {
    uint32_t entryAddr;
    const char *text;
};

constexpr ToggleTip kRemixPageTips[] = {
    { REMIX_SYM_Toggles__entry_skip_results_screen,
      "Skips the results screen after a match and goes straight to the character select. ON RESET only does it when the match "
      "ended with the reset combo (A+B+R+Z). Hold R or L to see the results anyway." },
    { REMIX_SYM_Toggles__entry_hold_to_pause,
      "Pausing needs START held for about half a second, so a quick tap no longer pauses by accident." },
    { REMIX_SYM_Toggles__entry_css_panel_menu,
      "Shows a panel button for each player on the character select screen, which opens extra per-player options." },
    { REMIX_SYM_Toggles__entry_practice_overlay,
      "Tints each fighter with a color for the state it is in (landing, hard landing, turning and others). A practice aid." },
    { REMIX_SYM_Toggles__entry_cinematic_entry,
      "Controls the camera focus during the countdown before a match. ALWAYS forces the cinematic focus, NEVER turns it off and "
      "DEFAULT leaves it random." },
    { REMIX_SYM_Toggles__entry_flash_on_z_cancel,
      "Makes the fighter flash when an aerial attack lands inside the Z-cancel window." },
    { REMIX_SYM_Toggles__entry_fps,
      "Shows a frame rate counter on screen. OVERCLOCKED measures against the overclocked frame time." },
    { REMIX_SYM_Toggles__entry_model_display,
      "Forces every fighter to use its high-polygon or low-polygon model. DEFAULT leaves each fighter as it is." },
    { REMIX_SYM_Toggles__entry_special_model,
      "Shows hitboxes and hurtboxes for practice. HITBOX replaces the fighter models, HITBOX+ keeps them with outlined boxes and "
      "ECB shows the fighter's collision shape." },
    { REMIX_SYM_Toggles__entry_advanced_hurtbox,
      "While hurtboxes are shown, colors them by state: cyan when reflecting or absorbing, grey during armor and dark cyan when "
      "the fighter cannot be grabbed." },
    { REMIX_SYM_Toggles__entry_hold_to_exit_training,
      "Leaving Training needs A held down, so you cannot exit the mode by accident." },
    { REMIX_SYM_Toggles__entry_improved_combo_meter,
      "Keeps the combo count going while the opponent is grabbed, thrown, stunned or in knockback, instead of ending it early." },
    { REMIX_SYM_Toggles__entry_tech_chase_combo_meter,
      "Also keeps the combo count going through techs and down bounces, so tech chases count as part of the combo." },
    { REMIX_SYM_Toggles__entry_combo_meter,
      "Shows a meter over each fighter with the hit count and damage of the current combo." },
    { REMIX_SYM_Toggles__entry_1v1_combo_meter_swap,
      "In 1v1 matches, puts each combo meter on the other player's side of the screen." },
    { REMIX_SYM_Toggles__entry_neutral_spawns,
      "Starts VS matches from fair, symmetric spawn positions instead of the stage's usual ones." },
    { REMIX_SYM_Toggles__entry_salty_runback,
      "Holding the chosen button combination at the end of a match restarts it at once on the same stage, with no results screen. "
      "Hold R or L to see the results instead." },
    { REMIX_SYM_Toggles__entry_variant_random,
      "Chooses which fighters Random Select can pick. INCLUDE BONUS adds the bonus fighters, ALL VARIANTS sometimes picks a "
      "character's variant and VANILLA ONLY picks only the original fighters." },
    { REMIX_SYM_Toggles__entry_disable_hud,
      "Hides parts of the on-screen interface. PAUSE hides the pause screen legends; ALL also hides the match HUD." },
    { REMIX_SYM_Toggles__entry_disable_aa,
      "Turns off anti-aliasing (edge smoothing) for a sharper, more pixelated image." },
    { REMIX_SYM_Toggles__entry_full_results,
      "Stock matches show the more detailed results screen that time matches use, with KO counts." },
    { REMIX_SYM_Toggles__entry_skip_start_cheer,
      "Skips the crowd cheer at the start of Training, or of Training and the Bonus/1P modes with the second option." },
    { REMIX_SYM_Toggles__entry_default_cpu_level,
      "Sets the starting CPU level of all four slots the first time you open VS after launching the game. DEFAULT keeps level 3." },
    { REMIX_SYM_Toggles__entry_puff_sing_anim,
      "Keeps the animated size of Jigglypuff's Sing effect. Turn it off to show the effect at a fixed size." },
    { REMIX_SYM_Toggles__entry_l_random_char,
      "Pressing L on the character select screen picks a random character for your token." },
    { REMIX_SYM_Toggles__entry_dpad_css_control,
      "Lets you move the cursor on the character select screen with the D-pad." },
    { REMIX_SYM_Toggles__entry_pk_thunder_reflect_crash_fix,
      "Fixes a crash that could happen when Ness's PK Thunder is reflected." },
    { REMIX_SYM_Toggles__entry_flash_guard,
      "Removes white screen flashes from effects such as the Flashbang, Deku Nut, Lightning and special zooms. Helps players "
      "sensitive to flashing lights." },
    { REMIX_SYM_Toggles__entry_screenshake,
      "Reduces or removes screen shake. LIGHT makes quakes mild and OFF removes them." },
    { REMIX_SYM_Toggles__entry_blastzone_gfx,
      "Changes the explosion shown when a fighter is knocked past the blast zone. OFF removes it and REDUCED makes it smaller." },
};

constexpr ToggleTip kGameplayPageTips[] = {
    { REMIX_SYM_Toggles__entry_hitstun,
      "MELEE uses Melee's knockback scaling, which shortens hitstun to about three quarters of normal. Not used in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_hitlag,
      "Sets how long both fighters freeze when a hit lands. JAPANESE, MELEE and ULTIMATE use the length of that game and NONE "
      "removes hitlag. Not used in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_di,
      "Sets how far directional influence can bend knockback. JAPANESE is weaker than normal and ULTIMATE weaker still. Not used "
      "in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_japanese_sounds,
      "Uses the Japanese version's hit, beam sword and projectile sounds. DEFAULT only for the fighters that had them there, "
      "ALWAYS for everyone and NEVER for no one." },
    { REMIX_SYM_Toggles__entry_j_stun_sleep,
      "Shortens the time needed to mash out of dizziness and sleep, as in the Japanese version. Not used in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_momentum_slide,
      "Allows the Japanese version's momentum slide when braking out of a run. Not used in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_shieldstun,
      "Sets how long a fighter is stuck after blocking a hit. JAPANESE, MELEE, BRAWL and ULTIMATE use that game's formula. Not "
      "used in 1P and Bonus modes." },
    { REMIX_SYM_Toggles__entry_z_cancel_opts,
      "Changes the Z-cancel timing. DISABLED never allows it, MELEE uses a 7 frame window, AUTO always succeeds and GLIDE MODE "
      "keeps you gliding after a missed Z-cancel with no landing lag." },
    { REMIX_SYM_Toggles__entry_punish_on_failed_z_cancel,
      "Punishes a missed Z-cancel with the chosen penalty, such as extra damage, a lava floor, shield break, a KO, bury, egg, "
      "sleep or a trip. RANDOM picks one each time." },
    { REMIX_SYM_Toggles__entry_improved_ai,
      "Makes CPU fighters better at teching, DI, recovering and fighting." },
    { REMIX_SYM_Toggles__entry_tripping,
      "Lets fighters trip while dashing, dash dancing or turning at run speed. LOW, HIGH and BRAWL set how often it happens." },
    { REMIX_SYM_Toggles__entry_rage,
      "Changes the knockback you deal based on your own damage percent. ULTIMATE and SMASH 4 make damaged fighters hit harder, "
      "BERSERK does so strongly and FATIGUE makes them hit weaker." },
    { REMIX_SYM_Toggles__entry_footstool,
      "Jump on another fighter in mid-air to bounce off it and send them tumbling." },
    { REMIX_SYM_Toggles__entry_air_dodge,
      "Adds an air dodge on the shield button in the air. Melee dodges in a direction, Ultimate dodges in place while drifting "
      "and Air Dash gives one dash per jump." },
    { REMIX_SYM_Toggles__entry_jab_lock,
      "Weak hits on a fighter bouncing on the ground keep them down instead of damaging them. The next hit after the bounce ends it." },
    { REMIX_SYM_Toggles__entry_ledge_jump,
      "Tap a C button while hanging on a ledge to climb up and finish with a jump." },
    { REMIX_SYM_Toggles__entry_perfect_shield,
      "Shielding just as a hit lands blocks it with no shield stun and sends projectiles and items back at the attacker." },
    { REMIX_SYM_Toggles__entry_parry,
      "Release the shield within 5 frames of raising it as an attack arrives to parry it: no shield damage and the attacker is "
      "left vulnerable. Beta." },
    { REMIX_SYM_Toggles__entry_spot_dodge,
      "Pull the stick down while holding Z on the ground to dodge in place. On a soft platform it drops through instead." },
    { REMIX_SYM_Toggles__entry_fast_fall_aerials,
      "Lets you fast fall while an aerial attack is still active." },
    { REMIX_SYM_Toggles__entry_ledge_trump,
      "Jumping to a ledge that another fighter is holding knocks them off and takes the ledge." },
    { REMIX_SYM_Toggles__entry_wall_teching,
      "Press Z just before hitting a wall to tech it and avoid bouncing off." },
    { REMIX_SYM_Toggles__entry_charged_smashes,
      "Hold A during a smash attack's charge frame to charge it. ON releases it after 60 frames and UNLIMITED CHARGE never "
      "releases it on its own." },
    { REMIX_SYM_Toggles__entry_item_containers,
      "Changes crates, barrels and capsules. OFF removes them; NEVER EXPLODE and ALWAYS EXPLODE fix what happens when they are "
      "hit or thrown." },
    { REMIX_SYM_Toggles__entry_game_speed,
      "Changes the speed of the whole game, from slow motion (1/8) up to 3x." },
    { REMIX_SYM_Toggles__entry_special_zoom,
      "Zooms the camera in slow motion on KOs and some special moves. MATCH END only on the deciding KO of a two-team match, ANY "
      "KO on every KO. Beta." },
    { REMIX_SYM_Toggles__entry_blastzone_warp,
      "Fighters that leave the blast zone reappear on the opposite side instead of being KO'd. Choose left/right, top/bottom or "
      "all. Beta." },
    { REMIX_SYM_Toggles__entry_single_button_mode,
      "A challenge mode that limits players to one button (A, B or R, optionally with C for jumping) plus START and the D-pad." },
    { REMIX_SYM_Toggles__entry_item_dropping,
      "Press R in the air to drop any held item instead of throwing it." },
    { REMIX_SYM_Toggles__entry_move_staling,
      "Changes how repeating a move affects its damage. DISABLED turns staling off, LENIENT and STRICT soften or harshen it, and "
      "WAIT FOR IT, REVERSE and CHEAP SHOT are unusual variants." },
    { REMIX_SYM_Toggles__entry_stopwatch_behaviour,
      "Forces what the Stopwatch does: always slow or always fast when picked up, or always or never backfire. DEFAULT backfires "
      "10% of the time." },
};

constexpr ToggleTip kMusicPageTips[] = {
    { REMIX_SYM_Toggles__entry_play_music, "Turns the stage music on or off." },
    { REMIX_SYM_Toggles__entry_random_music,
      "Picks each stage's music at random from the tracks enabled below. Holding a C button while the stage loads picks the "
      "alternate song instead." },
    { REMIX_SYM_Toggles__entry_preserve_salty_song,
      "A salty runback replays the previous random song instead of picking a new one." },
    { REMIX_SYM_Toggles__entry_menu_music,
      "Chooses the menu music: a fixed track, a random pick or off. Changing it plays a preview." },
    { REMIX_SYM_Toggles__entry_show_music_title,
      "Shows the song and game name at the start of a match." },
    { REMIX_SYM_Toggles__entry_bgm_volume, "Music volume, from 0 to 10." },
    { REMIX_SYM_Toggles__entry_fgm_volume, "Sound effects volume, from 0 to 10." },
};

constexpr ToggleTip kStagePageTips[] = {
    { REMIX_SYM_Toggles__entry_sss_layout,
      "TOURNAMENT uses the tournament stage select: Random only picks tournament-legal stages and the hazards of the chosen "
      "stage follow tournament rules." },
    { REMIX_SYM_Toggles__entry_hazard_mode,
      "Turns off stage hazards, stage movement or both. Works in VS and Training; press L on the stage select to cycle it." },
    { REMIX_SYM_Toggles__entry_whispy_mode,
      "Changes the wind of Whispy Woods on Dream Land. JAPANESE fades slower, SUPER and HYPER blow stronger." },
    { REMIX_SYM_Toggles__entry_saffron_poke_rate,
      "How often Pokemon appear on Saffron City. SUPER halves the wait, HYPER also opens the door at once and QUICK ATTACK is "
      "almost constant. Works in VS and Training." },
    { REMIX_SYM_Toggles__entry_announcer_mode,
      "Chooses where the Pokemon announcer comments on the match: STADIUM only on the Pokemon stages, ALL STAGES everywhere, or OFF." },
    { REMIX_SYM_Toggles__entry_dragon_king_hud,
      "Chooses when the HUD uses the lower Dragon King layout: DRAGON KING for that style of stage, ALL STAGES always, or OFF never." },
    { REMIX_SYM_Toggles__entry_camera_mode,
      "Changes the match camera. BONUS uses the Mushroom Kingdom camera on stages with the default one, FIXED locks the camera "
      "to a fixed view per stage and SCENE uses the stage's animated camera." },
    { REMIX_SYM_Toggles__entry_yi_clouds,
      "Makes the clouds of Yoshi's Island animate by changing size." },
};

constexpr ToggleTip kPokemonPageTips[] = {
    { REMIX_SYM_Toggles__entry_pokemon_voices,
      "Chooses the voices Pokemon use. JAPANESE uses the Japanese voices and RANDOM picks between the two at random." },
};

constexpr const char *kRandomMusicTrackTip = "Include this track in Random Music.";
constexpr const char *kRandomStageTip = "Include this stage in Random Stage select.";
constexpr const char *kPokemonTip = "Allow this Pokemon to appear.";

/**
 * Looks an entry address up in one page table.
 *
 * @param tips Table of a single page.
 * @param entryAddr Menu.entry address of the setting.
 * @return Tip text, nullptr when the table has no entry for that address.
 */
template <std::size_t Count>
const char *FindTip(const ToggleTip (&tips)[Count], uint32_t entryAddr) {
    const auto *match = std::find_if(std::begin(tips), std::end(tips),
                                     [entryAddr](const ToggleTip &tip) { return tip.entryAddr == entryAddr; });
    return (match == std::end(tips)) ? nullptr : match->text;
}

const char *FindFixedTip(const remix_toggles::Toggle &toggle) {
    switch (toggle.group) {
        case remix_toggles::Remix:
            return FindTip(kRemixPageTips, toggle.entryAddr);
        case remix_toggles::Gameplay:
            return FindTip(kGameplayPageTips, toggle.entryAddr);
        case remix_toggles::Music:
            return FindTip(kMusicPageTips, toggle.entryAddr);
        case remix_toggles::Stage:
            return FindTip(kStagePageTips, toggle.entryAddr);
        case remix_toggles::Pokemon:
            return FindTip(kPokemonPageTips, toggle.entryAddr);
        default:
            return nullptr;
    }
}

/* The fixed settings of the Music, Stage and Pokemon pages have their own tips; every other checkbox there is a list entry. */
const char *GenericListTip(remix_toggles::Group group) {
    switch (group) {
        case remix_toggles::Music:
            return kRandomMusicTrackTip;
        case remix_toggles::Stage:
            return kRandomStageTip;
        case remix_toggles::Pokemon:
            return kPokemonTip;
        default:
            return nullptr;
    }
}

} // namespace

const char *remix_toggle_tip(const remix_toggles::Toggle &toggle) {
    const char *fixedTip = FindFixedTip(toggle);
    if (fixedTip != nullptr) {
        return fixedTip;
    }
    if (!toggle.isBool) {
        return nullptr;
    }
    return GenericListTip(toggle.group);
}
