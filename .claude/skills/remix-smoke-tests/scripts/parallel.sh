#!/usr/bin/env bash
# Usage: parallel.sh <jobs> <mode> [args...]     (JOBS = game instances at once, 3 or 4 is the sweet spot on a 16 GB Mac)
#   matrix <fkind>...            NSP/DSP/USP of each fighter (special_matrix.sh logic, one instance per move)
#   stages [first] [last]        stage sweep, default 41..215
#   items  [kind]...             CPU item sweep (item_cpu_sweep.sh logic)
#   hazards <stage>...           hazard_sweep.sh logic, 4000 frames
#   custom <file>                one shell command per line of <file> (vs.sh / css.sh invocations), e.g. ad-hoc scenarios
# Every instance writes its own log (SSB64_LOG_PATH) and save, so runs do not collide. Windows still open in front.
# Output: one line per run, then <MODE>_DONE. Error lines of the matrix are counted per run.
set -u
HERE="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(builtin cd "$HERE/../../../.." && pwd)"
JOBS="$1"; MODE="$2"; shift 2
JOBDIR="$(mktemp -d)"
LIST="$JOBDIR/list.txt"
case "$MODE" in
  matrix)
    for f in "$@"; do for m in nsp dsp usp; do
      echo "SHOTS=450,520,600,700,800,900 bash $HERE/vs.sh $f,8 $HERE/../inputs/${m}_tap.txt m_${f}_${m} 1000" >> "$LIST"
    done; done ;;
  stages)
    first="${1:-41}"; last="${2:-215}"
    # BTT/BTP-class stages and the Rest Area run the bonus ground (Stages.asm bonus_fix_1_), whose item file only the
    # bonus scene loads: they are not VS stages, test them through scene 52 with SSB64_SPGAME_STAGE=3|7 and their owner.
    bonus_ids=" 77 78 79 81 82 83 84 86 89 100 101 122 123 127 129 135 137 138 139 160 161 168 170 172 175 178 183 185 194 197 203 206 207 "
    for s in $(seq "$first" "$last"); do
      case "$bonus_ids" in *" $s "*) continue ;; esac
      echo "SSB64_VS_STAGE=$s SHOTS=450 bash $HERE/vs.sh 0,8 $HERE/../inputs/none.txt st_$s 480" >> "$LIST"
    done ;;
  items)
    for k in ${@:-46 47 48 49 50 51 52 53 54 55 57 64 65}; do
      echo "SSB64_REMIX_ITEM_LOG=1 SSB64_VS_ITEMS=5 SSB64_REMIX_FORCE_ITEM=$k SHOTS=\$(seq -s, 600 60 2940) bash $HERE/vs.sh 0,8,6 $HERE/../inputs/none.txt cpu_$k 3000" >> "$LIST"
    done ;;
  hazards)
    for s in "$@"; do
      echo "SSB64_VS_STAGE=$s SHOTS=\$(seq -s, 300 150 3900) bash $HERE/vs.sh 0,8,6 $HERE/../inputs/none.txt hz_$s 4000" >> "$LIST"
    done ;;
  custom)
    cp "$1" "$LIST" ;;
  *) echo "unknown mode $MODE"; exit 2 ;;
esac
n=0
while IFS= read -r line; do
  n=$((n + 1))
  printf '%s\n' "$line" > "$JOBDIR/job_$(printf '%04d' "$n").sh"
done < "$LIST"
rm -f "$LIST"
ls "$JOBDIR"/job_*.sh | xargs -P "$JOBS" -n1 bash 2>&1 | grep -E "^vs=|^css="
rm -rf "$JOBDIR"
if [ "$MODE" = matrix ]; then
  for f in "$@"; do for m in nsp dsp usp; do
    n=$(grep -ciE 'abort|assert|fatal|unhandled' "$ROOT/build/agent-shots/vs/m_${f}_${m}/ssb64.log" 2>/dev/null)
    [ "${n:-0}" != "0" ] && echo "  m_${f}_${m}: $n error lines in ssb64.log"
  done; done
fi
echo "$(echo "$MODE" | tr a-z A-Z)_DONE"
