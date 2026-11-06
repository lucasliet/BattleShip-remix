# Headless unit tests of the self-updater logic shared by the Switch updater: release parsing and .nro install.
# No network, no window, no libultraship. One command: cmake --build <build> --target run_updater_tests
find_package(nlohmann_json CONFIG QUIET)

if (nlohmann_json_FOUND)
    add_executable(updater_tests EXCLUDE_FROM_ALL
        test_updater.cpp
        ${CMAKE_SOURCE_DIR}/port/enhancements/UpdateRelease.cpp
        ${CMAKE_SOURCE_DIR}/port/enhancements/UpdateInstall.cpp
    )
    target_include_directories(updater_tests PRIVATE ${CMAKE_SOURCE_DIR}/port/enhancements)
    target_link_libraries(updater_tests PRIVATE nlohmann_json::nlohmann_json)
    set_target_properties(updater_tests PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

    add_custom_target(run_updater_tests
        COMMAND updater_tests
        DEPENDS updater_tests
        COMMENT "Running the self-updater unit tests"
    )
endif()
