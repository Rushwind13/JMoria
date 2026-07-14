<!-- Project policy overlay. Non-comment content here is spliced into
     the reviewer rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Reviewer: JMoria Code Review Checklist

### Review Criteria for JMoria PRs

**Architectural Fit**:
- ✓ Does the change respect the state machine model? (New modes → new state class; existing states → extend OnHandleKey/OnUpdate)
- ✓ Is content data-driven where possible? (Monsters/items → resources; not hardcoded in logic)
- ✓ Render abstraction maintained? (No platform-specific code in game logic)

**Code Quality**:
- ✓ Passes `clang-format` (run `git clang-format --diff` on PR branch)
- ✓ Follows coding standards in `doc/coding-standards.md`
- ✓ No magic numbers; constants go in `src/Constants.h` or resources
- ✓ Comments only where code needs clarification; avoid over-commenting

**Platform & Build**:
- ✓ Makefile unchanged or changes are cross-platform (`uname` branching tested)?
- ✓ No new hardcoded paths or platform-specific #ifdef spam?
- ✓ If dependencies added: available on Darwin, Linux, and (ideally) Raspberry Pi?
- ✓ Build tested: At least `make ascii` on macOS; ideally on Linux too

**Testing**:
- ✓ Existing tests still pass? (`./test/runtests.sh --build` if available)
- ✓ If new user-facing feature, is there a Cucumber feature file or manual test plan documented?
- ✓ Game playable after changes? (manual smoke test recommended)

**Documentation**:
- ✓ Architecture changes documented in `doc/_JMoria Developer's Guide.md` or PR notes?
- ✓ New constants or data structures explained?
- ✓ Resource format changes (Monsters.txt/Items.txt) noted?

**Red Flags**:
- ❌ clang-format violations or inconsistent style
- ❌ Breaking existing tests without clear reason
- ❌ New platform-specific #ifdef; must use Makefile detection instead
- ❌ Hardcoded file paths or platform assumptions
- ❌ Changes to `.agentic/` core files (framework-lock.json, contracts/, roles/, scripts/) — these are read-only; use overlays
- ❌ Render pipeline logic (game logic that shouldn't know about ASCII vs OpenGL)

### Review Tools
- `.clang-format` — Check formatting
- `Makefile` — Verify build logic
- `doc/coding-standards.md` — Style reference
- `integration-profile.md` — Gate capabilities, known gaps

### Specific to Integration Phase
- ✓ Rendered agent files (`.github/agents/*.agent.md`) were regenerated after overlay changes? (See `render-agents.py`)
- ✓ Integration profile reviewed and signed off by GI? (Human gate)
- ✓ Framework files (`.agentic/contracts/`, etc.) untouched?
