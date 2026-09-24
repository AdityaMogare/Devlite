#include "devlite/probe.hpp"

namespace devlite {
namespace {

// Tools worth resolving. Order is irrelevant; the map keeps output stable.
constexpr const char* kProbedTools[] = {"python", "python3", "pip", "pip3",
                                        "git",    "cmake",   "make"};

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
}

}  // namespace devlite
