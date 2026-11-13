#!/usr/bin/env bash
# AppImage packaging tools for scripts/package-linux.sh, contained in .toolchain/host/bin.
#
# Same pinned versions and sha256 as scripts/linux/Dockerfile.jammy; bump both together.
# Nothing is installed on the system. Idempotent: a tool already present with the pinned
# sha256 is kept.
#
# Usage: scripts/linux/setup-appimage-tools.sh
# Then: PATH="$PWD/.toolchain/host/bin:$PATH" APPIMAGE_EXTRACT_AND_RUN=1 scripts/package-linux.sh

set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
BIN="$ROOT/.toolchain/host/bin"

APPIMAGETOOL_URL=https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage
APPIMAGETOOL_SHA256=ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
LINUXDEPLOY_URL=https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage
LINUXDEPLOY_SHA256=c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d

mkdir -p "$BIN"

# install_pinned <name> <url> <sha256>
install_pinned() {
  local name=$1 url=$2 sha=$3
  local target="$BIN/$name"
  if [[ -x "$target" ]] && [[ "$(sha256sum "$target" | awk '{print $1}')" == "$sha" ]]; then
    echo "ok  $name"
    return
  fi
  curl -fsSL --retry 3 -o "$target.part" "$url"
  local got
  got=$(sha256sum "$target.part" | awk '{print $1}')
  if [[ "$got" != "$sha" ]]; then
    rm -f "$target.part"
    echo "$name sha256 $got, expected $sha" >&2
    exit 1
  fi
  chmod +x "$target.part"
  mv "$target.part" "$target"
  echo "installed $name"
}

install_pinned appimagetool "$APPIMAGETOOL_URL" "$APPIMAGETOOL_SHA256"
install_pinned linuxdeploy "$LINUXDEPLOY_URL" "$LINUXDEPLOY_SHA256"
