<!-- Project policy overlay. Non-comment content here is spliced into
     the verifier rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Verifier: JMoria Integration & Acceptance Testing

### Verification Scope
You verify that changes work as intended and don't break existing behavior. For JMoria:

**Unit & Feature Level**:
- Build succeeds: `make ascii` compiles all source files, links correctly
- Cucumber tests pass: `./test/runtests.sh --build` runs feature suite (if dependencies installed)
- Formatters pass: `git clang-format --diff --staged` produces no output (clean format)

**Integration Level**:
- Game is playable after changes (manual smoke test)
- No regressions in existing game modes (command input, movement, combat, inventory, etc.)
- No console errors or memory issues during normal play

**Cross-Platform**:
- Build flags and Makefile changes verified on Darwin (macOS) and at least one Linux variant
- Dependencies still available on all supported platforms

### Test Execution Checklist

**Before merge**:
1. ✓ Clone fresh, checkout feature branch
2. ✓ Run `make clean && make ascii` — must succeed
3. ✓ Check formatting: `git clang-format --diff` on staged changes — must be clean
4. ✓ If test deps available: `cd test && ./runtests.sh --build` — must pass
5. ✓ Manual play test: `./jmoria --renderer=ascii`, test the feature, exit cleanly
6. ✓ No new compiler warnings or errors (check `-w` flag in Makefile if needed)

**Platform verification** (for Makefile or build script changes):
- Minimum: Verify `uname` branching logic is correct and both Darwin/Linux paths defined
- Ideal: Test build on Linux/Raspberry Pi (or document test assumptions)
- No new platform-specific #ifdef in game code; must be in Makefile or build scripts

**Framework integrity** (integration-specific):
- ✓ Overlay changes regenerated agents? (Run `.agentic/scripts/render-agents.py`)
- ✓ Rendered files (`.github/agents/*.agent.md`) committed alongside overlays?
- ✓ Agentic render-check CI workflow passes (`agentic-render-check.yml` green)?

### Known Constraints for Testing

**Hardcoded googletest path**: Makefile assumes googletest at `/opt/homebrew/Cellar/googletest/1.17.0/lib/` (macOS Homebrew specific). If tests fail to link on your system:
- Check installed googletest version: `brew list googletest`
- Update Makefile `LOCAL_LIB_PATHS` if path differs
- Document workaround for team

**Test dependencies not in CI yet**: `test/runtests.sh` requires manual setup (Homebrew, gem install). If CI doesn't have these:
- Mark tests as "requires manual verification" in acceptance notes
- Flag for future GitHub Actions setup

### Red Flags That Block Merge

- ❌ `make ascii` fails to build or link
- ❌ clang-format violations present
- ❌ Existing Cucumber tests fail
- ❌ Game crashes or hangs during basic play (movement, combat, menus)
- ❌ Framework files edited directly (use overlays only)
- ❌ Platform-specific code not portable (e.g., hardcoded /usr/local/lib on Linux)

### Acceptance Criteria Template

For each feature/fix:
```
✓ Builds: make ascii succeeds
✓ Format: clang-format clean
✓ Tests: [n/a if no test deps] or ./test/runtests.sh --build passes
✓ Play: Tested in ASCII renderer, no crashes
✓ Platform: [macOS tested | macOS + Linux tested | assumes Darwin only]
✓ Framework: No core .agentic/ files edited; overlays regenerated if changed
✓ Approved by: [Reviewer role]
```

### Integration Profile Reference
- `.agentic/runs/000-integration/integration-profile.md`
- Contains: build traps, test infrastructure status, cross-platform expectations
- Use to understand what's automated vs manual in this host
