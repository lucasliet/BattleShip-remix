# Headless unit tests of the Remix mode rules: pure C, no ROM, no window, no libultraship.
# One command: cmake --build <build> --target run_remix_mode_tests
add_executable(remix_mode_tests EXCLUDE_FROM_ALL
    test_main.c
    test_twelvecb_rules.c
    ${CMAKE_SOURCE_DIR}/decomp/src/gm/gmremixtwelvecbrules.c
)
target_compile_definitions(remix_mode_tests PRIVATE PORT=1 REGION_REMIX=1)
target_include_directories(remix_mode_tests PRIVATE ${CMAKE_SOURCE_DIR}/decomp/src)
set_target_properties(remix_mode_tests PROPERTIES C_STANDARD 11)

add_custom_target(run_remix_mode_tests
    COMMAND remix_mode_tests
    DEPENDS remix_mode_tests
    COMMENT "Running the Remix mode unit tests"
)
