#include "devlite/rules.hpp"

namespace devlite {

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

}  // namespace devlite
