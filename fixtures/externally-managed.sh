#!/usr/bin/env bash
# python3 reports an externally managed base install. Expect: python.externally-managed
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home"
cat > "$sandbox/bin/python3" <<'PY'
#!/bin/sh
printf '%s\n' \
  'version=3.12.4' \
  'executable=/usr/bin/python3' \
  'prefix=/usr' \
  'base_prefix=/usr' \
  'stdlib=/usr/lib/python3.12' \
  'has_pip=1' \
  'externally_managed=1'
PY
chmod +x "$sandbox/bin/python3"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
