#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace devlite {

enum class Status { Pass, Warn, Fail, Skipped, Error };

constexpr std::string_view to_string(Status s) {
  switch (s) {
    case Status::Pass: return "pass";
    case Status::Warn: return "warn";
    case Status::Fail: return "fail";
    case Status::Skipped: return "skipped";
    case Status::Error: return "error";
  }
  return "error";
}

struct Finding {
  std::string rule_id;
  Status status = Status::Pass;
  std::string title;
  std::string explanation;
  std::vector<std::string> suggested_actions;
  std::vector<std::string> evidence;
};

}  // namespace devlite
