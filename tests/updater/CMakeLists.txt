# Headless unit tests of the self-updater logic shared by the Switch updater: release parsing, .nro install and (with libzip) the PS5 title-zip install.
# No network, no window, no libultraship. One command: cmake --build <build> --target run_updater_tests
find_package(nlohmann_json CONFIG QUIET)
find_package(libzip QUIET)

if (nlohmann_json_FOUND)
    add_executable(updater_tests EXCLUDE_FROM_ALL
        test_updater.cpp
        ${CMAKE_SOURCE_DIR}/port/enhancements/UpdateRelease.cpp
        ${CMAKE_SOURCE_DIR}/port/enhancements/UpdateInstall.cpp
    )
    target_include_directories(updater_tests PRIVATE ${CMAKE_SOURCE_DIR}/port/enhancements)
    target_link_libraries(updater_tests PRIVATE nlohmann_json::nlohmann_json)
    set_target_properties(updater_tests PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    if (libzip_FOUND)
        target_sources(updater_tests PRIVATE ${CMAKE_SOURCE_DIR}/port/enhancements/UpdateZipInstall.cpp
            test_zip_install.cpp)
        target_link_libraries(updater_tests PRIVATE libzip::zip)
        target_compile_definitions(updater_tests PRIVATE UPDATER_TESTS_HAVE_ZIP)
    endif()

    add_custom_target(run_updater_tests
        COMMAND updater_tests
        DEPENDS updater_tests
        COMMENT "Running the self-updater unit tests"
    )
endif()
