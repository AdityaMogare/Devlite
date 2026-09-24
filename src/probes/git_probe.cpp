#include "devlite/probe.hpp"

namespace devlite {
namespace {

constexpr auto kTimeout = std::chrono::milliseconds(3000);

std::optional<std::string> config_value(ProcessRunner& runner, const fs::path& git,
                                        const std::string& key) {
  // --global only: reading local config would let a hostile repo run commands.
  const auto res = runner.run(git, {"config", "--global", "--get", key}, kTimeout);
  if (res.spawn_failed || res.timed_out || res.exit_code != 0) return std::nullopt;
  std::string value = res.stdout_text;
  while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) value.pop_back();
  if (value.empty()) return std::nullopt;
  return value;
}

}  // namespace

void probe_git(Facts& facts, ProcessRunner& runner) {
  auto it = facts.tools.find("git");
  if (it == facts.tools.end() || it->second.locations.empty()) return;
  const fs::path& git = it->second.locations.front();

  const auto version = runner.run(git, {"--version"}, kTimeout);
  if (version.spawn_failed) {
    it->second.probe_error = "exec_failed";
  } else if (version.timed_out) {
    it->second.probe_error = "timeout";
  } else {
    it->second.version = parse_version(version.stdout_text + version.stderr_text);
  }

  facts.git.user_name = config_value(runner, git, "user.name");
  facts.git.user_email = config_value(runner, git, "user.email");
}

}  // namespace devlite
