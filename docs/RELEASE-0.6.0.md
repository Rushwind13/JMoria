# JMoria Release 0.6.0

**Release Date**: April 11, 2026

## Overview

Release 0.6.0 represents a major milestone for JMoria, introducing groundbreaking features including a full ASCII renderer with advanced color support, an intelligent AI player bot with sophisticated pathfinding, and comprehensive improvements to both infrastructure and gameplay.

## Major Features

### ASCII Renderer (#155, #217)
- Full 256-color terminal support with intelligent 16-color fallback for legacy systems
- Enables headless and bot-friendly gameplay without requiring SDL2/OpenGL
- Prevents ncurses corruption by redirecting stderr to separate log file (#208)

### AI Player Bot System (#159)
- Goal-stack based exploration and pathfinding algorithm
- Intelligent decision-making for character progression and item management
- Foundation for automated gameplay testing and strategy evaluation

### Bot Visualization (#207, #220)
- Real-time agent thinking visualization during gameplay
- Enables debugging and analysis of bot decision-making process
- Enhances understanding of exploration strategy and pathfinding

### Improved Dungeon Generation (#181, #196, #216, #143, #174)
- Significantly reduced dead-end hallways in procedural generation
- More logical room connectivity and exploration flow
- AI-generated enhancements to level design algorithms

## Quality-of-Life Improvements

### Gameplay Features
- **Look Command** (#129): Examine tiles and entities in detail
- **Targeting System** (#132, #133): Enhanced projectile and spell targeting with visual feedback
- **Infravision** (#123): New character ability for enhanced vision
- **Wizard Mode** (#122): Enhanced debug commands for testing and development
- **Item Stacking** (#124): Improved inventory management with stackable items
- **Item Identification** (#127): Track identified vs. unidentified items with separate names

### Item System Enhancements
- **Scrolls** (#114, #126): New consumable item type with various effects
- **Wands** (#136): New magical item type for spellcasting
- **Cursed Items** (#113): Properly implemented cursed scroll of remove curse
- Enhanced equipment system with improved AC and bonus tracking

### User Interface
- **Intro Splash Screen** (#108, #109): Enhanced visual introduction with ASCII art and shading
- **FPS Meter** (#138): Performance monitoring during gameplay
- Improved terminal rendering and display management

## Infrastructure & Testing

### Cross-Platform Support (#153, #163, #168)
- Linux install script for dependency management
- Modern googletest compilation and support
- macOS and Linux build/test executable generation
- OpenGL include path fixes for cross-platform compatibility

### Testing Standards (#162, #161)
- Comprehensive testing documentation and standards
- Cucumber test infrastructure with Linux compatibility
- Updated test framework for modern gtest versions

### Documentation
- **Branching Strategy** (#152): Team collaboration and development workflow guidelines
- **Coding Standards** (#160, #161): Project-wide style and architecture guidelines
- **Project Culture & Philosophy** (#164): Design principles and development ethos
- **Developer Setup Guide**: Platform-specific setup instructions

## Bug Fixes & Optimizations

### Performance
- Reduced CPU usage in turn-based game loop (#154)
- Optimized display updates and rendering

### Gameplay Fixes
- Turn-based system now properly prevents monsters moving until player finishes command (#142)
- Resource list ordering now matches file order in-memory (#227)
- Fixed various AI behavior edge cases
- Cursor and movement system improvements

### Code Quality
- Fixed jlog2 utility function (#105, #102)
- String utility functions for improved memory management
- Removed problematic link-list implementations (#107)
- Enhanced logging and debug output

## Building & Running

### Build Options
```bash
make                 # Build with both ASCII and OpenGL renderers
make ascii           # ASCII-only executable (no SDL2/OpenGL required at runtime)
make opengl          # OpenGL-only executable
```

### Dependencies

**macOS:**
```bash
brew install sdl2 sdl2_image
```

**Linux/Debian:**
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libgl-dev build-essential
```

**Testing:**
- googletest and cucumber-cpp for BDD testing
- See [Developer-Setup-Guide.md](Developer-Setup-Guide.md) for comprehensive setup

## Known Issues & Future Work

See [WORKLIST.txt](../WORKLIST.txt), [WORKLIST_ai_player.md](../WORKLIST_ai_player.md), [WORKLIST_ascii_renderer.md](../WORKLIST_ascii_renderer.md), and related worklist files for ongoing development priorities.

## Contributors

Special thanks to B, L, and M -- all contributors who made this release possible through testing, development, and feedback.

## Version Info

- **Version**: 0.6.0
- **Game Engine**: SDL2/OpenGL with ASCII fallback
- **Language**: C++17
- **Platform Support**: macOS, Linux, Raspberry Pi OS
