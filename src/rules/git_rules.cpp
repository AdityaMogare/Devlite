#include "devlite/rules.hpp"

namespace devlite {

std::vector<Finding> rule_git_identity_unset(const Facts& facts) {
  const auto it = facts.tools.find("git");
  if (it == facts.tools.end() || it->second.locations.empty()) {
    return {Finding{"git.identity-unset", Status::Skipped, "Git not found",
                    "Git is not on your PATH, so its configuration was not checked.",
                    {},
                    {}}};
  }

  std::vector<std::string> missing;
  if (!facts.git.user_name) missing.push_back("user.name");
  if (!facts.git.user_email) missing.push_back("user.email");
  if (missing.empty()) return {};

  std::string which = missing.size() == 2 ? "name or email" : missing.front();
  return {Finding{
      "git.identity-unset", Status::Warn, "Git has no " + which + " configured",
      "Commits you make will be rejected by some hosts or attributed to the wrong "
      "person, and fixing the history afterwards is painful.",
      {"git config --global user.name \"Your Name\"",
       "git config --global user.email \"you@example.com\""},
      std::move(missing)}};
}

std::vector<Finding> rule_git_not_found(const Facts& facts) {
  if (!facts.repo.is_git_repo) return {};
  const auto it = facts.tools.find("git");
  if (it != facts.tools.end() && !it->second.locations.empty()) return {};
  return {Finding{
      "git.not-found", Status::Warn, "Git was not found",
      "This folder is a Git repository, but the git command is not on your PATH. You "
      "cannot commit, push, or see history until it is installed.",
      {"Install Git with `xcode-select --install`"},
      {}}};
}

}  // namespace devlite
