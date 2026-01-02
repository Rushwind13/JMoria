# JMoria Rendering Architecture

## Overview

JMoria uses a clean 3-layer abstraction for rendering game elements. This separates game logic from display representation from graphical rendering.

## The 3-Layer Tile System

```
Layer 1: Conceptual (Game Logic)
    DUNG_IDX_WALL, DUNG_IDX_FLOOR, MON_IDX_ORC, ITEM_IDX_SWORD
    │
    ▼
Layer 2: ASCII Representation
    TileIDs[] = ".#+'<<>>:#@"
    MonIDs[]  = "abcd...XY&.,$t"
    ItemIDs[] = "|)[](]...//\\"
    │
    ▼
Layer 3: Graphical Tiles (Sprite Sheet)
    char - ' ' - 1 = tileset index
    Rendered via OpenGL quads from PNG tileset
```

## Layer 1: Conceptual Types

Game logic uses abstract type constants defined in `src/Constants.h`:

### Dungeon Tiles
```c
#define DUNG_IDX_FLOOR          0   // Walkable floor
#define DUNG_IDX_WALL           1   // Solid wall
#define DUNG_IDX_DOOR           2   // Closed door
#define DUNG_IDX_OPEN_DOOR      3   // Open door
#define DUNG_IDX_UPSTAIRS       4   // Stairs up
#define DUNG_IDX_LONG_UPSTAIRS  5   // Long stairs up
#define DUNG_IDX_DOWNSTAIRS     6   // Stairs down
#define DUNG_IDX_LONG_DOWNSTAIRS 7  // Long stairs down
#define DUNG_IDX_RUBBLE         8   // Rubble
#define DUNG_IDX_SECRET_DOOR    9   // Secret door (looks like wall)
#define DUNG_IDX_PLAYER         10  // Player position
#define DUNG_IDX_MAX            11
```

### Monsters
Monster types use `MON_IDX_*` constants (55 types defined in `src/Constants.h`).

### Items
Item types use `ITEM_IDX_*` constants (31 types defined in `src/Constants.h`).

## Layer 2: ASCII Mapping

Each conceptual type maps to an ASCII character. This is the "roguelike" representation.

### Dungeon ASCII (`src/Dungeon.cpp`)
```c
unsigned char TileIDs[DUNG_IDX_MAX + 1] = ".#+'<<>>:#@";
```

| Type | ASCII | Meaning |
|------|-------|---------|
| DUNG_IDX_FLOOR | `.` | Floor |
| DUNG_IDX_WALL | `#` | Wall |
| DUNG_IDX_DOOR | `+` | Closed door |
| DUNG_IDX_OPEN_DOOR | `'` | Open door |
| DUNG_IDX_UPSTAIRS | `<` | Stairs up |
| DUNG_IDX_DOWNSTAIRS | `>` | Stairs down |
| DUNG_IDX_RUBBLE | `:` | Rubble |
| DUNG_IDX_SECRET_DOOR | `#` | Secret door |
| DUNG_IDX_PLAYER | `@` | Player |

### Monster ASCII (`src/Monster.cpp`)
```c
unsigned char MonIDs[MON_IDX_MAX + 1] = "abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t";
```

Letters represent monster types (see Developer's Guide for full mapping).

### Item ASCII (`src/Item.cpp`)
```c
unsigned char ItemIDs[ITEM_IDX_MAX + 1] = "|)[](]]\"=~{}{}&?!-_?$~//\\/|/|]!";
```

Symbols represent item types (weapons, armor, scrolls, etc.).

## Layer 3: Graphical Rendering

ASCII characters are converted to tileset indices for sprite rendering.

### Conversion Formula
```c
Uint8 tile_index = ASCII_char - ' ' - 1;
```

This shifts ASCII values (starting at space = 32) to 0-based tileset indices.

### Tileset Structure (`src/Tileset.h`)

The tileset is a PNG image with characters arranged in a grid:
- `Resources/Courier.png` (32x32 cells) or
- `Resources/SmallText6X8.png` (6x8 cells)

```c
class CTileset {
    int m_dwTilesPerRow;     // Characters per row in texture
    JFVector m_vTexels;      // Normalized texel size for UV coords

    // Convert linear index to 2D grid position
    JIVector& GetTile(int dwIndex, JIVector& vTile) {
        vTile.x = dwIndex % m_dwTilesPerRow;
        vTile.y = dwIndex / m_dwTilesPerRow;
        return vTile;
    }
};
```

### Rendering Pipeline

1. **Game retrieves tile type** from dungeon/monster/item data
2. **Look up ASCII character** from `TileIDs[]`, `MonIDs[]`, or `ItemIDs[]`
3. **Convert to tileset index**: `char - ' ' - 1`
4. **Calculate grid position**: `x = index % tiles_per_row`, `y = index / tiles_per_row`
5. **Render OpenGL quad** with texture coordinates pointing to sprite

## Key Files

| File | Layer | Purpose |
|------|-------|---------|
| `src/Constants.h` | 1 | Type constants (DUNG_IDX_*, MON_IDX_*, ITEM_IDX_*) |
| `src/Dungeon.cpp` | 2 | TileIDs[] array, dungeon rendering |
| `src/Monster.cpp` | 2 | MonIDs[] array, monster rendering |
| `src/Item.cpp` | 2 | ItemIDs[] array, item rendering |
| `src/Tileset.cpp` | 3 | Tileset loading, index-to-UV conversion |
| `src/Render.cpp` | 3 | OpenGL quad rendering |

## Benefits of This Architecture

1. **Separation of concerns**: Game logic doesn't know about rendering
2. **Easy to modify display**: Change ASCII mappings without touching game logic
3. **Flexible rendering**: Can swap tilesets (Courier vs SmallText) at compile time
4. **Traditional roguelike feel**: ASCII layer provides classic representation
5. **AI-friendly**: ASCII layer is human-readable for logging/debugging
