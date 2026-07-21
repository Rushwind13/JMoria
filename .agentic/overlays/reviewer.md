<!-- Project policy overlay. Non-comment content here is spliced into
     the reviewer rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Reviewer Role Specifics for JMoria

### Code Review Focus Areas (In Priority Order)

**1. Platform Compatibility (G1 — Hard Guardrail)**
- [ ] All Makefile edits detect platform via `uname -s` (macOS Darwin, Linux)
- [ ] No absolute paths like `/usr/local/lib` without conditional fallback
- [ ] Platform-specific flags in `ifeq ($(OS),...)` blocks, not hardcoded
- [ ] Homebrew paths (`/opt/homebrew/`) only in macOS conditional
- [ ] If new dependency added: verify it exists on all three platforms (Homebrew, apt-get, Raspberry Pi)
- **Action:** Failing this → request platform verification on Linux or Docker; don't approve without it

**2. Code Formatting (G2 — Enforced)**
- [ ] All C++ files conform to `.clang-format` config (100-char line limit, Allman braces)
- [ ] No manual formatting workarounds or style exceptions
- [ ] Pre-commit hook would have caught this; check if contributor has hook installed
- **Action:** Failing → request contributor run `clang-format -i <files>` and re-push

**3. Build System & Render Modes (G3)**
- [ ] Changes to `Makefile` don't break `make ascii`, `make opengl`, or `make` (both)
- [ ] No hardcoding of `RENDER_MODE`; use `#ifdef RENDER_ASCII` / `#ifdef RENDER_OPENGL` conditionally
- [ ] New `.cpp` files automatically linked (Makefile pattern rule handles it)
- [ ] Test binary targets unchanged: `make build` still produces `test/bin/AllSteps`
- **Action:** Failing → request contributor test all three render modes; document failures

**4. Data-Driven Design (G4 — Project Principle)**
- [ ] Game balancing numbers NOT in .cpp code (damage, AC, stat modifiers, spawn rates)
- [ ] Monster stats in `Resources/Monsters.txt`; item stats in `Resources/Items.txt`
- [ ] Only enum indices (`MON_IDX_*`, `ITEM_IDX_*`) and gameplay logic in .cpp
- [ ] If hardcoded values found: request move to resource file
- **Action:** Failing → comment with `Resources/` location where value belongs; block until moved

**5. State Machine Pattern (If adding new game state)**
- [ ] New state class inherits from `CStateBase`
- [ ] Implements `OnHandleKey(int key)` and `OnUpdate(float elapsed_ms)`
- [ ] State transitions via `CGame::SetState()`, not direct instantiation
- [ ] Destructor or `OnExit()` cleans up resources (prevent leaks)
- [ ] State doesn't directly modify peer state internals
- **Action:** Failing → request refactor to match pattern; provide example from existing state

**6. Wizard Mode Scope (G6)**
- [ ] If modifying wizard commands (`^t`, `^f`, `^i`, `^s`): verify they don't save scores
- [ ] Check for `IsWizardMode()` guard before any `SaveScore()` call
- [ ] Game balance must not be bypassable via wizard features
- **Action:** Failing → request `IsWizardMode()` guard; explain the constraint

**7. Test Coverage**
- [ ] Significant behavior changes include BDD feature files
- [ ] Test scenarios describe player actions and expected outcomes
- [ ] Step definitions use `TestContext` to set up game state
- [ ] Existing tests still pass (no test regressions)
- [ ] If `make verify` fails: request test fixes before approval
- **Action:** Failing → ask for test scenarios; if minimal change, request brief explanation why tests not needed

**8. Dependency & Linking**
- [ ] New external libraries have platform-specific availability verified
- [ ] Makefile link flags updated (if adding library)
- [ ] No missing object files in link step (all .cpp files compile)
- [ ] Include paths use `$(LOCAL_INCLUDE_PATHS)`, not absolute paths
- **Action:** Failing → request Makefile fixes and cross-platform testing

### Code Review Checklist (Copy into Each PR)
```markdown
### Code Review Checklist for JMoria Contributions

**Platform Compatibility:**
- [ ] Makefile changes tested on macOS and Linux (or documented platform-specific reasoning)
- [ ] No hardcoded paths; platform detection via `uname -s`
- [ ] All three render modes build: `make ascii`, `make opengl`, `make`

**Code Quality:**
- [ ] Passes `clang-format` (100-char limit, Allman braces)
- [ ] Builds without warnings (treat warnings as errors in code review)
- [ ] No memory leaks (valgrind OK if available)

**Architecture:**
- [ ] New game states inherit from `CStateBase`; use `OnHandleKey()` and `OnUpdate()`
- [ ] No circular dependencies; states access managers via `CGame` reference
- [ ] Game balance numbers in `Resources/*.txt`, not .cpp code

**Wizard Mode & Score Saving:**
- [ ] Wizard commands don't bypass game balance or score saving
- [ ] `IsWizardMode()` guards before `SaveScore()` calls (if applicable)

**Testing:**
- [ ] Significant changes include BDD feature files
- [ ] `make verify` passes (build + BDD tests)
- [ ] No test regressions

**Cross-Platform:**
- [ ] Builds on macOS and Linux (Docker acceptable)
- [ ] New dependencies available on all platforms
- [ ] Platform-specific code in separate files (e.g., `Render_MacOS.cpp`), not scattered
```

### Red Flags (Automatic Request for Changes)
- [ ] Hardcoded paths or `#ifdef` platform checks in game logic code
- [ ] Game balance numbers in .cpp files (should be in Resources/)
- [ ] State class doesn't follow `CStateBase` pattern
- [ ] Wizard mode bypasses score saving or enables cheating
- [ ] Makefile changes untested on Linux or missing platform detection
- [ ] New external dependency without cross-platform verification
- [ ] `make verify` fails; tests not fixed

### Approval Criteria
- ✅ All red flags resolved
- ✅ At least one of: tested on Linux OR macOS, or Docker verification provided
- ✅ Clang-format compliant
- ✅ No test regressions
- ✅ Architecture consistent with JMoria patterns
- ✅ If data-driven design applies: numbers in Resources/, not code
