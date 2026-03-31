#!/bin/sh
set -eu

# Optional env:
#   TT_RUNTIME=lima|podman|docker   (force runtime selection)
#   TT_LIMA_INSTANCE=tt-metal-dev   (override Lima instance name)
#   TT_LIMA_CONFIG=path/to/yaml     (override Lima config file)
#   TT_IMAGE=ghcr.io/...:tag        (override container image)

echo "Starting TT environment…"

# Resolve repo root (prefer git, fall back to current directory)
if command -v git >/dev/null 2>&1 && git rev-parse --show-toplevel >/dev/null 2>&1; then
  REPO_ROOT="$(git rev-parse --show-toplevel)"
else
  REPO_ROOT="$(pwd)"
fi

# Normalize REPO_ROOT (best-effort)
# shellcheck disable=SC2164
cd "$REPO_ROOT"
REPO_ROOT="$(pwd)"

IMAGE_DEFAULT="ghcr.io/tenstorrent/tt-metal/tt-metalium/ubuntu-22.04-dev-amd64:d6958801d86fe1e20a07f2be7795fac0b2c9b211"
TT_IMAGE="${TT_IMAGE:-$IMAGE_DEFAULT}"

OS="$(uname -s)"

TT_LIMA_INSTANCE="${TT_LIMA_INSTANCE:-tt-metal-dev}"
TT_LIMA_CONFIG="${TT_LIMA_CONFIG:-certifiable-build/etc/lima/tt-metal-dev.yaml}"

# Choose runtime
choose_runtime() {
  # If user forced it, obey.
  if [ "${TT_RUNTIME:-}" != "" ]; then
    echo "$TT_RUNTIME"
    return
  fi

  case "$OS" in
    Darwin)
      # Default to Lima on macOS.
      echo "lima"
      ;;
    Linux)
      if command -v podman >/dev/null 2>&1; then
        echo "podman"
      elif command -v docker >/dev/null 2>&1; then
        echo "docker"
      else
        echo "none"
      fi
      ;;
    *)
      echo "none"
      ;;
  esac
}

RUNTIME="$(choose_runtime)"

run_container() {
  runtime="$1"

  if ! command -v "$runtime" >/dev/null 2>&1; then
    echo "Error: requested runtime '$runtime' not found in PATH."
    exit 1
  fi

  # Pick a sensible interactive shell inside the container.
  # Most of these images have bash; fall back to sh if not.
  SHELL_CMD='if command -v bash >/dev/null 2>&1; then exec bash -l; else exec sh -l; fi'

  echo "Using $runtime to run container:"
  echo "  Image: $TT_IMAGE"
  echo "  Repo:  $REPO_ROOT"

  # --userns=keep-id helps on podman (Linux); harmless or ignored elsewhere.
  # We do NOT set --user because macOS Docker Desktop file permissions are quirky;
  # on Linux you can add it later if you care.
  exec "$runtime" run --rm -it \
    -v "$REPO_ROOT:$REPO_ROOT" \
    -w "$REPO_ROOT" \
    --userns=keep-id \
    "$TT_IMAGE" \
    sh -lc "$SHELL_CMD"
}

run_lima() {
  if ! command -v limactl >/dev/null 2>&1; then
    echo "Error: limactl not found. Install Lima (e.g., brew install lima)."
    exit 1
  fi

  if [ ! -f "$TT_LIMA_CONFIG" ]; then
    echo "Error: Lima config not found at: $TT_LIMA_CONFIG"
    echo "Set TT_LIMA_CONFIG to the correct path."
    exit 1
  fi

  # Create/start instance if needed.
  if limactl list -q | grep -Fx "$TT_LIMA_INSTANCE" >/dev/null 2>&1; then
    : # instance exists
  else
    echo "Creating Lima instance '$TT_LIMA_INSTANCE' from $TT_LIMA_CONFIG…"
    limactl create --name="$TT_LIMA_INSTANCE" "$TT_LIMA_CONFIG"
  fi

  echo "Starting Lima instance '$TT_LIMA_INSTANCE'…"
  limactl start "$TT_LIMA_INSTANCE" >/dev/null

  # Drop into an interactive shell, cd to repo root.
  # Note: With your current Lima YAML (containerd disabled), this lands you in the VM.
  # If you truly want "container inside Lima", enable containerd or run a container runtime in the VM.
  echo "Entering Lima shell (VM) at repo root: $REPO_ROOT"
  exec limactl shell "$TT_LIMA_INSTANCE" -- sh -lc "cd '$REPO_ROOT' && if command -v bash >/dev/null 2>&1; then exec bash -l; else exec sh -l; fi"
}

case "$RUNTIME" in
  docker|podman)
    run_container "$RUNTIME"
    ;;
  lima)
    # If user set TT_RUNTIME=docker/podman, they get a container on macOS too.
    run_lima
    ;;
  none)
    echo "Unsupported or missing runtime on OS '$OS'."
    echo "On Linux install podman or docker. On macOS install lima (or set TT_RUNTIME=docker/podman if you insist)."
    exit 1
    ;;
  *)
    echo "Error: unknown TT_RUNTIME='$RUNTIME' (expected lima|docker|podman)."
    exit 1
    ;;
esac
