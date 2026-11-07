#!/usr/bin/env bash
# Turns one relocatable object (the whole program, linked with `ld.lld -r`) into a PS5 native
# title folder with the ps5-native-app-boilerplate pipeline and the ps5-opengl runtime, the way
# ps5-opengl's tools/build-native-test-app.sh does for its own gates:
#   <app dir>/dist/<TITLE_ID>/{eboot.bin, sce_sys/param.json, sce_module/libc.prx, ...}
#
# Usage: native-app-stage.sh <app-dir> <title-id> <program.o> <param.json> [icon0.png]
# Env:   PS5_EXTRA_SDK_LIBS  extra SDK stub libraries to link, e.g. "libScePad.so libSceAudioOut.so"
#        PS5_APP_ASSETS      directory copied as <dist>/assets (optional)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
T="$ROOT/.toolchain/ps5"
# shellcheck disable=SC1091
source "$T/env.sh"

APP="$1"
TITLE_ID="$2"
PROGRAM_OBJECT="$(realpath "$3")"
PARAM_JSON="$(realpath "$4")"
ICON="${5:-}"
TEMPLATE="$PS5_NATIVE_APP_TEMPLATE"
OPENGL_SRC="$T/src/ps5-opengl"
PREFIX="$PS5_OPENGL_PREFIX"
SDK="$PS5_PAYLOAD_SDK"

(cd "$PREFIX" && sha256sum --check --strict --quiet manifest.sha256)

mkdir -p "$APP"
cp "$TEMPLATE/Makefile" "$APP/Makefile"
for directory in assets runtime sce_sys tooling tools; do
    mkdir -p "$APP/$directory"
    cp -a "$TEMPLATE/$directory/." "$APP/$directory/"
done

# The boilerplate's module header reserves an unbounded heap; ps5-opengl's titles cap it at
# 256 MiB and grow through app_heap.c (direct memory) instead.
heap_source="$APP/tooling/native/sce_module_writer.cpp"
heap_default='write_u64(result.data, result.heap_size, std::numeric_limits<std::uint64_t>::max());'
grep -qF "$heap_default" "$heap_source"
sed -i "s/$heap_default/write_u64(result.data, result.heap_size, 0x10000000ULL);/" "$heap_source"

# app_heap.c takes over malloc through --wrap; all six must be wrapped together.
link_script="$APP/tools/build.sh"
grep -qF -- '--eh-frame-hdr \' "$link_script"
sed -i 's/--eh-frame-hdr \\/--eh-frame-hdr --wrap=fopen --wrap=malloc --wrap=calloc --wrap=realloc --wrap=free --wrap=posix_memalign --wrap=malloc_usable_size \\/' \
    "$link_script"

cp "$TEMPLATE/tooling/native/ps5-pie.ld" "$APP/tooling/native/ps5-pie-base.ld"
cp "$OPENGL_SRC/native-app/ps5-pie.ld" "$APP/tooling/native/ps5-pie.ld"
cp "$OPENGL_SRC/native-app/app-symbols.map" "$APP/tooling/native/app-symbols.map"

rm -rf "$APP/src" "$APP/include" "$APP/vendor"
mkdir -p "$APP/src" "$APP/include" "$APP/vendor"
cp "$OPENGL_SRC/native-app/runtime_shims.c" "$APP/src/runtime_shims.c"
cp "$OPENGL_SRC/native-app/app_heap.c" "$APP/src/app_heap.c"
# The locale, catalog and thread-destructor functions libc++ needs that the title's libc lacks
# (the same shims ps5-opengl's own C++ CTS runner links); relative fopen paths resolve to /app0.
cp "$OPENGL_SRC/native-app/cts_runtime_shims.c" "$APP/src/cts_runtime_shims.c"
for headers in EGL GL KHR; do
    cp -a "$PREFIX/include/$headers" "$APP/include/"
done

cp "$PARAM_JSON" "$APP/sce_sys/param.json"
[[ -n "$ICON" ]] && cp "$ICON" "$APP/sce_sys/icon0.png"
if [[ -n "${PS5_APP_ASSETS:-}" ]]; then
    rm -rf "$APP/assets"
    cp -a "$PS5_APP_ASSETS" "$APP/assets"
fi

resource_dir="$("$PS5_CLANG" --print-resource-dir)"
compiler_runtime="$resource_dir/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a"
[[ -s "$compiler_runtime" ]] || compiler_runtime="$resource_dir/lib/linux/libclang_rt.builtins-x86_64.a"
static_libraries=("$PREFIX/lib/libPS5OpenGLCore33.a")
for library in ${PS5_EXTRA_SDK_LIBS:-}; do
    static_libraries+=("$SDK/target/lib/$library")
done
static_libraries+=("$SDK/target/lib/libunwind.a" "$SDK/target/lib/libc++abi.a" "$SDK/target/lib/libc++.a" "$compiler_runtime")
for library in "${static_libraries[@]}"; do
    test -s "$library"
done

{
    printf 'SEARCH_DIR("%s")\n' "$SDK/target/lib"
    printf 'SEARCH_DIR("%s")\n' "$PREFIX/lib"
    printf 'EXTERN(ps5_agc_gate2_run)\n'
    printf 'GROUP (\n'
    printf '  "%s"\n' "$PROGRAM_OBJECT"
    printf '  "%s"\n' "${static_libraries[@]}"
    printf ')\n'
} > "$APP/vendor/libprogram_group.a"
printf 'APP_INCLUDE_PATHS = include\nAPP_STATIC_ARCHIVES = vendor/libprogram_group.a\n' > "$APP/.env"

# A private, dereferenced copy of the SDK: the AGC link stubs below are written into its lib dir.
app_sdk="$APP/.deps/native/ps5-payload-sdk"
if [[ ! -x "$app_sdk/bin/prospero-lld" ]]; then
    mkdir -p "$APP/.deps/native"
    cp -rL "$SDK" "$app_sdk"
fi
mkdir -p "$APP/build/native-imports"
for stub in agc_link_stub:libSceAgc agc_driver_link_stub:libSceAgcDriver; do
    source_name="${stub%%:*}"
    library_name="${stub##*:}"
    PS5_PAYLOAD_SDK="$app_sdk" sh "$APP/tooling/prospero-clang18" -std=c11 -O2 -fPIC -ffunction-sections \
        -fdata-sections -c "$OPENGL_SRC/native-app/$source_name.c" -o "$APP/build/native-imports/$source_name.o"
    "$app_sdk/bin/prospero-lld" --shared -soname "$library_name.prx" -o "$app_sdk/target/lib/$library_name.so" \
        "$APP/build/native-imports/$source_name.o"
done

CXX="$T/llvm/bin/host-clang++" USE_CCACHE=0 make -C "$APP" --no-print-directory -j8 app HOST_CXX="$T/llvm/bin/host-clang++"
dist="$APP/dist/$TITLE_ID"
test -s "$dist/eboot.bin"
test -s "$dist/sce_module/libc.prx"
printf 'Native app: %s\n' "$dist"
sha256sum "$dist/eboot.bin"
