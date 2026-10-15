#!/usr/bin/env bash
# Usage: special_matrix.sh <fkind> [fkind ...]
# For each fighter: NSP, DSP and USP every 150 frames against Kirby (fkind 8), 1000 frames, 6 screenshots.
# Then greps each ssb64.log for abort/assert/fatal/unhandled.
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(builtin cd "$HERE/../../../.." && pwd)"
for f in "$@"; do
  for m in nsp dsp usp; do
    SHOTS=450,520,600,700,800,900 bash "$HERE/vs.sh" "$f,8" "$HERE/../inputs/${m}_tap.txt" "m_${f}_${m}" 1000 2>&1 | tail -1
    n=$(grep -ciE 'abort|assert|fatal|unhandled' "$ROOT/build/agent-shots/vs/m_${f}_${m}/ssb64.log")
    [ "$n" != "0" ] && echo "  m_${f}_${m}: $n error lines in ssb64.log"
  done
done
echo MATRIX_DONE
