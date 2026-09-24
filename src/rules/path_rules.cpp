#include "devlite/rules.hpp"

namespace devlite {

std::vector<Finding> rule_path_shadowed(const Facts& facts) {
  std::vector<Finding> findings;
  for (const auto& [name, tool] : facts.tools) {
    if (tool.locations.size() < 2) continue;

    std::vector<std::string> evidence;
    evidence.reserve(tool.locations.size());
    for (size_t i = 0; i < tool.locations.size(); ++i) {
      evidence.push_back(tool.locations[i].string() + (i == 0 ? " (used)" : ""));
    }
    findings.push_back(Finding{
        "path.shadowed", Status::Warn, "Multiple copies of " + name + " on your PATH",
        "Several copies of this command are installed and the first one on your PATH "
        "wins. If you installed a newer version and still see an old one, this is why.",
        {"Run `which -a " + name + "` and reorder your PATH so the copy you want comes first"},
        std::move(evidence)});
  }
  return findings;
}

std::vector<Finding> rule_path_empty_entry(const Facts& facts) {
  size_t empties = 0;
  for (const auto& entry : facts.path_entries) {
    if (entry.empty()) ++empties;
  }
  if (empties == 0) return {};

  return {Finding{
      "path.empty-entry", Status::Warn, "Your PATH contains an empty entry",
      "An empty entry in PATH means the current directory. Any folder you cd into can "
      "then shadow a real command with a file of the same name.",
      {"Check your shell profile for a PATH line with a leading, trailing or doubled ':'"},
      {"empty entries: " + std::to_string(empties)}}};
}

}  // namespace devlite
