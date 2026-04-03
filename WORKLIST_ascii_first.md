# WORKLIST: ASCII-First Renderer Architecture

## Philosophy

The game's core output is **characters and colors**. Dungeon tiles, monsters, items, and text are all fundamentally ASCII values with color attributes. The current architecture treats OpenGL as the primary renderer and encodes characters into tileset grid coordinates — then the ASCII renderer has to reverse-engineer the character back out. This is backwards.

**ASCII is the native representation.** The game code should speak in characters. The OpenGL renderer is a *presentation layer* that converts characters into textured quads. The ASCII renderer simply prints what the game already knows.

This is a successor to PR#155 (ASCII renderer implementation) and represents a deeper architectural alignment.

## Goals

1. Game code emits characters + colors, not tile indices or texture coordinates
2. ASCII renderer draws characters directly — no tileset loading, no SDL_image dependency
3. OpenGL renderer converts characters → texture coords as *its* responsibility
4. The game compiles and runs in three modes:
   - **ASCII-only**: no SDL, no OpenGL, no SDL_image — just ncurses
   - **OpenGL-only**: full SDL2 + OpenGL + SDL_image (current behavior)
   - **Both**: runtime selection via `--renderer=` flag (current PR#155 behavior)
5. Compile flags: `RENDER_ASCII`, `RENDER_OPENGL` (define one or both)

## Current State (Problems)

- `CTileset::Load()` calls `SDL_LoadBMP`/`IMG_Load` — requires SDL_image even in ASCII mode
- `CTileset::DrawTile()` passes `vTile` (grid coords) + `m_vTexels` (texel sizes) to the renderer
- `CRenderASCII::DrawTile()` reverse-engineers `char = tileIndex + ' ' + 1` from grid coords
- `JColor.h` and `StateBase.h` `#include "SDL2/SDL.h"` for `Uint8`, `SDL_Keysym`, `SDL_Keycode`, `SDLK_*`, `KMOD_*`
- `main.cpp` uses `SDL_GetTicks`/`SDL_Delay` even in ASCII mode (for frame timing)
- All source files compile together — no way to exclude `Render.cpp` or `RenderASCII.cpp`
- `RENDER_TILESET_POSTLOAD_NEEDED` in `JMDefs.h` is always defined (OpenGL-only concern)

## Tasks

### Phase 1: Decouple Character Representation from Tileset

The draw pipeline should pass characters, not tile grid coordinates.

- [x] **1.1** Add a character-based draw path to `IRenderBackend`
  - New virtual: `DrawChar(const JFVector &vPos, JVector &vSize, char ch)`
  - New virtual: `SetTileMetrics(int tilesPerRow, JFVector vTexels)` — stores tileset metrics for OpenGL
- [x] **1.2** Update `CTileset` to expose `DrawChar(char ch, vPos, vSize)` which calls `renderer->DrawChar` directly
  - `CTileset::PreDrawTile()` now calls `SetTileMetrics()` on the renderer
- [x] **1.3** Update `CRenderASCII::DrawChar()` — just `mvaddch` with the character directly
  - Existing `DrawTile` overloads now delegate to `DrawChar`
  - `TileIndexToChar()` retained only as legacy fallback in `DrawTile`
- [x] **1.4** Update `CRender::DrawChar()` — convert char to tile grid coords and draw textured quad
  - Uses stored metrics from `SetTileMetrics()` to compute texture coordinates
- [x] **1.5** Audit all callers of `CTileset::DrawTile()` — all converted to `DrawChar`
  - `DisplayText::DrawStr()` — passes `*ptr` directly instead of `*ptr - ' ' - 1`
  - `CDungeon::Draw()` — `m_dwIndex` renamed to `m_chTile`, stores char directly
  - `CPlayer::Draw()` — passes `'@'` directly
  - `CMonster::Draw()` — passes `MonIDs[m_md->m_dwIndex]` directly
  - `CItem::Draw()` — passes `ItemIDs[m_id->m_dwIndex]` directly

### Phase 2: SDL Type Abstraction

SDL types (`SDL_Keysym`, `SDL_Keycode`, `SDLK_*`, `KMOD_*`, `Uint8`) are used throughout the game logic. For ASCII-only builds, these need alternatives.

- [ ] **2.1** Create `JKeys.h` — define JMoria key types that mirror the SDL keysym interface
  - `JKeysym` struct with `sym` and `mod` fields
  - Key constants (`JKEY_a`..`JKEY_z`, `JKEY_RETURN`, `JKEY_ESCAPE`, etc.)
  - Modifier flags (`JMOD_SHIFT`, `JMOD_CTRL`, etc.)
  - When `RENDER_OPENGL` is defined, these can be typedefs/aliases to SDL equivalents
  - When ASCII-only, they're standalone integer constants matching the same values
- [ ] **2.2** Create `JTypes.h` — define `JUint8` / `JSint8` without SDL dependency
  - Simple: `typedef uint8_t JUint8;` etc. (from `<cstdint>`)
  - Or: when SDL is available, alias to `Uint8`
- [ ] **2.3** Update `StateBase.h` to use `JKeysym` instead of `SDL_Keysym`
- [ ] **2.4** Update all state classes (`CmdState`, `ModState`, `LookState`, etc.) to use JMoria key types
- [ ] **2.5** Update `JColor.h` to use `JUint8` instead of `Uint8`
- [ ] **2.6** Update `HandleEventsASCII()` in `Game.cpp` to produce `JKeysym` instead of `SDL_Keysym`

### Phase 3: Decouple Tileset from SDL_image

- [ ] **3.1** Guard `CTileset::Load()` image-loading code with `#ifdef RENDER_OPENGL`
  - ASCII-only: `Load()` only needs to store the character grid dimensions (hardcoded or from a simple config)
  - OpenGL: loads the PNG as before
- [ ] **3.2** Guard `#include "SDL2/SDL_image.h"` in `Tileset.cpp`
- [ ] **3.3** Guard `RENDER_TILESET_POSTLOAD_NEEDED` — only define when `RENDER_OPENGL`
- [ ] **3.4** ASCII-only tileset: store `m_dwTilesPerRow` and `m_vTexels` as compile-time constants or skip entirely if Phase 1 eliminates the need

### Phase 4: Frame Timing Without SDL

- [ ] **4.1** Abstract frame timing in `main.cpp`
  - ASCII-only: use `<chrono>` for `GetTicks` equivalent, `std::this_thread::sleep_for` for delay
  - OpenGL: continue using `SDL_GetTicks`/`SDL_Delay`
- [ ] **4.2** Remove `SDL_Init(SDL_INIT_TIMER)` from ASCII mode in `main.cpp`

### Phase 5: Compile Flag Infrastructure

- [ ] **5.1** Define `RENDER_ASCII` and `RENDER_OPENGL` flags
  - `JMDefs.h` or a new `BuildConfig.h`
  - Both defined = runtime selection (current behavior)
  - Only one = that renderer only
- [ ] **5.2** Guard `#include "Render.h"` / `#include "RenderASCII.h"` in `Game.cpp` with flags
- [ ] **5.3** Guard renderer creation in `Game::Init()` with flags
- [ ] **5.4** Guard `HandleEventsASCII()` and SDL event loop in `Game.cpp`
- [ ] **5.5** Guard `#include <curses.h>` in `Game.cpp` with `RENDER_ASCII`
- [ ] **5.6** Guard whole files: `Render.cpp` excluded when `!RENDER_OPENGL`, `RenderASCII.cpp` excluded when `!RENDER_ASCII`

### Phase 6: Makefile Build Modes

- [ ] **6.1** Add Makefile targets:
  - `make` / `make all` — builds with both renderers (default, current behavior)
  - `make ascii` — builds ASCII-only (no SDL, no OpenGL, no SDL_image; links only ncurses)
  - `make opengl` — builds OpenGL-only (no ncurses)
- [ ] **6.2** Conditional source file lists (exclude `Render.cpp` or `RenderASCII.cpp`)
- [ ] **6.3** Conditional linker flags (drop `-lSDL2 -lSDL2_image -framework OpenGL` for ASCII-only)
- [ ] **6.4** Pass `-DRENDER_ASCII` / `-DRENDER_OPENGL` via `CC_FLAGS`

### Phase 7: Validation

- [ ] **7.1** ASCII-only build compiles with zero SDL/OpenGL headers or libraries
- [ ] **7.2** OpenGL-only build compiles with zero ncurses headers or libraries  
- [ ] **7.3** Both-mode build works as before (runtime `--renderer=` selection)
- [ ] **7.4** ASCII-only binary runs on a headless machine (no X11/Wayland/display)
- [ ] **7.5** Existing tests still pass in all three build configurations

## Open Questions

- Should `RenderMode` enum still exist in ASCII-only builds? (Probably not — just compile it out)
- Should the `--renderer=` CLI arg be a no-op when only one renderer is compiled? (Probably yes, with a warning)
- Do we want a `JTimer` abstraction or is `#ifdef` in `main.cpp` sufficient?
- Test step definitions use SDL types — do tests need to support ASCII-only builds?
