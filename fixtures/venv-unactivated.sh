#!/usr/bin/env bash
# python3's prefix differs from base_prefix and VIRTUAL_ENV is unset.
# Expect: venv.unactivated
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
cat > "$sandbox/bin/python3" <<'PY'
#!/bin/sh
printf '%s\n' \
  'version=3.12.4' \
  'executable=/tmp/venv/bin/python3' \
  'prefix=/tmp/venv' \
  'base_prefix=/usr' \
  'stdlib=/tmp/venv/lib/python3.12' \
  'has_pip=1' \
  'externally_managed=0'
PY
chmod +x "$sandbox/bin/python3"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
