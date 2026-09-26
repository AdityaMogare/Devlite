# devlite

A lightweight developer environment doctor. One command tells you what is wrong
with your setup and how to fix it, in plain English.

**Status: v0.1, macOS only.** Path, Git, Python, virtualenv, repo, and build checks are implemented.

## Install

macOS 11 or newer:

```bash
brew tap adityamogare/devlite https://github.com/AdityaMogare/Devlite.git
brew install devlite
devlite doctor
```

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
ctest --test-dir build --output-on-failure     # 62 unit + golden tests
./tests/integration/run_fixtures.sh            # 14 fixture environments
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
| `src/probes/` | Gather facts. Platform, PATH, Git, Python, and repo |
| `src/rules/` | Pure functions from Facts to Findings |
| `src/format/` | Text and JSON output, both golden-tested |
| `fixtures/` | Scripted broken environments, run by the integration harness |

## What is not here yet

- A standalone install script. Homebrew is the supported install path.

## Architecture in one paragraph

Probes gather raw data into one `Facts` struct. Rules are pure functions from
`Facts` to `Finding` values and never touch the OS. This split is what makes
rules testable without a broken machine, removes any dependency ordering between
checks, and keeps output deterministic. Do not blur it.

