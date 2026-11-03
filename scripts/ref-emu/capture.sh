#!/bin/zsh
# Captures frames of a ROM in the reference emulator (M11 I8).
#
# Usage: scripts/ref-emu/capture.sh <rom> <frames-comma-list> <out-dir> [input-script]
#
# Runs the pure interpreter with the cxd4 LLE RSP and the angrylion software RDP, no audio, no speed limit, and writes
# the screenshots of the listed presented frames to <out-dir> (the emulator quits after the last one). The optional input
# script has the format of the port's SSB64_INPUT_SCRIPT. The frame index counts presented frames only; the port counts
# every VI, so align the two by content, not by index (docs/intro_divergence_triage_2026-07-30.md).
# Set up the emulator first with scripts/macos/setup-ref-emu.sh.

set -euo pipefail

ROOT=${0:A:h:h:h}
EMU="$ROOT/.toolchain/ref-emu"
[[ $# -ge 3 ]] || { echo "usage: $0 <rom> <frames> <out-dir> [input-script]" >&2; exit 2; }
ROM=${1:A}; FRAMES=$2; OUT=${3:A}; INPUT=${4:-}
[[ -x "$EMU/bin/mupen64plus" ]] || { echo "ERRO: emulador ausente, rode scripts/macos/setup-ref-emu.sh" >&2; exit 1; }

mkdir -p "$OUT" "$EMU/config"
export DYLD_LIBRARY_PATH="$EMU/prefix/lib:$EMU/lib"
[[ -z "$INPUT" ]] || export REF_EMU_INPUT=${INPUT:A}

"$EMU/bin/mupen64plus" --noosd --windowed --resolution 640x480 --nospeedlimit --emumode 0 \
  --configdir "$EMU/config" --datadir "$EMU/data" --plugindir "$EMU/plugins" --sshotdir "$OUT" \
  --gfx mupen64plus-video-angrylion-plus.dylib --rsp mupen64plus-rsp-cxd4.dylib \
  --audio dummy --input mupen64plus-input-script.dylib \
  --testshots "$FRAMES" "$ROM"
