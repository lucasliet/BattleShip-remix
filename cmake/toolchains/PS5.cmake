# CMake toolchain for the PS5 native title (ps5-payload-sdk + ps5-native-app-boilerplate +
# ps5-opengl), prepared by scripts/ps5/setup-toolchain.sh and scripts/ps5/build-gl-sdk.sh.
#
# Executables link with `ld.lld -r` into one relocatable object; scripts/ps5/native-app-stage.sh
# then links it with the GL SDK and the C++ runtime into eboot.bin.
#   cmake -B build-ps5 -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/PS5.cmake
cmake_minimum_required(VERSION 3.20)

set(PS5_TOOLCHAIN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.toolchain/ps5")
cmake_path(NORMAL_PATH PS5_TOOLCHAIN_ROOT)
set(PS5_PAYLOAD_SDK "${PS5_TOOLCHAIN_ROOT}/sdk")
set(PS5_OPENGL_PREFIX "${PS5_TOOLCHAIN_ROOT}/ps5-opengl-sdk")
set(PS5_SDL2_PREFIX "${PS5_TOOLCHAIN_ROOT}/ps5-sdl2-sdk")
set(PS5_NATIVE_APP_TEMPLATE "${PS5_TOOLCHAIN_ROOT}/src/ps5-native-app-boilerplate")

foreach(required_path IN ITEMS "${PS5_PAYLOAD_SDK}/target/include" "${PS5_OPENGL_PREFIX}/include"
        "${PS5_NATIVE_APP_TEMPLATE}/tooling/prospero-clang18")
    if(NOT EXISTS "${required_path}")
        message(FATAL_ERROR "PS5 toolchain file missing: ${required_path}. Run scripts/ps5/setup-toolchain.sh and scripts/ps5/build-gl-sdk.sh.")
    endif()
endforeach()

# scripts/ps5/ps5-clang{,++} and the boilerplate wrapper read these from the environment; try_compile and the build inherit them.
set(ENV{PS5_PAYLOAD_SDK} "${PS5_PAYLOAD_SDK}")
set(ENV{PS5_CLANG} "${PS5_TOOLCHAIN_ROOT}/llvm/bin/clang")
set(ENV{PS5_NATIVE_APP_TEMPLATE} "${PS5_NATIVE_APP_TEMPLATE}")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES PS5_TOOLCHAIN_ROOT)

set(CMAKE_SYSTEM_NAME PS5)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../platforms")

set(CMAKE_C_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../../scripts/ps5/ps5-clang")
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../../scripts/ps5/ps5-clang++")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_C_COMPILER_LAUNCHER env "PS5_PAYLOAD_SDK=${PS5_PAYLOAD_SDK}" "PS5_CLANG=${PS5_TOOLCHAIN_ROOT}/llvm/bin/clang"
    "PS5_NATIVE_APP_TEMPLATE=${PS5_NATIVE_APP_TEMPLATE}")
set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_C_COMPILER_LAUNCHER})
set(CMAKE_AR "${PS5_TOOLCHAIN_ROOT}/llvm/bin/llvm-ar")
set(CMAKE_RANLIB "${PS5_TOOLCHAIN_ROOT}/llvm/bin/llvm-ranlib")
set(PS5_RELOCATABLE_LINKER "${PS5_TOOLCHAIN_ROOT}/llvm/bin/ld.lld")

set(CMAKE_FIND_ROOT_PATH "${PS5_PAYLOAD_SDK}/target" "${PS5_OPENGL_PREFIX}" "${PS5_SDL2_PREFIX}")
set(CMAKE_PREFIX_PATH "${PS5_SDL2_PREFIX}" "${PS5_OPENGL_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_IGNORE_PREFIX_PATH "/opt/homebrew;/usr/local")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
