#!/usr/bin/env bash
# Uploads a staged title folder to a jailbroken PS5 over its FTP payload and asks ShadowMountPlus
# to rescan, so the title appears (or is replaced) on the home screen. Close the title first.
#
# Usage: scripts/ps5/deploy.sh <ps5-host> <dist/TITLE_ID dir> [ftp-port=2121] [smp-api-port=10101]
# The files go to /data/homebrew/<TITLE_ID>/; eboot.bin is uploaded last, as the boilerplate does,
# so a half-uploaded folder never carries a new executable with old data.
set -euo pipefail

HOST="$1"
DIST="$(realpath "$2")"
FTP_PORT="${3:-2121}"
API_PORT="${4:-10101}"
TITLE_ID="$(basename "$DIST")"
REMOTE="/data/homebrew/$TITLE_ID"
FTP="ftp://$HOST:$FTP_PORT"

upload() {
    local relative="$1"
    curl -sS --ftp-create-dirs -T "$DIST/$relative" "$FTP$REMOTE/$relative"
}

echo "==> uploading $TITLE_ID to $HOST:$REMOTE"
while IFS= read -r -d '' file; do
    relative="${file#"$DIST"/}"
    [[ "$relative" == "eboot.bin" ]] && continue
    upload "$relative"
done < <(find "$DIST" -type f -print0 | sort -z)
upload eboot.bin

echo "==> asking ShadowMountPlus to rescan"
curl -sS -X POST "http://$HOST:$API_PORT/api/v1/scan" -H 'Content-Type: application/json' -d '{}' || \
    echo "scan request failed (is ShadowMountPlus running with its API on $API_PORT?)"
echo
echo "==> done; open $TITLE_ID from the Games row"
