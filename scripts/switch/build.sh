#!/bin/zsh
# BattleShip — Nintendo Switch build (contained toolchain, no system state).
#
# Configures and cross-compiles the .nro using the devkitA64 toolchain
# extracted under .toolchain/. Assets (BattleShip.o2r, f3d.o2r) are generated
# host-side (desktop build / ExtractAssets) and copied onto the SD card — the
# Switch build never touches the ROM.
#
# Usage: scripts/switch/build.sh [us|jp] [extra cmake args...]

set -euo pipefail

ROOT=${0:A:h:h:h}
source "$ROOT/.toolchain/env.sh"

VER="${1:-us}"
if [[ "$VER" == "us" || "$VER" == "jp" ]]; then
  shift
fi

BUILD_DIR="$ROOT/build-switch"

cmake -S "$ROOT" -B "$BUILD_DIR" -GNinja \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBATTLESHIP_SWITCH=ON \
  -DDISABLE_SCRIPTING=ON \
  -DSSB64_VERSION="$VER" \
  "$@"

cmake --build "$BUILD_DIR" --target ssb64_nro -j "$(sysctl -n hw.ncpu)"

echo
echo "NRO: $BUILD_DIR/ssb64.nro"
