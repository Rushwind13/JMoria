<!-- Project policy overlay. Non-comment content here is spliced into
     the verifier rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Verifier Role Specifics for JMoria

### Verification Strategy

**Goal:** Ensure that a merged PR works correctly across all three platforms and doesn't break existing functionality.

**Verification phases:**
1. **Build phase:** Compile on all render modes and platforms
2. **Test phase:** Run BDD tests; ensure no regressions
3. **Functionality phase:** Manual testing of new features (if applicable)
4. **Regression phase:** Spot-check existing gameplay features

### Build Verification

**Targets to verify:**
```bash
make clean
make ascii          # ASCII-only build (ncurses)
make opengl         # OpenGL-only build (SDL2)
make                # Both renderers (default)
make build          # Full build including test executable
```

**Success criteria:**
- [ ] All three render modes compile without warnings or errors
- [ ] Executable created: `./jmoria` (or platform-specific binary)
- [ ] Test executable created: `test/bin/AllSteps`
- [ ] No linker errors; all object files linked correctly

**Platform verification:**
- [ ] **macOS (primary):** Run on macOS directly
- [ ] **Linux:** Use Docker or Linux VM
  - Base image: `ubuntu:latest` or `debian:bookworm`
  - Dependencies: `apt-get install build-essential libsdl2-dev libsdl2-image-dev libgl-dev libncurses-dev`
- [ ] **Raspberry Pi OS:** Conceptual (same as Linux Debian); actual hardware testing optional

**Render mode validation:**
- [ ] ASCII mode: `./jmoria --renderer=ascii` (if both-mode build)
- [ ] OpenGL mode: `./jmoria --renderer=opengl` (if both-mode build)
- [ ] Default (both): `./jmoria` starts interactively with renderer selection
- [ ] Single-mode builds: `./jmoria` uses that mode only

### Test Verification (BDD)

**Test execution:**
```bash
make verify         # Full verification: build + BDD
# OR
make build          # Build test executable
./test/runtests.sh  # Run BDD tests
```

**Test success criteria:**
- [ ] All feature scenarios pass (0 failures)
- [ ] No timeouts (tests hang or timeout = failure)
- [ ] Wire protocol handshake succeeds (cucumber connects to AllSteps)
- [ ] Test context sets up game state correctly (no setup failures)

**Test failure investigation:**
- [ ] Check if test executable built: `ls -la test/bin/AllSteps`
- [ ] Run with verbosity: `./test/runtests.sh --verbose` or `--tags @debug`
- [ ] Check for socket/port conflicts (wire protocol default: localhost:3902)
- [ ] Review step definitions: `test/features/step_definitions/`
- [ ] Check TestContext setup: `test/features/step_definitions/TestContext.hpp`

**Regression testing:**
- [ ] Run full test suite (not just new tests)
- [ ] If new tests added: verify they actually test the feature (not false positives)
- [ ] Check for skipped tests: `@skip`, `@pending` tags should not accumulate

### Functional Verification (Manual, if applicable)

**Spot-check for new features:**
- [ ] Feature works in ASCII mode (ncurses rendering)
- [ ] Feature works in OpenGL mode (if applicable)
- [ ] Feature doesn't crash the game
- [ ] Feature integrates with existing gameplay (no blocking bugs)
- [ ] Wizard mode still functions (if modified)
- [ ] Score saving not affected (if not intentionally changed)

**Gameplay sanity checks:**
- [ ] Player can move and fight monsters
- [ ] Inventory and equipment work
- [ ] Stairs up/down navigate dungeons
- [ ] Game responds to all documented keyboard commands
- [ ] Wizard mode (`Ctrl+T`, etc.) accessible and working

### Failure Triage

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| `Undefined reference to xyz` at link | Missing .cpp file or wrong object linked | Verify Makefile pattern rule includes new file |
| `clang: error: linker command failed` (cross-compile) | Platform mismatch in lib paths | Check $(LOCAL_LIB_PATHS) for target platform |
| Executable crashes on startup | Render initialization failed | Check --renderer flag; verify SDL2/OpenGL installed |
| `make verify` hangs | Wire protocol connection stuck | Kill any orphaned `test/bin/AllSteps` processes |
| BDD test timeout | Step definition hangs (infinite loop) | Review step code for blocking calls |
| Wizard mode can save scores (regression) | `IsWizardMode()` guard removed | Code review: restore guard before `SaveScore()` |
| New state crashes (e.g., UseState) | State didn't implement `OnUpdate()` or `OnHandleKey()` | Verify state inherits from `CStateBase` properly |

### Verification Checklist (Pre-Merge)

```markdown
## Verification Checklist

**Build Verification:**
- [ ] `make ascii` passes (ncurses)
- [ ] `make opengl` passes (SDL2)
- [ ] `make` passes (both renderers)
- [ ] No linker errors or unresolved symbols
- [ ] Executable runs: `./jmoria --version` or `./jmoria --help` (if supported)

**Test Verification:**
- [ ] `make build` creates test executable: `test/bin/AllSteps`
- [ ] `./test/runtests.sh` runs without timeout
- [ ] All BDD scenarios pass (green)
- [ ] No new test failures (regression check)
- [ ] Test feature scenarios are meaningful (not false positives)

**Platform Verification:**
- [ ] Builds on macOS (or primary platform)
- [ ] Builds on Linux (Docker acceptable)
- [ ] No hardcoded platform-specific paths or flags outside conditionals

**Functional Spot-Check (if feature touches gameplay):**
- [ ] Feature works in ASCII and OpenGL modes
- [ ] Existing features still work (no blocking regressions)
- [ ] Wizard mode not bypassed (score saving guard intact)
- [ ] Game doesn't crash on normal gameplay

**Code Quality:**
- [ ] Clang-format compliant (100-char limit, Allman braces)
- [ ] No memory leaks (if valgrind available)
- [ ] No new compiler warnings

**Result:** Pass ✅ / Fail ❌ (document failures below)
```

### Continuous Verification (Post-Merge Monitoring)

After merge, monitor for:
- [ ] User reports of crashes or broken features
- [ ] New issues filed on GitHub (regression tracking)
- [ ] Performance degradation (game loop FPS, memory usage)
- [ ] Wizard mode abuse or exploits (score saving bypass)

**If issues found after merge:**
- Log issue with reproduction steps
- Assign to implementer for hotfix or next sprint
- Update test suite to prevent regression
