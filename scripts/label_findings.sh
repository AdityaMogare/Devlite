#!/usr/bin/env bash
# Turns a doctor JSON report into a labeling sheet, or scores a filled sheet.
#
#   scripts/label_findings.sh [path-to-devlite] > sheet.csv
#   scripts/label_findings.sh --score sheet.csv
#
# The sheet columns are rule_id, status, title, correct. Fill correct with
# yes or no. Scoring prints "wrong / warnings" for warning rows only.
# An empty correct cell is unlabeled, not wrong.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

csv_quote() {
  local value="$1"
  value=${value//\"/\"\"}
  printf '"%s"' "$value"
}

score_sheet() {
  local sheet="$1"
  python3 - "$sheet" <<'PY'
import csv
import sys

wrong = 0
warnings = 0
with open(sys.argv[1], newline="") as handle:
    for row in csv.DictReader(handle):
        if (row.get("status") or "").strip().lower() != "warn":
            continue
        warnings += 1
        if (row.get("correct") or "").strip().lower() == "no":
            wrong += 1
print(f"{wrong} / {warnings}")
PY
}

write_sheet() {
  local devlite="${1:-$ROOT/build/devlite}"
  if [[ ! -x "$devlite" ]]; then
    echo "label_findings: devlite binary not found at $devlite" >&2
    exit 2
  fi
  local report
  report="$("$devlite" doctor --json)"
  printf '%s\n' 'rule_id,status,title,correct'
  printf '%s' "$report" | python3 -c 'import json,sys
def quote(value):
    return "\"" + value.replace("\"", "\"\"") + "\""
report=json.load(sys.stdin)
for finding in report.get("findings") or []:
    print(",".join([
        quote(str(finding.get("rule_id") or "")),
        quote(str(finding.get("status") or "")),
        quote(str(finding.get("title") or "")),
        quote(""),
    ]))
'
}

if [[ "${1:-}" == "--score" ]]; then
  if [[ $# -ne 2 || ! -f "$2" ]]; then
    echo "usage: scripts/label_findings.sh --score sheet.csv" >&2
    exit 2
  fi
  score_sheet "$2"
  exit 0
fi

write_sheet "${1:-}"
