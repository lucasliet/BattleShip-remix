#!/usr/bin/env bash
# Syntax-checks one decomp C file with the exact flags the game build uses
# (taken from build/compile_commands.json), without running a build.
# Usage: scripts/remix/syntax-check.sh decomp/src/ft/ftremix/<dir>/<file>.c
set -euo pipefail

ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
FILE="$(builtin cd "$(dirname "$1")" && pwd)/$(basename "$1")"
REFERENCE="decomp/src/ft/ftremix/falco/ftremixfalco.c"

FLAGS="$("$ROOT/.toolchain/host/venv/bin/python" -I - "$ROOT/build/compile_commands.json" "$REFERENCE" <<'EOF'
import json, shlex, sys
entries = json.load(open(sys.argv[1]))
entry = next(e for e in entries if e["file"].endswith(sys.argv[2]))
args = shlex.split(entry["command"]) if "command" in entry else entry["arguments"]
keep = []
skip = False
for arg in args[1:]:
    if skip:
        skip = False
        continue
    if arg in ("-o", "-c"):
        skip = True
        continue
    keep.append(arg)
print(shlex.join(keep))
EOF
)"

builtin cd "$ROOT/build"
eval "cc $FLAGS -fsyntax-only -Wall -Wno-unused-variable -Wno-unused-function -Wno-missing-braces \"$FILE\""
echo "syntax-check OK: $1"
