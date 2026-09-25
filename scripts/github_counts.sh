#!/usr/bin/env bash
# Prints a GitHub traffic and release-download snapshot for AdityaMogare/Devlite.
# Does not write the numbers anywhere. Traffic routes need a token that can
# read the repository.
#
#   usage: scripts/github_counts.sh
set -euo pipefail

REPO="AdityaMogare/Devlite"
API="https://api.github.com"

fetch() {
  local path="$1"
  if command -v gh >/dev/null 2>&1; then
    gh api "$path"
    return
  fi
  local token="${GITHUB_TOKEN:-${GH_TOKEN:-}}"
  if [[ -z "$token" ]]; then
    echo "github_counts: set GITHUB_TOKEN or authenticate gh" >&2
    exit 1
  fi
  curl -fsSL \
    -H "Authorization: Bearer $token" \
    -H "Accept: application/vnd.github+json" \
    -H "X-GitHub-Api-Version: 2022-11-28" \
    "$API/$path"
}

json_field() {
  local json="$1" field="$2"
  printf '%s' "$json" | python3 -c 'import json,sys
data=json.load(sys.stdin)
value=data
for key in sys.argv[1].split("."):
    value=value[key]
print(value)' "$field"
}

clones="$(fetch "repos/$REPO/traffic/clones")"
echo "14-day clones: $(json_field "$clones" count)"
echo "14-day unique cloners: $(json_field "$clones" uniques)"

views="$(fetch "repos/$REPO/traffic/views")"
echo "14-day page views: $(json_field "$views" count)"

if ! release="$(fetch "repos/$REPO/releases/latest" 2>/dev/null)"; then
  echo "latest release: none"
  exit 0
fi

printf '%s' "$release" | python3 -c 'import json,sys
release=json.load(sys.stdin)
assets=release.get("assets") or []
if not assets:
    print("latest release: no assets")
else:
    tag=release.get("tag_name") or "latest"
    for asset in assets:
        print("downloads %s/%s: %s" % (tag, asset.get("name",""), asset.get("download_count", 0)))
'
