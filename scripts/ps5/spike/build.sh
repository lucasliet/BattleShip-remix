#!/usr/bin/env bash
# Builds the Phase 0 spike title (PPSA99264) through cmake/toolchains/PS5.cmake.
# Output: .toolchain/ps5/spike-app/dist/PPSA99264/
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
T="$ROOT/.toolchain/ps5"
# shellcheck disable=SC1091
source "$T/env.sh"

HERE="$ROOT/scripts/ps5/spike"
APP="$T/spike-app"
BUILD="$APP/cmake-build"

cmake -S "$HERE" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/PS5.cmake"
cmake --build "$BUILD"

PS5_EXTRA_SDK_LIBS="libScePad.so libSceUserService.so libSceAudioOut.so" \
    bash "$ROOT/scripts/ps5/native-app-stage.sh" "$APP" PPSA99264 "$BUILD/spike.o" "$HERE/param.json"
