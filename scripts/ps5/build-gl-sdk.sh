#!/usr/bin/env bash
# Builds the PS5 OpenGL SDK (mihawk-99/ps5-opengl: Mesa 26.2 Gallium driver for the PS5 GPU, GL 4.6 /
# 3.3 core over EGL) and its static SDL2 integration from the pinned sources that
# scripts/ps5/setup-toolchain.sh cloned. Everything lands under .toolchain/ps5/:
#   ps5-opengl-sdk/  the GL SDK prefix (lib/, include/, lib/cmake/, lib/pkgconfig/)
#   ps5-sdl2-sdk/    the SDL2 prefix (lib/libSDL2.a, lib/cmake/SDL2/)
# Idempotent through markers in .toolchain/ps5/.extracted. Needs network for the first run
# (Mesa tarball and the pinned repositories of ps5-opengl's dependencies.json).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
T="$ROOT/.toolchain/ps5"
MARKERS="$T/.extracted"
# shellcheck disable=SC1091
source "$T/env.sh"

OPENGL_SRC="$T/src/ps5-opengl"
BOILERPLATE="$T/src/ps5-native-app-boilerplate"
SDL_SRC="$T/src/SDL"

step_done() { [[ -f "$MARKERS/$1" ]]; }
mark_done() { touch "$MARKERS/$1"; }

if ! step_done boilerplate-native-deps; then
    echo "==> boilerplate native dependencies (zlib for the host tool)"
    bash "$BOILERPLATE/tools/setup-native-dependencies.sh"
    mark_done boilerplate-native-deps
fi

if ! step_done ps5-opengl-sources; then
    echo "==> ps5-opengl sources (Mesa + PSBC + pinned repositories)"
    # fetch-sources.py has no retry; a reset connection on the Mesa download failed release CI.
    # It skips what is already fetched, so running it again resumes.
    for attempt in 1 2 3; do
        make -C "$OPENGL_SRC" source-fetch && break
        [[ $attempt == 3 ]] && { echo "ps5-opengl source-fetch failed after $attempt attempts" >&2; exit 1; }
        echo "==> source-fetch failed (attempt $attempt), retrying in 15s"
        sleep 15
    done
    mark_done ps5-opengl-sources
fi

if ! step_done ps5-opengl-sdk; then
    echo "==> ps5-opengl SDK (gl46); this compiles Mesa and the shader compiler"
    # The steps of `make sdk-gl46` without its host self-checks: at the pinned revision
    # (ec06d8c) `make test-compiler` and verify_gl33_capability_audit.py fail on stale
    # receipts of the repository's own test fixtures, not on the built SDK.
    (
        cd "$OPENGL_SRC"
        bash toolchain/build-opengnm-psbc.sh
        PS5_PAYLOAD_SDK="$PS5_PAYLOAD_SDK" bash toolchain/build-opengnm-psbc-ps5.sh
        PS5_MESA_CROSS_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.ini" bash toolchain/build-mesa-ps5.sh
        bash toolchain/install-ps5-opengl-gl46.sh "$PS5_OPENGL_PREFIX"
        python3 tests/ps5/verify_gl46_link_surface.py
    )
    mark_done ps5-opengl-sdk
fi

if ! step_done ps5-sdl2-sdk; then
    echo "==> SDL2 static integration"
    rm -rf "$OPENGL_SRC/build/sdl2-native"
    python3 "$OPENGL_SRC/integration/SDL2/build.py" native \
        --sdl-source "$SDL_SRC" \
        --sdk-prefix "$PS5_OPENGL_PREFIX" \
        --out "$OPENGL_SRC/build/sdl2-native" \
        --payload-sdk "$PS5_PAYLOAD_SDK" \
        --compiler-wrapper "$BOILERPLATE/tooling/prospero-clang18"
    rm -rf "$PS5_SDL2_PREFIX"
    cp -r "$OPENGL_SRC/build/sdl2-native/sdk" "$PS5_SDL2_PREFIX"
    mark_done ps5-sdl2-sdk
fi

echo "==> GL SDK ready: $PS5_OPENGL_PREFIX and $PS5_SDL2_PREFIX"
