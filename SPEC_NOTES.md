# Deviations from the spec

Small things discovered while building the scaffold. Fold these back into the
spec document when convenient.

1. **`Facts` gained a `GitInfo` member.** The spec's `git.identity-unset` rule
   needs `user.name` and `user.email`, but the spec's `Facts` had nowhere to put
   them. Added `struct GitInfo { optional<string> user_name, user_email; }`.

2. **`find_all_on_path` dedupes by resolved target.** `/bin` is a symlink to
   `/usr/bin` on macOS, and Homebrew fills `/opt/homebrew/bin` with links into
   Cellar. Without deduplication, `path.shadowed` fires on every tool on a
   perfectly healthy machine, which is exactly the false positive that destroys
   trust in a doctor tool. Locations now hold one entry per distinct file.

3. **`rule_git_identity_unset` returns `Status::Skipped` when git is absent**
   rather than returning nothing, so `--verbose` can show that the check was
   considered and why it could not run.

4. **The clean fixture uses an isolated PATH.** Including `/usr/bin` let the
   host machine's real tools leak in and shadow the fixture's own shims. A
   fixture that depends on the developer's machine state is not a fixture.
