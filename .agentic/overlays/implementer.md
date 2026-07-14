<!-- Project policy overlay. Non-comment content here is spliced into
     the implementer rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Implementer: JMoria Development Workflow

### Your Scope
You implement features and fixes in C++ for JMoria. All changes must:
1. **Compile and link**: `make ascii` or `make` on macOS (verify with clean build)
2. **Pass clang-format**: Run `clang-format -i <files>` on staged changes before commit
3. **Not break platforms**: If you modify build scripts, test cross-platform assumptions (or document test status)
4. **Respect architecture**: Use state machine for modes; data-driven for content (see Architect overlay for patterns)

### Build & Test Workflow

**Build the game**:
```bash
make clean
make ascii        # Terminal-only (fastest for development)
# or: make        # Both renderers, runtime selection
# or: make opengl # Graphics-only
```

**Build and run tests** (if test dependencies are installed):
```bash
cd test
./runtests.sh --build
```

**Code formatting** (required before commit):
```bash
clang-format -i src/YourFile.cpp src/YourFile.h
```

**Verify no build/test regressions**:
- Baseline: Run `make ascii` on clean repo → verify executable works
- After changes: Run same build → must succeed
- If tests exist: `./test/runtests.sh --build` must pass or report known breakage

### Common Development Tasks

**Adding a new monster**:
1. Edit `Resources/Monsters.txt` (add stats, type, behavior)
2. Add `MON_IDX_*` constant to `src/Constants.h`
3. Add emoji to `MonIds` list in `src/Monster.cpp`
4. Rebuild: `make ascii` → test in game

**Adding a new item effect**:
1. Edit `Resources/Items.txt` or effects data
2. If code needed: implement handler in `src/Effect.cpp`
3. Add `EFFECT_IDX_*` constant to `src/Constants.h`
4. Rebuild and test

**Adding a new game mode**:
1. Create `src/CNewModeState.cpp` + `.h` (extends `CStateBase`)
2. Implement `OnHandleKey()`, `OnUpdate()`, enter/exit handlers
3. Add enum to `StateEnum` in `src/Constants.h`
4. Link from existing state via `CGame::SetState()`
5. Test via manual gameplay or Cucumber feature

### Files You'll Work With Frequently
- `src/*.cpp` / `src/*.h` — Game logic, states, AI, rendering
- `src/Constants.h` — Indices, enums, magic numbers
- `Resources/Monsters.txt`, `Resources/Items.txt` — Data definitions
- `Makefile` — Build configuration (rarely; only if adding dependencies or platforms)
- `test/features/*.feature` — Cucumber tests (if adding testable feature)

### Guardrails for Implementer
1. **clang-format is mandatory**: Staged changes MUST pass `git clang-format --diff --staged`; documentation note or CI will enforce
2. **Makefile is finicky**: If you add a file, update `SOURCES` list; if you add a dependency, test both Darwin and Linux branching
3. **Test path trap**: Makefile has hardcoded googletest path `/opt/homebrew/Cellar/googletest/1.17.0/`; test link will fail if path differs on your machine; see `doc/Developer-Setup-Guide.md` for workaround
4. **Platform compatibility check**: If you modify *any* build script, shell command, or preprocessor flag, document which platforms you tested on
5. **No breaking changes**: If refactoring core classes (Game, Player, Dungeon), ensure existing feature tests still pass

### Debugging
- **ASCII build recommended for development**: Faster than OpenGL, no framework dependencies
- **Game logs**: `clockstep_log*.txt` auto-created in repo root (may be useful for debugging)
- **Manual testing**: Use `tmux` to play the game after changes; test the specific feature you implemented
- **Wizard mode**: `Ctrl-T` (teleport), `Ctrl-S` (summon), etc. — useful for testing without long runs

### Before Committing
1. ✓ Code compiles: `make clean && make ascii`
2. ✓ Code is formatted: `clang-format -i src/Changed*.cpp src/Changed*.h`
3. ✓ Existing tests pass: `./test/runtests.sh --build` (if test dependencies installed)
4. ✓ Manual test of feature (play the game or verify expected behavior)
5. ✓ Commit message is clear (describe *what* and *why*, not just *how*)

### Integration Profile Available
- Location: `.agentic/runs/000-integration/integration-profile.md`
- Contains: build platforms, test infrastructure status, conventions, known traps
- Reference if build fails or you're unsure about cross-platform impact
