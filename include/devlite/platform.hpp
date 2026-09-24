#pragma once
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace devlite {
namespace fs = std::filesystem;

struct ProcessResult {
  int exit_code = -1;
  std::string stdout_text;
  std::string stderr_text;
  bool timed_out = false;
  bool spawn_failed = false;
};

class Environment {
 public:
  virtual ~Environment() = default;
  virtual std::optional<std::string> get(const std::string& name) const = 0;
};

class ProcessRunner {
 public:
  virtual ~ProcessRunner() = default;
  // exe must be an absolute path. argv array only: never a shell string.
  virtual ProcessResult run(const fs::path& exe,
                            const std::vector<std::string>& args,
                            std::chrono::milliseconds timeout) = 0;
};

std::unique_ptr<Environment> make_real_environment();
std::unique_ptr<ProcessRunner> make_process_runner();

// Every PATH hit for name, in PATH order. Empty PATH entries are skipped here
// but recorded by the path probe, because an empty entry is itself a finding.
std::vector<fs::path> find_all_on_path(std::string_view name, const Environment& env);

// PATH split on ':', empty entries preserved.
std::vector<std::string> split_path(const std::string& path_value);

// First N.N or N.N.N sequence in the text, or nullopt.
std::optional<std::string> parse_version(std::string_view text);

}  // namespace devlite
