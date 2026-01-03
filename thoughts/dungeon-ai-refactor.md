# Dungeon AI Logging Refactoring Plan

## Context

PR #157 review identified architectural concerns in `DumpToAILog()`:
- RLE encoding in Dungeon.cpp is a render/output concern
- `extern MonIDs/ItemIDs` violates separation of dungeon/item/monster
- AI log duplicates rendering patterns instead of reusing them
- Wall types don't distinguish lit/tunnelable

## Current Architecture

### Character ID Arrays (Defined Separately)
```
TileIDs[]  → src/Dungeon.cpp:15   - ".#+'<<>>:#@"
MonIDs[]   → src/Monster.cpp:340  - "abcd...XY&.,$t"
ItemIDs[]  → src/Item.cpp:166     - "|)[](]...//\\"
```

### Dungeon Rendering Flow (src/Dungeon.cpp)
```
Draw()
  ├─ PreDraw()
  ├─ DrawDungeon()   → Iterates grid, checks visibility/lighting
  ├─ DrawItems()     → Iterates m_llItems linked list
  ├─ DrawMonsters()  → Iterates m_llMonsters linked list
  └─ PostDraw()
```

### Current DumpToAILog() Issues
1. Has `extern unsigned char MonIDs[]` and `ItemIDs[]` (code smell)
2. Builds its own view extraction logic (duplicates rendering)
3. RLE encoding is inline (should be utility or Render)
4. Doesn't respect DUNG_FLAG_LIT for unlit tiles
5. 200+ lines of tightly coupled code

---

## Proposed Refactoring

### Option A: Extract to Render/Utility (Recommended)

Move output encoding to appropriate locations while keeping Dungeon responsible for data access.

#### Step 1: Create AIRender utility (new file)

**New file: `src/AIRender.h` / `AIRender.cpp`**
```cpp
// Character ID access - each class owns its own
char GetTileChar(int tileType);      // Calls TileIDs[type]
char GetMonsterChar(CMonster *mon);  // Monster knows its own char
char GetItemChar(CItem *item);       // Item knows its own char

// RLE encoding utility
int RLEEncodeRow(const char *row, int len, char *output);
bool IsAllWalls(const char *row, int len);

// View extraction
void ExtractViewport(CDungeon *dungeon, JIVector playerPos,
                     int radius, char map[VIEW_SIZE][VIEW_SIZE+1]);
```

#### Step 2: Add GetChar() to Monster and Item classes

**In Monster.h:**
```cpp
char GetChar() const { return MonIDs[m_md->m_dwIndex]; }
```

**In Item.h:**
```cpp
char GetChar() const { return ItemIDs[m_id->m_dwIndex]; }
```

This eliminates the need for `extern` - each class owns its character representation.

#### Step 3: Refactor DumpToAILog()

**Simplified Dungeon.cpp:**
```cpp
void CDungeon::DumpToAILog()
{
    if(!AILog_IsActive()) return;

    // Use AIRender utilities
    char map[VIEW_SIZE][VIEW_SIZE+1];
    JRect viewBounds = AIRender_ExtractViewport(this, playerPos, VIEW_RADIUS, map);

    // Overlay items (using Item::GetChar())
    OverlayItemsToMap(map, viewBounds);

    // Overlay monsters (using Monster::GetChar())
    OverlayMonstersToMap(map, viewBounds);

    // Output uses AIRender for RLE
    AIRender_OutputDungeonJSON(this, map, viewBounds);
}
```

#### Step 4: Handle unlit tiles

Add visibility check in ExtractViewport:
```cpp
if(!(tile->m_dwFlags & DUNG_FLAG_LIT) &&
   !(tile->m_dwFlags & DUNG_FLAG_SEEN))
{
    map[vy][vx] = ' ';  // Unknown tile
}
```

#### Step 5: Wall type distinction

For now, keep `#` for all walls. Future enhancement could add:
- `#` for regular wall
- `%` for tunnelable rubble
- Different char for secret doors (currently shows as `#`)

---

## Files to Modify

| File | Changes |
|------|---------|
| `src/AIRender.h` (new) | RLE encoding, viewport extraction declarations |
| `src/AIRender.cpp` (new) | RLE and viewport implementation |
| `src/Monster.h` | Add `GetChar()` method |
| `src/Item.h` | Add `GetChar()` method |
| `src/Dungeon.cpp` | Remove `extern`, use GetChar(), add visibility check, simplify DumpToAILog |
| `Makefile` | Add AIRender.o to build |

---

## Testing Plan

1. Run existing unit tests
2. Smoke test with `JMORIA_AI_LOG=1`
3. Verify AI logs still parse correctly with deep-dive.py
4. Check unlit areas show as blank in logs

---

## Decisions

1. **Option A: Extract** - Create AIRender utility with RLE, viewport extraction
2. **Player view only** - Unlit/unseen tiles show as blank (prevents AI "cheating")
3. **Defer wall types** - Keep all walls as `#` for now

---

## Success Criteria

1. ✅ No `extern` declarations for ID arrays
2. ✅ Each class owns its character representation
3. ✅ Unlit/unseen tiles show as blank in logs
4. ✅ Existing AI log format preserved (RLE, JSON structure)
5. ✅ All tests pass

---

## Next Steps

1. Copy this plan to `thoughts/dungeon-ai-refactor.md` for tracking
2. Implement in order: AIRender → GetChar() methods → Dungeon.cpp refactor
3. Test incrementally after each step
