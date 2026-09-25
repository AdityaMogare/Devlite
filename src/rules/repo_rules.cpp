#include <cctype>
#include <optional>
#include <vector>

#include "devlite/rules.hpp"

namespace devlite {
namespace {

struct Version {
  int major = 0;
  int minor = 0;
  int patch = 0;
};

std::optional<Version> parse_dotted(std::string_view text) {
  if (text.empty()) return std::nullopt;
  Version v;
  int* parts[] = {&v.major, &v.minor, &v.patch};
  size_t part = 0;
  size_t i = 0;
  while (i < text.size()) {
    if (part >= 3 || !std::isdigit(static_cast<unsigned char>(text[i]))) return std::nullopt;
    int value = 0;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
      value = value * 10 + (text[i] - '0');
      ++i;
    }
    *parts[part++] = value;
    if (i == text.size()) return v;
    if (text[i] != '.' || part == 3) return std::nullopt;
    ++i;
  }
  return std::nullopt;
}

bool older_than(const Version& have, const Version& need) {
  if (have.major != need.major) return have.major < need.major;
  if (have.minor != need.minor) return have.minor < need.minor;
  return have.patch < need.patch;
}

std::string trim_copy(std::string_view text) {
  size_t begin = 0;
  while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
  size_t end = text.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
  return std::string(text.substr(begin, end - begin));
}

// Accepts a single >=X.Y or >=X.Y.Z specifier. Anything else cannot be judged.
std::optional<Version> minimum_version(const std::string& specifier) {
  const std::string text = trim_copy(specifier);
  if (text.size() < 4 || text[0] != '>' || text[1] != '=') return std::nullopt;
  size_t i = 2;
  while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) ++i;
  if (text.find_first_of(",<>!", i) != std::string::npos) return std::nullopt;
  return parse_dotted(std::string_view(text).substr(i));
}

Finding skipped(std::string evidence) {
  return Finding{"repo.requires-python", Status::Skipped, "Python requirement was not checked",
                 "The project asks for a Python version this tool could not compare.",
                 {},
                 {std::move(evidence)}};
}

}  // namespace

std::vector<Finding> rule_repo_requires_python(const Facts& facts) {
  if (!facts.repo.requires_python || facts.repo.requires_python->empty()) return {};
  const std::string& spec = *facts.repo.requires_python;
  const auto need = minimum_version(spec);
  if (!need) return {skipped(spec)};
  if (!facts.python) return {skipped(spec)};
  const auto have = parse_dotted(facts.python->version);
  if (!have) return {skipped(spec)};
  if (!older_than(*have, *need)) return {};
  return {Finding{"repo.requires-python", Status::Warn, "Python is older than this project requires",
                  "pyproject.toml asks for Python " + spec + ", and the python3 on your PATH is " +
                      facts.python->version + ".",
                  {"Install a newer Python, or select it before working in this directory"},
                  {spec, facts.python->version}}};
}

}  // namespace devlite
