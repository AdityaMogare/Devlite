// POSIX process runner with a real timeout: std::future cannot cancel a hung
// child, so the deadline is enforced by killing the process. See SPEC section 5.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <thread>

#include "devlite/platform.hpp"

extern char** environ;

namespace devlite {
namespace {

constexpr size_t kMaxCapture = 64 * 1024;
using Clock = std::chrono::steady_clock;

// macOS has no pipe2(), so CLOEXEC is set in a second call. The single-call
// form used in most Linux examples will not compile here.
bool make_pipe(int fds[2]) {
  if (::pipe(fds) != 0) return false;
  for (int i = 0; i < 2; ++i) {
    int flags = ::fcntl(fds[i], F_GETFD);
    if (flags == -1 || ::fcntl(fds[i], F_SETFD, flags | FD_CLOEXEC) == -1) {
      ::close(fds[0]);
      ::close(fds[1]);
      return false;
    }
  }
  return true;
}

void append_capped(std::string& sink, const char* data, size_t len) {
  if (sink.size() >= kMaxCapture) return;
  sink.append(data, std::min(len, kMaxCapture - sink.size()));
}

class PosixProcessRunner final : public ProcessRunner {
 public:
  ProcessResult run(const fs::path& exe, const std::vector<std::string>& args,
                    std::chrono::milliseconds timeout) override {
    ProcessResult result;

    int out_fds[2];
    int err_fds[2];
    if (!make_pipe(out_fds)) {
      result.spawn_failed = true;
      return result;
    }
    if (!make_pipe(err_fds)) {
      ::close(out_fds[0]);
      ::close(out_fds[1]);
      result.spawn_failed = true;
      return result;
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, out_fds[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, err_fds[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, out_fds[0]);
    posix_spawn_file_actions_addclose(&actions, err_fds[0]);
#if defined(__APPLE__) || defined(__GLIBC__)
    // Never run a child in the user's repo: a hostile .git/config can turn an
    // innocent-looking command into arbitrary execution.
    posix_spawn_file_actions_addchdir_np(&actions, "/tmp");
#endif

    std::vector<std::string> argv_storage;
    argv_storage.reserve(args.size() + 1);
    argv_storage.push_back(exe.string());
    for (const auto& a : args) argv_storage.push_back(a);

    std::vector<char*> argv;
    argv.reserve(argv_storage.size() + 1);
    for (auto& s : argv_storage) argv.push_back(s.data());
    argv.push_back(nullptr);

    auto env_storage = child_environment();
    std::vector<char*> envp;
    envp.reserve(env_storage.size() + 1);
    for (auto& s : env_storage) envp.push_back(s.data());
    envp.push_back(nullptr);

    pid_t pid = 0;
    int rc = ::posix_spawn(&pid, exe.c_str(), &actions, nullptr, argv.data(), envp.data());
    posix_spawn_file_actions_destroy(&actions);
    ::close(out_fds[1]);
    ::close(err_fds[1]);

    if (rc != 0) {
      ::close(out_fds[0]);
      ::close(err_fds[0]);
      result.spawn_failed = true;
      return result;
    }

    pump(out_fds[0], err_fds[0], timeout, result);
    ::close(out_fds[0]);
    ::close(err_fds[0]);
    reap(pid, result);
    return result;
  }

 private:
  static std::vector<std::string> child_environment() {
    std::vector<std::string> env;
    for (char** e = environ; e != nullptr && *e != nullptr; ++e) {
      std::string entry(*e);
      if (entry.rfind("GIT_TERMINAL_PROMPT=", 0) == 0) continue;
      env.push_back(std::move(entry));
    }
    // Nothing may block waiting for input.
    env.push_back("GIT_TERMINAL_PROMPT=0");
    return env;
  }

  static void pump(int out_fd, int err_fd, std::chrono::milliseconds timeout,
                   ProcessResult& result) {
    bool out_open = true;
    bool err_open = true;
    const auto deadline = Clock::now() + timeout;

    while (out_open || err_open) {
      const auto now = Clock::now();
      if (now >= deadline) {
        result.timed_out = true;
        return;
      }
      const auto remaining =
          std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();

      std::array<pollfd, 2> fds{};
      int out_slot = -1;
      int err_slot = -1;
      nfds_t n = 0;
      if (out_open) {
        fds[n] = {out_fd, POLLIN, 0};
        out_slot = static_cast<int>(n++);
      }
      if (err_open) {
        fds[n] = {err_fd, POLLIN, 0};
        err_slot = static_cast<int>(n++);
      }

      const int ready = ::poll(fds.data(), n, static_cast<int>(remaining));
      if (ready == 0) {
        result.timed_out = true;
        return;
      }
      if (ready < 0) {
        if (errno == EINTR) continue;
        return;
      }

      auto drain = [&](int slot, int fd, bool& open, std::string& sink) {
        if (slot < 0 || fds[slot].revents == 0) return;
        char buf[4096];
        const ssize_t got = ::read(fd, buf, sizeof(buf));
        if (got > 0) {
          append_capped(sink, buf, static_cast<size_t>(got));
        } else if (got == 0) {
          open = false;
        } else if (errno != EINTR && errno != EAGAIN) {
          open = false;
        }
      };
      drain(out_slot, out_fd, out_open, result.stdout_text);
      drain(err_slot, err_fd, err_open, result.stderr_text);
    }
  }

  static void reap(pid_t pid, ProcessResult& result) {
    int status = 0;
    if (result.timed_out) {
      ::kill(pid, SIGTERM);
      const auto grace = Clock::now() + std::chrono::milliseconds(200);
      bool reaped = false;
      while (Clock::now() < grace) {
        const pid_t r = ::waitpid(pid, &status, WNOHANG);
        if (r == pid) {
          reaped = true;
          break;
        }
        if (r < 0 && errno != EINTR) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      if (!reaped) {
        ::kill(pid, SIGKILL);
        while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
        }
      }
    } else {
      while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
      }
    }

    if (WIFEXITED(status)) {
      result.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      result.exit_code = -WTERMSIG(status);
    }
  }
};

}  // namespace

std::unique_ptr<ProcessRunner> make_process_runner() {
  return std::make_unique<PosixProcessRunner>();
}

}  // namespace devlite
