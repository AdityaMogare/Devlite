#!/usr/bin/env bash
# A .venv directory exists and VIRTUAL_ENV points somewhere else.
# Expect: venv.dir-inactive
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home" "$sandbox/.venv"
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
ln -s "$(command -v git)" "$sandbox/bin/git"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" VIRTUAL_ENV="/other/venv" \
  "$DEVLITE" doctor --json --path "$sandbox"
