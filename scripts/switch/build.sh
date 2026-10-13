#!/bin/zsh
# BattleShip — Nintendo Switch build (contained toolchain, no system state).
#
# Configures and cross-compiles the .nro using the devkitA64 toolchain
# extracted under .toolchain/. Assets (BattleShip.o2r, f3d.o2r) are generated
# host-side (desktop build / ExtractAssets) and copied onto the SD card — the
# Switch build never touches the ROM.
#
# Usage: [VERSION=us|jp|remix] scripts/switch/build.sh [us|jp|remix] [extra cmake args...]

set -euo pipefail

ROOT=${0:A:h:h:h}
source "$ROOT/.toolchain/env.sh"

VER="${VERSION:-remix}"
if [[ "${1:-}" == "us" || "${1:-}" == "jp" || "${1:-}" == "remix" ]]; then
  VER="$1"
  shift
fi
[[ "$VER" == "us" || "$VER" == "jp" || "$VER" == "remix" ]] || { echo "VERSION must be us|jp|remix (got '$VER')" >&2; exit 1; }
if [[ "$VER" == "remix" ]]; then APP_NAME="BattleShip-Remix"; else APP_NAME="BattleShip"; fi

BUILD_DIR="$ROOT/build-switch"
HOST_PYTHON="$ROOT/.toolchain/host/venv/bin/python"

# tools/render_nameplates.py and generate_arrow_svg.py need Pillow to bake CSS headers
if [[ ! -x "$HOST_PYTHON" ]]; then
  uv venv "$ROOT/.toolchain/host/venv"
  uv pip install --python "$HOST_PYTHON" pillow
fi

cmake -S "$ROOT" -B "$BUILD_DIR" -GNinja \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" \
  -DPython3_EXECUTABLE="$HOST_PYTHON" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBATTLESHIP_SWITCH=ON \
  -DDISABLE_SCRIPTING=ON \
  -DSSB64_VERSION="$VER" \
  "$@"

cmake --build "$BUILD_DIR" --target ssb64_nro -j "${JOBS:-4}"

echo
echo "NRO: $BUILD_DIR/$APP_NAME.nro"
