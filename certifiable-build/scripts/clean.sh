#!/bin/sh
set -eu

usage() {
  code=${1:-1}
  cat <<EOF
Usage: $(basename "$0") [@clang] [@gcc] [...]

Cleans build artifacts for the specified bdep configurations, without
removing configuration state.

Defaults to: @clang @gcc

Environment:
  PROJECT       Project name (default: basename of repo root)
  BDEP          bdep command (default: bdep)
  CONFIGS_ROOT  Base configs dir (default: ../build2/configs)
EOF
  exit "$code"
}

SCRIPT_DIR=$(CDPATH='' cd -- "$(dirname "$0")" && pwd)
REPO_ROOT=$(CDPATH='' cd -- "$SCRIPT_DIR/../.." && pwd)
cd "$REPO_ROOT"

PROJECT=${PROJECT:-$(basename "$REPO_ROOT")}
BDEP=${BDEP:-bdep}
CONFIGS_ROOT=${CONFIGS_ROOT:-../build2/configs}

case "${1:-}" in
  -h|--help)
    usage 0
    ;;
esac

if [ "$#" -eq 0 ]; then
  set -- @clang @gcc
fi

clang_dir="${CONFIGS_ROOT}/${PROJECT}-clang"
gcc_dir="${CONFIGS_ROOT}/${PROJECT}-gcc"

missing=""

for cfg in "$@"; do
  case "$cfg" in
    @clang)
      if [ ! -d "$clang_dir" ]; then
        missing="${missing} @clang(${clang_dir})"
      fi
      ;;
    @gcc)
      if [ ! -d "$gcc_dir" ]; then
        missing="${missing} @gcc(${gcc_dir})"
      fi
      ;;
  esac
done

if [ -n "$missing" ]; then
  echo "Warning: missing config directories:${missing}"
  echo "Nothing to clean for missing configs; continuing with any that exist."
fi

echo "Cleaning build artifacts for configs: $*"
# Cleans build outputs, preserves configuration state.
"$BDEP" clean -a "$@"

echo "Clean complete."
