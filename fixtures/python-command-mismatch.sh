#!/usr/bin/env bash
# python and python3 are different files. Expect: python.command-mismatch
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
printf '#!/bin/sh\necho "Python 2.7.18"\n' > "$sandbox/bin/python"
printf '#!/bin/sh\necho "Python 3.12.4"\n' > "$sandbox/bin/python3"
chmod +x "$sandbox/bin/python" "$sandbox/bin/python3"
ln -s "$(command -v git)" "$sandbox/bin/git"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
