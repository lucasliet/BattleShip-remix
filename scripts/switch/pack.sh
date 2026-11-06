#!/bin/zsh
# BattleShip — packaging for Nintendo Switch.
#
# 1. dist/switch/SD layout: everything the user copies to the SD card.
# 2. dist/battleship-switch-sdk.tar.zst: repo + .toolchain bundle for an
#    offline, self-contained build on another machine.
#
# Usage: scripts/switch/pack.sh [--sdk]

set -euo pipefail

ROOT=${0:A:h:h:h}
BUILD_DIR="$ROOT/build-switch"
SD="$ROOT/dist/switch/SD"

VER="${VERSION:-remix}"
[[ "$VER" == "us" || "$VER" == "jp" || "$VER" == "remix" ]] || { echo "VERSION must be us|jp|remix (got '$VER')" >&2; exit 1; }
if [[ "$VER" == "remix" ]]; then APP="BattleShip-Remix"; else APP="BattleShip"; fi
TARGET_DIR="$SD/switch/$APP"

[[ -f "$BUILD_DIR/$APP.nro" ]] || { echo "ERRO: NRO ausente; rode scripts/switch/build.sh antes." >&2; exit 1; }

mkdir -p "$TARGET_DIR"
cp "$BUILD_DIR/$APP.nro" "$TARGET_DIR/$APP.nro"

# Runtime archives generated host-side (user's own ROM). Required on the SD.
for f in "$APP.o2r" "$APP.o2r.recipe" f3d.o2r; do
  src="$ROOT/build/$f"
  [[ -f "$src" ]] || src="$ROOT/$f"
  if [[ -f "$src" ]]; then cp "$src" "$TARGET_DIR/"; else
    echo "AVISO: $f não encontrado — gere com o build desktop (ExtractAssets) e copie para $TARGET_DIR/" >&2
  fi
done

for extra in gamecontrollerdb.txt config.yml; do
  [[ -f "$ROOT/$extra" ]] && cp "$ROOT/$extra" "$TARGET_DIR/"
done
[[ -d "$ROOT/assets" ]] && cp -a "$ROOT/assets" "$TARGET_DIR/"

cat > "$TARGET_DIR/README.txt" <<EOF
$APP for Nintendo Switch (homebrew)

1. Copy the switch/ folder to the root of the SD card.
2. The .o2r files come from YOUR copy of the game (no assets are distributed).
   If they are missing, extract them with the desktop build and copy them to switch/$APP/.
3. Launch from the Homebrew Menu in full-memory mode (hold R while opening an
   installed game). Applet mode does not have enough memory.
EOF

echo "Pacote SD: $SD"

if [[ "${1:-}" == "--sdk" ]]; then
  OUT="$ROOT/dist/battleship-switch-sdk.tar.zst"
  echo "== sdk bundle: $OUT (repo + .toolchain) =="
  tar -cJf "$OUT" -C "$ROOT" --exclude '.git' --exclude 'build*' --exclude 'dist' \
    .toolchain scripts/switch CMakeLists.txt cmake config.yml yamls tools \
    libultraship torch decomp port include assets docs .gitmodules .gitignore 2>/dev/null ||
  tar -cJf "$OUT" -C "$ROOT" .toolchain scripts/switch
  ls -la "$OUT"
fi
