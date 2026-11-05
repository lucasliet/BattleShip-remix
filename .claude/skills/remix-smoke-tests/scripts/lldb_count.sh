#!/usr/bin/env bash
# Usage: lldb_count.sh <name> <fkinds|-> <input_script> <frames> <spec>...
#   Same environment as vs.sh (VS shortcut, scene 22) but under system lldb: every <spec> becomes an auto-continue
#   breakpoint and the hit count of each is printed on the single "vs=" line (so parallel.sh keeps it).
#   spec:  <function>              breakpoint by exported function name
#          <function>@<condition>  with an lldb condition, e.g. ftRemixThrownRulesKeepsStatusAtAnimEnd@'$x0 == 52'
#          <file.c>:<line>         breakpoint by file and line (static functions)
#   fkinds "-" skips SSB64_VS_FKINDS (use it with LLDB_START_SCENE=54 SSB64_TRAIN_FKINDS=... or LLDB_START_SCENE=16).
# Env: LLDB_START_SCENE (default 22), LLDB_TIMEOUT seconds (default 420), SHOTS, any SSB64_* variable of the caller.
#      LLDB_PRE: extra lldb commands separated by "|", added after the spec breakpoints and before "run" (their breakpoints list last, unlabelled).
# Output files: build/agent-shots/vs/<name>/{lldb.cmds,lldb.out,ssb64.log,stdout.log}.
# Hit counts of 0 with "(unresolved)" mean the symbol was not found (static or inlined): use <file.c>:<line>.
set -u
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
name="$1"; fkinds="$2"; input="$3"; frames="$4"; shift 4
dir="$ROOT/build/agent-shots/vs/$name"
mkdir -p "$dir"
rm -f "$dir"/frame_*.png
input="$(builtin cd "$(dirname "$input")" && pwd)/$(basename "$input")"
cmds="$dir/lldb.cmds"
labels="$dir/lldb.labels"
: > "$cmds"; : > "$labels"
echo "process handle SIGSEGV SIGBUS -s true -p false" >> "$cmds"
for spec in "$@"; do
  fn="${spec%%@*}"
  cond=""
  case "$spec" in *@*) cond="${spec#*@}" ;; esac
  if printf '%s' "$fn" | grep -qE '^[A-Za-z0-9_]+\.c:[0-9]+$'; then
    setcmd="breakpoint set -f ${fn%%:*} -l ${fn##*:}"
  else
    setcmd="breakpoint set -n $fn"
  fi
  if [ -n "$cond" ]; then
    setcmd="$setcmd -c '$cond'"
  fi
  echo "$setcmd -G true" >> "$cmds"
  printf '%s\n' "$spec" | tr -c 'A-Za-z0-9_.:\n' '_' >> "$labels"
done
if [ -n "${LLDB_PRE:-}" ]; then printf '%s\n' "$LLDB_PRE" | tr '|' '\n' >> "$cmds"; fi
{ echo "run"; echo "breakpoint list"; echo "quit"; } >> "$cmds"
builtin cd "$ROOT/build" || exit 2
start_scene="${LLDB_START_SCENE:-22}"
if [ "$fkinds" = "-" ]; then fk_env=""; else fk_env="SSB64_VS_FKINDS=$fkinds"; fi
env SSB64_START_SCENE="$start_scene" ${fk_env:+$fk_env} SSB64_INPUT_SCRIPT="$input" SSB64_MAX_FRAMES="$frames" \
  SSB64_LOG_PATH="$dir/ssb64.log" SSB64_SAVE_PATH="$dir/save.bin" \
  SSB64_SCREENSHOT_FRAMES="${SHOTS:-600}" SSB64_SCREENSHOT_DIR="$dir" \
  timeout -s INT -k 30 "${LLDB_TIMEOUT:-420}" lldb -b -s "$cmds" ./BattleShip-Remix > "$dir/lldb.out" 2> "$dir/stdout.log"
rc=$?
summary="$(grep -E '^[[:space:]]*[0-9]+: ' "$dir/lldb.out" | awk -v labels="$labels" '
  BEGIN { n = 0; while ((getline line < labels) > 0) { n++; L[n] = line } }
  { i++; hits = 0; loc = 0
    if (match($0, /locations = [0-9]+/)) loc = substr($0, RSTART + 12, RLENGTH - 12)
    if (match($0, /hit count = [0-9]+/)) hits = substr($0, RSTART + 12, RLENGTH - 12)
    printf " %s=%s%s", L[i], hits, (loc == 0 ? "(unresolved)" : "") }')"
echo "vs=$name rc=$rc shots=$(ls "$dir"/*.png 2>/dev/null | wc -l | tr -d ' ') hits:$summary"
