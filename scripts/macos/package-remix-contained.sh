#!/bin/zsh
# Packages BattleShip-Remix as a macOS .app + .dmg + .zip WITHOUT dylibbundler
# or Homebrew. The desktop build links every third-party library statically
# from .toolchain/host/deps, so the executable only depends on system
# frameworks and nothing needs relocating.
#
# Inputs (read-only):
#   build-macos-release/BattleShip-Remix   Release build with -DNON_PORTABLE=ON
#   BUILD_ASSETS_DIR (default build/)      BattleShip-Remix.o2r, .o2r.recipe, f3d.o2r,
#                                          assets/css_icons/*.png
# Output: dist/desktop/BattleShip-Remix.app, then the .dmg and .zip next to it.
#
# The .o2r is ROM-derived: packages produced here are for private testing,
# not for public redistribution.
#
# Usage: [DIST_VERSION=2.0.1-port.abc1234] [TMPDIR=<scratch parent>] scripts/macos/package-remix-contained.sh
# DIST_VERSION defaults to 2.0.1-port.<short HEAD>; scratch files go under TMPDIR (default build-macos-release/).

set -euo pipefail

ROOT=${0:A:h:h:h}
APP_NAME="BattleShip-Remix"
APP_BUNDLE_ID="com.ssb-decomp-re.battleship-remix"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-macos-release}"
ASSETS_DIR="${BUILD_ASSETS_DIR:-$ROOT/build}"
OUT_DIR="${OUT_DIR:-$ROOT/dist/desktop}"
DIST_VERSION="${DIST_VERSION:-2.0.1-port.$(git -C "$ROOT" rev-parse --short HEAD)}"
BUNDLE_VERSION="${DIST_VERSION%%-*}"
ARCH="$(uname -m)"
APP="$OUT_DIR/$APP_NAME.app"
STEM="$APP_NAME-$DIST_VERSION-macOS-$ARCH"

fail() { print -u2 "ERROR: $1"; exit 1 }

# Scratch space (iconset, dmg staging) under a known writable dir, removed on every exit path.
WORK_PARENT="${TMPDIR:-$ROOT/build-macos-release}"
mkdir -p "$WORK_PARENT"
WORK="$(mktemp -d "$WORK_PARENT/remix-package.XXXXXX")" || fail "cannot create a scratch dir under $WORK_PARENT"
trap 'rm -rf "$WORK"' EXIT

for required in \
    "$BUILD_DIR/$APP_NAME" \
    "$ASSETS_DIR/$APP_NAME.o2r" \
    "$ASSETS_DIR/$APP_NAME.o2r.recipe" \
    "$ASSETS_DIR/f3d.o2r" \
    "$ROOT/assets/icon-remix.png" \
    "$ROOT/gamecontrollerdb.txt"; do
    [[ -f "$required" ]] || fail "missing input: $required"
done

if otool -L "$BUILD_DIR/$APP_NAME" | grep -qE '(/opt/homebrew|/usr/local|\.toolchain)'; then
    otool -L "$BUILD_DIR/$APP_NAME" | grep -E '(/opt/homebrew|/usr/local|\.toolchain)' >&2
    fail "binary links a non-system dylib; it needs relocating before packaging"
fi

rm -rf "$APP" "$OUT_DIR/$STEM.dmg" "$OUT_DIR/$STEM.zip"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources/assets/custom/fonts" "$APP/Contents/Resources/licenses"

cp "$BUILD_DIR/$APP_NAME" "$APP/Contents/MacOS/$APP_NAME"
cp "$ASSETS_DIR/$APP_NAME.o2r" "$ASSETS_DIR/$APP_NAME.o2r.recipe" "$ASSETS_DIR/f3d.o2r" "$APP/Contents/Resources/"
cp "$ROOT/gamecontrollerdb.txt" "$APP/Contents/Resources/"
cp "$ROOT/assets/custom/fonts/"*.ttf "$ROOT/assets/custom/fonts/"*-OFL.txt "$APP/Contents/Resources/assets/custom/fonts/"
cp "$ROOT/LICENSE" "$APP/Contents/Resources/LICENSE"
cp "$ROOT/libultraship/LICENSE" "$APP/Contents/Resources/licenses/libultraship-LICENSE.txt"
cp "$ROOT/torch/LICENSE" "$APP/Contents/Resources/licenses/torch-LICENSE.txt"
if [[ -d "$ASSETS_DIR/assets/css_icons" ]]; then
    mkdir -p "$APP/Contents/Resources/assets/css_icons"
    cp "$ASSETS_DIR/assets/css_icons/"*.png "$APP/Contents/Resources/assets/css_icons/"
fi

ICONSET="$WORK/AppIcon.iconset"
mkdir -p "$ICONSET"
for size in 16 32 128 256 512; do
    sips -z $size $size "$ROOT/assets/icon-remix.png" --out "$ICONSET/icon_${size}x${size}.png" >/dev/null
    sips -z $((size * 2)) $((size * 2)) "$ROOT/assets/icon-remix.png" --out "$ICONSET/icon_${size}x${size}@2x.png" >/dev/null
done
iconutil -c icns "$ICONSET" -o "$APP/Contents/Resources/AppIcon.icns"

cat > "$APP/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>                <string>$APP_NAME</string>
    <key>CFBundleDisplayName</key>         <string>$APP_NAME</string>
    <key>CFBundleIdentifier</key>          <string>$APP_BUNDLE_ID</string>
    <key>CFBundleVersion</key>             <string>$BUNDLE_VERSION</string>
    <key>CFBundleShortVersionString</key>  <string>$DIST_VERSION</string>
    <key>CFBundlePackageType</key>         <string>APPL</string>
    <key>CFBundleSignature</key>           <string>????</string>
    <key>CFBundleExecutable</key>          <string>$APP_NAME</string>
    <key>CFBundleIconFile</key>            <string>AppIcon</string>
    <key>LSMinimumSystemVersion</key>      <string>11.0</string>
    <key>NSHighResolutionCapable</key>     <true/>
    <key>NSSupportsAutomaticGraphicsSwitching</key> <true/>
</dict>
</plist>
EOF

chmod +x "$APP/Contents/MacOS/$APP_NAME"
codesign --force --sign - --timestamp=none \
    --entitlements "$ROOT/cmake/macos_entitlements.plist" \
    "$APP/Contents/MacOS/$APP_NAME"
codesign --force --sign - --timestamp=none \
    --entitlements "$ROOT/cmake/macos_entitlements.plist" \
    "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"

STAGE="$WORK/dmg"
mkdir -p "$STAGE"
cp -R "$APP" "$STAGE/"
ln -s /Applications "$STAGE/Applications"
hdiutil create -volname "$APP_NAME $DIST_VERSION" -srcfolder "$STAGE" -ov -format UDZO "$OUT_DIR/$STEM.dmg" >/dev/null

ditto -c -k --keepParent "$APP" "$OUT_DIR/$STEM.zip"

ls -l "$OUT_DIR/$STEM.dmg" "$OUT_DIR/$STEM.zip"
