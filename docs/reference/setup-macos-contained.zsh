#!/bin/zsh
set -euo pipefail
ROOT=${0:A:h:h}
TC="$ROOT/.toolchain"
DKP="$TC/opt/devkitpro"
PKGS="$TC/pkgs"

extract_all() {
  mkdir -p "$DKP"
  local tmp="$TC/.extract-tmp"
  rm -rf "$tmp"; mkdir -p "$tmp"
  for f in "$PKGS"/*.pkg.tar.*; do
    tar -xf "$f" -C "$tmp"
    if [[ -d "$tmp/opt/devkitpro" ]]; then
      cp -a "$tmp/opt/devkitpro/." "$DKP/"
      rm -rf "$tmp/opt"
    else
      cp -a "$tmp/." "$DKP/"
      rm -rf "$tmp/"*(N)
    fi
  done
  rm -rf "$tmp"
  grep -rl '/opt/devkitpro' "$DKP/portlibs" --include='*.pc' --include='*.la' 2>/dev/null |
    while read -r f; do sed -i '' "s|/opt/devkitpro|$DKP|g" "$f"; done
}

fetch_extras() {
  mkdir -p "$TC/ninja"
  [[ -x "$TC/ninja/ninja" ]] || {
    curl -fsSL -o "$TC/ninja.zip" https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-mac.zip
    unzip -oq "$TC/ninja.zip" -d "$TC/ninja"; rm -f "$TC/ninja.zip"
  }
  [[ -d "$TC/cmake" ]] || {
    curl -fsSL -o "$TC/cmake.tar.gz" https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-macos-universal.tar.gz
    tar -xzf "$TC/cmake.tar.gz" -C "$TC"; mv "$TC/cmake-3.31.6-macos-universal" "$TC/cmake"; rm -f "$TC/cmake.tar.gz"
  }
  [[ -d "$TC/pkgconf" ]] || {
    curl -fsSL -o "$TC/pkgconf.tar.xz" https://github.com/pkgconf/pkgconf/releases/download/pkgconf-3.0.7/pkgconf-3.0.7.tar.xz
    tar -xJf "$TC/pkgconf.tar.xz" -C "$TC"
    (cd "$TC/pkgconf-3.0.7" && ./configure --prefix="$TC/pkgconf" >/dev/null 2>&1 && make -j8 >/dev/null 2>&1 && make install >/dev/null)
    rm -rf "$TC/pkgconf-3.0.7" "$TC/pkgconf.tar.xz"
  }
  [[ -x "$ROOT/tools/makerom" ]] || {
    curl -fsSL -o "$ROOT/tools/makerom.zip" https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.19.0/makerom-v0.19.0-macos_arm64.zip
    unzip -oq "$ROOT/tools/makerom.zip" -d "$ROOT/tools"; chmod +x "$ROOT/tools/makerom"; rm -f "$ROOT/tools/makerom.zip"
  }
}

if [[ ! -x "$DKP/devkitARM/bin/arm-none-eabi-gcc" ]]; then
  if ls "$PKGS"/*.pkg.tar.* >/dev/null 2>&1; then
    extract_all
  else
    cat <<'MSG'
No cached packages in .toolchain/pkgs.
pkg.devkitpro.org blocks curl on some networks (Cloudflare TLS filter);
downloads may need a real browser. Fetch every URL listed in
.toolchain/package-urls.txt, .toolchain/urls-love.txt, and
devkitARM-gdb-14.1-1-arm64.pkg.tar.zst (from packages/osx/arm64/)
into .toolchain/pkgs/, then rerun this script.
MSG
    exit 2
  fi
fi

fetch_extras
mkdir -p "$TC/bin"
ln -sf "$TC/cmake/CMake.app/Contents/bin/cmake" "$TC/bin/cmake"
ln -sf "$TC/ninja/ninja" "$TC/bin/ninja"
ln -sf "$TC/pkgconf/bin/pkgconf" "$TC/bin/pkg-config"
ln -sf "$ROOT/tools/makerom" "$TC/bin/makerom"

cat > "$ROOT/.env.macos" <<EOF
export DEVKITPRO="$DKP"
export DEVKITARM="$DKP/devkitARM"
export PATH="$TC/bin:\$DEVKITARM/bin:\$DEVKITPRO/tools/bin:\$PATH"
EOF

if [[ ! -d "$TC/venv-luma" ]]; then
  python3 -m venv "$TC/venv-luma"
  "$TC/venv-luma/bin/pip" install -q git+https://github.com/LumaTeam/luma3ds_exception_dump_parser.git
fi

source "$ROOT/.env.macos"
echo "DEVKITPRO=$DEVKITPRO"
"$DEVKITARM/bin/arm-none-eabi-gcc" --version | head -1
"$DEVKITARM/bin/arm-none-eabi-gdb" --version | head -1
cmake --version | head -1
ninja --version
echo 'Contained toolchain ready. Source .env.macos before building.'
