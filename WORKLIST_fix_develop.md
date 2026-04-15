# WORKLIST: Fix Develop Branch (Code Smell Rework)

**Status**: Active  
**Based on**: PR#251 code review (e602823 → develop diff)  
**Strategy**: Create fixes branch from develop, apply targeted fixes on top of committed code  
**Total Items**: 35 fixes across 14 files  
**Approach**: Separate fix commits by logical area (refactoring, includes, architecture, etc.)

---

## src/main.cpp (6 items)

**CRITICAL: Eliminate standard library includes**

- [ ] Line 4: Remove/reduce cstdlib, cstring, ctime includes — goal: use Util functions instead
- [ ] Line 30: Replace `strcmp` with `Util::jstrcmp`
- [ ] Line 32: Replace `strcmp` with `Util::jstrcmp`
- [ ] Line 40: Replace `printf`/similar with `JLog`
- [ ] Line 45: Replace `printf`/similar with `JLog`
- [ ] Line 49: Replace `printf`/similar with `JLog`
- [ ] Line 57: **Create Util function** for this operation (open issue #XXX)

---

## src/JColor.h (1 item)

- [ ] Line 4: Remove unused includes — verify they're not needed before deletion

---

## src/JKeys.h (1 item)

**CRITICAL: Eliminate cstdint dependency**

- [ ] Line 10: Avoid cstdint include — long-term project philosophy. Investigate if we can use custom types instead

---

## src/JTypes.h (1 item)

**CRITICAL: Eliminate cstdint dependency**

- [ ] Line 9: Can this be done without cstdint? Potentially use `unsigned char` for what's currently typed as uint8?

---

## src/Util.cpp (1 item)

- [ ] Line 14: Remove or make conditional — either remove code block entirely or add `#ifndef` flag if still required

---

## src/TargetState.cpp (4 items)

**ARCHITECTURE: Refactor with JLinkList instance IDs**

- [ ] Line 111: Move "first-turn consequence" logic into `GetVisibleMonsters()` method, not in TargetState
- [ ] Line 123: **Refactor** with new `GetDungeon()->GetMonster(instanceId)` method — requires JLinkList `m_dwInstanceId`
- [ ] Line 129: **Refactor** `Add(pMon, ...m_dwIndex, instance_id)` to avoid uint32→CMonster* casting chatter.  
  - Targets is LinkList<Monsters>, should store dwIndex (monster type?) + dwInstanceId (monster serial#)
- [ ] Line 210: **Update comment/code**: targets will have pmon index and instanceid (after line 129 refactor)

---

## src/ClockStepState.cpp (2 items)

**REFACTORING: Remove duplicate code**

- [ ] Line 12: Avoid including Dungeon and Player headers — this is duplicate code from elsewhere. Consolidate includes/functionality.
- [ ] Line 121: **Deduplicate** — this function duplicates `OnChangeLevel()`. Merge or create shared utility.

---

## src/DungeonConstants.h (1 item)

**BUG**: Incorrect constant value

- [ ] Line 45: **FIX**: Set to 100, not 1000 (5000' max depth assumes 50' per dungeon level)

---

## src/DungeonMap.h (4 items)

**ARCHITECTURE: Separate test functions from gameplay code**

- [ ] Line 127: `DungeonGenDiagnostics` — make this a compile-time flag `#ifdef DUNGEON_GEN_DIAGNOSTICS` (optional debug feature)
- [ ] Line 207: **Make compile-time flag** — expose/hide based on build mode
- [ ] Line 223: **Make compile-time flag** — expose/hide based on build mode
- [ ] Line 226: **Move to test code**: `import`, `Compare` are test functions, not gameplay. Keep `export`.

**Rationale**: We shouldn't ship gameplay-critical systems with test harnesses embedded.

---

## src/DungeonMap.cpp (4 items)

**CRITICAL: Remove external dependencies from core gameplay**

- [ ] Line 4: Remove `#include <ctime>` and similar — **Why is ctime needed?** Find alternate approach using Util functions.

**ARCHITECTURE: Move test functions out of gameplay**

- [ ] Line 1359: Move to test code, not in gameplay
- [ ] Line 1401: Move to test code, not in gameplay
- [ ] Line 1520: Move to test code, not in gameplay

---

## src/Dungeon.cpp (1 item)

**REFACTORING: Extract level population logic**

- [ ] Line 169: Call `PopulateLevel(depth)` instead of inline code — consolidates item/monster/scenery placement logic

---

## src/Dungeon.h (1 item)

**NAMING**: Function rename for clarity

- [ ] Line 178: Rename function from current name to `GetMonster()` (or verify actual name) — clarifies it retrieves a single monster by ID

---

## src/Game.cpp (8 items)

**CRITICAL: Remove mode-specific code from core Game logic**

- [ ] Line 94: **FIX**: `#if/#elif` block is malformed — has nested else-if's that duplicate outer elif structure. Simplify conditional logic.

**ARCHITECTURE: Move ASCII-specific code to ASCIIRender**

- [ ] Line 594: This code belongs in `ASCIIRender`, not in `Game`. Extract and move.

**ARCHITECTURE: Support both renderers equally**

- [ ] Line 632: **BUG**: This feature only works with one renderer — update to work with both (OpenGL + ASCII)
- [ ] Line 643: **Feature**: Allow this visualization feature in both renderers, not just one
- [ ] Line 793: **ARCHITECTURE**: Why is key mapping happening in Game? Where does OpenGL renderer handle this?  
  - **ACTION**: Standardize key handling — move to input abstraction layer or ensure parity

**ARCHITECTURE: Move state-specific logic to state handlers**

- [ ] Line 871: Move this code into `CmdState` or appropriate state handler
- [ ] Line 890: This logic belongs in a state handler, not Game class — extract method

---

## Summary by Category

### Eliminatng External Dependencies (7 items)
- main.cpp: Replace stdlib with Util functions (lines 4, 30, 32, 40, 45, 49, 57)
- JTypes.h: Eliminate cstdint (line 9)
- JKeys.h: Eliminate cstdint (line 10)
- DungeonMap.cpp: Remove ctime (line 4)

### Architecture/Separation of Concerns (9 items)
- Game.cpp: Move ASCII code to ASCIIRender (line 594)
- Game.cpp: Move key mapping logic (line 793)
- Game.cpp: Move state logic to handlers (lines 871, 890)
- DungeonMap.h: Move test functions (lines 207, 223, 226)
- DungeonMap.cpp: Move test functions (lines 1359, 1401, 1520)

### Refactoring/Deduplication (6 items)
- ClockStepState.cpp: Remove duplicate includes (line 12)
- ClockStepState.cpp: Deduplicate OnChangeLevel (line 121)
- Dungeon.cpp: Extract PopulateLevel (line 169)
- TargetState.cpp: Refactor with instance IDs (lines 123, 129)
- Game.cpp: Fix conditional logic (line 94)

### Bug Fixes (2 items)
- DungeonConstants.h: Wrong constant (line 45)
- Game.cpp: Feature parity (line 632)

### Code Cleanup (3 items)
- JColor.h: Remove unused includes (line 4)
- Util.cpp: Remove conditional code (line 14)
- DungeonMap.h: Compile-time flags (line 127)

### Naming/Clarity (1 item)
- Dungeon.h: Rename function (line 178)

### Data Structure Improvements (4 items)
- TargetState.cpp: Leverage JLinkList.m_dwInstanceId (lines 111, 123, 129, 210)
- Dungeon.h: Better naming (line 178)

---

## Suggested Commit Order

1. **Phase 1 - Safety/Bugs** (fixes that don't break anything)
   - DungeonConstants.h: Fix constant value
   - Game.cpp: Fix malformed conditional (line 94)
   - JColor.h: Remove unused includes

2. **Phase 2 - Utility Functions** (establish support functions)
   - main.cpp: Create/use Util::jstrcmp, Util::JLog wrappers
   - Util.cpp: Clean up as needed
   - Create missing Util functions (line 57)

3. **Phase 3 - Type/Header Cleanup**
   - JTypes.h: Investigation/solution for cstdint
   - JKeys.h: Investigation/solution for cstdint
   - DungeonMap.cpp: Remove ctime, find Util alternative

4. **Phase 4 - Architecture/Separation**
   - DungeonMap.h/cpp: Compile flags, move test functions
   - Game.cpp: Move ASCII code, move state logic, standardize key mapping
   - ClockStepState.cpp: Clean up includes, deduplicate

5. **Phase 5 - Data Structure Improvements** (requires careful testing)
   - TargetState.cpp: Refactor with JLinkList instance IDs
   - Dungeon.h: Rename function
   - Dungeon.cpp: Extract PopulateLevel

---

## Testing Strategy

After each commit:
- Compile with ASCII mode: `make clean && make ascii`
- Run test suite: `cd test && ./runtests.sh`
- Manual bot test: `bash /tmp/run_bot_test.sh`

After Phase 5, full integration test:
- Run bot crawl for 50+ iterations: `./scripts/crawl.sh` (sample run)

---

## Notes

- PR#251 is read-only for reference — the actual fixes go in a new branch from develop
- Each fix is localized to avoid merge conflicts
- Compile-time flags keep debug/test code accessible for developers without bloating gameplay
- Instance ID refactoring (TargetState) is foundational for future entity lookup improvements
