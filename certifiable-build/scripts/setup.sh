#!/bin/sh
set -eu

OS="$(uname -s)"

if command -v brew >/dev/null 2>&1; then
  echo "Homebrew already installed."
else
  NONINTERACTIVE=1 /bin/bash -c \
    "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

  case "$OS" in
    Linux)
      test -d ~/.linuxbrew && eval "$(~/.linuxbrew/bin/brew shellenv)"
      test -d /home/linuxbrew/.linuxbrew && eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"
      echo "eval \"\$($(brew --prefix)/bin/brew shellenv)\"" >> ~/.bashrc
      ;;
    Darwin)
      if [ -x /opt/homebrew/bin/brew ]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
        return 0
      elif [ -x /usr/local/bin/brew ]; then
        eval "$(/usr/local/bin/brew shellenv)"
        return 0
      fi
      ;;
    *)
      echo "Unsupported OS: $OS"
      exit 1
      ;;
  esac
fi

echo "Homebrew version:"
brew --version

brew update
brew install --overwrite --force gcc llvm build2
brew link --overwrite --force gcc llvm build2

gcc --version
g++ --version
clang --version
clang++ --version
bdep --version

echo "System dependencies installed successfully."
