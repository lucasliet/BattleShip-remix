#!/usr/bin/env bash
# Usage: vs.sh <fkinds> <input_script> <name> [frames]
#   fkinds: comma list of up to 4 port fkinds (decimal), P1 human, the rest CPU (e.g. 0,8 or 0,8,6)
#   input_script: SSB64_INPUT_SCRIPT file (inputs/none.txt for no input)
#   name: output folder under build/agent-shots/vs/
# Env passed through: SHOTS (frame list, default 600,900,...), SSB64_VS_STAGE, SSB64_VS_ITEMS,
# SSB64_REMIX_FORCE_ITEM, SSB64_REMIX_FORCE_ITEM_AT_P1, SSB64_REMIX_ITEM_LOG, SSB64_DUMP_FILE_ID.
set -u
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
dir="$ROOT/build/agent-shots/vs/$3"
mkdir -p "$dir"
input="$(builtin cd "$(dirname "$2")" && pwd)/$(basename "$2")"
rm -f "$dir"/frame_*.png
builtin cd "$ROOT/build" || exit 2
frames=${4:-2200}
SSB64_START_SCENE=22 SSB64_VS_FKINDS=$1 SSB64_INPUT_SCRIPT=$input SSB64_MAX_FRAMES=$frames \
  SSB64_SAVE_PATH=$dir/save.bin SSB64_SCREENSHOT_FRAMES=${SHOTS:-600,900,1200,1500,1800,2100} SSB64_SCREENSHOT_DIR=$dir \
  timeout 300 ./BattleShip-Remix >"$dir/stdout.log" 2>&1
rc=$?
cp "$HOME/Library/Application Support/BattleShip/ssb64.log" "$dir/ssb64.log" 2>/dev/null
flake=$(grep -c "nextDrawable" "$dir/stdout.log" 2>/dev/null)
echo "vs=$3 rc=$rc shots=$(ls "$dir"/*.png 2>/dev/null | wc -l | tr -d ' ') metal_flake=${flake:-0}"
