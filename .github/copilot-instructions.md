# JMoria AI Coding Guide

JMoria is a from-scratch C++ roguelike implementation (homage to IMoria) using SDL2/OpenGL with a state-machine architecture and data-driven monster/item definitions.

## Architecture Overview

**State Machine Pattern**: Game flow managed through `CStateBase` subclasses. Each game mode (command input, targeting, inventory use, rest, look, etc.) is a separate state. States handle keyboard input via `OnHandleKey()` and updates via `OnUpdate()`. Switch states using `CGame::SetState()`.

**Core Components**:
- `CGame` - central coordinator holding references to all managers
- `CDungeon` - dungeon generator and tile grid (100x100)
- `CPlayer` - player state, inventory, equipment, intrinsics
- `CAIMgr` - AI controller for monsters
- `CRender` - SDL2/OpenGL rendering
- `CDisplayText` - UI text regions (Msgs, Stats, Inv, Equip, Use, EndGame)

**Dungeon Architecture**: 100x100 grid starting as solid rock (`DUNG_IDX_WALL`). Rooms/hallways carved out via `DungeonMap::FillArea`. In normal gameplay, visibility is primarily seen/lit driven; CLOCKSTEP and wizard mode include visibility bypass behavior for debugging. Complex generation logic in [src/DungeonMap.cpp](src/DungeonMap.cpp) - see [_JMoria Developer's Guide.md](_JMoria%20Developer's%20Guide.md) for generation architecture notes.

## Data Files & Resource System

**Monster Definitions** ([Resources/Monsters.txt](Resources/Monsters.txt)): Custom format parsed by `CDataFile::ReadMonster()`. Each monster block contains: `Plural`, `Speed`, `MoveType` (AI behavior), `HD` (hit dice), `AC`, `Level`, `ExpValue`, `Type`, `Attack`, `Color`, `Flags`.

**Item Definitions** ([Resources/Items.txt](Resources/Items.txt)): Parsed by `CDataFile::ReadItem()`. Format: `Plural`, `Type`, `Value`, `Flags`, `Color`, `AC`, `ACBonus`, `Damage`, `To-HitBonus`, `To-DamBonus`, `Level`, `Speed`, `Weight`, `Effect`.

**Adding Monsters/Items**:
1. New flavor of existing type: Edit `Monsters.txt` or `Items.txt` only
2. New type: Edit data file + add `MON_IDX_*` or `ITEM_IDX_*` to [src/Constants.h](src/Constants.h) + add emoji to `MonIds`/`ItemIds` list in [src/Monster.cpp](src/Monster.cpp) or [src/Item.cpp](src/Item.cpp)

**Utility scripts**: Use `./scripts/find_monster.sh`, `./scripts/find_item.sh`, `./scripts/list_monster.sh`, `./scripts/list_item.sh` to search resource files.

## Build & Test Workflow

**Build**: `make` (creates `jmoria` executable with both OpenGL and ASCII renderer support)
- `make ascii` — builds ASCII-only executable (ncurses, no SDL/OpenGL required at runtime). Use this for headless/bot use.
- `make opengl` — builds OpenGL-only executable
- `make` — builds with both renderers (default)

**Dependencies**:
- macOS: `brew install sdl2 sdl2_image` (OpenGL framework included)
- Linux/Debian: `sudo apt-get install libsdl2-dev libsdl2-image-dev libgl-dev build-essential`
- Raspberry Pi OS (Debian): Same as Linux/Debian above

**Testing**: Uses Cucumber-CPP (BDD) with GoogleTest
- Dependencies (Linux/Debian): `sudo apt-get install googletest libboost-all-dev cmake libasio-dev libtclap-dev nlohmann-json3-dev ruby rubygems`
- Build googletest: `cd /usr/src/googletest && sudo cmake . && sudo cmake --build . --target install`
- Build cucumber-cpp: Clone from github.com/cucumber/cucumber-cpp, build with `cmake .. -DCUKE_ENABLE_GTEST=ON && cmake --build . && sudo cmake --build . --target install`
- Install cucumber gem: `sudo gem install cucumber -v 7.1.0` (version 7.x required for wire protocol support; versions 8.0+ removed it)
- Run: `./test/runtests.sh` or `make test` (builds test executable)
- Tests require `SDL_VIDEODRIVER=dummy` for headless operation
- Test structure: Feature files in [test/features/](test/features/), step definitions in [test/features/step_definitions/](test/features/step_definitions/)
- Test executable: `test/bin/AllSteps` runs as background process during cucumber execution
- Setup context defined in [test/features/step_definitions/TestContext.hpp](test/features/step_definitions/TestContext.hpp)

**Code Style**: Must use `clang-format` before committing. Pre-commit hook enforces `git clang-format` on staged changes. Run `clang-format -i <files>` to format.

**Cross-Platform Compatibility**: ALL build scripts and code must remain compatible with macOS, Linux, and Raspberry Pi OS. When editing build scripts:
- Detect platform using `uname` and set platform-specific flags
- macOS: `-lc++` for stdlib, `-framework OpenGL` for OpenGL
- Linux: `-lstdc++` for stdlib, `-lGL` for OpenGL
- Use C++17 standard (current Makefile setting)
- Test changes don't break other platforms before committing

## Development Patterns

**Wizard Mode Commands** (debug/testing): `^t` teleport, `^f` set flags, `^i` create item, `^s` summon monster. These enable Wizard Mode automatically.

**DisplayText Regions** (UI layout):
- Msgs: Top 2 rows (gameplay output)
- Stats: Left sidebar (AC, HP, Level, etc.)
- Inv: Upper right sidebar (inventory)
- Equip: Lower right sidebar (equipment)
- Use: Central popup (shown when needed for inv/equip commands)

**Tile Bindings**: Monster types mapped to ASCII characters (lowercase = minor, uppercase = major). See monster type chart in [_JMoria Developer's Guide.md](_JMoria%20Developer's%20Guide.md) (e.g., `o` = orc, `D` = ancient dragon, `J` = oozes/jellies).

**FileParse Utility**: Handles custom resource file format with angle-bracket-delimited strings `<value>`, NdM dice notation, and block structure. Used for all game data loading.

## Known Issues & Context

- Dungeon generation has historical bugs (see `WORKLIST.txt` comments from 2003-2005)
- `DungeonMap::FillArea` is complex - stepwise calculation implemented in 2017 for debugging
- Equipment system currently hardcoded but moving toward `ITEM_FLAG_EQUIPMENT` data-driven approach
- Score file auto-created at `Resources/Scores.txt` if missing
- Graphics tileset: `Resources/Courier.png` (emoji-based)

## Reference Files

- [_JMoria Developer's Guide.md](_JMoria%20Developer's%20Guide.md) - Monster/item addition, tile bindings
- [Developer-Setup-Guide.md](Developer-Setup-Guide.md) - Platform-specific setup, clang-format hook
- [WORKLIST.txt](WORKLIST.txt) - Feature backlog and TODOs
- [Player Docs.txt](Player%20Docs.txt) - Keyboard commands and gameplay
