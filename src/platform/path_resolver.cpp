#include <unistd.h>

#include <cctype>
#include <set>
#include <system_error>

#include "devlite/platform.hpp"

namespace devlite {

std::vector<std::string> split_path(const std::string& path_value) {
  std::vector<std::string> entries;
  std::string current;
  for (char c : path_value) {
    if (c == ':') {
      entries.push_back(current);
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  entries.push_back(current);
  return entries;
}

std::vector<fs::path> find_all_on_path(std::string_view name, const Environment& env) {
  std::vector<fs::path> hits;
  const auto raw = env.get("PATH");
  if (!raw) return hits;

  // Two PATH entries can reach the SAME file: /bin is a symlink to /usr/bin on
  // macOS, and Homebrew fills /opt/homebrew/bin with links into Cellar. Those
  // are not shadowing, so dedupe by resolved target and keep the first name.
  std::set<fs::path> seen_targets;

  for (const auto& entry : split_path(*raw)) {
    if (entry.empty()) continue;  // recorded by the path probe, not resolved here
    fs::path candidate = fs::path(entry) / std::string(name);
    std::error_code ec;
    if (!fs::is_regular_file(candidate, ec) || ec) continue;
    if (::access(candidate.c_str(), X_OK) != 0) continue;

    fs::path target = fs::weakly_canonical(candidate, ec);
    if (ec) target = candidate;
    if (!seen_targets.insert(target).second) continue;

    hits.push_back(candidate);
  }
  return hits;
}

std::optional<std::string> parse_version(std::string_view text) {
  for (size_t i = 0; i < text.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(text[i]))) continue;
    if (i > 0 && (std::isdigit(static_cast<unsigned char>(text[i - 1])) ||
                  text[i - 1] == '.')) {
      continue;
    }
    size_t j = i;
    int dots = 0;
    bool last_was_digit = false;
    while (j < text.size()) {
      char c = text[j];
      if (std::isdigit(static_cast<unsigned char>(c))) {
        last_was_digit = true;
        ++j;
      } else if (c == '.' && last_was_digit && dots < 2 && j + 1 < text.size() &&
                 std::isdigit(static_cast<unsigned char>(text[j + 1]))) {
        ++dots;
        last_was_digit = false;
        ++j;
      } else {
        break;
      }
    }
    if (dots >= 1) return std::string(text.substr(i, j - i));
  }
  return std::nullopt;
}

}  // namespace devlite
