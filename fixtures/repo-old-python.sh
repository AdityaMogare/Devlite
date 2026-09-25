#!/usr/bin/env bash
# pyproject.toml requires >=3.11 and python3 reports 3.9.6. Expect: repo.requires-python
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
cat > "$sandbox/bin/python3" <<'PY'
#!/bin/sh
printf '%s\n' \
  'version=3.9.6' \
  'executable=/usr/bin/python3' \
  'prefix=/usr' \
  'base_prefix=/usr' \
  'stdlib=/usr/lib/python3.9' \
  'has_pip=1' \
  'externally_managed=0'
PY
chmod +x "$sandbox/bin/python3"
cat > "$sandbox/pyproject.toml" <<'TOML'
[project]
name = "fixture"
requires-python = ">=3.11"
TOML
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
