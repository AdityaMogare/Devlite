#include <algorithm>
#include <sstream>

#include "devlite/format.hpp"

namespace devlite {
namespace {

struct Palette {
  const char* fail;
  const char* warn;
  const char* pass;
  const char* dim;
  const char* reset;
};

Palette palette(bool color) {
  if (!color) return {"", "", "", "", ""};
  return {"\033[31m", "\033[33m", "\033[32m", "\033[2m", "\033[0m"};
}

const char* label(Status s) {
  switch (s) {
    case Status::Fail: return "FAIL";
    case Status::Warn: return "WARN";
    case Status::Pass: return "PASS";
    case Status::Skipped: return "SKIP";
    case Status::Error: return "ERR ";
  }
  return "ERR ";
}

int rank(Status s) {
  switch (s) {
    case Status::Fail: return 0;
    case Status::Error: return 1;
    case Status::Warn: return 2;
    case Status::Skipped: return 3;
    case Status::Pass: return 4;
  }
  return 5;
}

}  // namespace

std::string format_text(const std::vector<Finding>& findings, const FormatOptions& opts) {
  const Palette p = palette(opts.color);
  std::ostringstream out;

  size_t pass = 0, warn = 0, fail = 0, skipped = 0, error = 0;
  for (const auto& f : findings) {
    switch (f.status) {
      case Status::Pass: ++pass; break;
      case Status::Warn: ++warn; break;
      case Status::Fail: ++fail; break;
      case Status::Skipped: ++skipped; break;
      case Status::Error: ++error; break;
    }
  }

  // Stable sort keeps registry order within a status group.
  std::vector<const Finding*> ordered;
  ordered.reserve(findings.size());
  for (const auto& f : findings) ordered.push_back(&f);
  std::stable_sort(ordered.begin(), ordered.end(),
                   [](const Finding* a, const Finding* b) {
                     return rank(a->status) < rank(b->status);
                   });

  out << "\n";
  for (const Finding* f : ordered) {
    if (!opts.verbose && (f->status == Status::Pass || f->status == Status::Skipped)) {
      continue;
    }
    const char* color = f->status == Status::Fail   ? p.fail
                        : f->status == Status::Warn ? p.warn
                                                    : p.pass;
    out << "  " << color << label(f->status) << p.reset << "  " << f->title << "\n";
    if (!f->explanation.empty()) out << "        " << f->explanation << "\n";
    for (const auto& action : f->suggested_actions) out << "        -> " << action << "\n";
    for (const auto& ev : f->evidence) out << "        " << p.dim << ev << p.reset << "\n";
    out << "\n";
  }

  out << "  " << pass << " passed, " << warn << " warning" << (warn == 1 ? "" : "s")
      << ", " << fail << " failure" << (fail == 1 ? "" : "s");
  if (skipped > 0) out << ", " << skipped << " skipped";
  if (error > 0) out << ", " << error << " internal error" << (error == 1 ? "" : "s");
  out << "\n";
  return out.str();
}

}  // namespace devlite
