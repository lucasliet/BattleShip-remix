# Headless unit tests of the Remix mode rules: pure C, no ROM, no window, no libultraship.
# One command: cmake --build <build> --target run_remix_mode_tests
add_executable(remix_mode_tests EXCLUDE_FROM_ALL
    test_main.c
    test_twelvecb_rules.c
    test_1p_mode_names.c
    test_1p_mode_rules.c
    test_1p_hrc_rules.c
    test_1p_hrc_signs.c
    test_1p_hrc_travel.c
    test_1p_remix_rules.c
    test_timedstock_rules.c
    test_koth_rules.c
    test_tugofwar_rules.c
    test_tagteam_rules.c
    test_smashketball_rules.c
    test_stamina_rules.c
    test_ai_tables_rules.c
    test_ai_commands_rules.c
    test_ai_prevent_rules.c
    test_ai_combat_rules.c
    test_ai_cloud_rules.c
    test_ai_weight_rules.c
    test_ai_cpulevel_rules.c
    test_ai_level_rules.c
    test_ai_longrange_rules.c
    test_ai_lv10_rules.c
    test_ai_movement_rules.c
    test_ai_prediction_rules.c
    test_ai_roll_rules.c
    test_ai_specials_rules.c
    test_ai_teaching_rules.c
    test_stopwatch_bg_rules.c
    test_training_rules.c
    test_ai_di_rules.c
    test_blueshell_rules.c
    test_ai_leftover_rules.c
    test_vsstats_rules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtwelvecbrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixmodename.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixmoderules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrcrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrcsignrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrctravelrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premix1prules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtimedstockrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixkothrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtugofwarrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtagteamrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixsmashketballrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixstaminarules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaitablesrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaicommandsrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaipreventrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaicombatrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixairecoveryrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaicloudrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaiweightrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/mn/mnplayers/mnremixcpulevelrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixailevelrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixailongrangerules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixailv10rules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaimovementrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaipredictionrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixairollrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaispecialsrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/it/itremix/itremixstopwatchbgrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1ptrainingrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaidirules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/it/itremix/itremixblueshellrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaipostsheikrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/mn/mnvsmode/mnremixvsstatsrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/ft/ftremix/ftremixaiteachingrules.c
)
target_compile_definitions(remix_mode_tests PRIVATE PORT=1 REGION_REMIX=1)
target_include_directories(remix_mode_tests PRIVATE ${CMAKE_SOURCE_DIR}/decomp/src)
set_target_properties(remix_mode_tests PROPERTIES C_STANDARD 11)

add_custom_target(run_remix_mode_tests
    COMMAND remix_mode_tests
    DEPENDS remix_mode_tests
    COMMENT "Running the Remix mode unit tests"
)
