<!-- Project policy overlay. Non-comment content here is spliced into
     the implementer rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Implementer Role Specifics for JMoria

### Coding Standards
- **Language:** C++17 (set in Makefile, `-std=c++17`)
- **Style:** clang-format (`.clang-format` config; Allman braces, 100-char column limit)
- **Enforcement:** Pre-commit hook blocks commits that don't pass `git clang-format`
- **Action:** Before pushing, run `clang-format -i src/YourFile.cpp` or fix hook failures

### State Machine Pattern (Architecture Pattern)
- **Base class:** `CStateBase` (all game states inherit from this)
- **State activation:** `CGame::SetState(new CYourState())`
- **Input handling:** Override `OnHandleKey(int key)` to process keyboard
- **Update loop:** Override `OnUpdate(float elapsed_ms)` for per-frame logic
- **Exit cleanup:** Destructor or `OnExit()` method to release resources
- **Pattern example:** `CmdState` (command input), `TargetState` (targeting), `UseState` (inventory use)

### Interdependencies & Module Structure
- **CGame** — central coordinator; holds references to `CDungeon`, `CPlayer`, `CAIMgr`, `CRender`, `CDisplayText`
- **CDungeon** — 100×100 tile grid; manages room/hallway generation via `DungeonMap::FillArea()`
- **CPlayer** — player state, inventory, equipment, intrinsics
- **CAIMgr** — monster AI controller (behavior per `MoveType` in resource file)
- **CRender** — abstraction layer; routes to `RenderASCII` or `RenderOpenGL`
- **CDisplayText** — UI text regions (Msgs, Stats, Inv, Equip, Use, EndGame)
- **Dependency rule:** Avoid circular includes; use forward declarations in headers

### Build & Linking
- **Object files:** `.cpp` → `.o` in same directory (Makefile `src/%.o: src/%.cpp`)
- **Platform flags:** Do NOT hardcode paths; use `$(LOCAL_INCLUDE_PATHS)`, `$(LOCAL_LIB_PATHS)`
- **Render mode:** Check `#ifdef RENDER_ASCII` / `#ifdef RENDER_OPENGL` if render-specific logic needed
- **Linking:** Ensure new `.cpp` files compile to `.o` (Makefile will auto-link); test all render modes

### Data-Driven Design (Enforce Separation)
- **Game logic:** C++ code; no stat tables, no monster definitions, no item balancing numbers
- **Game balance:** `Resources/Monsters.txt`, `Resources/Items.txt`; parsed at runtime by `CDataFile` class
- **Format:** Custom; parsed by `FileParse` utility (angle-bracket delimiters, NdM dice notation)
- **Constraint:** If your feature needs tuning (damage, AC, spawn rates), add to resource file, NOT `.cpp`
- **Why:** Designers can iterate balance without recompilation

### Multi-Renderer Support
- **Build targets:** `make ascii`, `make opengl`, `make` (both)
- **Test requirement:** Verify new feature builds in ALL three modes
- **Platform differences:** macOS defaults to `-framework OpenGL`; Linux uses `-lGL`
- **Conditional code:** Use `#ifdef RENDER_ASCII` / `#ifdef RENDER_OPENGL` sparingly; prefer abstraction via `CRender` interface
- **Example:** Don't call SDL or ncurses directly; route through `CRender::DrawTile()` or similar

### Testing & Validation
- **BDD framework:** Cucumber-CPP with GoogleTest wire protocol
- **Test files:** `.feature` (Gherkin) in `test/features/`; step implementations in `test/features/step_definitions/`
- **Test context:** `TestContext.hpp` sets up game state for each scenario
- **Build before test:** `make build` produces `test/bin/AllSteps`; then `./test/runtests.sh`
- **Feature coverage:** Add `.feature` files for significant behavioral changes (e.g., new AI, item effects)
- **Example:** `test/features/monster_ai.feature` might test chase, flee, ranged attack behaviors

### Cross-Platform Code
- **Platform detection:** Use `#ifdef __APPLE__` (macOS), `#ifdef __linux__` (Linux), not `uname -s`
- **Path handling:** Use relative paths or `getenv()` for runtime config; avoid `/usr/local` hardcoding
- **Dependency checks:** If adding a library, verify it's available on all three platforms (Homebrew, apt-get, Raspberry Pi repos)
- **Testing:** Changes to Makefile or build-related `.cpp` must be tested on macOS and Linux

### Code Review Self-Checklist
Before submitting a PR:
- [ ] `clang-format` passes (`git clang-format --check`)
- [ ] Builds on all three render modes: `make ascii`, `make opengl`, `make`
- [ ] No hardcoded paths or platform-specific `#ifdef` blocks without fallback
- [ ] Game data (numbers, definitions) lives in `Resources/`, not .cpp
- [ ] State machine pattern followed (if adding new state)
- [ ] Test coverage added or verified (BDD feature files or unit tests)
- [ ] Cross-platform Makefile edits tested on macOS and Linux (or Docker)
- [ ] No breaking changes to `CGame` interface without updating all state classes
