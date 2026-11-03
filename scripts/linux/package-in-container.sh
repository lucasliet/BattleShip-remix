#!/usr/bin/env bash
# Runs inside the Dockerfile.jammy image: packs /out/BattleShip-Remix as an AppImage in /out.
# Mounts: /src (repo, ro; reads build/ for the .o2r, f3d.o2r and css icons), /out (binary in, AppImage out), /work (scratch).
# Env: DIST_VERSION (file name tag, default 2.0.1-port.local), ARCH_TAG (default x86_64).
# The AppImage embeds the ROM-derived BattleShip-Remix.o2r: personal test package only, never redistributed
# (M11 decision 6; the distributable package extracts the .o2r on first run, M11 Fase 7).
set -euo pipefail
APP_NAME=BattleShip-Remix
ARCH_TAG="${ARCH_TAG:-x86_64}"
DIST_VERSION="${DIST_VERSION:-2.0.1-port.local}"
SRC=/src
OUT=/out
APPDIR=/work/AppDir
ASSETS="${SRC}/build"
export APPIMAGE_EXTRACT_AND_RUN=1
export ARCH="$ARCH_TAG"

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin/assets/css_icons" "$APPDIR/usr/share/$APP_NAME/assets/custom/fonts" "$APPDIR/usr/share/$APP_NAME/licenses"
cp "$OUT/$APP_NAME" "$APPDIR/usr/bin/$APP_NAME"
cp "$ASSETS/$APP_NAME.o2r" "$ASSETS/$APP_NAME.o2r.recipe" "$ASSETS/f3d.o2r" "$APPDIR/usr/bin/"
cp "$ASSETS"/assets/css_icons/*.png "$APPDIR/usr/bin/assets/css_icons/"
cp "$SRC/gamecontrollerdb.txt" "$APPDIR/usr/share/$APP_NAME/"
cp "$SRC"/assets/custom/fonts/*.ttf "$SRC"/assets/custom/fonts/*-OFL.txt "$APPDIR/usr/share/$APP_NAME/assets/custom/fonts/"
cp "$SRC/LICENSE" "$APPDIR/usr/share/$APP_NAME/LICENSE"
cp "$SRC/libultraship/LICENSE" "$APPDIR/usr/share/$APP_NAME/licenses/libultraship-LICENSE.txt"
cp "$SRC/torch/LICENSE" "$APPDIR/usr/share/$APP_NAME/licenses/torch-LICENSE.txt"

cat > "$APPDIR/$APP_NAME.desktop" <<DESK
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=$APP_NAME
Icon=$APP_NAME
Categories=Game;ArcadeGame;
Terminal=false
DESK

mkdir -p "$APPDIR/usr/share/icons/hicolor/512x512/apps" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
convert "$SRC/assets/icon-remix.png" -resize 256x256 "$APPDIR/$APP_NAME.png"
cp "$APPDIR/$APP_NAME.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
cp "$SRC/assets/icon-remix.png" "$APPDIR/usr/share/icons/hicolor/512x512/apps/$APP_NAME.png"
cp "$APPDIR/$APP_NAME.png" "$APPDIR/.DirIcon"

linuxdeploy --appdir "$APPDIR" \
  --executable "$APPDIR/usr/bin/$APP_NAME" \
  --desktop-file "$APPDIR/$APP_NAME.desktop" \
  --icon-file "$APPDIR/$APP_NAME.png"

rm -f "$APPDIR/AppRun"
cat > "$APPDIR/AppRun" <<RUN
#!/bin/sh
HERE="\$(dirname "\$(readlink -f "\${0}")")"
export PATH="\$HERE/usr/bin:\$PATH"
export LD_LIBRARY_PATH="\$HERE/usr/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
cd "\$HERE/usr/share/$APP_NAME" || exit 1
exec "\$HERE/usr/bin/$APP_NAME" "\$@"
RUN
chmod +x "$APPDIR/AppRun"

OUTFILE="$OUT/${APP_NAME}-${DIST_VERSION}-linux-${ARCH_TAG}.AppImage"
rm -f "$OUTFILE"
appimagetool "$APPDIR" "$OUTFILE"
chmod +x "$OUTFILE"
ls -l "$OUTFILE"
