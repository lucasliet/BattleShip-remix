#!/bin/zsh
# Contained macOS host deps for the desktop build: SDL2 + GLEW built from
# source into .toolchain/host/deps (static, nothing installed on the system).
# The app configure then uses CMAKE_PREFIX_PATH to find them.
#
# Usage: scripts/macos/setup-host-deps.sh [--offline]

set -euo pipefail
ROOT=${0:A:h:h:h}
HOST="$ROOT/.toolchain/host"
DEPS="$HOST/deps"
PKGS="$ROOT/.toolchain/pkgs"
OFFLINE=0
[[ "${1:-}" == "--offline" ]] && OFFLINE=1
mkdir -p "$PKGS" "$DEPS"

source "$ROOT/.toolchain/env.sh"   # contained cmake/ninja

fetch() {
  (( OFFLINE )) && return 1
  curl -fsSL --retry 3 -o "$2" "$1"
}

# --- pins (SHA256 preenchido no primeiro download verificado) ---------------
SDL2_VER=2.30.10
GLEW_VER=2.1.0

build_sdl2() {
  [[ -f "$DEPS/lib/libSDL2.a" ]] && return 0
  local t="SDL2-$SDL2_VER"
  [[ -d "$PKGS/$t" ]] || {
    fetch "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VER/$t.tar.gz" "$PKGS/$t.tar.gz" \
      || { echo "baixe manualmente: $t.tar.gz -> .toolchain/pkgs/ e tar -xzf lá" >&2; return 1; }
    tar -xzf "$PKGS/$t.tar.gz" -C "$PKGS"
  }
  cmake -S "$PKGS/$t" -B "$PKGS/$t/build" -GNinja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$DEPS" \
    -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF \
    -DSDL_AUDIO=ON -DSDL_JOYSTICK=ON -DSDL_HAPTIC=ON -DSDL_POWER=ON -DSDL_TIMERS=ON \
    -DSDL_RENDER=OFF -DSDL_CAMERA=OFF -DSDL_VULKAN=OFF -DSDLWayland=OFF
  cmake --build "$PKGS/$t/build" -j "$(sysctl -n hw.ncpu)"
  cmake --install "$PKGS/$t/build"
}

build_glew() {
  [[ -f "$DEPS/lib/libGLEW.a" ]] && return 0
  local t="glew-$GLEW_VER"
  [[ -d "$PKGS/$t" ]] || {
    fetch "https://github.com/nigels-com/glew/releases/download/glew-$GLEW_VER/glew-$GLEW_VER.tgz" "$PKGS/$t.tgz" \
      || { echo "baixe manualmente: glew-$GLEW_VER.tgz -> .toolchain/pkgs/" >&2; return 1; }
    tar -xzf "$PKGS/$t.tgz" -C "$PKGS"
  }
  # GLEW upstream's build system is fragile outside /usr; the library is a
  # single translation unit, so compile it directly (static, no GLU dep).
  mkdir -p "$DEPS/lib" "$DEPS/include"
  cc -O2 -fPIC -DGLEW_STATIC -DGLEW_NO_GLU -I"$PKGS/$t/include" \
    -c "$PKGS/$t/src/glew.c" -o "$PKGS/$t/glew.o"
  ar rcs "$DEPS/lib/libGLEW.a" "$PKGS/$t/glew.o"
  cp -R "$PKGS/$t/include/GL" "$DEPS/include/"
}

build_sdl2
build_glew

cat >> "$ROOT/.toolchain/env.sh" <<EOF
# Host desktop deps (SDL2/GLEW estáticos, gerados por setup-host-deps.sh)
export SSB64_MACOS_DEPS="$DEPS"
EOF

echo "Deps host prontas em $DEPS"
echo "Configure do app: CMAKE_PREFIX_PATH=$DEPS cmake -S . -B build -GNinja ..."
