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

APP="BattleShip"
TARGET_DIR="$SD/switch/$APP"

[[ -f "$BUILD_DIR/ssb64.nro" ]] || { echo "ERRO: NRO ausente; rode scripts/switch/build.sh antes." >&2; exit 1; }

mkdir -p "$TARGET_DIR"
cp "$BUILD_DIR/ssb64.nro" "$TARGET_DIR/$APP.nro"

# Runtime archives generated host-side (user's own ROM). Required on the SD.
for f in BattleShip.o2r BattleShip.o2r.recipe f3d.o2r; do
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

cat > "$TARGET_DIR/LEIA-ME.txt" <<EOF
$APP — Nintendo Switch (homebrew)

1. Copie a pasta switch/ para a raiz do cartao SD.
2. Os arquivos .o2r sao derivados da SUA copia do jogo (nenhum asset distribuido).
   Se faltam, gere-os numa maquina com o build desktop e copie para switch/$APP/.
3. Execute via Homebrew Menu em modo full-memory (segure R ao abrir um jogo
   instalado) — applet mode nao tem RAM suficiente.
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
