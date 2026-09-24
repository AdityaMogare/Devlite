#!/usr/bin/env bash
# A healthy environment: exactly one python3 and one git on PATH, git identity
# configured, no empty PATH entries. Expect: zero findings, exit 0.
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
printf '#!/bin/sh\necho "Python 3.12.4"\n' > "$sandbox/bin/python3"
chmod +x "$sandbox/bin/python3"
ln -s "$(command -v git)" "$sandbox/bin/git"

cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

# PATH is the sandbox alone, so the machine's own tools cannot leak in and
# make a healthy fixture look shadowed.
env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json
