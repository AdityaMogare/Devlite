#include "devlite/rules.hpp"

namespace devlite {
namespace {

const PythonInfo* python_info(const Facts& facts) {
  return facts.python ? &*facts.python : nullptr;
}

bool python_on_path(const Facts& facts) {
  const auto it = facts.tools.find("python3");
  return it != facts.tools.end() && !it->second.locations.empty();
}

}  // namespace

std::vector<Finding> rule_python_not_found(const Facts& facts) {
  if (python_on_path(facts) || python_info(facts) != nullptr) return {};
  return {Finding{
      "python.not-found", Status::Warn, "Python 3 was not found",
      "python3 is not on your PATH, so this machine cannot run Python projects.",
      {"Install Python 3 from python.org or with `brew install python`"},
      {}}};
}

std::vector<Finding> rule_python_version_unknown(const Facts& facts) {
  const PythonInfo* info = python_info(facts);
  if (info == nullptr || !info->version.empty()) return {};
  return {Finding{"python.version-unknown", Status::Warn, "Python reported no version",
                  "python3 ran, but it did not report a version, so later checks cannot "
                  "tell whether it is new enough for a project.",
                  {"Run `python3 --version` and reinstall Python if it prints nothing"},
                  {info->executable.string()}}};
}

std::vector<Finding> rule_python_no_pip(const Facts& facts) {
  const PythonInfo* info = python_info(facts);
  if (info == nullptr || info->has_pip) return {};
  return {Finding{"python.no-pip", Status::Warn, "This Python has no pip",
                  "You cannot install packages for this interpreter until pip is available.",
                  {"python3 -m ensurepip --upgrade"},
                  {info->executable.string()}}};
}

std::vector<Finding> rule_python_externally_managed(const Facts& facts) {
  const PythonInfo* info = python_info(facts);
  if (info == nullptr || !info->externally_managed) return {};
  if (info->prefix != info->base_prefix) return {};
  return {Finding{
      "python.externally-managed", Status::Warn, "Python blocks system-wide pip installs",
      "This Python is marked externally managed. pip install into it will fail. Use a "
      "virtualenv so packages stay with the project.",
      {"python3 -m venv .venv", "source .venv/bin/activate"},
      {info->prefix.string()}}};
}

std::vector<Finding> rule_python_pip_mismatch(const Facts& facts) {
  const PythonInfo* info = python_info(facts);
  if (info == nullptr || !info->pip_python) return {};
  if (*info->pip_python == info->executable) return {};
  return {Finding{
      "python.pip-mismatch", Status::Warn, "pip3 belongs to a different Python",
      "The pip3 command on your PATH is not the one for the python3 that runs. Packages "
      "you install can land in a different interpreter than the one you execute.",
      {"Use `python3 -m pip` instead of `pip3`"},
      {info->executable.string(), info->pip_python->string()}}};
}

}  // namespace devlite
