#pragma once
#include <functional>
#include <string>
#include <vector>

#include "devlite/facts.hpp"
#include "devlite/finding.hpp"

namespace devlite {

// Rules are PURE functions of Facts: no I/O, no exceptions, no globals.
using Rule = std::function<std::vector<Finding>(const Facts&)>;

struct RegisteredRule {
  std::string id;
  std::string group;  // "python" | "path" | "git" | "venv" | "repo" | "build"
  Rule fn;
};

const std::vector<RegisteredRule>& all_rules();

// Runs rules in registry order, so output is deterministic.
std::vector<Finding> evaluate(const Facts& facts,
                              const std::vector<std::string>& only_groups = {});

}  // namespace devlite
