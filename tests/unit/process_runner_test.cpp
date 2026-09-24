#include <gtest/gtest.h>

#include <sys/wait.h>

#include "devlite/platform.hpp"

namespace devlite {
namespace {

constexpr auto kShort = std::chrono::milliseconds(2000);

fs::path shell() { return fs::exists("/bin/sh") ? "/bin/sh" : "/usr/bin/sh"; }

TEST(ProcessRunner, CapturesStdoutAndExitZero) {
  auto runner = make_process_runner();
  const auto r = runner->run(shell(), {"-c", "echo hello"}, kShort);
  EXPECT_FALSE(r.spawn_failed);
  EXPECT_FALSE(r.timed_out);
  EXPECT_EQ(r.exit_code, 0);
  EXPECT_EQ(r.stdout_text, "hello\n");
}

TEST(ProcessRunner, CapturesStderrSeparately) {
  auto runner = make_process_runner();
  const auto r = runner->run(shell(), {"-c", "echo oops 1>&2"}, kShort);
  EXPECT_TRUE(r.stdout_text.empty());
  EXPECT_EQ(r.stderr_text, "oops\n");
}

TEST(ProcessRunner, ReportsNonZeroExit) {
  auto runner = make_process_runner();
  EXPECT_EQ(runner->run(shell(), {"-c", "exit 7"}, kShort).exit_code, 7);
}

TEST(ProcessRunner, MissingExecutableFailsToSpawn) {
  auto runner = make_process_runner();
  const auto r = runner->run("/nonexistent/binary", {}, kShort);
  EXPECT_TRUE(r.spawn_failed);
}

// The one that matters: a hung child must be killed, not waited on forever.
TEST(ProcessRunner, TimeoutKillsTheChild) {
  auto runner = make_process_runner();
  const auto start = std::chrono::steady_clock::now();
  const auto r = runner->run(shell(), {"-c", "sleep 30"}, std::chrono::milliseconds(300));
  const auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_TRUE(r.timed_out);
  EXPECT_LT(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count(), 3);
  // No orphan: every child has been reaped, so there is nothing left to wait for.
  EXPECT_EQ(::waitpid(-1, nullptr, WNOHANG), -1);
}

}  // namespace
}  // namespace devlite
