#!/usr/bin/env bash
# Downloads a file from the PS5 over its FTP payload and prints it (receipts and game logs).
# Usage: scripts/ps5/fetch-log.sh <ps5-host> <remote path> [ftp-port=2121]
set -euo pipefail

HOST="$1"
REMOTE="$2"
FTP_PORT="${3:-2121}"
curl -sS "ftp://$HOST:$FTP_PORT$REMOTE"
