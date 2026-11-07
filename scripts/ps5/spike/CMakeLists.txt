# The Phase 0 spike built through cmake/toolchains/PS5.cmake (the toolchain's smoke test).
cmake_minimum_required(VERSION 3.20)
project(battleship_ps5_spike C CXX ASM)

set(PORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../port")
add_executable(spike
    spike_main.cpp
    "${PORT_DIR}/coroutine_android.cpp"
    "${PORT_DIR}/coroutine_x86_64.S")
target_compile_features(spike PRIVATE cxx_std_20)
target_compile_definitions(spike PRIVATE GL_GLEXT_PROTOTYPES=1)
target_include_directories(spike PRIVATE "${PS5_OPENGL_PREFIX}/include" "${PORT_DIR}")
target_compile_options(spike PRIVATE -O2 -Wall)
