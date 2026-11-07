#!/usr/bin/env bash
# BattleShip — PS5 native title build (Linux x86_64 host, contained toolchain under .toolchain/ps5).
#
# Cross-compiles the game into one relocatable object through cmake/toolchains/PS5.cmake, then
# stages the title folder build-ps5/ps5-app/dist/PPSA99264 (eboot.bin, sce_sys, sce_module).
# Prepare the toolchain once with scripts/ps5/setup-toolchain.sh and scripts/ps5/build-gl-sdk.sh.
#
# Usage: [VERSION=remix|us] [JOBS=n] scripts/ps5/build.sh [remix|us] [extra cmake args...]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck disable=SC1091
source "$ROOT/.toolchain/ps5/env.sh"

VER="${VERSION:-remix}"
if [[ "${1:-}" == "us" || "${1:-}" == "remix" ]]; then
    VER="$1"
    shift
fi
[[ "$VER" == "us" || "$VER" == "remix" ]] || { echo "VERSION must be us|remix (got '$VER')" >&2; exit 1; }

BUILD_DIR="$ROOT/build-ps5"
HOST_PYTHON="$ROOT/.toolchain/ps5/venv/bin/python"
# tools/render_nameplates.py and generate_arrow_svg.py need Pillow to bake CSS headers.
"$HOST_PYTHON" -I -c 'import PIL' 2>/dev/null || uv pip install -q -p "$HOST_PYTHON" pillow

cmake -S "$ROOT" -B "$BUILD_DIR" -GNinja \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/PS5.cmake" \
    -DPython3_EXECUTABLE="$HOST_PYTHON" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_SCAN_FOR_MODULES=OFF \
    -DDISABLE_SCRIPTING=ON \
    -DSSB64_VERSION="$VER" \
    "$@"

cmake --build "$BUILD_DIR" --target ssb64_ps5_title -j "${JOBS:-8}"
