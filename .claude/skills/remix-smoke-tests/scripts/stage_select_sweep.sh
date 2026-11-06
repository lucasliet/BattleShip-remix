#!/usr/bin/env bash
# Usage: stage_select_sweep.sh [gkind ...]
# Opens the VS stage select on each stage (SSB64_MAPS_START_GKIND) and screenshots it at frame 300; the default list is
# the 85 stages of Stages.stage_table_normal. Frames land in build/agent-shots/vs/ss_<gkind>/frame_300.png.
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIST="${*:-0 2 4 3 8 13 5 6 1 7 14 16 53 42 9 11 49 74 43 61 52 51 67 62 41 64 136 153 71 75 55 48 44 45 46 76 190 93 54 57 56 90 92 69 68 72 73 47 85 88 94 50 95 96 99 111 124 125 126 131 132 133 134 141 142 156 157 158 159 162 163 166 176 179 180 186 187 191 192 193 195 198 199 202 210}"
for g in $LIST; do
  SSB64_MAPS_START_GKIND=$g SHOTS=300 bash "$HERE/css.sh" "$HERE/../inputs/css_pick_wait.txt" "ss_$g" 310 | tail -1
done
echo STAGESELECT_DONE
