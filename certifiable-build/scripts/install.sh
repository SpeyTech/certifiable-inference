#!/bin/sh
set -eu

usage() {
  cat <<EOF
Usage: $(basename "$0") [BUILD_DIR] [PREFIX]
  BUILD_DIR: Build directory (env: BUILD_DIR, default: ../<srcdir>-gcc)
  PREFIX:    Install prefix (env: PREFIX, default: /usr/local)
EOF
  exit 1
}

cd "$(dirname "$0")/../.." || exit 1
SRCDIR="$(basename "$(pwd)")"
BUILD_DIR="${1:-${BUILD_DIR:-../$SRCDIR-gcc}}"
PREFIX="${2:-${PREFIX:-/usr/local}}"
B="${B:-b}"

[ -d "$BUILD_DIR" ] || { echo "Build directory not found. Run build first."; exit 1; }

echo "Installing to $PREFIX..."
"$B" install: "$BUILD_DIR" \
  "config.install.root=$PREFIX"
