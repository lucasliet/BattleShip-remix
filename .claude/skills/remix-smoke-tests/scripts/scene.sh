#!/usr/bin/env bash
# Usage: scene.sh <scene> <input_script> <name> [frames] [timeout_s]
# Boots into scene <scene> (SCKind enum, decomp/src/sc/scdef.h: 9 VS mode menu, 16 VS CSS, 17 1P CSS, 18 Training CSS,
# 19/20 Bonus CSS, 24 VS results, 27 N64 logo, 52 1P game, 53 bonus stage, 54 Training, 55 Congratulations, 56 credits,
# 58 Data menu, 59 Sound Test, 61 Auto-Demo) and plays the input script. Other SSB64_* variables of the caller pass
# through (SSB64_SPGAME_STAGE, SSB64_REMIX_1P_MODE, SSB64_REMIX_PANEL, SSB64_REMIX_TOGGLES, ...).
# Env: SHOTS (frame list, default 300).
set -u
if [ $# -lt 3 ]; then
  echo "usage: scene.sh <scene> <input_script> <name> [frames] [timeout_s]" >&2
  exit 2
fi
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
dir="$ROOT/build/agent-shots/vs/$3"
mkdir -p "$dir"
input="$(builtin cd "$(dirname "$2")" && pwd)/$(basename "$2")"
rm -f "$dir"/frame_*.png
builtin cd "$ROOT/build" || exit 2
frames=${4:-900}
limit=${5:-300}
SSB64_START_SCENE=$1 SSB64_INPUT_SCRIPT=$input SSB64_MAX_FRAMES=$frames \
  SSB64_LOG_PATH=$dir/ssb64.log SSB64_SAVE_PATH=$dir/save.bin SSB64_SCREENSHOT_FRAMES=${SHOTS:-300} SSB64_SCREENSHOT_DIR=$dir \
  timeout "$limit" ./BattleShip-Remix >"$dir/stdout.log" 2>&1
rc=$?
echo "scene=$1 run=$3 rc=$rc shots=$(ls "$dir"/*.png 2>/dev/null | wc -l | tr -d ' ') metal_flake=$(grep -c 'CAMetalLayer nextDrawable' "$dir/stdout.log")"
