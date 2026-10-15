#!/usr/bin/env bash
# Usage: item_cpu_sweep.sh [kind ...]   (Remix item kinds in decimal; default: the 13 standard items)
# Mario (human, idle) + Kirby + Yoshi (CPUs pick items up), every spawned item replaced by <kind>, fast spawn,
# 3000 frames, a screenshot every 60 frames, and the item event log counted per event.
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(builtin cd "$HERE/../../../.." && pwd)"
kinds="${*:-46 47 48 49 50 51 52 53 54 55 57 64 65}"
for k in $kinds; do
  SSB64_REMIX_ITEM_LOG=1 SSB64_VS_ITEMS=5 SSB64_REMIX_FORCE_ITEM=$k SHOTS=$(seq -s, 600 60 2940) \
    bash "$HERE/vs.sh" 0,8,6 "$HERE/../inputs/none.txt" "cpu_$k" 3000 2>&1 | tail -1
  echo "  cpu_$k events: $(grep '\[remix-item\]' "$ROOT/build/agent-shots/vs/cpu_$k/ssb64.log" | awk '{print $2}' | sort | uniq -c | tr '\n' ' ')"
done
echo CPU_DONE
