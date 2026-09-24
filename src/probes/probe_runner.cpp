#include "devlite/probe.hpp"

namespace devlite {

Facts gather_facts(const ProbeOptions& opts, Environment& env, ProcessRunner& runner) {
  Facts facts;
  facts.working_directory = opts.working_directory;
  probe_platform(facts, env);
  probe_path(facts, env);
  probe_git(facts, runner);
  return facts;
}

}  // namespace devlite
