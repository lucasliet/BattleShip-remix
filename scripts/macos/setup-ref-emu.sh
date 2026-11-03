#!/bin/zsh
# Reference N64 emulator for visual comparison (M11 I8): mupen64plus 2.6.0 with the pure interpreter, the cxd4 LLE RSP and
# the angrylion cycle-accurate software RDP, plus a scripted input plugin (scripts/ref-emu/input-script).
#
# Everything lives in .toolchain/ref-emu (git-ignored, GPL code that must never enter the repository). Nothing is installed
# on the system. Sources come from pinned tarballs checked against the sha256 below. Idempotent: a built piece is skipped.
#
# Usage: scripts/macos/setup-ref-emu.sh
# Then capture with scripts/ref-emu/capture.sh.

set -euo pipefail

ROOT=${0:A:h:h:h}
TC="$ROOT/.toolchain"
PKGS="$TC/pkgs"
SRC="$PKGS/ref-emu-src"
EMU="$TC/ref-emu"
BUILD="$EMU/build"
PREFIX="$EMU/prefix"
CMAKE="$TC/host/cmake/CMake.app/Contents/bin/cmake"
export PATH="$TC/host/ninja:$PATH"
JOBS=${JOBS:-4}

mkdir -p "$SRC" "$BUILD" "$PREFIX"

# fetch_pinned <file> <url> <sha256> -> echoes the extracted source dir
fetch_pinned() {
  local file=$1 url=$2 sha=$3 tarball="$SRC/$1"
  [[ -f "$tarball" ]] || curl -fsSL --retry 3 -o "$tarball" "$url"
  local got=$(shasum -a 256 "$tarball" | awk '{print $1}')
  [[ "$got" == "$sha" ]] || { echo "ERRO: sha256 de $file difere (esperado $sha, obtido $got)" >&2; exit 1; }
  local dir="$BUILD/${file%.tar.gz}"
  if [[ ! -d "$dir" ]]; then
    mkdir -p "$dir.tmp" && tar -xf "$tarball" -C "$dir.tmp" --strip-components=1 && mv "$dir.tmp" "$dir"
  fi
  echo "$dir"
}

M64P=https://github.com/mupen64plus
SDL_TAR=SDL2-2.30.10.tar.gz
[[ -f "$SRC/$SDL_TAR" ]] || cp "$PKGS/$SDL_TAR" "$SRC/$SDL_TAR"
SDL=$(fetch_pinned $SDL_TAR https://github.com/libsdl-org/SDL/releases/download/release-2.30.10/SDL2-2.30.10.tar.gz \
  f59adf36a0fcf4c94198e7d3d776c1b3824211ab7aeebeb31fe19836661196aa)
PNG=$(fetch_pinned libpng-1.6.44.tar.gz https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.44.tar.gz \
  0ef5b633d0c65f780c4fced27ff832998e71478c13b45dfb6e94f23a82f64f7c)
CORE=$(fetch_pinned mupen64plus-core-2.6.0.tar.gz $M64P/mupen64plus-core/archive/refs/tags/2.6.0.tar.gz \
  e8445c01955ee5f9cb2cc8fa6e5d470223cb3560ddda49adbca1e85fce6b1a6d)
UI=$(fetch_pinned mupen64plus-ui-console-2.6.0.tar.gz $M64P/mupen64plus-ui-console/archive/refs/tags/2.6.0.tar.gz \
  b9d525fbe76df49fc72f80fd46afd7f0abb1c388f82050cd1edc92ecff0754e5)
RSP=$(fetch_pinned mupen64plus-rsp-cxd4-2.6.0.tar.gz $M64P/mupen64plus-rsp-cxd4/archive/refs/tags/2.6.0.tar.gz \
  a40eabd118784623e4d91a5d8a5a9a2f2c7dee2392ec30235b0dd6c6fdf88135)
AUDIO=$(fetch_pinned mupen64plus-audio-sdl-2.6.0.tar.gz $M64P/mupen64plus-audio-sdl/archive/refs/tags/2.6.0.tar.gz \
  0803b84870bf035e5352f91bd9bf2d3f0661b48ae5a17bc6363e931478daf137)
RDP=$(fetch_pinned angrylion-rdp-plus-1.6.tar.gz https://github.com/ata4/angrylion-rdp-plus/archive/refs/tags/v1.6.tar.gz \
  f4f65535da77e994b7da3502bf062f89ed45608be903882378030d3a301dea67)

CMAKE_COMMON=(-G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX"
  -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local" -DCMAKE_PREFIX_PATH="$PREFIX")

build_cmake() {
  local marker=$1 src=$2; shift 2
  [[ -e "$PREFIX/$marker" ]] && { echo "ok  $marker"; return; }
  "$CMAKE" -S "$src" -B "$src/_b" "${CMAKE_COMMON[@]}" "$@"
  "$CMAKE" --build "$src/_b" -j "$JOBS"
  "$CMAKE" --install "$src/_b"
}

# Shared SDL2: the core, the audio plugin and the input plugin must share one SDL instance.
build_cmake lib/libSDL2.dylib "$SDL" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
build_cmake lib/libpng16.dylib "$PNG" -DPNG_SHARED=ON -DPNG_STATIC=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF

# mupen64plus pieces use the unix makefiles; PREFIX-less installs are copied by hand below.
M64P_FLAGS=(NO_ASM=1 OSD=0 VULKAN=0 NEW_DYNAREC=0 USE_GLES=0 NO_SRC=1
  SDL_CONFIG="$PREFIX/bin/sdl2-config" PKG_CONFIG=false
  SDL_CFLAGS="-I$PREFIX/include/SDL2 -D_REENTRANT" SDL_LDLIBS="-L$PREFIX/lib -lSDL2"
  LIBPNG_CFLAGS="-I$PREFIX/include" LIBPNG_LDLIBS="-L$PREFIX/lib -lpng16"
  ZLIB_CFLAGS="" ZLIB_LDLIBS="-lz"
  APIDIR="$CORE/src/api" )

if [[ ! -e "$EMU/lib/libmupen64plus.dylib" ]]; then
  make -C "$CORE/projects/unix" all "${M64P_FLAGS[@]}" -j "$JOBS"
  mkdir -p "$EMU/lib" "$EMU/data" "$EMU/include"
  cp "$CORE/projects/unix/libmupen64plus.dylib" "$EMU/lib/"
  cp "$CORE/data/"* "$EMU/data/"
  cp "$CORE/src/api/"*.h "$EMU/include/"
fi
M64P_FLAGS[-1]=APIDIR="$EMU/include"
for pair in "$UI:mupen64plus" "$RSP:mupen64plus-rsp-cxd4.dylib" "$AUDIO:mupen64plus-audio-sdl.dylib"; do
  dir=${pair%%:*}; out=${pair##*:}
  [[ -e "$EMU/bin/$out" || -e "$EMU/plugins/$out" ]] && { echo "ok  $out"; continue; }
  make -C "$dir/projects/unix" all "${M64P_FLAGS[@]}" -j "$JOBS"
  mkdir -p "$EMU/bin" "$EMU/plugins"
  if [[ "$out" == mupen64plus ]]; then cp "$dir/projects/unix/mupen64plus" "$EMU/bin/"; else cp "$dir/projects/unix/$out" "$EMU/plugins/"; fi
done

# angrylion RDP plus (mupen64plus build target of its CMake project)
if [[ ! -e "$EMU/plugins/mupen64plus-video-angrylion-plus.dylib" ]]; then
  "$CMAKE" -S "$RDP" -B "$RDP/_b" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_MUPEN64PLUS=ON -DBUILD_PROJECT64=OFF \
    -DCMAKE_PREFIX_PATH="$PREFIX" -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local"
  "$CMAKE" --build "$RDP/_b" -j "$JOBS"
  find "$RDP/_b" -name 'mupen64plus-video-angrylion-plus*' -type f -exec cp {} "$EMU/plugins/" \;
fi

# scripted input plugin
"$ROOT/scripts/ref-emu/build-input-script.sh"

echo "Reference emulator ready in $EMU"
