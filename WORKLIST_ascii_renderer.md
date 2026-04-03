# JMoria ASCII Renderer Enhancement - Work List

## Executive Summary

This document describes the architecture and implementation plan for adding ASCII terminal rendering to JMoria, supplementing the existing OpenGL-based graphical renderer. The game already converts DungeonTiles, Monsters, and Items to ASCII characters for font texture lookup, so this enhancement leverages that existing infrastructure.

## Current Architecture Analysis

### Existing Rendering Pipeline

1. **Render Layer** (`Render.h/cpp`)
   - Uses SDL2 + OpenGL
   - Screen size: 640x480 (configurable: SCREEN_WIDTH, SCREEN_HEIGHT)
   - Viewport setup with orthographic projection
   - Texture-based tile rendering

2. **Dungeon Rendering** (`Dungeon.h/cpp`)
   - Dungeon size: 100x100 tiles (DUNG_WIDTH x DUNG_HEIGHT)
   - Uses CTileset for drawing tiles (font: SmallText6x8.png, 6x8 pixels per char)
   - Already converts tile types to ASCII via tile indices
   - Viewport shows subset of dungeon centered on player
   - Three rendering phases: DrawDungeon(), DrawItems(), DrawMonsters()

3. **ASCII Conversion (Already Implemented!)**
   - **Monsters**: `unsigned char MonIDs[MON_IDX_MAX+1] = "abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t";`
     - Monster drawing: `Uint8 monster_tile = MonIDs[m_md->m_dwIndex] - ' ' - 1;`
   - **Items**: `unsigned char ItemIDs[ITEM_IDX_MAX+1] = "|)[](]]\"=~{}{}&?!-_?$~/\\/";`
     - Item drawing: `Uint8 item_tile = ItemIDs[m_id->m_dwIndex] - ' ' - 1;`
   - **Tiles**: Dungeon tiles use `m_dtd->m_dwIndex` directly for font lookup
   - Font tileset maps ASCII characters (space through ~) to tile indices

4. **DisplayText Areas** (`DisplayText.h/cpp`)
   - Already renders text as ASCII characters
   - Multiple text areas with specific screen positions:
     - Messages: JRect(0, 0, 640, 45) - top banner
     - Stats: JRect(0, 50, 150, 480) - left sidebar
     - Inventory: JRect(440, 50, 640, 340) - right upper
     - Equipment: JRect(440, 345, 640, 480) - right lower
     - Use menu: JRect(200, 40, 440, 480) - center overlay
     - End game: JRect(0, 0, 640, 480) - full screen
   - Font drawing: FONT_DRAW_W=6, FONT_DRAW_H=8
   - Supports scrolling, pagination, bounding boxes, text wrapping

5. **Game Loop** (`Game.cpp`)
   - Init() → sets up renderer and display texts
   - Draw() → calls dungeon, player, and all DisplayText::Draw()
   - HandleEvents() → processes SDL events

## Design Goals

1. **Flexibility**: User can choose renderer at startup (or runtime?)
2. **Portability**: Support Linux, Mac, and potentially more platforms
3. **Code Reuse**: Maximize use of existing ASCII conversion and DisplayText code
4. **Two Rendering Modes**:
   - **Mode A**: Single terminal window (80x24 or 125x40 fixed size)
   - **Mode B**: Multiple terminal windows (one per DisplayText area)
5. **Feature Parity**: All status text, messages, FPS, tombstone visible
6. **Partial Viewport**: Dungeon larger than screen is acceptable

## Proposed Architecture

### 1. Abstract Renderer Interface

Create a base renderer interface that both OpenGL and ASCII renderers implement:

```cpp
// RenderBase.h - Abstract renderer interface
class IRenderBackend
{
public:
    virtual ~IRenderBackend() {}
    
    // Initialization
    virtual JResult Init(int width, int height, int bpp) = 0;
    virtual void Term() = 0;
    
    // Frame management
    virtual void PreDraw() = 0;
    virtual void PostDraw() = 0;
    virtual void SwapBuffers() = 0;
    
    // Tile drawing (world space)
    virtual void PreDrawTile() = 0;
    virtual void PostDrawTile() = 0;
    virtual void SetTileColor(JColor color) = 0;
    virtual bool DrawTile(const JFVector &vPos, JVector &vSize, 
                         JIVector &vTile, JFVector &vTexels) = 0;
    
    // Text rendering (screen space)
    virtual void PreDrawText(JRect bounds) = 0;
    virtual void PostDrawText() = 0;
    virtual void DrawChar(int x, int y, char ch, JColor color) = 0;
    virtual void DrawTextBoundingBox(JRect rect, uint8 alpha) = 0;
    
    // Configuration
    virtual int GetScreenWidth() const = 0;
    virtual int GetScreenHeight() const = 0;
    virtual int GetCharWidth() const = 0;
    virtual int GetCharHeight() const = 0;
};
```

### 2. Refactor Existing OpenGL Renderer

```cpp
// RenderGL.h - OpenGL implementation
class CRenderGL : public CRender, public IRenderBackend
{
public:
    // Implement all IRenderBackend methods
    // Keep existing OpenGL-specific code
};
```

### 3. ASCII Renderer Implementation

#### 3A. Single Terminal Mode (ncurses-based)

Use ncurses for portable terminal manipulation:

```cpp
// RenderASCII.h
#include <ncurses.h>

class CRenderASCII : public IRenderBackend
{
public:
    CRenderASCII();
    virtual ~CRenderASCII();
    
    // Configuration
    enum TerminalMode {
        MODE_80x24,
        MODE_125x40,
        MODE_CUSTOM
    };
    
    JResult Init(int width, int height, int bpp) override;
    void Term() override;
    
    void PreDraw() override;
    void PostDraw() override;
    void SwapBuffers() override;
    
    // Tile drawing - converts world coords to screen coords
    void PreDrawTile() override;
    void PostDrawTile() override;
    void SetTileColor(JColor color) override;
    bool DrawTile(const JFVector &vPos, JVector &vSize, 
                 JIVector &vTile, JFVector &vTexels) override;
    
    // Text rendering
    void PreDrawText(JRect bounds) override;
    void PostDrawText() override;
    void DrawChar(int x, int y, char ch, JColor color) override;
    void DrawTextBoundingBox(JRect rect, uint8 alpha) override;
    
    // ASCII-specific
    void SetTerminalMode(TerminalMode mode);
    void DefineLayout(const ASCIILayout &layout);
    
    int GetScreenWidth() const override { return m_termWidth; }
    int GetScreenHeight() const override { return m_termHeight; }
    int GetCharWidth() const override { return 1; }
    int GetCharHeight() const override { return 1; }
    
private:
    // Terminal state
    int m_termWidth;
    int m_termHeight;
    WINDOW *m_mainWindow;
    
    // Double buffering for terminal
    char **m_backBuffer;     // [height][width]
    short **m_colorBuffer;   // [height][width] - ncurses color pair indices
    
    // Viewport/camera for dungeon
    JIVector m_dungeonViewOffset;  // Top-left corner of visible dungeon
    JRect m_dungeonViewport;       // Screen area for dungeon (in chars)
    
    // Layout regions (in character coordinates)
    ASCIILayout m_layout;
    
    // Color management
    JColor m_currentColor;
    void InitColors();
    short GetColorPair(JColor color);
    
    // Helper functions
    void ClearBackBuffer();
    void FlushBackBuffer();
    char TileIndexToChar(int tileIndex);
    JIVector WorldToScreen(const JFVector &worldPos);
};
```

Layout definition:
```cpp
// ASCIILayout.h
struct ASCIILayoutRegion
{
    JRect bounds;        // Character-based coordinates
    bool enabled;
    const char *label;
};

struct ASCIILayout
{
    ASCIILayoutRegion dungeon;
    ASCIILayoutRegion messages;
    ASCIILayoutRegion stats;
    ASCIILayoutRegion inventory;
    ASCIILayoutRegion equipment;
    
    // Predefined layouts
    static ASCIILayout Create80x24();
    static ASCIILayout Create125x40();
};

// Example 80x24 layout:
// +--------------------------------------------------+
// | Messages (2 lines)                               |
// +----------+---------------------------+-----------+
// | Stats    | Dungeon                   | Inventory |
// | (10 ch)  | (50x20 viewport)          | (18 ch)   |
// |          |                           |           |
// |          |                           +-----------+
// |          |                           | Equipment |
// |          |                           | (18 ch)   |
// +----------+---------------------------+-----------+
// | Status line (1 line)                             |
// +--------------------------------------------------+
```

#### 3B. Multiple Terminal Windows Mode

Alternative implementation using multiple terminal windows (tmux integration or separate terminal spawning):

```cpp
// RenderASCIIMultiWindow.h
class CRenderASCIIMultiWindow : public IRenderBackend
{
public:
    // Each DisplayText gets its own terminal window
    struct TerminalWindow {
        pid_t pid;           // For external terminal process
        int ptyFd;           // Pseudo-terminal file descriptor
        JRect screenPos;     // Position on physical screen
        JRect size;          // Size in characters
        char **buffer;       // Content buffer
        bool dirty;          // Needs redraw
    };
    
private:
    std::map<std::string, TerminalWindow*> m_windows;
    TerminalWindow *m_dungeonWindow;
    TerminalWindow *m_messagesWindow;
    TerminalWindow *m_statsWindow;
    TerminalWindow *m_inventoryWindow;
    TerminalWindow *m_equipmentWindow;
    
    // Window management
    TerminalWindow* CreateWindow(const char *name, JRect size, JRect screenPos);
    void WriteToWindow(TerminalWindow *win, int x, int y, const char *text);
    void RefreshWindow(TerminalWindow *win);
};
```

### 4. DisplayText Modifications

Update DisplayText to work with abstract renderer:

```cpp
// DisplayText.h - modifications
class CDisplayText
{
public:
    // ... existing code ...
    
    // Make rendering backend-agnostic
    void Draw();  // Calls renderer through abstract interface
    
private:
    IRenderBackend *m_pRenderer;  // Instead of accessing CRender directly
    
    // Helper to get char width/height from renderer
    int GetCharWidth() { return m_pRenderer->GetCharWidth(); }
    int GetCharHeight() { return m_pRenderer->GetCharHeight(); }
};
```

### 5. Tileset Abstraction

Make Tileset work with both texture-based and ASCII rendering:

```cpp
// TileSet.h - modifications
class CTileset
{
public:
    // ... existing code ...
    
    // Add ASCII mapping
    char GetASCIIChar(int tileIndex);
    void SetASCIIMap(const char *asciiMap);
    
private:
    const char *m_asciiMap;  // Maps tile indices to ASCII chars
    bool m_isTextureMode;    // true for OpenGL, false for ASCII
};
```

### 6. Configuration System

Add runtime configuration for renderer selection:

```cpp
// Config.h - new file
enum RenderMode {
    RENDER_OPENGL,
    RENDER_ASCII_SINGLE,
    RENDER_ASCII_MULTI
};

struct JMoriaConfig
{
    RenderMode renderMode;
    int termWidth;
    int termHeight;
    bool useColor;
    
    // Load from config file or command line
    static JMoriaConfig LoadFromArgs(int argc, char **argv);
    static JMoriaConfig LoadFromFile(const char *path);
};
```

## Implementation Plan

### Phase 1: Infrastructure (Refactoring)

**Task 1.1: Create Abstract Renderer Interface** ✅ DONE
- Files: `RenderBase.h` (new)
- Define IRenderBackend interface
- Document all required methods

**Task 1.2: Refactor CRender to Use Interface** ✅ DONE
- Files: `Render.h`, `Render.cpp`, `Game.h`, `Game.cpp`
- Make CRender implement IRenderBackend
- Keep OpenGL implementation unchanged
- Update CGame to use IRenderBackend pointer

**Task 1.3: Update DisplayText for Abstraction** ✅ DONE
- Files: `DisplayText.cpp`
- Replaced `#include "Render.h"` with `#include "RenderBase.h"`
- DisplayText now only depends on IRenderBackend interface
- DisplayText.h needed no changes (no direct renderer dependency)

**Task 1.4: Update Dungeon Rendering** ✅ DONE
- Files: `Dungeon.cpp`
- Replaced `#include "Render.h"` with `#include "RenderBase.h"`
- Dungeon now only depends on IRenderBackend interface
- Dungeon.h needed no changes

**Task 1.5: Update TileSet** ✅ DONE
- Files: `Tileset.cpp`
- Replaced `#include "Render.h"` with `#include "RenderBase.h"`
- TileSet now only depends on IRenderBackend interface
- TileSet.h needed no changes
- SDL2 includes retained for texture loading (image I/O, not renderer-specific)

### Phase 2: ASCII Single-Window Renderer

**Task 2.1: Implement RenderASCII Base** ✅ DONE
- Files: `src/RenderASCII.h` (new), `src/RenderASCII.cpp` (new)
- Initialize ncurses (raw mode, no echo, hidden cursor, non-blocking input)
- Uses ncurses erase()/refresh() for double buffering
- Implements full IRenderBackend interface

**Task 2.2: Implement Color Support** ✅ DONE
- 8 ncurses color pairs (white, red, green, yellow, blue, magenta, cyan, dark)
- Maps JColor RGBA to nearest terminal color via dominant channel detection
- Graceful fallback when terminal has no color support

**Task 2.3: Define ASCII Layouts** ✅ DONE
- ASCIILayout struct with regions defined inline in RenderASCII.h/cpp (no separate files needed)
- 80x24 standard layout: messages(2 rows) | stats(12w) + dungeon(50w) + inv/equip(18w) 
- 125x40 extended layout: messages(3 rows) | stats(20w) + dungeon(75w) + inv/equip(30w)
- Layout auto-selected based on Init() size parameters

**Task 2.4: Implement Dungeon Viewport** ✅ DONE
- MapX/MapY convert world tile coords to terminal dungeon region chars
- Camera following player via PreDrawObjects bounds (same as OpenGL path)
- Translation support for dungeon centering
- Bounds checking prevents drawing outside terminal

**Task 2.5: Implement Text Region Rendering** ✅ DONE
- MapX/MapY detect pixel-space context (DisplayText) vs world-space (Dungeon)
- Pixel coords divided by FONT_DRAW_W/H (6/8) to get terminal char positions
- Box-drawing characters (ACS_ULCORNER, ACS_HLINE, etc.) for bounding boxes

**Task 2.6: Tile-to-ASCII Mapping** ✅ DONE
- TileIndexToChar() reverses the font texture index back to ASCII: index + ' ' + 1
- Existing MonIDs/ItemIDs/TileIDs arrays already produce correct tile indices
- No separate mapping needed — the font lookup IS the ASCII char

**Task 2.7: Build System Updates** ✅ DONE
- Added `-lncurses` to both macOS and Linux linker flags in Makefile
- RenderASCII.cpp auto-discovered by wildcard `$(wildcard src/*.cpp)`
  - Stairs up: '<'
  - Stairs down: '>'
  - Player: '@'

### Phase 3: Configuration and Integration

**Task 3.1: Command-Line Arguments**
- Files: `main.cpp`
- Add --renderer option (opengl, ascii, ascii-multi)
- Add --term-size option (80x24, 125x40, WxH)
- Add --no-color option

**Task 3.2: Runtime Renderer Factory**
- Files: `RenderFactory.h` (new), `RenderFactory.cpp` (new)
- Create appropriate renderer based on config
- Handle initialization errors gracefully

**Task 3.3: Update Game Initialization**
- Files: `Game.cpp`
- Use renderer factory
- Adjust DisplayText sizes based on renderer capabilities
- Handle renderer-specific initialization

**Task 3.4: Input Handling Updates**
- Files: `Game.cpp`
- SDL events for OpenGL mode
- stdin/getch() for ASCII mode
- Unified input abstraction

### Phase 4: Multiple Window Mode (Optional/Future)

**Task 4.1: Implement Multi-Window Renderer**
- Files: `RenderASCIIMultiWindow.h/cpp` (new)
- Pseudo-terminal management
- Launch separate terminal windows
- IPC for window updates

**Task 4.2: Window Position Management**
- Screen layout configuration
- Window spawning and positioning
- Handle window focus

**Task 4.3: Testing and Integration**
- Test on multiple terminal emulators
- Handle window closing/reopening
- Synchronization between windows

### Phase 5: Testing and Polish

**Task 5.1: Cross-Platform Testing**
- Test on Linux with various terminals (xterm, gnome-terminal, konsole)
- Test on macOS with Terminal.app and iTerm2
- Verify ncurses compatibility

**Task 5.2: Performance Optimization**
- Minimize terminal redraws
- Efficient double buffering
- Dirty region tracking

**Task 5.3: Documentation**
- Update README.md with new renderer options
- Create ASCII_RENDERER.md guide
- Document terminal requirements

**Task 5.4: Edge Cases**
- Handle terminal resize (for OpenGL mode comparison)
- Handle color vs. monochrome terminals
- Graceful degradation for small terminals

## Technical Details

### Coordinate Systems

1. **World Coordinates** (Dungeon space)
   - 100x100 tiles (DUNG_WIDTH x DUNG_HEIGHT)
   - Floating-point positions
   - Origin at dungeon (0,0)

2. **Viewport Coordinates** (OpenGL)
   - 640x480 pixels
   - Camera centered on player
   - Orthographic projection with DUNG_WIDTH/HEIGHT units

3. **Screen Coordinates** (ASCII)
   - 80x24 or 125x40 characters
   - Fixed layout regions
   - Dungeon viewport may be 50x20 chars

### ASCII Coordinate Conversion

```cpp
// Convert world position to screen character position
JIVector CRenderASCII::WorldToScreen(const JFVector &worldPos)
{
    JIVector screenPos;
    
    // Get player position (camera center)
    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    
    // Dungeon viewport dimensions
    int viewWidth = m_layout.dungeon.bounds.Width();
    int viewHeight = m_layout.dungeon.bounds.Height();
    
    // Calculate relative to camera center
    int relX = (int)(worldPos.x - playerPos.x) + (viewWidth / 2);
    int relY = (int)(worldPos.y - playerPos.y) + (viewHeight / 2);
    
    // Offset by viewport position
    screenPos.x = m_layout.dungeon.bounds.left + relX;
    screenPos.y = m_layout.dungeon.bounds.top + relY;
    
    return screenPos;
}
```

### Color Mapping

```cpp
void CRenderASCII::InitColors()
{
    if (!has_colors()) {
        m_useColor = false;
        return;
    }
    
    start_color();
    
    // Define color pairs (foreground, background)
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK);
    // ... more pairs for various game colors
}

short CRenderASCII::GetColorPair(JColor color)
{
    // Map JColor (RGBA) to nearest ncurses color pair
    // Simple approach: map to 8 basic colors
    // Advanced: use extended color pairs for 256-color terminals
    
    if (!m_useColor) return 0;
    
    // Determine primary color component
    if (color.r > 128 && color.g < 128 && color.b < 128)
        return 2;  // Red
    else if (color.r < 128 && color.g > 128 && color.b < 128)
        return 3;  // Green
    // ... more color mapping logic
    
    return 1;  // Default white
}
```

### Box-Drawing Characters

For bounding boxes, use Unicode box-drawing characters:

```cpp
void CRenderASCII::DrawTextBoundingBox(JRect rect, uint8 alpha)
{
    // Use box-drawing characters
    // ┌─┐
    // │ │
    // └─┘
    
    int left = rect.left;
    int right = rect.right;
    int top = rect.top;
    int bottom = rect.bottom;
    
    // Top border
    mvaddch(top, left, ACS_ULCORNER);
    for (int x = left + 1; x < right; x++)
        mvaddch(top, x, ACS_HLINE);
    mvaddch(top, right, ACS_URCORNER);
    
    // Side borders
    for (int y = top + 1; y < bottom; y++) {
        mvaddch(y, left, ACS_VLINE);
        mvaddch(y, right, ACS_VLINE);
    }
    
    // Bottom border
    mvaddch(bottom, left, ACS_LLCORNER);
    for (int x = left + 1; x < right; x++)
        mvaddch(bottom, x, ACS_HLINE);
    mvaddch(bottom, right, ACS_LRCORNER);
}
```

## Dependencies

### Required Libraries

**For ASCII Rendering:**
- ncurses (libncurses-dev on Linux, already included on macOS)
- Alternative: pdcurses for Windows (future)

**Existing Dependencies:**
- SDL2
- SDL2_image
- OpenGL

### Build System Changes

Update Makefile:

```makefile
# Makefile additions
LIBS_GL = -lSDL2 -lSDL2_image -lGL
LIBS_ASCII = -lncurses
LIBS = $(LIBS_GL) $(LIBS_ASCII)

# Conditional compilation
CFLAGS += -DSUPPORT_OPENGL -DSUPPORT_ASCII

# Object files
OBJS = ... RenderASCII.o ASCIILayout.o RenderFactory.o ...
```

## Example Usage

```bash
# Run with OpenGL (default)
./jmoria

# Run with ASCII single window (80x24)
./jmoria --renderer=ascii --term-size=80x24

# Run with ASCII single window (125x40)
./jmoria --renderer=ascii --term-size=125x40

# Run without color (for compatibility)
./jmoria --renderer=ascii --no-color

# Future: multi-window mode
./jmoria --renderer=ascii-multi
```

## Advantages of This Approach

1. **Minimal Changes to Game Logic**: Game code (Player, Dungeon, Monster, Item) unchanged
2. **Leverages Existing ASCII**: Already converts tiles/monsters/items to ASCII
3. **Portable**: ncurses works on Linux, macOS, BSD, and more
4. **Maintains OpenGL**: Both renderers can coexist
5. **Standard Roguelike Look**: Familiar to roguelike players
6. **Lightweight**: Can run over SSH or on resource-constrained systems
7. **Flexible**: Easy to add new terminal sizes or layouts

## Potential Challenges

1. **Input Handling**: SDL vs. ncurses input handling requires abstraction
2. **FPS Counter**: Less meaningful in ASCII mode (may skip or show update rate)
3. **Color Limitations**: Terminal colors less rich than OpenGL
4. **Font Dependency**: OpenGL mode needs SmallText6x8.png; ASCII mode doesn't
5. **Multi-Window Complexity**: OS-specific window management
6. **Testing**: Need to test on many terminal types and sizes

## Future Enhancements

1. **Resizable Terminal**: Handle SIGWINCH and redraw on terminal resize
2. **Mouse Support**: Use ncurses mouse events in ASCII mode
3. **Themes**: Define multiple ASCII layouts and color schemes
4. **Tiles Mode**: Use CP437 extended ASCII for more graphical tiles
5. **Animation**: Simple ASCII animations for effects
6. **Split-Screen**: Show multiple dungeon levels or map views

## Files to Create

New files:
1. `RenderBase.h` - Abstract renderer interface
2. `RenderASCII.h/cpp` - ASCII single-window renderer
3. `RenderASCIIMultiWindow.h/cpp` - ASCII multi-window renderer (future)
4. `ASCIILayout.h/cpp` - Layout definitions and management
5. `RenderFactory.h/cpp` - Renderer creation factory
6. `Config.h/cpp` - Configuration management
7. `ASCII_RENDERER.md` - Documentation

Modified files:
1. `Render.h/cpp` - Implement IRenderBackend
2. `DisplayText.h/cpp` - Use abstract renderer
3. `Dungeon.h/cpp` - Use abstract renderer
4. `TileSet.h/Tileset.cpp` - Add ASCII support
5. `Game.h/cpp` - Renderer factory, input abstraction
6. `main.cpp` - Command-line argument parsing
7. `Makefile` - Add ncurses, new object files
8. `README.md` - Document new renderer options

## Conclusion

This ASCII renderer enhancement maintains the spirit of classic roguelikes while preserving the modern OpenGL renderer. By leveraging existing ASCII conversion infrastructure and using standard ncurses, the implementation should be portable and relatively straightforward. The phased approach allows incremental development and testing, with the single-window mode providing immediate value and the multi-window mode as an optional advanced feature.

The key insight is that **JMoria is already 90% of the way there** - it already thinks in ASCII for tile representation, it just needs a terminal-based output layer instead of OpenGL texture rendering.
