#!/usr/bin/env bash
# Installs the pinned Python tools used to study Remix assembly (rabbitizer,
# the MIPS disassembler behind tools/remix_disasm_rom.py) into
# .toolchain/host/venv. Idempotent; uv's cache stays under .toolchain.
set -euo pipefail

ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VENV="$ROOT/.toolchain/host/venv"
PYTHON="$VENV/bin/python"
REQUIREMENTS="$ROOT/.toolchain/cache/remix-tools.requirements.txt"

export UV_CACHE_DIR="$ROOT/.toolchain/cache/uv"

if [[ ! -x "$PYTHON" ]]; then
  uv venv --python 3.12 "$VENV"
fi

if "$PYTHON" -I -c "import rabbitizer" 2>/dev/null; then
  echo "remix tools already installed"
  exit 0
fi

mkdir -p "$(dirname "$REQUIREMENTS")"
cat > "$REQUIREMENTS" <<'EOF'
rabbitizer==1.12.0 \
    --hash=sha256:3f8ec6050da274dd037e23ac9ef828dac23f4a90aacbbdf0382928a55c4b1492 \
    --hash=sha256:db0c9baf34133854bcf0747f20e1cf76fe6a7a7fb94fafc5c0a1276f813cfe0b
EOF

uv pip install --python "$PYTHON" --require-hashes -r "$REQUIREMENTS"
