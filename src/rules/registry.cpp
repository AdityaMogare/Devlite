#include <algorithm>

#include "devlite/rules.hpp"

namespace devlite {

const std::vector<RegisteredRule>& all_rules() {
  // Registry order IS output order. Adding a rule here is the only wiring needed.
  static const std::vector<RegisteredRule> rules = {
      {"path.shadowed", "path", rule_path_shadowed},
      {"path.empty-entry", "path", rule_path_empty_entry},
      {"git.identity-unset", "git", rule_git_identity_unset},
      {"git.not-found", "git", rule_git_not_found},
      {"python.not-found", "python", rule_python_not_found},
      {"python.version-unknown", "python", rule_python_version_unknown},
      {"python.no-pip", "python", rule_python_no_pip},
      {"python.externally-managed", "python", rule_python_externally_managed},
      {"python.pip-mismatch", "python", rule_python_pip_mismatch},
      {"python.command-mismatch", "python", rule_python_command_mismatch},
      {"venv.not-active", "venv", rule_venv_not_active},
      {"venv.unactivated", "venv", rule_venv_unactivated},
      {"venv.dir-inactive", "venv", rule_venv_dir_inactive},
      {"repo.requires-python", "repo", rule_repo_requires_python},
      {"build.tool-missing", "build", rule_build_tool_missing},
  };
  return rules;
}

std::vector<Finding> evaluate(const Facts& facts,
                              const std::vector<std::string>& only_groups) {
  std::vector<Finding> findings;
  for (const auto& rule : all_rules()) {
    if (!only_groups.empty() &&
        std::find(only_groups.begin(), only_groups.end(), rule.group) == only_groups.end()) {
      continue;
    }
    auto produced = rule.fn(facts);
    findings.insert(findings.end(), std::make_move_iterator(produced.begin()),
                    std::make_move_iterator(produced.end()));
  }
  return findings;
}

}  // namespace devlite
