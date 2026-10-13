#!/bin/zsh
# Rewrites hardcoded /opt/devkitpro prefixes inside the extracted devkitPro
# tree to the repository-contained location. devkitPro ships .pc/.la/.cmake/
# .specs files with absolute paths; running the toolchain from any other
# prefix requires rewriting them (devkitPro explicitly only supports
# /opt/devkitpro as-is).
#
# Usage: scripts/switch/relocate-toolchain.sh [old_prefix]
#   old_prefix defaults to /opt/devkitpro.

set -euo pipefail

ROOT=${0:A:h:h:h}
DKP="$ROOT/.toolchain/opt/devkitpro"
OLD="${1:-/opt/devkitpro}"

[[ -d "$DKP" ]] || { echo "ERRO: $DKP não existe; rode setup-toolchain.sh antes." >&2; exit 1; }
[[ "$OLD" == "$DKP" ]] && { echo "prefixo já correto ($DKP)"; exit 0; }

echo "relocando '$OLD' -> '$DKP'"
# Only text config formats; binaries are ELF and position-independent enough.
grep -rlF "$OLD" "$DKP" \
  --include='*.pc' --include='*.la' --include='*.cmake' \
  --include='*.specs' --include='*.sh' --include='*.py' \
  2>/dev/null | while read -r f; do
  # $DKP itself contains $OLD (…/.toolchain/opt/devkitpro), so shield paths that
  # are already relocated or a re-run would nest the prefix inside itself.
  sed -i '' -e "s|$DKP|@@DKP@@|g" -e "s|$OLD|$DKP|g" -e "s|@@DKP@@|$DKP|g" "$f"
done

# Sanity: the pkg-config wrapper must now resolve portlibs under the repo.
sample=$(grep -m1 '^prefix=' "$DKP/portlibs/switch/lib/pkgconfig/sdl2.pc" 2>/dev/null || true)
[[ "$sample" == "prefix=$DKP/portlibs/switch" ]] || {
  echo "AVISO: sdl2.pc prefix inesperado: $sample" >&2
}
echo "relocação concluída"
