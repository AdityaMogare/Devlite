# devlite

A lightweight developer environment doctor. One command tells you what is wrong
with your setup and how to fix it, in plain English.

**Status: v0.1 scaffold, macOS only.** Milestones 0-2 of the spec are complete
and tested; the Python probe and its rules are the next piece.

## Build and run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
./build/devlite doctor
./build/devlite doctor --json
```

Requires Xcode Command Line Tools (`xcode-select --install`) and CMake
(`brew install cmake ninja`). Dependencies are fetched by CMake; nothing is
vendored.

## Test

```bash
ctest --test-dir build --output-on-failure     # 26 unit + golden tests
./tests/integration/run_fixtures.sh            # 4 fixture environments
```

Sanitizers, after any change under `src/platform/`:

```bash
cmake -S . -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g"
cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
```

## What is here

| Path | Contents |
| --- | --- |
| `include/devlite/facts.hpp` | The data model. Frozen for v1. |
| `src/platform/` | The only code that touches the OS: env, PATH, subprocess |
| `src/probes/` | Gather facts. Platform, PATH and Git are done; Python and repo are not |
| `src/rules/` | Pure functions from Facts to Findings. Three of ten written |
| `src/format/` | Text and JSON output, both golden-tested |
| `fixtures/` | Scripted broken environments, run by the integration harness |

## What is not here yet

- The Python probe (`src/probes/python_probe.cpp`) and its five rules. This is
  the highest-value next step: see spec sections 6 and 7 for the exact
  subprocess invocation and rule table.
- The venv and repo rules.
- Release packaging: Homebrew tap and install script.

## Architecture in one paragraph

Probes gather raw data into one `Facts` struct. Rules are pure functions from
`Facts` to `Finding` values and never touch the OS. This split is what makes
rules testable without a broken machine, removes any dependency ordering between
checks, and keeps output deterministic. Do not blur it.

