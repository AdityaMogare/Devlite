#include "devlite/platform.hpp"

#include <cstdlib>

namespace devlite {
namespace {

class RealEnvironment final : public Environment {
 public:
  std::optional<std::string> get(const std::string& name) const override {
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) return std::nullopt;
    return std::string(value);
  }
};

}  // namespace

std::unique_ptr<Environment> make_real_environment() {
  return std::make_unique<RealEnvironment>();
}

}  // namespace devlite
