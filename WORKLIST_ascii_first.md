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

- [x] **2.1** Create `JKeys.h` — define JMoria key types that mirror the SDL keysym interface
  - `JKeysym` struct with `sym` and `mod` fields (with default constructor)
  - `JKeycode` (`int32_t`) and `JKeymod` (`uint16_t`) typedefs
  - Key constants (`JKEY_a`..`JKEY_z`, `JKEY_RETURN`, `JKEY_ESCAPE`, `JKEY_F1`, arrows, keypad, etc.)
  - Modifier flags (`JMOD_SHIFT`, `JMOD_CTRL`, `JMOD_NONE`, etc.)
  - Values match SDL2 exactly so OpenGL event translation is a trivial cast
- [x] **2.2** Create `JTypes.h` — define `Uint8` / `Sint8` / `Uint16` without SDL dependency
  - `typedef uint8_t Uint8; typedef int8_t Sint8; typedef uint16_t Uint16;` (from `<cstdint>`)
- [x] **2.3** Update `StateBase.h` to use `JKeysym` instead of `SDL_Keysym`
  - Includes `JKeys.h` instead of `SDL2/SDL.h`
  - All `SDLK_*` → `JKEY_*`, `KMOD_*` → `JMOD_*`
- [x] **2.4** Update all state classes (`CmdState`, `ModState`, `LookState`, etc.) to use JMoria key types
  - Mass-replaced `SDL_Keysym` → `JKeysym`, `SDLK_*` → `JKEY_*`, `KMOD_*` → `JMOD_*` across 13 state .h/.cpp pairs
  - Function pointer typedefs updated (`typedef int (CXxxState::*XxxKeyHandler)(JKeysym *keysym)`)
  - `RangedState::GosubState()` — stack-allocated JKeysym instead of heap-allocated SDL_Keysym
- [x] **2.5** Update `JColor.h` to use `Uint8` from `JTypes.h` instead of `SDL2/SDL.h`
  - Removed SDL include, added `<cstdlib>` and `<cstring>` for `atoi`/`strtok`
  - `JMDefs.h` updated to include `JTypes.h`
- [x] **2.6** Update `HandleEventsASCII()` and `HandleEvents()` in `Game.cpp`
  - ASCII path: produces `JKeysym` with `JKEY_*` / `JMOD_*` directly (was already constructing SDL_Keysym)
  - OpenGL path: translates `SDL_Keysym` → `JKeysym` at the event boundary
  - `SetState()` — stack-allocated JKeysym instead of heap-allocated SDL_Keysym (3 instances)
  - `Render.h` — added direct `#include "SDL2/SDL.h"` (OpenGL renderer needs SDL)

### Phase 3: Decouple Tileset from SDL_image

- [x] **3.1** Guard `CTileset::Load()` image-loading code with `#ifdef RENDER_TILESET_POSTLOAD_NEEDED`
  - OpenGL path: loads PNG, creates GL texture, computes grid metrics from image dimensions (unchanged)
  - ASCII-only `#else` path: sets best-effort defaults for `m_dwTilesPerRow`/`m_vTexels`, returns `JSUCCESS`
- [x] **3.2** Guard `#include "SDL2/SDL.h"` and `#include "SDL2/SDL_image.h"` in `Tileset.cpp`
  - Both includes now inside `#ifdef RENDER_TILESET_POSTLOAD_NEEDED`
  - Verified: `Tileset.cpp` compiles with `-URENDER_TILESET_POSTLOAD_NEEDED` (no SDL dependency)
- [ ] **3.3** Guard `RENDER_TILESET_POSTLOAD_NEEDED` — only define when `RENDER_OPENGL` (deferred to Phase 5)
- [x] **3.4** ASCII-only tileset: `m_dwTilesPerRow = 95`, `m_vTexels = (1/95, 1.0)` as defaults
  - These are only used for DrawChar→DrawTile conversion in the OpenGL renderer; the ASCII renderer bypasses tile coords entirely

### Phase 4: Frame Timing Without SDL

- [x] **4.1** Abstract frame timing in `main.cpp`
  - Replaced `#include "SDL2/SDL.h"` with `<chrono>` + `<thread>`
  - Added `GetTicks()` (steady_clock) and `Delay()` (sleep_for) static helpers
  - All `SDL_GetTicks()` → `GetTicks()`, `SDL_Delay()` → `Delay()`
  - Works for both renderers — no more SDL dependency in main.cpp
- [x] **4.2** Remove `SDL_Init(SDL_INIT_TIMER)` from ASCII mode in `main.cpp`
  - Entire block removed (was only needed for SDL timer functions)
  - OpenGL mode: `CRender::InitSDL()` still handles full SDL initialization

### Phase 5: Compile Flag Infrastructure

- [x] **5.1** Define `RENDER_ASCII` and `RENDER_OPENGL` flags in `JMDefs.h`
  - If neither is defined externally, both are defined (default = runtime selection)
  - `RENDER_TILESET_POSTLOAD_NEEDED` now gated on `RENDER_OPENGL`
- [x] **5.2** Guard `#include "Render.h"` / `#include "RenderASCII.h"` in `Game.cpp` with flags
  - `Render.h` + `SDL2/SDL.h` under `#ifdef RENDER_OPENGL`
  - `RenderASCII.h` under `#ifdef RENDER_ASCII` (curses.h comes transitively)
- [x] **5.3** Guard renderer creation in `Game::Init()` with flags
  - `#if defined(RENDER_ASCII) && defined(RENDER_OPENGL)` for runtime selection
  - Single-renderer `#elif` paths for each
- [x] **5.4** Guard `HandleEventsASCII()` and SDL event loop in `Game.cpp`
  - ASCII dispatch under `#ifdef RENDER_ASCII`, SDL event loop under `#ifdef RENDER_OPENGL`
  - `HandleEventsASCII` definition and `UpdateASCIILayout` under `#ifdef RENDER_ASCII`
  - `CRender` forward declaration in `Game.h` under `#ifdef RENDER_OPENGL`
- [x] **5.5** Removed redundant `#include <curses.h>` from `Game.cpp` (already in `RenderASCII.h`)
- [x] **5.6** Guard whole files: `Render.cpp` / `RenderASCII.cpp` wrapped in `#ifdef RENDER_OPENGL` / `#ifdef RENDER_ASCII`
- [x] **5.7** Guard `m_TileSet->Texture()` calls in `DisplayText.cpp` and `Dungeon.cpp`
  - Pass `0` when `RENDER_TILESET_POSTLOAD_NEEDED` is not defined
- [x] Verified: `make CC_FLAGS="-DRENDER_ASCII"` compiles with zero errors
- [x] Verified: default `make` (both renderers) compiles with zero errors

### Phase 6: Makefile Build Modes

- [x] **6.1** Add Makefile targets:
  - `make` / `make all` — builds with both renderers (default, current behavior)
  - `make ascii` — builds ASCII-only (no SDL, no OpenGL, no SDL_image; links only ncurses)
  - `make opengl` — builds OpenGL-only (no ncurses)
- [x] **6.2** Source files all compile in every mode — `#ifdef` guards in Phase 5 make excluded renderers compile to empty translation units (no need for conditional source lists)
- [x] **6.3** Conditional linker flags via `RENDER_MODE` variable (`ascii`/`opengl`/`both`)
  - ASCII: links only `-lncurses`
  - OpenGL: links `-lSDL2 -lSDL2_image -framework OpenGL` (macOS) / `-lGL` (Linux), no ncurses
  - Both: links all libraries (default)
- [x] **6.4** `RENDER_DEFINES` passes `-DRENDER_ASCII` / `-DRENDER_OPENGL` via compile rule
- [x] Verified: `make` (default/both) builds clean
- [x] Verified: `make ascii` builds clean, links only ncurses
- [x] Verified: `make opengl` builds clean, links only SDL2+OpenGL

### Phase 7: Validation

- [x] **7.1** ASCII-only build compiles with zero SDL/OpenGL headers or libraries
  - `otool -L jmoria` shows only `libncurses` (no SDL2, no SDL2_image, no OpenGL)
- [x] **7.2** OpenGL-only build compiles with zero ncurses headers or libraries
  - `otool -L jmoria` shows SDL2+SDL2_image+OpenGL (no ncurses)
- [x] **7.3** Both-mode build works as before (runtime `--renderer=` selection)
  - `otool -L jmoria` shows all four libraries (SDL2, SDL2_image, ncurses, OpenGL)
- [x] **7.4** ASCII-only binary runs on a headless machine (no X11/Wayland/display)
  - Verified: launches ncurses with `DISPLAY` and `WAYLAND_DISPLAY` unset
- [x] **7.5** Existing tests still pass in default build configuration
  - 90 scenarios, 411 steps — all passed

## Open Questions (all answered; AI look here)

- Should `RenderMode` enum still exist in ASCII-only builds? (Probably not — just compile it out) correct, compile it out
- Should the `--renderer=` CLI arg be a no-op when only one renderer is compiled? (Probably yes, with a warning) yes
- Do we want a `JTimer` abstraction or is `#ifdef` in `main.cpp` sufficient? prefer JTimer unless it's trivial
- Test step definitions use SDL types — do tests need to support ASCII-only builds? yes
