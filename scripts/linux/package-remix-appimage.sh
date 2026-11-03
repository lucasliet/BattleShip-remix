#!/usr/bin/env bash
# Host driver: builds BattleShip-Remix for Linux x86_64 in a podman container and packs a personal-test AppImage
# (it embeds the user's ROM-derived .o2r; see package-in-container.sh).
#
# Usage: scripts/linux/package-remix-appimage.sh [build|package|all]   (default all)
# Env:   DIST_VERSION (default 2.0.1-port.<short HEAD>), IMAGE (default localhost/ssb64-linux-jammy-amd64),
#        WORK_VOLUME (default ssb64-linux-work; persistent source copy + build tree, so rebuilds are incremental).
# Needs: podman with an amd64 machine (emulated on Apple silicon), build/BattleShip-Remix.o2r extracted on the host.
# Output: build-linux-release/BattleShip-Remix and build-linux-release/BattleShip-Remix-<version>-linux-x86_64.AppImage
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
STEP="${1:-all}"
IMAGE="${IMAGE:-localhost/ssb64-linux-jammy-amd64}"
WORK_VOLUME="${WORK_VOLUME:-ssb64-linux-work}"
DIST_VERSION="${DIST_VERSION:-2.0.1-port.$(git -C "$ROOT" rev-parse --short HEAD)}"
OUT="$ROOT/build-linux-release"

case "$STEP" in
  build|package|all) ;;
  *) echo "ERROR: step '$STEP' is not one of build, package, all" >&2; exit 2 ;;
esac
command -v podman >/dev/null || { echo "ERROR: podman not found on PATH" >&2; exit 1; }
mkdir -p "$OUT"

podman build --platform linux/amd64 -t "$IMAGE" -f "$ROOT/scripts/linux/Dockerfile.jammy" "$ROOT/scripts/linux"

run_in_container() {
  podman run --rm --platform linux/amd64 \
    -v "$ROOT:/src:ro" -v "$OUT:/out" -v "$WORK_VOLUME:/work" -v "$ROOT/scripts/linux:/scripts:ro" \
    -e BATTLESHIP_VERSION="$DIST_VERSION" -e DIST_VERSION="$DIST_VERSION" -e ARCH_TAG=x86_64 \
    "$IMAGE" bash "/scripts/$1"
}

if [[ "$STEP" == build || "$STEP" == all ]]; then
  run_in_container build-in-container.sh
fi
if [[ "$STEP" == package || "$STEP" == all ]]; then
  [[ -f "$ROOT/build/BattleShip-Remix.o2r" ]] || { echo "ERROR: missing $ROOT/build/BattleShip-Remix.o2r (run ExtractAssets)" >&2; exit 1; }
  run_in_container package-in-container.sh
fi
