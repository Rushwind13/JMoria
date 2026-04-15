# WORKLIST: Fix Develop Branch (Code Smell Rework)

**Status**: Phase 5 complete + post-merge refactoring done  
**Based on**: PR#251 code review (e602823 → develop diff)  
**Strategy**: Create fixes branch from develop, apply targeted fixes on top of committed code  
**Total Items**: 35 fixes across 14 files  
**Approach**: Separate fix commits by logical area (refactoring, includes, architecture, etc.)

---

## Completed Work

### Phase 1 – Safety/Bugs
- [x] DungeonConstants.h: DUNG_MAXDEPTH fixed to 100
- [x] Game.cpp: Malformed `#if/#elif` triple-branch fixed

### Phase 2 – Utility Functions
- [x] main.cpp: Replaced `strcmp` with `Util::jstrcmp`, `printf` with `JLog`, `time(NULL)` with `SeedRandomFromClock()`
- [x] Util.cpp: Removed redundant conditional `#include <sys/time.h>`

### Phase 3 – Type/Header Cleanup
- [x] JTypes.h: Plain C++ types, added `Sint32`
- [x] JKeys.h: Uses `JTypes.h` instead of `<cstdint>`
- [x] DungeonMap.cpp: Removed dead `#include <ctime>`

### Phase 4 – Architecture/Separation
- [x] ClockStepState.cpp: Removed 3 redundant includes
- [x] Game.cpp: Unified `Draw()` panel logic for both renderers
- [x] Game.cpp/CmdState: Panel toggles (i/e/C) moved to `CmdState::IsToggleCommand()` with `JHANDLED_NOTURN`

### Phase 5 – Data Structure Improvements
- [x] Player.cpp: `GetVisibleMonsters()` lazy-init
- [x] Dungeon.cpp: `PopulateLevel(depth)` extracted
- [x] Dungeon.h: `FindMonsterByInstanceId` → `GetMonster()` → removed entirely

### Post-Merge Refactoring (after PR#250 merge)
- [x] JLinkList ownership model: `m_bOwnsData` flag, constructor param `JLinkList<T>(false)` for non-owning
- [x] `m_llVisibleMonsters` changed from `JLinkList<uint32>` to `JLinkList<CMonster>(false)` — direct pointers, no round-trip
- [x] `m_llTargets` eliminated entirely — TargetState uses `GetVisibleMonsters()` directly
- [x] Dangling pointer fix: `ClearVisibleMonsters()` called in `RemoveMonster()`
- [x] `GetMonster()`/`FindMonsterByInstanceId()` removed — zero callers remain
- [x] `Dungeon::Drop()` passes instanceId to `Add()`

---

## Remaining Work

### Deferred (too invasive for this PR)

- [ ] **ClockStepState line 121**: Duplicates `OnChangeLevel()` — merge or extract shared utility
- [ ] **Game.cpp line 594**: ASCII-specific code belongs in `RenderASCII`, not `Game`
- [ ] **Game.cpp line 793**: Key mapping in Game — standardize input abstraction
- [ ] **DungeonMap.h/cpp**: Test functions (`import`, `Compare`) embedded in gameplay code — move behind `#ifdef` or to test code
  - Lines 127, 207, 223, 226 in .h; lines 1359, 1401, 1520 in .cpp
- [ ] **JColor.h**: `<cstdlib>`/`<cstring>` needed by `strtok`/`atoi` in `SetColor(char*)` — needs Util wrappers first

### New: JLinkList Usage Improvements

**Definition lookups via linked list scan** (should be array-indexed):
- [ ] **Dungeon.cpp line 483**: `m_llMonsterDefs->GetLink(which_monster)` — O(n) scan by `m_dwIndex`. Definitions are indexed by `MON_IDX_*` enum; should be a flat array for O(1) lookup
- [ ] **Dungeon.cpp line 525**: `m_llItemDefs->GetLink(which_item)` — same issue with `ITEM_IDX_*` enum

**Stackable item lookup** (scan then re-scan):
- [ ] **Player.cpp line 301**: `GetLink(m_id->m_dwIndex)` + manual `next` iteration for stackable items. Combines O(n) index scan + O(m) name comparison. Could be simplified with a type-keyed lookup.
