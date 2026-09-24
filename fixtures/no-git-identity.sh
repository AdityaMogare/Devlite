#!/usr/bin/env bash
# Git present but no global user.name or user.email. Expect: git.identity-unset
set -euo pipefail
DEVLITE="$1"
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/home"
env -i HOME="$sandbox/home" PATH="/usr/bin:/bin" "$DEVLITE" doctor --json
