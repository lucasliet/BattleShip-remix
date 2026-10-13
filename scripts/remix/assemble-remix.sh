#!/bin/zsh
# Assembles the Smash Remix 2.0.1 source to get its symbol map (label ->
# N64 RAM address), the input of tools/gen_remix_actions.py.
#
# Everything stays under .toolchain/ and build/:
#   - Remix source: git clone pinned to the 2.0.1 commit in .toolchain/pkgs/smashremix-src
#   - bass v14 (ARM9 fork): pinned tarball, patched with the read8/16/32 and export
#     directives Remix's own bass.exe adds (scripts/remix/bass-remix-directives.patch),
#     built with the host clang into .toolchain/host/bin/bass
#   - Remix assembles on top of the vanilla US ROM (baserom.us.z64), linked as its
#     roms/original.z64; code labels do not depend on the injected files.
#
# Output: build/remixasm/remix.sym (and remix.z64, used only for checking).
# Check: the code image (ROM 0x2C00000, 4 MB) should match baserom.remix.z64 except
# for ~1k words of data read from the injected files.
#
# Usage: scripts/remix/assemble-remix.sh

set -euo pipefail

ROOT=${0:A:h:h:h}
PKGS="$ROOT/.toolchain/pkgs"
BIN="$ROOT/.toolchain/host/bin"
REMIX_SRC="$PKGS/smashremix-src"
REMIX_URL=https://github.com/JSsixtyfour/smashremix.git
REMIX_COMMIT=5e04fe7fcd023cd43c71f25f89bb6e810d254d55
BASS_COMMIT=78e297331587eff0b2107dabe81ee036d2d01780
BASS_SHA256=fdb749200ac325db8a07a0a07c2e7ec782cdd07761241067f362af767b92ea5d
BASS_TARBALL="$PKGS/bass-${BASS_COMMIT:0:12}.tar.gz"
BASS_SRC="$PKGS/bass-src"
OUT="$ROOT/build/remixasm"

fetch_remix_source() {
  if [[ ! -d "$REMIX_SRC/.git" ]]; then
    git clone --quiet "$REMIX_URL" "$REMIX_SRC"
  fi
  git -C "$REMIX_SRC" checkout --quiet "$REMIX_COMMIT"
}

build_bass() {
  if [[ -x "$BIN/bass" ]]; then
    return
  fi
  if [[ ! -f "$BASS_TARBALL" ]]; then
    curl -fsSL --retry 3 -o "$BASS_TARBALL" "https://codeload.github.com/ARM9/bass/tar.gz/$BASS_COMMIT"
  fi
  local got=$(shasum -a 256 "$BASS_TARBALL" | awk '{print $1}')
  if [[ "$got" != "$BASS_SHA256" ]]; then
    echo "bass tarball sha256 $got, expected $BASS_SHA256" >&2
    exit 1
  fi
  rm -rf "$BASS_SRC" && mkdir -p "$BASS_SRC"
  tar xzf "$BASS_TARBALL" -C "$BASS_SRC" --strip-components=1
  patch -d "$BASS_SRC" -p1 --quiet < "$ROOT/scripts/remix/bass-remix-directives.patch"
  mkdir -p "$BIN"
  clang++ -std=c++17 -O2 -w -I"$BASS_SRC" -o "$BIN/bass" "$BASS_SRC/bass/bass.cpp"
}

assemble() {
  if [[ ! -f "$ROOT/baserom.us.z64" ]]; then
    echo "baserom.us.z64 is required (Remix assembles on top of the vanilla US ROM)" >&2
    exit 1
  fi
  ln -sf "$ROOT/baserom.us.z64" "$REMIX_SRC/roms/original.z64"
  mkdir -p "$OUT"
  (builtin cd "$REMIX_SRC" && "$BIN/bass" -create -o "$OUT/remix.z64" -sym "$OUT/remix.sym" main.asm > "$OUT/bass.log")
  echo "symbols: $(wc -l < "$OUT/remix.sym") -> $OUT/remix.sym"
}

fetch_remix_source
build_bass
assemble
