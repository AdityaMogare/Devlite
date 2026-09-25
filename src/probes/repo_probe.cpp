#include <fstream>
#include <system_error>

#include "devlite/probe.hpp"

namespace devlite {
namespace {

bool is_file_or_dir(const fs::path& path) {
  std::error_code ec;
  const auto status = fs::status(path, ec);
  if (ec) return false;
  return fs::is_regular_file(status) || fs::is_directory(status);
}

std::optional<std::string> quoted_after(const std::string& line, std::string_view key) {
  const auto at = line.find(key);
  if (at == std::string::npos) return std::nullopt;
  const auto eq = line.find('=', at + key.size());
  if (eq == std::string::npos) return std::nullopt;
  const auto first = line.find_first_of("\"'", eq + 1);
  if (first == std::string::npos) return std::nullopt;
  const char quote = line[first];
  const auto last = line.find(quote, first + 1);
  if (last == std::string::npos) return std::nullopt;
  return line.substr(first + 1, last - first - 1);
}

std::optional<std::string> read_requires_python(const fs::path& pyproject) {
  std::ifstream in(pyproject);
  if (!in) return std::nullopt;
  std::string line;
  while (std::getline(in, line)) {
    if (auto value = quoted_after(line, "requires-python")) return value;
  }
  return std::nullopt;
}

}  // namespace

void probe_repo(Facts& facts) {
  const fs::path& root = facts.working_directory;
  facts.repo.is_git_repo = is_file_or_dir(root / ".git");

  const char* markers[] = {"pyproject.toml", "requirements.txt", "Pipfile", "setup.py"};
  for (const char* name : markers) {
    std::error_code ec;
    if (fs::is_regular_file(root / name, ec) && !ec) facts.repo.markers.emplace_back(name);
  }

  facts.repo.requires_python = read_requires_python(root / "pyproject.toml");
}

}  // namespace devlite
