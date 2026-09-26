#include <algorithm>

#include "devlite/rules.hpp"

namespace devlite {
namespace {

bool has_marker(const Facts& facts, std::string_view name) {
  return std::find(facts.repo.markers.begin(), facts.repo.markers.end(), name) !=
         facts.repo.markers.end();
}

}  // namespace

std::vector<Finding> rule_venv_not_active(const Facts& facts) {
  if (!facts.virtual_env) return {};
  if (!facts.python) {
    return {Finding{"venv.not-active", Status::Skipped, "Virtualenv was not checked",
                    "VIRTUAL_ENV is set, but python3 could not be inspected, so this "
                    "check could not tell which environment is active.",
                    {},
                    {facts.virtual_env->string()}}};
  }
  if (facts.python->prefix == *facts.virtual_env) return {};
  return {Finding{
      "venv.not-active", Status::Warn, "VIRTUAL_ENV does not match this Python",
      "The VIRTUAL_ENV variable names one environment, but python3 is a different "
      "interpreter. Commands will not use the environment you think is active.",
      {"source \"" + facts.virtual_env->string() + "/bin/activate\""},
      {facts.virtual_env->string(), facts.python->prefix.string()}}};
}

std::vector<Finding> rule_venv_unactivated(const Facts& facts) {
  if (!facts.python) return {};
  if (facts.python->prefix == facts.python->base_prefix) return {};
  if (facts.virtual_env) return {};
  return {Finding{
      "venv.unactivated", Status::Warn, "This Python is a virtualenv that is not active",
      "python3 points at a virtualenv, but VIRTUAL_ENV is unset. Scripts and pip may "
      "not agree on which environment they are using.",
      {"source \"" + facts.python->prefix.string() + "/bin/activate\""},
      {facts.python->prefix.string()}}};
}

std::vector<Finding> rule_venv_dir_inactive(const Facts& facts) {
  if (!has_marker(facts, ".venv")) return {};
  const fs::path expected = (facts.working_directory / ".venv").lexically_normal();
  if (facts.virtual_env && facts.virtual_env->lexically_normal() == expected) return {};
  std::vector<std::string> evidence{expected.string()};
  if (facts.virtual_env) evidence.push_back(facts.virtual_env->string());
  return {Finding{
      "venv.dir-inactive", Status::Warn, "This project's virtualenv is not active",
      "A .venv folder is in this directory, but VIRTUAL_ENV does not point at it. "
      "Commands will use a different Python than the one this project set up.",
      {"source \"" + expected.string() + "/bin/activate\""},
      std::move(evidence)}};
}

}  // namespace devlite
