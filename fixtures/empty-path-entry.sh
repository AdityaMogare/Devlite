#!/usr/bin/env bash
# PATH with a doubled colon, which means the current directory.
# Expect: path.empty-entry
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/home"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="/usr/bin::/bin" "$DEVLITE" doctor --json
