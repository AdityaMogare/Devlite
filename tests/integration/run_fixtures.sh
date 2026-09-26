#!/usr/bin/env bash
# Runs devlite under each fixture environment and asserts which rules fired.
# Docker cannot help here: it only runs Linux images, and v1 ships macOS.
#
#   usage: tests/integration/run_fixtures.sh [path-to-devlite]
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEVLITE="${1:-$ROOT/build/devlite}"

if [[ ! -x "$DEVLITE" ]]; then
  echo "devlite binary not found at $DEVLITE" >&2
  echo "build it first: cmake --build build" >&2
  exit 2
fi

pass=0
fail=0
clean_ok=0

# fixture | rule ids that must be present (space separated, "-" for none)
run_fixture() {
  local name="$1" expected="$2"
  local output
  if ! output="$("$ROOT/fixtures/$name.sh" "$DEVLITE" 2>&1)"; then
    : # non-zero exit is expected when findings exist
  fi

  local ok=1
  if [[ "$expected" == "-" ]]; then
    if grep -q '"rule_id"' <<< "$output"; then ok=0; fi
  else
    for rule in $expected; do
      grep -q "\"rule_id\": \"$rule\"" <<< "$output" || ok=0
    done
  fi

  if [[ $ok -eq 1 ]]; then
    echo "  ok    $name"
    pass=$((pass + 1))
    if [[ "$name" == "clean" ]]; then clean_ok=1; fi
  else
    echo "  FAIL  $name (expected: $expected)"
    echo "$output" | sed 's/^/        /'
    fail=$((fail + 1))
  fi
}

echo "running fixtures against $DEVLITE"
run_fixture clean "-"
if [[ $clean_ok -eq 1 ]]; then
  echo "lab false positives: 0 (clean fixture)"
fi
run_fixture shadowed-python "path.shadowed"
run_fixture empty-path-entry "path.empty-entry"
run_fixture no-git-identity "git.identity-unset"
run_fixture no-python "python.not-found"
run_fixture externally-managed "python.externally-managed"
run_fixture pip-mismatch "python.pip-mismatch"
run_fixture venv-not-active "venv.not-active"
run_fixture venv-unactivated "venv.unactivated"
run_fixture repo-old-python "repo.requires-python"
run_fixture git-not-found "git.not-found"
run_fixture python-command-mismatch "python.command-mismatch"
run_fixture venv-dir-inactive "venv.dir-inactive"
run_fixture build-tool-missing "build.tool-missing"

echo "$pass passed, $fail failed"
[[ $fail -eq 0 ]]
