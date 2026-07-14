<!-- Project policy overlay. Non-comment content here is spliced into
     the analyst rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Analyst: JMoria Codebase Overview

### Project Scope
JMoria is a C++ roguelike game engine (homage to IMoria) with:
- **Core systems**: Player, AI, dungeon generation, items, effects, display
- **Architecture**: State machine (game modes as state classes)
- **Data sources**: `Resources/Monsters.txt`, `Resources/Items.txt` (custom text format, parsed by `FileParse`)
- **Renders**: ASCII (ncurses) and OpenGL (SDL2) side-by-side in codebase

### Before Starting Analysis
1. **Read architecture docs** (15 min): `doc/_JMoria Developer's Guide.md` for state system, dungeon layout, tile bindings
2. **Skim key files** (10 min): `src/Constants.h` (game constants, type indices), `src/Game.cpp` (main coordinator)
3. **Understand data format**: `src/FileParse.cpp` parses resources; custom format with angle-bracket delimiters `<value>`
4. **Know the build**: Makefile supports `make ascii`, `make opengl`, `make` (both); platform detection via `uname`

### Key Conventions for Analyst Tasks
- **File organization**: Roughly one class per file (`CPlayer.cpp`, `CDungeon.cpp`, etc.)
- **Constants live in**: `src/Constants.h` (indices for monsters, items, effects, etc.) and `.txt` data files
- **Monsters/items**: If a new type is needed, edit `.txt` file, add constant to `Constants.h`, add emoji to type lists
- **State transitions**: Game modes managed by `CGame::SetState()`; states are in `src/*State.cpp`
- **Testing**: Cucumber-CPP framework in `test/`; feature files + C++ step definitions; see `test/runtests.sh`

### Analysis Guardrails
- **Clang-format**: Before delivering findings, run `clang-format -i` on any code samples you might generate
- **Platform assumptions**: Note if a finding is specific to macOS vs Linux; JMoria runs on both
- **Data format**: If analyzing monsters or items, remember they are parsed from text files, not hardcoded

### Integration Profile Available
- Location: `.agentic/runs/000-integration/integration-profile.md`
- Contains: environment, gates, conventions, guardrails — use for context on host capabilities
