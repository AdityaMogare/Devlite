#pragma once
#include <string>
#include <vector>

#include "devlite/facts.hpp"
#include "devlite/finding.hpp"

namespace devlite {

struct FormatOptions {
  bool color = false;
  bool verbose = false;
};

std::string format_text(const std::vector<Finding>& findings, const FormatOptions& opts);
std::string format_json(const Facts& facts, const std::vector<Finding>& findings);

}  // namespace devlite
