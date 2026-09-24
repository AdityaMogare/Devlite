#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "devlite/format.hpp"
#include "devlite/rule.hpp"

namespace devlite {
namespace {

std::string read_golden(const std::string& name) {
  std::ifstream in(std::string(DEVLITE_GOLDEN_DIR) + "/" + name);
  std::ostringstream buf;
  buf << in.rdbuf();
  return buf.str();
}

std::vector<Finding> sample_findings() {
  return {
      Finding{"path.shadowed", Status::Warn, "Multiple copies of git on your PATH",
              "Several copies of this command are installed.",
              {"Run `which -a git`"},
              {"/opt/homebrew/bin/git (used)", "/usr/bin/git"}},
      Finding{"git.identity-unset", Status::Fail, "Git has no name or email configured",
              "Commits will be attributed to the wrong person.",
              {"git config --global user.name \"Your Name\""},
              {}},
      Finding{"path.empty-entry", Status::Pass, "PATH has no empty entries", "", {}, {}},
  };
}

Facts sample_facts() {
  Facts f;
  f.platform.os = "macos";
  f.platform.arch = "arm64";
  return f;
}

TEST(TextFormatter, MatchesGoldenAndOrdersFailuresFirst) {
  FormatOptions opts;
  const std::string actual = format_text(sample_findings(), opts);
  EXPECT_EQ(actual, read_golden("doctor_text.txt"));
  EXPECT_LT(actual.find("FAIL"), actual.find("WARN"));
}

TEST(TextFormatter, HidesPassingChecksUnlessVerbose) {
  FormatOptions quiet;
  EXPECT_EQ(format_text(sample_findings(), quiet).find("PASS"), std::string::npos);
  FormatOptions loud;
  loud.verbose = true;
  EXPECT_NE(format_text(sample_findings(), loud).find("PASS"), std::string::npos);
}

TEST(TextFormatter, EmitsNoEscapeCodesWithoutColor) {
  FormatOptions opts;
  EXPECT_EQ(format_text(sample_findings(), opts).find('\033'), std::string::npos);
}

TEST(JsonFormatter, MatchesGolden) {
  EXPECT_EQ(format_json(sample_facts(), sample_findings()), read_golden("doctor.json"));
}

TEST(JsonFormatter, IsDeterministic) {
  EXPECT_EQ(format_json(sample_facts(), sample_findings()),
            format_json(sample_facts(), sample_findings()));
}

}  // namespace
}  // namespace devlite
