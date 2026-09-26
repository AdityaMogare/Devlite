#!/usr/bin/env bash
# A .git directory and no git on PATH. Expect: git.not-found
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/bin" "$sandbox/home" "$sandbox/.git"
printf '#!/bin/sh\necho "Python 3.12.4"\n' > "$sandbox/bin/python3"
chmod +x "$sandbox/bin/python3"

env -i HOME="$sandbox/home" PATH="$sandbox/bin" "$DEVLITE" doctor --json --path "$sandbox"
