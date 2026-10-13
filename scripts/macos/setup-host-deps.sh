#!/bin/zsh
# Contained macOS desktop dependencies for BattleShip.
#
# Builds the libraries libultraship needs (libzip, nlohmann_json, tinyxml2,
# spdlog) from pinned source tarballs into .toolchain/host/deps. Nothing is
# installed on the system (no brew, no /usr/local, no sudo). SDL2 and GLEW
# are built the same way from the tarballs cached in .toolchain/pkgs.
#
# Idempotent: a dependency whose marker file already exists is skipped.
# Source tarballs are checked against the sha256 pinned below.
#
# Usage: scripts/macos/setup-host-deps.sh
# Then configure with the flags printed at the end.

set -euo pipefail

ROOT=${0:A:h:h:h}
TC="$ROOT/.toolchain"
PKGS="$TC/pkgs"
DEPS="$TC/host/deps"
BUILD="$PKGS/host-deps-build"

source "$TC/env.sh"
mkdir -p "$PKGS" "$DEPS" "$BUILD"

CMAKE_COMMON=(
  -G Ninja -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="$DEPS"
  -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local"
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DBUILD_SHARED_LIBS=OFF
)

# fetch_pinned <name> <url> <sha256> -> echoes the extracted source dir
fetch_pinned() {
  local name=$1 url=$2 sha=$3 tarball="$PKGS/${2:t}"
  if [[ ! -f "$tarball" ]]; then
    curl -fsSL --retry 3 -o "$tarball" "$url"
  fi
  local got=$(shasum -a 256 "$tarball" | awk '{print $1}')
  [[ "$got" == "$sha" ]] || { echo "ERRO: sha256 de $name difere (esperado $sha, obtido $got)" >&2; exit 1; }
  local dir="$PKGS/$name"
  if [[ ! -d "$dir" ]]; then
    mkdir -p "$dir.tmp" && tar -xf "$tarball" -C "$dir.tmp" --strip-components=1 && mv "$dir.tmp" "$dir"
  fi
  echo "$dir"
}

# build_cmake <marker-file> <source-dir> <cmake args...>
build_cmake() {
  local marker=$1 src=$2; shift 2
  [[ -e "$DEPS/$marker" ]] && { echo "ok  $marker"; return; }
  local bdir="$BUILD/${src:t}"
  cmake -S "$src" -B "$bdir" "${CMAKE_COMMON[@]}" "$@"
  cmake --build "$bdir" -j "${JOBS:-4}"
  cmake --install "$bdir"
}

LIBZIP=$(fetch_pinned libzip-1.11.3 \
  https://github.com/nih-at/libzip/releases/download/v1.11.3/libzip-1.11.3.tar.gz \
  76653f135dde3036036c500e11861648ffbf9e1fc5b233ff473c60897d9db0ea)
build_cmake lib/libzip.a "$LIBZIP" \
  -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_OSSFUZZ=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF \
  -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF -DENABLE_COMMONCRYPTO=OFF \
  -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=OFF

NLOHMANN=$(fetch_pinned nlohmann_json-3.11.3 \
  https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz \
  d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d)
build_cmake share/cmake/nlohmann_json/nlohmann_jsonConfig.cmake "$NLOHMANN" \
  -DJSON_BuildTests=OFF -DJSON_Install=ON

TINYXML2=$(fetch_pinned tinyxml2-10.0.0 \
  https://github.com/leethomason/tinyxml2/archive/refs/tags/10.0.0.tar.gz \
  3bdf15128ba16686e69bce256cc468e76c7b94ff2c7f391cc5ec09e40bff3839)
build_cmake lib/libtinyxml2.a "$TINYXML2" -Dtinyxml2_BUILD_TESTING=OFF

# 1.14.x bundles a fmt that fails to build with current Xcode clang (consteval FMT_STRING)
SPDLOG=$(fetch_pinned spdlog-1.15.3 \
  https://github.com/gabime/spdlog/archive/refs/tags/v1.15.3.tar.gz \
  15a04e69c222eb6c01094b5c7ff8a249b36bb22788d72519646fb85feb267e67)
build_cmake lib/libspdlog.a "$SPDLOG" -DSPDLOG_BUILD_EXAMPLE=OFF -DSPDLOG_BUILD_TESTS=OFF

# A previous hand-built SDL2 had SDL_RENDER_DISABLED, which makes the game abort
# at startup ("SDL not built with rendering support").
SDL2=$(fetch_pinned SDL2-2.30.10 \
  https://github.com/libsdl-org/SDL/releases/download/release-2.30.10/SDL2-2.30.10.tar.gz \
  f59adf36a0fcf4c94198e7d3d776c1b3824211ab7aeebeb31fe19836661196aa)
build_cmake lib/libSDL2.a "$SDL2" \
  -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF -DSDL_RENDER=ON -DSDL_METAL=ON -DSDL_OPENGL=ON

echo
echo "GLEW: esperado em $DEPS/lib (libGLEW.a, glew 2.1.0 em .toolchain/pkgs)."
echo "Configure com:"
echo "  cmake -S $ROOT -B $ROOT/build -G Ninja -DCMAKE_PREFIX_PATH=$DEPS \\"
echo "    -DCMAKE_IGNORE_PREFIX_PATH='/opt/homebrew;/usr/local' -DGLEW_USE_STATIC_LIBS=ON"
