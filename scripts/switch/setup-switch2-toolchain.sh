#!/usr/bin/env bash
# Adds the Nintendo Switch 2 devkit (ounce-devkit: libounce, OunceVK and the Switch2.cmake toolchain file)
# to a devkitPro tree. Experimental: the Switch 2 build is not part of any release.
#
# The archive is pinned by sha256 and unpacked over the contained devkitPro in .toolchain/opt/devkitpro,
# which scripts/switch/setup-toolchain.sh (macOS) or setup-toolchain-linux.sh must have created first.
# It only adds libounce/, portlibs/switch2/, switch2vars.sh and ounce-devkit-version.txt; libnx and the
# Switch 1 portlibs are left as they are.
#
# Idempotent: re-run at any time; an already-installed devkit of the pinned version is kept.
#
# Usage: scripts/switch/setup-switch2-toolchain.sh [devkitpro-dir]
#        (the argument is for containers that ship devkitPro elsewhere, such as the CI image)
# Then:  scripts/switch/build-switch2.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PKGS="$ROOT/.toolchain/pkgs"
DKP="${1:-$ROOT/.toolchain/opt/devkitpro}"

DEVKIT_VERSION="20261115b"
DEVKIT_SHA256="e6866198bfb44144c8227288c6315772a6a505cacca54b08cd0e6232a84f4729"
DEVKIT_FILE="ounce-devkit-$DEVKIT_VERSION.tar.xz"
DEVKIT_URL="https://github.com/ouncebrew/ounce-devkit/releases/download/devkit-$DEVKIT_VERSION/$DEVKIT_FILE"

for tool in curl tar xz; do
  command -v "$tool" >/dev/null || { echo "ERROR: '$tool' not found on the host" >&2; exit 1; }
done
[[ -f "$DKP/cmake/devkitA64.cmake" ]] || {
  echo "ERROR: no devkitA64 in $DKP; run scripts/switch/setup-toolchain.sh (macOS) or setup-toolchain-linux.sh first" >&2
  exit 1
}

# Prints the sha256 of a file with whichever tool the host has (sha256sum on Linux, shasum on macOS).
sha256_of() {
  if command -v sha256sum >/dev/null; then
    sha256sum "$1" | cut -d' ' -f1
  else
    shasum -a 256 "$1" | cut -d' ' -f1
  fi
}

if [[ -f "$DKP/libounce/cmake/Switch2.cmake" ]] &&
   [[ "$(head -n 1 "$DKP/ounce-devkit-version.txt" 2>/dev/null)" == "ounce-devkit $DEVKIT_VERSION" ]]; then
  echo "ounce-devkit $DEVKIT_VERSION already installed in $DKP"
  exit 0
fi

mkdir -p "$PKGS"
tarball="$PKGS/$DEVKIT_FILE"
if [[ -f "$tarball" && "$(sha256_of "$tarball")" != "$DEVKIT_SHA256" ]]; then
  echo "  [stale] $DEVKIT_FILE (sha256 differs) - removing"
  rm -f "$tarball"
fi
if [[ ! -f "$tarball" ]]; then
  echo "  [fetch] $DEVKIT_FILE"
  curl -fsSL --retry 3 -o "$tarball.part" "$DEVKIT_URL"
  mv "$tarball.part" "$tarball"
fi
actual="$(sha256_of "$tarball")"
[[ "$actual" == "$DEVKIT_SHA256" ]] || {
  echo "ERROR: $DEVKIT_FILE has sha256 $actual, expected $DEVKIT_SHA256" >&2
  exit 1
}

echo "  [extract] $DEVKIT_FILE -> $DKP"
rm -rf "$DKP/libounce" "$DKP/portlibs/switch2"
tar -xJf "$tarball" -C "$DKP" --no-same-owner

[[ -f "$DKP/libounce/cmake/Switch2.cmake" && -f "$DKP/portlibs/switch2/lib/libEGL.a" ]] || {
  echo "ERROR: $DEVKIT_FILE did not provide libounce/cmake/Switch2.cmake and portlibs/switch2/lib/libEGL.a" >&2
  exit 1
}
echo "Switch 2 devkit ready: scripts/switch/build-switch2.sh"
