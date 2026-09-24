#include <nlohmann/json.hpp>

#include "devlite/format.hpp"

namespace devlite {

std::string format_json(const Facts& facts, const std::vector<Finding>& findings) {
  nlohmann::ordered_json root;
  root["schema_version"] = 1;
  root["devlite_version"] = DEVLITE_VERSION;
  root["platform"] = {{"os", facts.platform.os}, {"arch", facts.platform.arch}};

  int pass = 0, warn = 0, fail = 0, skipped = 0, error = 0;
  nlohmann::ordered_json items = nlohmann::ordered_json::array();
  for (const auto& f : findings) {
    switch (f.status) {
      case Status::Pass: ++pass; break;
      case Status::Warn: ++warn; break;
      case Status::Fail: ++fail; break;
      case Status::Skipped: ++skipped; break;
      case Status::Error: ++error; break;
    }
    items.push_back({{"rule_id", f.rule_id},
                     {"status", std::string(to_string(f.status))},
                     {"title", f.title},
                     {"explanation", f.explanation},
                     {"suggested_actions", f.suggested_actions},
                     {"evidence", f.evidence}});
  }

  root["summary"] = {{"pass", pass}, {"warn", warn},       {"fail", fail},
                     {"skipped", skipped}, {"error", error}};
  root["findings"] = items;
  return root.dump(2) + "\n";
}

}  // namespace devlite
