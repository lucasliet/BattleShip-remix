#!/usr/bin/env bash
# Runs inside the Dockerfile.jammy image: Release build of BattleShip-Remix from the read-only checkout at /src.
# Mounts: /src (repo, ro), /work (persistent volume: source copy + build tree), /out (binary output).
# Env: BATTLESHIP_VERSION (version string baked into the binary, default 2.0.1-port.local).
set -euo pipefail
VER_STRING="${BATTLESHIP_VERSION:-2.0.1-port.local}"

# Refresh the source copy every run; tar keeps the mtimes, so ninja only rebuilds what changed.
mkdir -p /work/src
tar -C /src --exclude=./.git --exclude='./build*' --exclude=./libultraship/build-nx \
  --exclude=./.toolchain --exclude=./dist --exclude=./workspace --exclude=./logs \
  --exclude='*.z64' --exclude='*.o2r' --exclude='*.o2r.recipe' --exclude=./.claude -cf - . | tar -C /work/src -xf -
cd /work/src
export CC=clang CXX=clang++

# Gitignored codegen the host build makes on its own (see scripts/new-worktree.sh).
(cd decomp/src/credits
 for f in staff.credits.us.txt titles.credits.us.txt; do python3 /work/src/tools/creditsTextConverter.py "$f" >/dev/null; done
 for f in info.credits.us.txt companies.credits.us.txt; do python3 /work/src/tools/creditsTextConverter.py -paragraphFont "$f" >/dev/null; done)

cmake -B /work/build -S /work/src -GNinja -DCMAKE_BUILD_TYPE=Release -DNON_PORTABLE=ON \
  -DSSB64_VERSION=remix -DBATTLESHIP_VERSION="$VER_STRING"
cmake --build /work/build --target ssb64 -j"$(nproc)"
mkdir -p /out
cp /work/build/BattleShip-Remix /out/
echo BUILD_DONE
