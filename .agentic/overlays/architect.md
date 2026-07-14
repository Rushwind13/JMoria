<!-- Project policy overlay. Non-comment content here is spliced into
     the architect rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Architect: JMoria Design Principles

### Project Philosophy
JMoria is a **from-scratch roguelike** with deep technical ownership and deliberate patterns:
- **State machine first**: All game modes (command input, targeting, rest, etc.) are separate `CStateBase` subclasses
- **Data-driven content**: Monsters, items, effects defined in `.txt` resource files; code is renderer/engine
- **Cross-platform**: Single codebase, three build modes (ASCII terminal, OpenGL graphics, or both)
- **Test-driven culture**: Cucumber-CPP feature files guide development; state machine makes testing tractable

### Architectural Guidelines for New Features

**Rule 1: State Machine for Complex Modes**
- New gameplay mode? → Create a new `CStateBase` subclass (e.g., `CNewModeState`)
- Implement `OnHandleKey()`, `OnUpdate()`, `OnEnter()`, `OnExit()` contract
- Transition via `CGame::SetState(new_state)`
- This isolates complexity and makes testing clean

**Rule 2: Data-Driven, Not Code-Driven**
- New monster type? → Add line to `Resources/Monsters.txt`, constant to `Constants.h`, emoji to `MonIds` (in `Monster.cpp`)
- New item effect? → Add to `Resources/Items.txt` or effects table
- Code parses via `CDataFile::ReadMonster()`, `CDataFile::ReadItem()`; no monster-specific logic in code
- This lets designers iterate without recompilation

**Rule 3: Render Abstraction**
- Game logic (AI, combat, turns) is completely separate from rendering
- `CRender` is an interface; implementations: `CRenderASCII`, `CRenderOpenGL`
- New display mode? Implement a new Render subclass; game logic unchanged
- Build flag `RENDER_ASCII` vs `RENDER_OPENGL` controls which one compiles

**Rule 4: Cross-Platform from Day One**
- Makefile uses `uname -s` to detect Darwin vs Linux
- Any new dependencies must be available on both platforms (or conditional build flags)
- Test changes on macOS (required); Linux assumed to follow same pattern
- Raspberry Pi OS is Debian-based; treat like Linux

### Design Review Checklist
- ✓ Does the feature fit the state machine model or require new architectural pattern?
- ✓ Can logic be data-driven (resources) or must it be in code?
- ✓ Does the change affect rendering? If so, is render abstraction maintained?
- ✓ Will it build on Darwin/Linux/Pi with current toolchain?
- ✓ Is there a feature test (Cucumber) or will it need one?

### Known Architectural Debt (from WORKLIST)
- Dungeon generation has historical bugs (comments from 2003–2005 era code); see `DungeonMap.cpp`
- Equipment system currently hardcoded; moving toward `ITEM_FLAG_EQUIPMENT` data-driven approach
- Hardcoded test paths in Makefile (googletest path); future: CMake or dynamic detection
- **Do not redesign these during this integration**; document findings and flag for future phase

### Vendor/Dependency Stance
- No package manager (C++17, pure Makefile)
- Core dependencies: SDL2, ncurses, OpenGL (widely available)
- Test dependencies: GoogleTest, Cucumber-CPP (macOS: Homebrew; Linux: apt)
- No vendored code; assume system libraries

### Integration Profile Reference
- `.agentic/runs/000-integration/integration-profile.md`
- Contains: gate capabilities, platform traps, test infrastructure readiness
- Use to understand what automation is available vs manual effort
