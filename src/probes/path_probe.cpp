#include <system_error>

#include "devlite/probe.hpp"

namespace devlite {
namespace {

// Tools worth resolving. Order is irrelevant; the map keeps output stable.
constexpr const char* kProbedTools[] = {"python", "python3", "pip", "pip3",
                                        "git",    "cmake",   "make"};

// Homebrew often links both names at one interpreter. Record one path so the
// mismatch rule does not treat that as two programs.
void align_same_python_file(Facts& facts) {
  auto plain = facts.tools.find("python");
  auto versioned = facts.tools.find("python3");
  if (plain == facts.tools.end() || versioned == facts.tools.end()) return;
  if (plain->second.locations.empty() || versioned->second.locations.empty()) return;

  std::error_code plain_ec;
  std::error_code versioned_ec;
  const fs::path plain_file =
      fs::weakly_canonical(plain->second.locations.front(), plain_ec);
  const fs::path versioned_file =
      fs::weakly_canonical(versioned->second.locations.front(), versioned_ec);
  if (plain_ec || versioned_ec || plain_file != versioned_file) return;
  plain->second.locations.front() = versioned->second.locations.front();
}

}  // namespace

void probe_path(Facts& facts, const Environment& env) {
  if (const auto raw = env.get("PATH")) {
    facts.path_entries = split_path(*raw);
  }
  if (const auto venv = env.get("VIRTUAL_ENV"); venv && !venv->empty()) {
    facts.virtual_env = fs::path(*venv);
  }

  for (const char* name : kProbedTools) {
    ToolInfo info;
    info.name = name;
    info.locations = find_all_on_path(name, env);
    if (info.locations.empty()) info.probe_error = "not_found";
    facts.tools[name] = std::move(info);
  }
  align_same_python_file(facts);
}

}  // namespace devlite
