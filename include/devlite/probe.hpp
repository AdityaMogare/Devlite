#pragma once
#include "devlite/facts.hpp"
#include "devlite/platform.hpp"

namespace devlite {

// Probes record what they saw, including failures. They never judge.
void probe_platform(Facts& facts, const Environment& env);
void probe_path(Facts& facts, const Environment& env);
void probe_git(Facts& facts, ProcessRunner& runner);

struct ProbeOptions {
  fs::path working_directory;
};

// Fixed order: platform, path, git. (python, repo: see SPEC section 6)
Facts gather_facts(const ProbeOptions& opts, Environment& env, ProcessRunner& runner);

}  // namespace devlite
