#include <cctype>

#include "devlite/probe.hpp"

namespace devlite {
namespace {

constexpr auto kTimeout = std::chrono::milliseconds(3000);

// Prints key=value lines. A fixture shim can emit the same lines without
// being a real interpreter.
constexpr char kPythonScript[] = R"PY(
import importlib.util, pathlib, sys, sysconfig
prefix = sys.prefix
stdlib = sysconfig.get_path("stdlib") or ""
has_pip = importlib.util.find_spec("pip") is not None
ext = pathlib.Path(prefix, "EXTERNALLY-MANAGED").is_file()
ver = ".".join(str(p) for p in sys.version_info[:3])
print("version=" + ver)
print("executable=" + sys.executable)
print("prefix=" + prefix)
print("base_prefix=" + sys.base_prefix)
print("stdlib=" + stdlib)
print("has_pip=" + ("1" if has_pip else "0"))
print("externally_managed=" + ("1" if ext else "0"))
)PY";

std::string trim(std::string s) {
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
  return s;
}

std::map<std::string, std::string> parse_kv(const std::string& text) {
  std::map<std::string, std::string> kv;
  std::string line;
  for (size_t i = 0; i <= text.size(); ++i) {
    if (i == text.size() || text[i] == '\n') {
      const auto eq = line.find('=');
      if (eq != std::string::npos) {
        kv[trim(line.substr(0, eq))] = trim(line.substr(eq + 1));
      }
      line.clear();
    } else if (text[i] != '\r') {
      line.push_back(text[i]);
    }
  }
  return kv;
}

bool is_python_filename(const fs::path& path) {
  const std::string name = path.filename().string();
  if (name == "python" || name == "python2" || name == "python3") return true;
  return name.rfind("python3.", 0) == 0;
}

// pip --version names an interpreter only when a token is an absolute path
// whose filename is python or python3. A site-packages path is ignored.
std::optional<fs::path> interpreter_path_in(const std::string& text) {
  std::string token;
  auto consider = [&](std::string word) {
    while (!word.empty() && std::ispunct(static_cast<unsigned char>(word.back()))) {
      word.pop_back();
    }
    if (word.size() < 2 || word[0] != '/') return;
    fs::path path(word);
    if (is_python_filename(path)) token = word;
  };
  std::string word;
  for (size_t i = 0; i <= text.size(); ++i) {
    if (i == text.size() || std::isspace(static_cast<unsigned char>(text[i]))) {
      if (!word.empty()) consider(word);
      word.clear();
    } else {
      word.push_back(text[i]);
    }
  }
  if (token.empty()) return std::nullopt;
  return fs::path(token);
}

void note_probe_error(Facts& facts, const char* error) {
  auto it = facts.tools.find("python3");
  if (it != facts.tools.end()) it->second.probe_error = error;
}

}  // namespace

void probe_python(Facts& facts, ProcessRunner& runner) {
  auto it = facts.tools.find("python3");
  if (it == facts.tools.end() || it->second.locations.empty()) return;
  const fs::path& python = it->second.locations.front();

  const auto res = runner.run(python, {"-c", kPythonScript}, kTimeout);
  if (res.spawn_failed) {
    note_probe_error(facts, "exec_failed");
    return;
  }
  if (res.timed_out) {
    note_probe_error(facts, "timeout");
    return;
  }
  if (res.exit_code != 0) {
    note_probe_error(facts, "exec_failed");
    return;
  }

  const auto kv = parse_kv(res.stdout_text);
  const char* keys[] = {"version", "executable", "prefix", "base_prefix",
                        "stdlib",  "has_pip",    "externally_managed"};
  for (const char* key : keys) {
    if (kv.find(key) == kv.end()) {
      note_probe_error(facts, "exec_failed");
      return;
    }
  }

  PythonInfo info;
  info.version = kv.at("version");
  info.executable = kv.at("executable");
  info.prefix = kv.at("prefix");
  info.base_prefix = kv.at("base_prefix");
  info.stdlib = kv.at("stdlib");
  info.has_pip = kv.at("has_pip") == "1";
  info.externally_managed = kv.at("externally_managed") == "1";

  auto pip = facts.tools.find("pip3");
  if (pip != facts.tools.end() && !pip->second.locations.empty()) {
    const auto pip_res = runner.run(pip->second.locations.front(), {"--version"}, kTimeout);
    if (!pip_res.spawn_failed && !pip_res.timed_out && pip_res.exit_code == 0) {
      info.pip_python = interpreter_path_in(pip_res.stdout_text + pip_res.stderr_text);
    }
  }

  facts.python = std::move(info);
}

}  // namespace devlite
