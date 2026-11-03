#!/bin/zsh
# Builds the scripted input plugin of the reference emulator into .toolchain/ref-emu/plugins (M11 I8).
set -euo pipefail
ROOT=${0:A:h:h:h}
EMU="$ROOT/.toolchain/ref-emu"
mkdir -p "$EMU/plugins"
cc -O2 -dynamiclib -arch arm64 -I"$EMU/include" -o "$EMU/plugins/mupen64plus-input-script.dylib" "$ROOT/scripts/ref-emu/input-script.c"
echo "ok  mupen64plus-input-script.dylib"
