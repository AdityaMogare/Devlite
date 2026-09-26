#!/usr/bin/env bash
# CMakeLists.txt and Makefile are present, and cmake and make are not on PATH.
# Expect: build.tool-missing
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
printf '#!/bin/sh\necho "Python 3.12.4"\n' > "$sandbox/bin/python3"
chmod +x "$sandbox/bin/python3"
ln -s "$(command -v git)" "$sandbox/bin/git"
printf 'cmake_minimum_required(VERSION 3.24)\n' > "$sandbox/CMakeLists.txt"
printf 'all:\n\t@true\n' > "$sandbox/Makefile"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
