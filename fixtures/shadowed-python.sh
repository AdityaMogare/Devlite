#!/usr/bin/env bash
# Two python3 binaries on PATH. Expect: path.shadowed
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/first" "$sandbox/second" "$sandbox/home"
printf '#!/bin/sh\necho "Python 3.12.4"\n' > "$sandbox/first/python3"
printf '#!/bin/sh\necho "Python 3.9.6"\n' > "$sandbox/second/python3"
chmod +x "$sandbox/first/python3" "$sandbox/second/python3"
cat > "$sandbox/home/.gitconfig" <<'GITCONFIG'
[user]
	name = Fixture User
	email = fixture@example.com
GITCONFIG

env -i HOME="$sandbox/home" \
  PATH="$sandbox/first:$sandbox/second:/usr/bin:/bin" "$DEVLITE" doctor --json
