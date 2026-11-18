#!/bin/zsh
# BattleShip — Nintendo Switch 2 build (experimental; contained toolchain, no system state).
#
# Same flow as scripts/switch/build.sh, with libounce's Switch2.cmake toolchain file in place of
# devkitPro's Switch.cmake (scripts/switch/setup-switch2-toolchain.sh adds it under .toolchain/).
# The result is checked by tools/verify_switch2_package.py: an .nro with the Switch 2 startup tag
# that links libounce and OunceVK and nothing from libnx. Assets are generated host-side, as for
# the Switch.
#
# Usage: [VERSION=us|jp|remix] scripts/switch/build-switch2.sh [us|jp|remix] [extra cmake args...]

set -euo pipefail

ROOT=${0:A:h:h:h}
source "$ROOT/.toolchain/env.sh"

VER="${VERSION:-remix}"
if [[ "${1:-}" == "us" || "${1:-}" == "jp" || "${1:-}" == "remix" ]]; then
  VER="$1"
  shift
fi
[[ "$VER" == "us" || "$VER" == "jp" || "$VER" == "remix" ]] || { echo "VERSION must be us|jp|remix (got '$VER')" >&2; exit 1; }
case "$VER" in
  remix) APP_NAME="BattleShip-Remix" ;;
  jp) APP_NAME="BattleShip-JP" ;;
  *) APP_NAME="BattleShip" ;;
esac

BUILD_DIR="$ROOT/build-switch2"
HOST_PYTHON="$ROOT/.toolchain/host/venv/bin/python"
TOOLCHAIN_FILE="$DEVKITPRO/libounce/cmake/Switch2.cmake"

[[ -f "$TOOLCHAIN_FILE" ]] || { echo "$TOOLCHAIN_FILE not found; run scripts/switch/setup-switch2-toolchain.sh first" >&2; exit 1; }

# tools/render_nameplates.py and generate_arrow_svg.py need Pillow to bake CSS headers
if [[ ! -x "$HOST_PYTHON" ]]; then
  uv venv "$ROOT/.toolchain/host/venv"
  uv pip install --python "$HOST_PYTHON" pillow
fi

cmake -S "$ROOT" -B "$BUILD_DIR" -GNinja \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
  -DPython3_EXECUTABLE="$HOST_PYTHON" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBATTLESHIP_SWITCH=ON \
  -DDISABLE_SCRIPTING=ON \
  -DSSB64_VERSION="$VER" \
  "$@"

cmake --build "$BUILD_DIR" --target ssb64_nro -j "${JOBS:-4}"

"$HOST_PYTHON" -I "$ROOT/tools/verify_switch2_package.py" \
  --nro "$BUILD_DIR/$APP_NAME.nro" \
  --elf "$BUILD_DIR/$APP_NAME.elf" \
  --map "$BUILD_DIR/$APP_NAME-switch2.map"

echo
echo "NRO: $BUILD_DIR/$APP_NAME.nro"
