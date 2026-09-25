#pragma once
#include "devlite/rule.hpp"

namespace devlite {

// One free function per rule. Pure: Facts in, Findings out.
std::vector<Finding> rule_path_shadowed(const Facts& facts);
std::vector<Finding> rule_path_empty_entry(const Facts& facts);
std::vector<Finding> rule_git_identity_unset(const Facts& facts);
std::vector<Finding> rule_python_not_found(const Facts& facts);
std::vector<Finding> rule_python_version_unknown(const Facts& facts);
std::vector<Finding> rule_python_no_pip(const Facts& facts);
std::vector<Finding> rule_python_externally_managed(const Facts& facts);
std::vector<Finding> rule_python_pip_mismatch(const Facts& facts);
std::vector<Finding> rule_venv_not_active(const Facts& facts);
std::vector<Finding> rule_venv_unactivated(const Facts& facts);
std::vector<Finding> rule_repo_requires_python(const Facts& facts);

}  // namespace devlite
