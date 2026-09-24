#pragma once
#include "devlite/rule.hpp"

namespace devlite {

// One free function per rule. Pure: Facts in, Findings out.
std::vector<Finding> rule_path_shadowed(const Facts& facts);
std::vector<Finding> rule_path_empty_entry(const Facts& facts);
std::vector<Finding> rule_git_identity_unset(const Facts& facts);

}  // namespace devlite
