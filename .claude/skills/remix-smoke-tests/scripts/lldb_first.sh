#!/usr/bin/env bash
# Usage: lldb_first.sh <name> <fkinds|-> <input_script> <frames> <function>[@<condition>] [extra lldb command]...
#   Stops at the first hit of <function> (optionally with an lldb condition such as '$x0 == 44'), prints the argument
#   registers, runs "finish" and prints the return registers. The single "vs=" line carries the values so parallel.sh keeps it.
#   x0/x1/x2 are integer or pointer arguments and return value; s0/s1 are float arguments and return value (arm64).
# Env: LLDB_START_SCENE (default 22), LLDB_TIMEOUT seconds (default 420), SHOTS, any SSB64_* variable of the caller.
# Full output: build/agent-shots/vs/<name>/lldb.out.
set -u
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
name="$1"; fkinds="$2"; input="$3"; frames="$4"; spec="$5"; shift 5
dir="$ROOT/build/agent-shots/vs/$name"
mkdir -p "$dir"
rm -f "$dir"/frame_*.png
input="$(builtin cd "$(dirname "$input")" && pwd)/$(basename "$input")"
cmds="$dir/lldb.cmds"
fn="${spec%%@*}"
cond=""
case "$spec" in *@*) cond="${spec#*@}" ;; esac
if printf '%s' "$fn" | grep -qE '^[A-Za-z0-9_]+\.c:[0-9]+$'; then
  setcmd="breakpoint set -f ${fn%%:*} -l ${fn##*:}"
else
  setcmd="breakpoint set -n $fn"
fi
[ -n "$cond" ] && setcmd="$setcmd -c '$cond'"
{
  echo "process handle SIGSEGV SIGBUS -s true -p false"
  echo "$setcmd"
  echo "run"
  echo "platform shell echo FIRST_HIT_ARGS"
  echo "register read x0 x1 x2 s0 s1"
  for extra in "$@"; do echo "$extra"; done
  echo "finish"
  echo "platform shell echo FIRST_HIT_RETURN"
  echo "register read x0 s0"
  echo "kill"
  echo "quit"
} > "$cmds"
builtin cd "$ROOT/build" || exit 2
start_scene="${LLDB_START_SCENE:-22}"
if [ "$fkinds" = "-" ]; then fk_env=""; else fk_env="SSB64_VS_FKINDS=$fkinds"; fi
env SSB64_START_SCENE="$start_scene" ${fk_env:+$fk_env} SSB64_INPUT_SCRIPT="$input" SSB64_MAX_FRAMES="$frames" \
  SSB64_LOG_PATH="$dir/ssb64.log" SSB64_SAVE_PATH="$dir/save.bin" \
  SSB64_SCREENSHOT_FRAMES="${SHOTS:-600}" SSB64_SCREENSHOT_DIR="$dir" \
  timeout -s INT -k 30 "${LLDB_TIMEOUT:-420}" lldb -b -s "$cmds" ./BattleShip-Remix > "$dir/lldb.out" 2> "$dir/stdout.log"
rc=$?
args="$(sed -n '/FIRST_HIT_ARGS/,/FIRST_HIT_RETURN/p' "$dir/lldb.out" | grep -E '^[[:space:]]*(x0|x1|x2|s0|s1) =' | tr -s ' ' | tr '\n' ';')"
ret="$(sed -n '/FIRST_HIT_RETURN/,$p' "$dir/lldb.out" | grep -E '^[[:space:]]*(x0|s0) =|Return value' | tr -s ' ' | tr '\n' ';')"
hit="no"; grep -q 'FIRST_HIT_ARGS' "$dir/lldb.out" && grep -q 'stop reason = breakpoint' "$dir/lldb.out" && hit="yes"
echo "vs=$name rc=$rc first_hit=$hit args:[$args] return:[$ret]"
