#!/usr/bin/env bash
# Usage: stage_sweep.sh [first_id] [last_id] [prefix]
# Runs every stage id (decimal, default 41..215) for 480 frames with Mario vs Mario and one screenshot at frame 450.
# Prints one line per stage; a line without rc=0 is a crash (rerun it alone before investigating: Metal flake).
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
first=${1:-41}
last=${2:-215}
prefix=${3:-st}
for s in $(seq "$first" "$last"); do
  SSB64_VS_STAGE=$s SHOTS=450 bash "$HERE/vs.sh" 0,0 "$HERE/../inputs/none.txt" "${prefix}_$s" 480 2>&1 | tail -1
done
echo STAGESWEEP_DONE
