#pragma once
#include <map>

#include "devlite/facts.hpp"
#include "devlite/platform.hpp"

namespace devlite::testing {

class FakeEnvironment final : public Environment {
 public:
  void set(std::string name, std::string value) { vars_[std::move(name)] = std::move(value); }
  std::optional<std::string> get(const std::string& name) const override {
    const auto it = vars_.find(name);
    if (it == vars_.end()) return std::nullopt;
    return it->second;
  }

 private:
  std::map<std::string, std::string> vars_;
};

// Builds a Facts by hand so rule tests need no real machine state.
class FactsBuilder {
 public:
  FactsBuilder& with_platform(std::string os, std::string arch) {
    facts_.platform.os = std::move(os);
    facts_.platform.arch = std::move(arch);
    return *this;
  }
  FactsBuilder& with_tool(const std::string& name, std::vector<fs::path> locations) {
    ToolInfo info;
    info.name = name;
    info.locations = std::move(locations);
    if (info.locations.empty()) info.probe_error = "not_found";
    facts_.tools[name] = std::move(info);
    return *this;
  }
  FactsBuilder& with_path_entries(std::vector<std::string> entries) {
    facts_.path_entries = std::move(entries);
    return *this;
  }
  FactsBuilder& with_git_identity(std::optional<std::string> name,
                                  std::optional<std::string> email) {
    facts_.git.user_name = std::move(name);
    facts_.git.user_email = std::move(email);
    return *this;
  }
  Facts build() const { return facts_; }

 private:
  Facts facts_;
};

}  // namespace devlite::testing
