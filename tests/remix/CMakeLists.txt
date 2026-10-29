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
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtwelvecbrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixmodename.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixmoderules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrcrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrcsignrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premixhrctravelrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/sc/sc1pmode/sc1premix1prules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtimedstockrules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixkothrules.c
)
target_compile_definitions(remix_mode_tests PRIVATE PORT=1 REGION_REMIX=1)
target_include_directories(remix_mode_tests PRIVATE ${CMAKE_SOURCE_DIR}/decomp/src)
set_target_properties(remix_mode_tests PROPERTIES C_STANDARD 11)

add_custom_target(run_remix_mode_tests
    COMMAND remix_mode_tests
    DEPENDS remix_mode_tests
    COMMENT "Running the Remix mode unit tests"
)
