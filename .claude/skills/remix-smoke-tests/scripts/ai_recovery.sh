#!/usr/bin/env bash
# Usage: ai_recovery.sh [runs] [jobs] [frames]
#   runs:   number of four-CPU matches (default 10, cycles through 10 fighter/stage sets)
#   jobs:   game instances at once (default 4)
#   frames: SSB64_MAX_FRAMES per match (default 14000, long enough for a 3-stock match at level 9)
# Every match is level 9, 3 stocks, all four ports CPU (SSB64_VS_ALL_CPU=1), with SSB64_REMIX_AI_LOG=1.
# Output: one line per run (episodes ok ko open) and the total `episodes=<n> ok=<n> ko=<n> open=<n>`.
# open = episodes without a recover_end (still airborne at the end of the run, or a lost end event).
# Env: STAGES="6 0 3 ..." replaces the stage list; VS_TIMEOUT=<seconds> per match (default 900).
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(builtin cd "$HERE/../../../.." && pwd)"
RUNS="${1:-10}"
JOBS="${2:-4}"
FRAMES="${3:-14000}"
LEVEL=9
STOCKS=3

FKINDS=(
  "0,1,2,3"
  "4,5,6,7"
  "8,9,10,11"
  "29,33,52,55"
  "56,58,59,62"
  "63,64,68,72"
  "73,0,9,55"
  "6,52,5,29"
  "2,59,10,68"
  "7,58,11,33"
)
STAGE_LIST=(${STAGES:-6 0 3 7 5 8 2 4 1 41})

JOBFILE="$(mktemp)"
for i in $(seq 1 "$RUNS"); do
  fkinds="${FKINDS[$(( (i - 1) % ${#FKINDS[@]} ))]}"
  stage="${STAGE_LIST[$(( (i - 1) % ${#STAGE_LIST[@]} ))]}"
  name="ai_rec_$i"
  rm -f "$ROOT/build/agent-shots/vs/$name/ssb64.log"
  echo "SSB64_VS_ALL_CPU=1 SSB64_VS_CPU_LEVEL=$LEVEL SSB64_VS_STOCKS=$STOCKS SSB64_VS_STAGE=$stage SSB64_REMIX_AI_LOG=1 SHOTS=600 AUDIO_SECONDS=1 VS_TIMEOUT=${VS_TIMEOUT:-900} bash $HERE/vs.sh $fkinds $HERE/../inputs/none.txt $name $FRAMES" >> "$JOBFILE"
done

bash "$HERE/parallel.sh" "$JOBS" custom "$JOBFILE"
rm -f "$JOBFILE"

total_begin=0; total_ok=0; total_ko=0
for i in $(seq 1 "$RUNS"); do
  log="$ROOT/build/agent-shots/vs/ai_rec_$i/ssb64.log"
  begin=$(grep -c "\[remix-ai\] recover_begin" "$log" 2>/dev/null)
  ok=$(grep -c "\[remix-ai\] recover_end .*result=ok" "$log" 2>/dev/null)
  ko=$(grep -c "\[remix-ai\] recover_end .*result=ko" "$log" 2>/dev/null)
  begin=${begin:-0}; ok=${ok:-0}; ko=${ko:-0}
  echo "ai_rec_$i episodes=$begin ok=$ok ko=$ko open=$(( begin - ok - ko ))"
  total_begin=$(( total_begin + begin )); total_ok=$(( total_ok + ok )); total_ko=$(( total_ko + ko ))
done
echo "episodes=$total_begin ok=$total_ok ko=$total_ko open=$(( total_begin - total_ok - total_ko ))"
