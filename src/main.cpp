#include <unistd.h>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <iostream>

#include "devlite/format.hpp"
#include "devlite/probe.hpp"
#include "devlite/rule.hpp"

namespace {

bool should_color(bool no_color_flag) {
  if (no_color_flag) return false;
  if (std::getenv("NO_COLOR") != nullptr) return false;
  return ::isatty(STDOUT_FILENO) == 1;
}

}  // namespace

int main(int argc, char** argv) {
  CLI::App app{"devlite - a developer environment doctor", "devlite"};
  app.set_version_flag("--version", std::string(DEVLITE_VERSION));
  app.require_subcommand(1);

  bool as_json = false;
  bool no_color = false;
  bool verbose = false;
  std::string fail_on = "fail";
  std::string working_dir = ".";
  std::vector<std::string> only;

  auto* doctor = app.add_subcommand("doctor", "Inspect this machine and report problems");
  doctor->add_flag("--json", as_json, "Machine-readable output");
  doctor->add_flag("--no-color", no_color, "Disable ANSI color");
  doctor->add_flag("-v,--verbose", verbose, "Include passing checks");
  doctor->add_option("--fail-on", fail_on, "Exit non-zero at: fail | warn")
      ->check(CLI::IsMember({"fail", "warn"}));
  doctor->add_option("--path", working_dir, "Directory to inspect");
  doctor->add_option("--only", only, "Run one group: path, git, python, venv, repo (repeatable)")
      ->check(CLI::IsMember({"path", "git", "python", "venv", "repo"}));

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e) == 0 ? 0 : 2;
  }

  try {
    auto env = devlite::make_real_environment();
    auto runner = devlite::make_process_runner();

    devlite::ProbeOptions opts;
    opts.working_directory = working_dir;
    const devlite::Facts facts = devlite::gather_facts(opts, *env, *runner);
    const auto findings = devlite::evaluate(facts, only);

    if (as_json) {
      std::cout << devlite::format_json(facts, findings);
    } else {
      devlite::FormatOptions fmt;
      fmt.color = should_color(no_color);
      fmt.verbose = verbose;
      std::cout << devlite::format_text(findings, fmt);
    }

    const bool warn_is_failure = (fail_on == "warn");
    for (const auto& f : findings) {
      if (f.status == devlite::Status::Fail) return 1;
      if (warn_is_failure && f.status == devlite::Status::Warn) return 1;
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "devlite: internal error: " << e.what() << "\n";
    return 3;
  }
}
