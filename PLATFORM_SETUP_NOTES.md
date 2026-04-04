# JMoria Platform Setup Notes - Linux/Raspberry Pi OS (December 2025)

## Platform Information
- **Platform**: Raspberry Pi OS (Debian Trixie)
- **Architecture**: arm64 (aarch64)
- **Date**: December 26, 2025

## Dependencies Installed

### Core Build Dependencies
```bash
sudo apt-get install -y build-essential libsdl2-dev libsdl2-image-dev libgl-dev
```

### Test Dependencies
```bash
# Required packages
sudo apt-get install -y googletest libboost-all-dev cmake libasio-dev libtclap-dev nlohmann-json3-dev ruby rubygems

# Build and install GoogleTest
cd /usr/src/googletest
sudo cmake .
sudo cmake --build . --target install

# Clone, build, and install cucumber-cpp with GTest support
cd /tmp
git clone https://github.com/cucumber/cucumber-cpp.git
cd cucumber-cpp
git submodule update --init
cmake -E make_directory build
cd build
cmake .. -DCUKE_ENABLE_GTEST=ON
cmake --build .
sudo cmake --build . --target install

# CRITICAL: Install Cucumber gem version 7.1.0 (wire protocol removed in 8.0+)
sudo gem install cucumber -v 7.1.0
```

## Code Changes Required

### Platform-Neutral OpenGL Headers
Modified [src/Render.cpp](src/Render.cpp) to use conditional compilation for OpenGL headers:

```cpp
#ifdef __APPLE__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif
```

This change makes the code work on both macOS (which uses `OpenGL/gl.h`) and Linux/BSD (which use `GL/gl.h`).

### Headless Mode for Testing
Modified [src/Render.cpp](src/Render.cpp) to support headless mode when running with dummy SDL video driver:

```cpp
// In CRender::Init()
const char* videoDriver = getenv("SDL_VIDEODRIVER");
bool isHeadless = (videoDriver != NULL && strcmp(videoDriver, "dummy") == 0);

if (isHeadless)
{
    JLog( LOG_LEVEL_INFO, true, "Running in headless mode (SDL_VIDEODRIVER=dummy), skipping graphics initialization" );
    m_hWindow = NULL;
    return JSUCCESS;
}
```

All OpenGL and SDL_GL functions guarded with `if (m_hWindow == NULL) return;` checks to prevent crashes in headless mode.

This enables tests to run without a display server using `SDL_VIDEODRIVER=dummy`.

## Build Results

### Main Executable
```bash
make
# Creates: jmoria (400 KB)
```

### Test Executable
```bash
make test
# Creates: test/bin/AllSteps (5.9 MB)
```

Both executables built successfully without errors.

## Test Results

**Status (historical snapshot, 2025-12-26)**: 91 out of 92 scenarios passing (99% pass rate)

At the time this note was written, the single failing test was a game logic issue:
- `features/equipment.feature:85` - "Cursed Equipment can be uncursed with scroll of remove curse"
- Issue: Scroll of Remove Curse was not properly uncursing equipped items
- This was tracked as gameplay logic, not platform/test infrastructure

For current scenario counts and pass/fail status, use `./test/runtests.sh` (or CI) rather than this historical setup note.

## Open Work Items (from WORKLIST.txt)

### High Priority Architecture Items
- **Data File System**: Implement DAT file reading (XML/INI/passwd format) for dungeon, races, classes, spells, savefile
- **Unit Testing**: Continue expanding test coverage (cucumber-cpp infrastructure now in place)
- **Architecture Diagrams**: Need comprehensive architecture diagram and expanded Developer's Guide
- **Code Ownership**: Clarify ownership of MonsterDefs and DungeonTileDefs (currently in Dungeon)
- **Manager Pattern**: Load textures only once using manager pattern (see flipcode article reference)

### Gameplay Features Needed
- **Status Effects**: Implement poison, paralysis, etc.
- **Flavored Attacks**: Implement breath weapons reading m_dwFlags for type (fire, ice, etc.)
- **Peaceful Monsters**: Cannot be targeted, invulnerable
- **Town Level**: Add shops and player house for storage
- **Character Classes**: Currently only one class available
- **Magic System**: Spells not yet implemented

### Content Gaps
- Many monster types listed but not implemented (Trolls, Giants, Ettin, Mind Flayers, etc.)
- Magic item system not fully implemented (ego items, unique items)
- Door types incomplete (locked, secret, broken doors need different icons)
- Trap system incomplete (secret/visible traps)

### Technical Debt
- **Combat System**: Very hardcoded, needs refactoring
- **Equipment System**: Currently hardcoded, moving toward ITEM_FLAG_EQUIPMENT data-driven approach
- **Renderer Modernization**: Current renderer ready to be refactored into CRenderBase with C3DRender and C2DRender subclasses

### Known Bugs
- Dungeon generation has historical bugs (see WORKLIST.txt comments from 2003-2005)
- DungeonMap::FillArea is complex and may have issues
- DisplayText crashes if string needs line break but has word too long to fit
- Text output mipmapping looks poor, needs different sized font PNGs

## Notes for Future Developers

1. **Platform Neutrality**: The project now builds cleanly on macOS (Intel/ARM), Linux (x86_64/ARM), and Raspberry Pi OS. The OpenGL header fix in Render.cpp is the only platform-specific code required.

2. **Test Infrastructure**: Full BDD test infrastructure is in place with Cucumber-CPP and GoogleTest. Run tests with `./test/runtests.sh` or `make test`.

3. **Data-Driven Design**: The game uses custom data files ([Resources/Monsters.txt](Resources/Monsters.txt), [Resources/Items.txt](Resources/Items.txt)) for content. Use the utility scripts in `scripts/` to query these files.

4. **State Machine Architecture**: Game flow managed through CStateBase subclasses. See copilot instructions for details.

5. **Emoji-Based Graphics**: The game uses `Resources/Courier.png` as a tileset with emoji-based graphics.

## Dependency Summary for New Developers

### macOS
```bash
brew install sdl2 sdl2_image
# OpenGL framework included with system
```

### Debian/Ubuntu/Raspberry Pi OS
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libgl-dev build-essential
```

### Test Dependencies (All Platforms)
- GoogleTest (manual build from /usr/src/googletest on Linux, `brew install googletest` on macOS)
- Cucumber-CPP (manual build from GitHub with `-DCUKE_ENABLE_GTEST=ON`)
- Ruby cucumber gem (`sudo gem install cucumber`)
- Boost libraries, Asio, TCLAP, nlohmann-json (package managers)

## Build Verification

✅ Main executable builds (400 KB)  
✅ Test executable builds (5.9 MB)  
✅ No compiler errors or warnings  
✅ Platform-neutral code established  
✅ Agent instructions updated  

---

*Setup verified on Raspberry Pi OS (Debian Trixie) arm64 - December 26, 2025*
