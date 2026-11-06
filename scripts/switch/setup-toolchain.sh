#!/bin/zsh
# Contained Nintendo Switch toolchain bootstrap for BattleShip.
#
# Downloads and extracts devkitA64 + portlibs from pkg.devkitpro.org into
# .toolchain/opt/devkitpro (never /opt, never the system), pins every tarball
# against scripts/switch/packages.lock, extracts host cmake/ninja into
# .toolchain/host, relocates hardcoded prefixes, and validates the compiler.
#
# Zero persistent system state: everything lives inside the repository.
# Idempotent: re-run at any time; already-verified artifacts are kept.
#
# Usage: scripts/switch/setup-toolchain.sh [--offline]
#   --offline  use only .toolchain/pkgs cache (no network)

set -euo pipefail

ROOT=${0:A:h:h:h}
TC="$ROOT/.toolchain"
DKP="$TC/opt/devkitpro"
PKGS="$TC/pkgs"
HOST="$TC/host"
LOCK="$ROOT/scripts/switch/packages.lock"
OFFLINE=0
[[ "${1:-}" == "--offline" ]] && OFFLINE=1

mkdir -p "$PKGS" "$HOST" "$DKP"

# curl flags: pkg.devkitpro.org's Cloudflare front rejects the default macOS
# curl HTTP/2 fingerprint (devkitPro/pacman#63); HTTP/1.1 + dkp-pacman UA works.
fetch() { # fetch <url> <out>
  if (( OFFLINE )); then return 1; fi
  curl --http1.1 -fsSL --retry 3 -A dkp-pacman/6.0.2 -o "$2" "$1"
}

# Emit "<repo> <filename> <sha256> <url>" lines for the dkp packages.
dkp_manifest() {
  python3 - "$LOCK" <<'PY'
import json, sys
lock = json.load(open(sys.argv[1]))
base = lock["base_urls"]
for repo in ("osx", "libs"):
    for name, p in sorted(lock["packages"][repo].items()):
        url = f"{base['libs']}/{p['filename']}" if repo == "libs" else f"{base['osx']}/{p['filename']}"
        print(f"{repo} {p['filename']} {p['sha256']} {url}")
PY
}

# --- devkitPro packages -----------------------------------------------------
echo "== devkitPro packages =="
missed=()
while read -r repo file sha url; do
  if [[ -f "$PKGS/$file" ]]; then
    echo "  [cache] $file"
  else
    echo "  [fetch] $file"
    fetch "$url" "$PKGS/$file" || missed+=("$file|$url")
  fi
done < <(dkp_manifest)

# Verify every cached tarball against the pinned sha256.
bad=()
while read -r repo file sha url; do
  [[ -f "$PKGS/$file" ]] || continue
  if ! echo "$sha  $file" | (cd "$PKGS" && shasum -a 256 -c -) >/dev/null 2>&1; then
    bad+=("$file")
  fi
done < <(dkp_manifest)
if (( ${#bad[@]} > 0 )); then
  echo "ERRO: sha256 divergente, apagando para re-download: ${bad[*]}" >&2
  for f in $bad; do rm -f "$PKGS/$f"; done
  exit 1
fi

if (( ${#missed[@]} > 0 )); then
  echo >&2
  echo "Alguns pacotes nao puderam ser baixados (rede/Cloudflare)." >&2
  echo "Baixe manualmente no navegador e salve em .toolchain/pkgs/, depois rode de novo:" >&2
  for entry in $missed; do echo "  ${entry#*|}" >&2; done
  exit 2
fi

# --- extract into $DKP ------------------------------------------------------
echo "== extraindo para $DKP =="
tmp="$TC/.extract-tmp"
marks="$TC/.extracted"
mkdir -p "$marks"
while read -r repo file sha url; do
  [[ -f "$marks/$file" ]] && continue
  echo "  [extract] $file"
  rm -rf "$tmp"; mkdir -p "$tmp"
  tar -xf "$PKGS/$file" -C "$tmp"
  if [[ -d "$tmp/opt/devkitpro" ]]; then
    cp -a "$tmp/opt/devkitpro/." "$DKP/"
  else
    cp -a "$tmp/." "$DKP/"
  fi
  touch "$marks/$file"
done < <(dkp_manifest)
rm -rf "$tmp"

# --- host cmake / ninja -----------------------------------------------------
echo "== host cmake/ninja =="
python3 - "$LOCK" <<'PY' > "$TC/.host-manifest"
import json, sys
lock = json.load(open(sys.argv[1]))
for h in lock["host"]:
    print(f"{h['file']} {h['sha256']} {h['url']}")
PY
missed=()
while read -r file sha url; do
  if [[ ! -f "$PKGS/$file" ]]; then
    echo "  [fetch] $file"
    fetch "$url" "$PKGS/$file" || { missed+=("$url"); continue; }
  fi
  echo "$sha  $file" | (cd "$PKGS" && shasum -a 256 -c -) >/dev/null || { echo "sha256 divergente: $file" >&2; exit 1; }
done < "$TC/.host-manifest"
if (( ${#missed[@]} > 0 )); then
  echo "Baixe manualmente e salve em .toolchain/pkgs/:" >&2
  for u in $missed; do echo "  $u" >&2; done
  exit 2
fi

[[ -x "$HOST/ninja/ninja" ]] || unzip -oq "$PKGS/ninja-mac.zip" -d "$HOST/ninja"
[[ -d "$HOST/cmake" ]] || {
  tar -xzf "$PKGS/cmake-3.31.6-macos-universal.tar.gz" -C "$HOST"
  mv "$HOST/cmake-3.31.6-macos-universal" "$HOST/cmake"
}

# --- relocate + env ---------------------------------------------------------
"$ROOT/scripts/switch/relocate-toolchain.sh"

mkdir -p "$TC/bin"
ln -sf "$HOST/cmake/CMake.app/Contents/bin/cmake" "$TC/bin/cmake"
ln -sf "$HOST/ninja/ninja" "$TC/bin/ninja"

cat > "$TC/env.sh" <<EOF
# BattleShip contained Switch toolchain — source por sessão; NUNCA adicionar a shell profile.
export DEVKITPRO="$DKP"
export DEVKITA64="$DKP/devkitA64"
export PATH="$TC/bin:\$DEVKITA64/bin:\$DEVKITPRO/tools/bin:\$PATH"
# Impede que o CMake consulte/registre o registry do usuário (~/.cmake).
export CMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
export CMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON
EOF

# --- validate ---------------------------------------------------------------
echo "== validação =="
source "$TC/env.sh"
"$DEVKITA64/bin/aarch64-none-elf-gcc" --version | head -1
"$DEVKITPRO/tools/bin/elf2nro" --help >/dev/null 2>&1 && echo "elf2nro OK"
cmake --version | head -1
ninja --version
echo "Toolchain Switch contida pronta: source .toolchain/env.sh antes de buildar."
