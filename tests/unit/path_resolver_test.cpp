#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "fake_environment.hpp"

namespace devlite {
namespace {

using testing::FakeEnvironment;

class PathResolverTest : public ::testing::Test {
 protected:
  void SetUp() override {
    root_ = fs::temp_directory_path() / fs::path("devlite_test_" + std::to_string(::getpid()));
    fs::create_directories(root_ / "a");
    fs::create_directories(root_ / "b");
  }
  void TearDown() override {
    std::error_code ec;
    fs::remove_all(root_, ec);
  }
  fs::path make_exe(const std::string& dir, const std::string& name, bool executable = true) {
    const fs::path p = root_ / dir / name;
    std::ofstream(p) << "#!/bin/sh\n";
    fs::permissions(p, executable ? fs::perms::owner_all : fs::perms::owner_read);
    return p;
  }
  fs::path root_;
};

TEST_F(PathResolverTest, SplitKeepsEmptyEntries) {
  EXPECT_EQ(split_path("/usr/bin::/bin"), (std::vector<std::string>{"/usr/bin", "", "/bin"}));
  EXPECT_EQ(split_path(":/bin"), (std::vector<std::string>{"", "/bin"}));
}

TEST_F(PathResolverTest, FindsEveryHitInPathOrder) {
  const auto first = make_exe("a", "python3");
  const auto second = make_exe("b", "python3");
  FakeEnvironment env;
  env.set("PATH", (root_ / "a").string() + ":" + (root_ / "b").string());

  const auto hits = find_all_on_path("python3", env);
  ASSERT_EQ(hits.size(), 2u);
  EXPECT_EQ(hits[0], first);
  EXPECT_EQ(hits[1], second);
}

TEST_F(PathResolverTest, SkipsNonExecutableAndEmptyEntries) {
  make_exe("a", "tool", /*executable=*/false);
  FakeEnvironment env;
  env.set("PATH", (root_ / "a").string() + "::");
  EXPECT_TRUE(find_all_on_path("tool", env).empty());
}

TEST_F(PathResolverTest, NoPathVariableIsNotACrash) {
  FakeEnvironment env;
  EXPECT_TRUE(find_all_on_path("git", env).empty());
}

TEST_F(PathResolverTest, SymlinkedDuplicateIsNotShadowing) {
  make_exe("a", "git");
  fs::create_directory_symlink(root_ / "a", root_ / "link");
  FakeEnvironment env;
  env.set("PATH", (root_ / "a").string() + ":" + (root_ / "link").string());

  // Same file reached two ways: one hit, not a shadowing warning.
  EXPECT_EQ(find_all_on_path("git", env).size(), 1u);
}

TEST(ParseVersion, PicksFirstDottedNumber) {
  EXPECT_EQ(parse_version("git version 2.39.5 (Apple Git-154)"), "2.39.5");
  EXPECT_EQ(parse_version("Python 3.12.4"), "3.12.4");
  EXPECT_EQ(parse_version("cmake version 3.29"), "3.29");
  EXPECT_FALSE(parse_version("no version here").has_value());
  EXPECT_FALSE(parse_version("12345").has_value());
}

}  // namespace
}  // namespace devlite
