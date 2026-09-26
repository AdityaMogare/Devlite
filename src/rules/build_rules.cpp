#include <algorithm>

#include "devlite/rules.hpp"

namespace devlite {
namespace {

bool has_marker(const Facts& facts, std::string_view name) {
  return std::find(facts.repo.markers.begin(), facts.repo.markers.end(), name) !=
         facts.repo.markers.end();
}

bool tool_present(const Facts& facts, const char* name) {
  const auto it = facts.tools.find(name);
  return it != facts.tools.end() && !it->second.locations.empty();
}

}  // namespace

std::vector<Finding> rule_build_tool_missing(const Facts& facts) {
  std::vector<Finding> findings;
  if (has_marker(facts, "CMakeLists.txt") && !tool_present(facts, "cmake")) {
    findings.push_back(Finding{
        "build.tool-missing", Status::Warn, "CMake was not found",
        "This project has a CMakeLists.txt, but cmake is not on your PATH, so it cannot "
        "be configured or built.",
        {"Install CMake with `brew install cmake`"},
        {"CMakeLists.txt"}});
  }
  if (has_marker(facts, "Makefile") && !tool_present(facts, "make")) {
    findings.push_back(Finding{
        "build.tool-missing", Status::Warn, "make was not found",
        "This project has a Makefile, but make is not on your PATH, so it cannot be built.",
        {"Install the Xcode Command Line Tools with `xcode-select --install`"},
        {"Makefile"}});
  }
  return findings;
}

}  // namespace devlite
