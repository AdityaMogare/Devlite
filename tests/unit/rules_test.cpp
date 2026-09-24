#include <gtest/gtest.h>

#include "devlite/rules.hpp"
#include "fake_environment.hpp"

namespace devlite {
namespace {

using testing::FactsBuilder;

// Every rule needs three cases: it fires, it stays silent, and it survives
// facts that were never gathered. See SPEC section 10.

TEST(PathShadowed, FiresOncePerShadowedTool) {
  const Facts f = FactsBuilder{}
                      .with_tool("python3", {"/opt/homebrew/bin/python3", "/usr/bin/python3"})
                      .with_tool("git", {"/usr/bin/git"})
                      .build();
  const auto findings = rule_path_shadowed(f);
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "path.shadowed");
  EXPECT_EQ(findings[0].status, Status::Warn);
  ASSERT_EQ(findings[0].evidence.size(), 2u);
  EXPECT_NE(findings[0].evidence[0].find("(used)"), std::string::npos);
}

TEST(PathShadowed, SilentWhenEveryToolResolvesOnce) {
  const Facts f = FactsBuilder{}.with_tool("git", {"/usr/bin/git"}).build();
  EXPECT_TRUE(rule_path_shadowed(f).empty());
}

TEST(PathShadowed, SilentWhenNoToolsWereProbed) {
  EXPECT_TRUE(rule_path_shadowed(FactsBuilder{}.build()).empty());
}

TEST(PathEmptyEntry, FiresOnEmptyEntry) {
  const Facts f = FactsBuilder{}.with_path_entries({"/usr/bin", "", "/bin"}).build();
  const auto findings = rule_path_empty_entry(f);
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "path.empty-entry");
}

TEST(PathEmptyEntry, SilentOnCleanPath) {
  const Facts f = FactsBuilder{}.with_path_entries({"/usr/bin", "/bin"}).build();
  EXPECT_TRUE(rule_path_empty_entry(f).empty());
}

TEST(PathEmptyEntry, SilentWhenPathWasNeverRead) {
  EXPECT_TRUE(rule_path_empty_entry(FactsBuilder{}.build()).empty());
}

TEST(GitIdentity, FiresWhenBothUnset) {
  const Facts f = FactsBuilder{}
                      .with_tool("git", {"/usr/bin/git"})
                      .with_git_identity(std::nullopt, std::nullopt)
                      .build();
  const auto findings = rule_git_identity_unset(f);
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Warn);
  EXPECT_EQ(findings[0].evidence.size(), 2u);
}

TEST(GitIdentity, SilentWhenConfigured) {
  const Facts f = FactsBuilder{}
                      .with_tool("git", {"/usr/bin/git"})
                      .with_git_identity("Ada", "ada@example.com")
                      .build();
  EXPECT_TRUE(rule_git_identity_unset(f).empty());
}

TEST(GitIdentity, SkippedWhenGitMissing) {
  const auto findings = rule_git_identity_unset(FactsBuilder{}.build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Skipped);
}

TEST(Registry, EvaluatesInRegistryOrderAndFiltersByGroup) {
  const Facts f = FactsBuilder{}
                      .with_tool("python3", {"/a/python3", "/b/python3"})
                      .with_path_entries({""})
                      .with_git_identity(std::nullopt, std::nullopt)
                      .build();

  const auto all = evaluate(f);
  ASSERT_GE(all.size(), 3u);
  EXPECT_EQ(all[0].rule_id, "path.shadowed");
  EXPECT_EQ(all[1].rule_id, "path.empty-entry");

  const auto git_only = evaluate(f, {"git"});
  ASSERT_EQ(git_only.size(), 1u);
  EXPECT_EQ(git_only[0].rule_id, "git.identity-unset");
}

}  // namespace
}  // namespace devlite
