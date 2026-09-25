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

PythonInfo sample_python() {
  PythonInfo info;
  info.executable = "/usr/bin/python3";
  info.version = "3.12.4";
  info.prefix = "/usr";
  info.base_prefix = "/usr";
  info.stdlib = "/usr/lib/python3.12";
  info.has_pip = true;
  info.externally_managed = false;
  return info;
}

TEST(PythonNotFound, FiresWhenPython3IsAbsent) {
  const auto findings = rule_python_not_found(FactsBuilder{}.build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "python.not-found");
  EXPECT_EQ(findings[0].status, Status::Warn);
}

TEST(PythonNotFound, SilentWhenPythonWasProbed) {
  const Facts f = FactsBuilder{}.with_python(sample_python()).build();
  EXPECT_TRUE(rule_python_not_found(f).empty());
}

TEST(PythonNotFound, SilentWhenPython3IsOnPathButUnprobed) {
  const Facts f = FactsBuilder{}.with_tool("python3", {"/usr/bin/python3"}).build();
  EXPECT_TRUE(rule_python_not_found(f).empty());
}

TEST(PythonVersionUnknown, FiresWhenVersionEmpty) {
  PythonInfo info = sample_python();
  info.version.clear();
  const auto findings = rule_python_version_unknown(FactsBuilder{}.with_python(info).build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Warn);
}

TEST(PythonVersionUnknown, SilentWhenVersionPresent) {
  EXPECT_TRUE(rule_python_version_unknown(FactsBuilder{}.with_python(sample_python()).build()).empty());
}

TEST(PythonVersionUnknown, SilentWhenPythonWasNeverProbed) {
  EXPECT_TRUE(rule_python_version_unknown(FactsBuilder{}.build()).empty());
}

TEST(PythonNoPip, FiresWithoutPip) {
  PythonInfo info = sample_python();
  info.has_pip = false;
  const auto findings = rule_python_no_pip(FactsBuilder{}.with_python(info).build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "python.no-pip");
}

TEST(PythonNoPip, SilentWhenPipPresent) {
  EXPECT_TRUE(rule_python_no_pip(FactsBuilder{}.with_python(sample_python()).build()).empty());
}

TEST(PythonNoPip, SilentWhenPythonWasNeverProbed) {
  EXPECT_TRUE(rule_python_no_pip(FactsBuilder{}.build()).empty());
}

TEST(PythonExternallyManaged, FiresForBaseInstall) {
  PythonInfo info = sample_python();
  info.externally_managed = true;
  const auto findings =
      rule_python_externally_managed(FactsBuilder{}.with_python(info).build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Warn);
}

TEST(PythonExternallyManaged, SilentInsideVirtualenv) {
  PythonInfo info = sample_python();
  info.externally_managed = true;
  info.prefix = "/tmp/venv";
  info.base_prefix = "/usr";
  EXPECT_TRUE(
      rule_python_externally_managed(FactsBuilder{}.with_python(info).build()).empty());
}

TEST(PythonExternallyManaged, SilentWhenPythonWasNeverProbed) {
  EXPECT_TRUE(rule_python_externally_managed(FactsBuilder{}.build()).empty());
}

TEST(PythonPipMismatch, FiresWhenPipPointsElsewhere) {
  PythonInfo info = sample_python();
  info.pip_python = "/other/bin/python3";
  const auto findings = rule_python_pip_mismatch(FactsBuilder{}.with_python(info).build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "python.pip-mismatch");
}

TEST(PythonPipMismatch, SilentWhenPipMatches) {
  PythonInfo info = sample_python();
  info.pip_python = info.executable;
  EXPECT_TRUE(rule_python_pip_mismatch(FactsBuilder{}.with_python(info).build()).empty());
}

TEST(PythonPipMismatch, SilentWhenPipWasNotIdentified) {
  EXPECT_TRUE(rule_python_pip_mismatch(FactsBuilder{}.with_python(sample_python()).build()).empty());
}

TEST(VenvNotActive, FiresWhenPrefixDiffers) {
  const Facts f = FactsBuilder{}
                      .with_python(sample_python())
                      .with_virtual_env("/tmp/venv")
                      .build();
  const auto findings = rule_venv_not_active(f);
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Warn);
}

TEST(VenvNotActive, SilentWhenPrefixMatches) {
  PythonInfo info = sample_python();
  info.prefix = "/tmp/venv";
  const Facts f = FactsBuilder{}.with_python(info).with_virtual_env("/tmp/venv").build();
  EXPECT_TRUE(rule_venv_not_active(f).empty());
}

TEST(VenvNotActive, SkippedWhenPythonWasNeverProbed) {
  const auto findings = rule_venv_not_active(FactsBuilder{}.with_virtual_env("/tmp/venv").build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Skipped);
}

TEST(VenvUnactivated, FiresWhenVenvPrefixAndNoVariable) {
  PythonInfo info = sample_python();
  info.prefix = "/tmp/venv";
  info.base_prefix = "/usr";
  const auto findings = rule_venv_unactivated(FactsBuilder{}.with_python(info).build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].rule_id, "venv.unactivated");
}

TEST(VenvUnactivated, SilentWhenActivated) {
  PythonInfo info = sample_python();
  info.prefix = "/tmp/venv";
  info.base_prefix = "/usr";
  const Facts f = FactsBuilder{}.with_python(info).with_virtual_env("/tmp/venv").build();
  EXPECT_TRUE(rule_venv_unactivated(f).empty());
}

TEST(VenvUnactivated, SilentWhenPythonWasNeverProbed) {
  EXPECT_TRUE(rule_venv_unactivated(FactsBuilder{}.build()).empty());
}

TEST(RepoRequiresPython, FiresWhenOlder) {
  PythonInfo info = sample_python();
  info.version = "3.9.6";
  const Facts f = FactsBuilder{}.with_python(info).with_requires_python(">=3.11").build();
  const auto findings = rule_repo_requires_python(f);
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Warn);
}

TEST(RepoRequiresPython, SilentWhenNewEnough) {
  const Facts f =
      FactsBuilder{}.with_python(sample_python()).with_requires_python(">=3.11").build();
  EXPECT_TRUE(rule_repo_requires_python(f).empty());
}

TEST(RepoRequiresPython, SkippedWhenPythonWasNeverProbed) {
  const auto findings =
      rule_repo_requires_python(FactsBuilder{}.with_requires_python(">=3.11").build());
  ASSERT_EQ(findings.size(), 1u);
  EXPECT_EQ(findings[0].status, Status::Skipped);
}

}  // namespace
}  // namespace devlite
