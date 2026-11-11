#!/usr/bin/env bash
# Contained Nintendo Switch toolchain bootstrap for BattleShip on a Linux x86_64 host.
#
# Linux counterpart of scripts/switch/setup-toolchain.sh: downloads devkitA64 and the
# portlibs from pkg.devkitpro.org into .toolchain/opt/devkitpro (never /opt, never the
# system), pins every tarball by sha256 and writes .toolchain/env.sh. The host-independent
# packages ("libs") come from scripts/switch/packages.lock, the Linux host binaries from
# scripts/switch/packages.linux.lock. cmake and ninja come from the host.
#
# Idempotent: re-run at any time; already-verified artifacts are kept.
#
# Usage: scripts/switch/setup-toolchain-linux.sh
# Then:  scripts/switch/build.sh   (a zsh script; it sources .toolchain/env.sh)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TC="$ROOT/.toolchain"
DKP="$TC/opt/devkitpro"
PKGS="$TC/pkgs"
MARKS="$TC/.extracted-linux"
LIBS_LOCK="$ROOT/scripts/switch/packages.lock"
LINUX_LOCK="$ROOT/scripts/switch/packages.linux.lock"

for tool in cmake ninja curl tar zstd xz; do
  command -v "$tool" >/dev/null || { echo "ERRO: '$tool' nao encontrado no host" >&2; exit 1; }
done

mkdir -p "$PKGS" "$DKP" "$MARKS" "$TC/bin"

manifest() {
  uv run -q --no-project python -I - "$LIBS_LOCK" "$LINUX_LOCK" <<'EOF'
import json, sys
libs = json.load(open(sys.argv[1]))
linux = json.load(open(sys.argv[2]))
for p in libs["packages"]["libs"].values():
    print(p["filename"], p["sha256"], f"{libs['base_urls']['libs']}/{p['filename']}")
for p in linux["packages"].values():
    print(p["filename"], p["sha256"], f"{linux['base_url']}/{p['filename']}")
EOF
}

echo "== devkitPro packages =="
while read -r file sha url; do
  tarball="$PKGS/$file"
  if [[ -f "$tarball" ]] && ! echo "$sha  $tarball" | sha256sum -c - >/dev/null 2>&1; then
    echo "  [stale] $file (sha256 differs, probably the macOS build) - removing"
    rm -f "$tarball" "$MARKS/$file"
  fi
  if [[ ! -f "$tarball" ]]; then
    echo "  [fetch] $file"
    curl --http1.1 -fsSL --retry 3 -A dkp-pacman/6.0.2 -o "$tarball.part" "$url"
    mv "$tarball.part" "$tarball"
  fi
  echo "$sha  $tarball" | sha256sum -c - >/dev/null || { echo "ERRO: sha256 divergente em $file" >&2; exit 1; }
done < <(manifest)

echo "== extraindo para $DKP =="
tmp="$TC/.extract-tmp"
while read -r file sha url; do
  [[ -f "$MARKS/$file" ]] && continue
  echo "  [extract] $file"
  rm -rf "$tmp"; mkdir -p "$tmp"
  tar -xf "$PKGS/$file" -C "$tmp"
  if [[ -d "$tmp/opt/devkitpro" ]]; then
    cp -a "$tmp/opt/devkitpro/." "$DKP/"
  else
    cp -a "$tmp/." "$DKP/"
  fi
  touch "$MARKS/$file"
done < <(manifest)
rm -rf "$tmp"

echo "== relocando /opt/devkitpro -> $DKP =="
grep -rlF "/opt/devkitpro" "$DKP" \
  --include='*.pc' --include='*.la' --include='*.cmake' \
  --include='*.specs' --include='*.sh' --include='*.py' 2>/dev/null |
  while read -r f; do
    sed -i -e "s|$DKP|@@DKP@@|g" -e "s|/opt/devkitpro|$DKP|g" -e "s|@@DKP@@|$DKP|g" "$f"
  done

ln -sf "$(command -v cmake)" "$TC/bin/cmake"
ln -sf "$(command -v ninja)" "$TC/bin/ninja"

cat > "$TC/env.sh" <<EOF
# BattleShip contained Switch toolchain - source per session; NEVER add to a shell profile.
export DEVKITPRO="$DKP"
export DEVKITA64="$DKP/devkitA64"
export PATH="$TC/bin:\$DEVKITA64/bin:\$DEVKITPRO/tools/bin:\$PATH"
export CMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
export CMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON
EOF

echo "== validacao =="
"$DKP/devkitA64/bin/aarch64-none-elf-gcc" --version | head -1
"$DKP/tools/bin/elf2nro" --help >/dev/null 2>&1 && echo "elf2nro OK"
echo "Toolchain Switch pronta: scripts/switch/build.sh"
