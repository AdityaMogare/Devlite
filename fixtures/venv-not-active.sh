#!/usr/bin/env bash
# VIRTUAL_ENV names a directory that is not python3's prefix. Expect: venv.not-active
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home" "$sandbox/venv"
cat > "$sandbox/bin/python3" <<'PY'
#!/bin/sh
printf '%s\n' \
  'version=3.12.4' \
  'executable=/usr/bin/python3' \
  'prefix=/usr' \
  'base_prefix=/usr' \
  'stdlib=/usr/lib/python3.12' \
  'has_pip=1' \
  'externally_managed=0'
PY
chmod +x "$sandbox/bin/python3"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" VIRTUAL_ENV="$sandbox/venv" \
  "$DEVLITE" doctor --json --path "$sandbox"
