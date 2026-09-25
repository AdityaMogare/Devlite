# Proof metrics

These numbers are collected outside the binary. `devlite doctor` does not phone home, count installs, or time itself. A doctor run stays on the machine.

Each number below is allowed to claim only what its source measured. Do not turn one of them into a field rate, a percentage the program computed, or a count of Homebrew installs.

## Downloads

`scripts/github_counts.sh` asks the GitHub API for `AdityaMogare/Devlite` and prints a snapshot:

- 14-day clone count and unique cloners, from the repository traffic API
- 14-day page views, from the same traffic API
- Download count for each asset on the latest release, when a release exists

That is GitHub's own 14-day window, not lifetime installs. A personal Homebrew tap does not receive install counts, so the script does not print one. The script does not write a number back into the repo. Traffic endpoints need a token that can see the repository; without one, the script stops and prints nothing to treat as a count.

## Lab false-positive rate

`tests/integration/run_fixtures.sh` runs the `clean` fixture, which must emit no `rule_id`. When that fixture passes, the harness prints:

```text
lab false positives: 0 (clean fixture)
```

A failed clean fixture stays a failure and does not print a zero. This is the rate on the scripted healthy machine. It is not a rate from real developer machines.

## Field false-positive rate

`scripts/label_findings.sh` runs `devlite doctor --json` and writes a CSV with `rule_id`, `status`, `title`, and an empty `correct` column. A person opens that file and fills `correct` with `yes` or `no` for each row. Given the filled CSV, the script prints `wrong / warnings`: how many warning rows were marked `no`, over how many warning rows were in the sheet.

An empty `correct` cell is unlabeled. It is not counted as wrong. The sample size to report is ten to twenty machines. The repo does not contain a filled sample, and the script does not invent one.

## Time saved

No clock is added to `devlite`. Time saved is a comparison a person records by hand.

Use the same broken fixture for both timings. For each person:

1. Start from the fixture's broken environment and no `devlite` report. Write down the clock time when they start, and the clock time when they can explain what is wrong and what they would change. The difference is the manual time, in minutes.
2. Reset to that same fixture. Run `devlite doctor` and read the report. Write down the clock time when they start reading, and the clock time when they can explain the same problem from the report. The difference is the report time, in minutes.
3. Subtract report time from manual time for that person.

Across the people who completed both timings, report the median of those differences. Do not publish a percentage computed by the program. A person who only did one of the two timings is left out of the median.
