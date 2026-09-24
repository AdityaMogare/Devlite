#include <sys/utsname.h>

#include "devlite/probe.hpp"

namespace devlite {

void probe_platform(Facts& facts, const Environment& env) {
  utsname info{};
  if (::uname(&info) == 0) {
    const std::string sysname(info.sysname);
    if (sysname == "Darwin") {
      facts.platform.os = "macos";
    } else if (sysname == "Linux") {
      facts.platform.os = "linux";
    } else {
      facts.platform.os = sysname;
    }
    facts.platform.arch = info.machine;
    facts.platform.os_version = info.release;
  }

  if (const auto shell = env.get("SHELL")) {
    facts.platform.shell = fs::path(*shell).filename().string();
  }
}

}  // namespace devlite
