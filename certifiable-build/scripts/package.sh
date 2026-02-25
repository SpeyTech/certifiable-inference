#!/bin/sh
set -eu

usage() {
  cat <<EOF
Usage: $(basename "$0") [BUILD_DIR] [VERSION] [REVISION]
  BUILD_DIR: Build directory (env: BUILD_DIR, default: ../<srcdir>-gcc)
  VERSION:   Package version (env: VERSION, required)
  REVISION:  Package revision (env: REVISION, required)
EOF
  exit 1
}

cd "$(dirname "$0")/../.." || exit 1
SRCDIR="$(basename "$(pwd)")"
BUILD_DIR="${1:-${BUILD_DIR:-../$SRCDIR-gcc}}"
VERSION="${2:-${VERSION:-}}"
REVISION="${3:-${REVISION:-}}"

[ -z "$VERSION" ] && usage
[ -z "$REVISION" ] && usage
[ -d "$BUILD_DIR" ] || { echo "Build directory not found. Run build first."; exit 1; }

echo "Creating package $VERSION-$REVISION..."
# Add packaging logic here
echo "Package created."
