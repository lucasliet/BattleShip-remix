#!/usr/bin/env bash
# Builds the Phase 0 spike title (PPSA99264) from spike_main.cpp and the port's asm coroutine
# backend. Output: .toolchain/ps5/spike-app/dist/PPSA99264/
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
T="$ROOT/.toolchain/ps5"
# shellcheck disable=SC1091
source "$T/env.sh"

HERE="$ROOT/scripts/ps5/spike"
APP="$T/spike-app"
OBJ="$APP/obj"
WRAPPER="$PS5_NATIVE_APP_TEMPLATE/tooling/prospero-clang18"
FLAGS=(-D__PS5__ -O2 -fPIC -ffunction-sections -fdata-sections -Wall -DGL_GLEXT_PROTOTYPES=1
       -I"$PS5_OPENGL_PREFIX/include" -I"$ROOT/port")

mkdir -p "$OBJ"
sh "$WRAPPER" "${FLAGS[@]}" -std=c++20 -fexceptions -frtti -c "$HERE/spike_main.cpp" -o "$OBJ/spike_main.o"
sh "$WRAPPER" "${FLAGS[@]}" -std=c++20 -fexceptions -frtti -c "$ROOT/port/coroutine_android.cpp" -o "$OBJ/coroutine_android.o"
sh "$WRAPPER" "${FLAGS[@]}" -c "$ROOT/port/coroutine_x86_64.S" -o "$OBJ/coroutine_x86_64.o"
"$T/llvm/bin/ld.lld" -r -o "$OBJ/spike.o" "$OBJ/spike_main.o" "$OBJ/coroutine_android.o" "$OBJ/coroutine_x86_64.o"

PS5_EXTRA_SDK_LIBS="libScePad.so libSceUserService.so libSceAudioOut.so" \
    bash "$ROOT/scripts/ps5/native-app-stage.sh" "$APP" PPSA99264 "$OBJ/spike.o" "$HERE/param.json"
