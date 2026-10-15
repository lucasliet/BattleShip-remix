#!/usr/bin/env bash
# Usage: css.sh <input_script> <name> [frames]
# Boots into the VS character select (scene 16) and plays the input script: pick fighters, START, stage select, A.
# Env: SHOTS (frame list, default 300).
set -u
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
dir="$ROOT/build/agent-shots/vs/$2"
mkdir -p "$dir"
input="$(builtin cd "$(dirname "$1")" && pwd)/$(basename "$1")"
rm -f "$dir"/frame_*.png
builtin cd "$ROOT/build" || exit 2
frames=${3:-900}
SSB64_START_SCENE=16 SSB64_INPUT_SCRIPT=$input SSB64_MAX_FRAMES=$frames \
  SSB64_SAVE_PATH=$dir/save.bin SSB64_SCREENSHOT_FRAMES=${SHOTS:-300} SSB64_SCREENSHOT_DIR=$dir \
  timeout 300 ./BattleShip-Remix >"$dir/stdout.log" 2>&1
rc=$?
cp "$HOME/Library/Application Support/BattleShip/ssb64.log" "$dir/ssb64.log" 2>/dev/null
echo "css=$2 rc=$rc shots=$(ls "$dir"/*.png 2>/dev/null | wc -l | tr -d ' ')"
