#!/bin/sh
set -eu

usage() {
  cat <<EOF
Usage: $(basename "$0") [VERSION]
  VERSION: Release version (env: VERSION, required)
EOF
  exit 1
}

VERSION="${1:-${VERSION:-}}"

[ -z "$VERSION" ] && usage

echo "Publishing release $VERSION..."
# Add release logic here
echo "Release published."
