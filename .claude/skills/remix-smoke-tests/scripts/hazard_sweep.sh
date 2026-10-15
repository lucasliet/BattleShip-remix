#!/usr/bin/env bash
# Usage: hazard_sweep.sh <stage id decimal> [...]
# Mario (human, idle) + Kirby + Yoshi CPUs on each stage for 4000 frames, a screenshot every 150 frames from 300
# (25 shots, one 5x5 contact sheet per stage via sheet.py). Hazards act on timers (Onett car about frame 2700, Corneria
# Great Fox volleys around 2430), so the run is long on purpose.
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(builtin cd "$HERE/../../../.." && pwd)"
for s in "$@"; do
  SSB64_VS_STAGE=$s SHOTS=$(seq -s, 300 150 3900) bash "$HERE/vs.sh" 0,8,6 "$HERE/../inputs/none.txt" "hz_$s" 4000 2>&1 | tail -1
  "$ROOT/.toolchain/host/venv/bin/python" -I "$HERE/sheet.py" "hz_$s" --max 25 >/dev/null
  grep -h "remix-stage\|\[remix-" "$ROOT/build/agent-shots/vs/hz_$s/ssb64.log" | sort | uniq -c | head -5
done
echo HAZARD_DONE
