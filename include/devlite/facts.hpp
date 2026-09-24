// Frozen for v1. See SPEC section 4 before changing anything here.
#pragma once
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace devlite {
namespace fs = std::filesystem;

struct PlatformInfo {
  std::string os;          // "macos" (linux later)
  std::string arch;        // "arm64" | "x86_64"
  std::string os_version;  // best-effort, may be empty
  std::string shell;       // basename of $SHELL, may be empty
};

struct ToolInfo {
  std::string name;                        // "python3", "git", "pip3"
  std::vector<fs::path> locations;         // every PATH hit, in order
  std::optional<std::string> version;      // parsed, e.g. "3.12.4"
  std::optional<std::string> probe_error;  // "not_found" | "timeout" | "exec_failed"
};

struct PythonInfo {
  fs::path executable;
  std::string version;
  fs::path prefix;
  fs::path base_prefix;
  fs::path stdlib;
  bool has_pip = false;
  bool externally_managed = false;
  std::optional<fs::path> pip_python;
};

struct GitInfo {
  std::optional<std::string> user_name;
  std::optional<std::string> user_email;
};

struct RepoInfo {
  bool is_git_repo = false;
  std::vector<std::string> markers;
  std::optional<std::string> requires_python;
};

struct Facts {
  PlatformInfo platform;
  std::map<std::string, ToolInfo> tools;
  std::optional<PythonInfo> python;
  std::optional<fs::path> virtual_env;
  std::vector<std::string> path_entries;  // PATH split, empties preserved
  GitInfo git;
  RepoInfo repo;
  fs::path working_directory;
};

}  // namespace devlite
