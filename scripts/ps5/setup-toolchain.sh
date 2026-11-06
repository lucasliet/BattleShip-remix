#!/usr/bin/env bash
# Prepares the self-contained PS5 homebrew toolchain under .toolchain/ps5 (Linux x86_64 host):
# the ps5-payload-sdk, a pinned LLVM 18, the pinned source repositories (ps5-opengl, the native
# app boilerplate, SDL, UFS2Tool) and a uv venv with meson. Nothing is installed on the system.
# Every download is pinned by sha256 or git revision in scripts/ps5/packages.lock.
#
# Usage: scripts/ps5/setup-toolchain.sh [--offline]
# Then:  source .toolchain/ps5/env.sh && scripts/ps5/build-gl-sdk.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOCK="$ROOT/scripts/ps5/packages.lock"
T="$ROOT/.toolchain/ps5"
PKGS="$T/pkgs"
SRC="$T/src"
MARKERS="$T/.extracted"
OFFLINE=0
[[ "${1:-}" == "--offline" ]] && OFFLINE=1

mkdir -p "$PKGS" "$SRC" "$MARKERS"

lock_field() {
    uv run -q --no-project python -I - "$LOCK" "$@" <<'EOF'
import json, sys
node = json.load(open(sys.argv[1]))
for key in sys.argv[2:]:
    node = node[key]
print(" ".join(node) if isinstance(node, list) else node)
EOF
}

fetch_pinned() {
    local name="$1"
    local url filename sha
    url=$(lock_field archives "$name" url)
    filename=$(lock_field archives "$name" filename)
    sha=$(lock_field archives "$name" sha256)
    if [[ ! -f "$PKGS/$filename" ]]; then
        if [[ $OFFLINE == 1 ]]; then
            echo "offline: missing $PKGS/$filename (download it from $url)" >&2
            exit 2
        fi
        echo "==> downloading $filename"
        curl -fL --retry 3 -o "$PKGS/$filename.part" "$url"
        mv "$PKGS/$filename.part" "$PKGS/$filename"
    fi
    echo "$sha  $PKGS/$filename" | sha256sum --check --quiet --strict
}

extract_once() {
    local marker="$1" dest="$2"; shift 2
    [[ -f "$MARKERS/$marker" ]] && return
    rm -rf "$dest"
    mkdir -p "$dest"
    "$@"
    touch "$MARKERS/$marker"
}

clone_pinned() {
    local name="$1"
    local url rev dir
    url=$(lock_field repositories "$name" url)
    rev=$(lock_field repositories "$name" revision)
    dir="$SRC/$name"
    if [[ ! -d "$dir/.git" ]]; then
        if [[ $OFFLINE == 1 ]]; then
            echo "offline: missing clone $dir ($url)" >&2
            exit 2
        fi
        echo "==> cloning $name"
        git clone -q "$url" "$dir"
    fi
    if ! git -C "$dir" cat-file -e "$rev^{commit}" 2>/dev/null; then
        [[ $OFFLINE == 1 ]] && { echo "offline: $name lacks $rev" >&2; exit 2; }
        git -C "$dir" fetch -q origin
    fi
    git -C "$dir" checkout -q "$rev"
    echo "$name @ $(git -C "$dir" rev-parse --short HEAD)"
}

echo "==> archives"
fetch_pinned ps5-payload-sdk
fetch_pinned llvm
extract_once "$(lock_field archives ps5-payload-sdk filename)" "$T/sdk" \
    bash -c "unzip -q '$PKGS/$(lock_field archives ps5-payload-sdk filename)' -d '$T/sdk.tmp' && mv '$T/sdk.tmp/ps5-payload-sdk'/* '$T/sdk/' && rm -rf '$T/sdk.tmp'"
extract_once "$(lock_field archives llvm filename)" "$T/llvm" \
    tar -xJf "$PKGS/$(lock_field archives llvm filename)" -C "$T/llvm" --strip-components=1

# The Ubuntu 18.04 LLVM binaries import four terminfo symbols from libtinfo.so.5 (only to detect
# terminal colours); newer distros ship libtinfo.so.6 with other symbol versions, so build a stub
# that answers "no terminfo" and keep it inside the toolchain.
build_tinfo_stub() {
    local host_cc
    host_cc=$(command -v cc || command -v clang || command -v gcc)
    cat > "$T/llvm/lib/tinfo_stub.c" <<'EOF'
int setupterm(char *term, int fd, int *errret) { if (errret) *errret = 0; return -1; }
int tigetnum(const char *cap) { return -2; }
void *set_curterm(void *term) { return 0; }
int del_curterm(void *term) { return -1; }
EOF
    printf 'NCURSES_TINFO_5.0.19991023 { global: setupterm; tigetnum; set_curterm; del_curterm; local: *; };\n' \
        > "$T/llvm/lib/tinfo_stub.map"
    "$host_cc" -shared -fPIC -Wl,--version-script="$T/llvm/lib/tinfo_stub.map" -Wl,-soname,libtinfo.so.5 \
        -o "$T/llvm/lib/libtinfo.so.5" "$T/llvm/lib/tinfo_stub.c"
}
[[ -e "$T/llvm/lib/libtinfo.so.5" ]] || build_tinfo_stub

# Host helper tools (the boilerplate's ps5-native-tool and libc-builder) are C++20 programs;
# clang 18 cannot parse a newer system libstdc++, so they build against the tarball's libc++,
# statically, through this wrapper.
cat > "$T/llvm/bin/host-clang++" <<EOF
#!/usr/bin/env bash
LIBDIR="$T/llvm/lib/x86_64-unknown-linux-gnu"
exec "$T/llvm/bin/clang++" -stdlib=libc++ -nostdlib++ "\$@" "\$LIBDIR/libc++.a" "\$LIBDIR/libc++abi.a" "\$LIBDIR/libunwind.a" -lpthread -ldl
EOF
chmod +x "$T/llvm/bin/host-clang++"

echo "==> repositories"
for name in $(uv run -q --no-project python -I -c 'import json,sys; print(" ".join(json.load(open(sys.argv[1]))["repositories"]))' "$LOCK"); do
    clone_pinned "$name"
done

echo "==> python venv"
if [[ ! -x "$T/venv/bin/python" ]]; then
    uv venv -q -p "$(lock_field python version)" "$T/venv"
fi
# shellcheck disable=SC2046
uv pip install -q -p "$T/venv/bin/python" $(lock_field python packages)

# ps5-opengl's Makefile expects the boilerplate as a sibling with the SDK under .deps/native.
BOILERPLATE="$SRC/ps5-native-app-boilerplate"
mkdir -p "$BOILERPLATE/.deps/native"
ln -sfn "$T/sdk" "$BOILERPLATE/.deps/native/ps5-payload-sdk"
# The boilerplate invokes its wrapper as `sh tooling/prospero-clang18`; ps5-opengl's SDL2
# integration hands it to CMake as the compiler, which needs the executable bit.
chmod +x "$BOILERPLATE/tooling/prospero-clang18"

cat > "$T/env.sh" <<EOF
# Generated by scripts/ps5/setup-toolchain.sh; source it per shell session.
export PS5_TOOLCHAIN="$T"
export PS5_PAYLOAD_SDK="$T/sdk"
export LLVM_CONFIG="$T/llvm/bin/llvm-config"
export PS5_CLANG="$T/llvm/bin/clang"
export PS5_NATIVE_APP_TEMPLATE="$BOILERPLATE"
export PS5_OPENGL_PREFIX="$T/ps5-opengl-sdk"
export PS5_SDL2_PREFIX="$T/ps5-sdl2-sdk"
export PATH="$T/llvm/bin:$T/venv/bin:$T/sdk/bin:\$PATH"
export LD_LIBRARY_PATH="$T/llvm/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
export CMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
EOF
echo "==> done: source $T/env.sh"
