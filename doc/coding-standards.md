# JMoria Coding Standards

## Overview

These standards were established based on code review feedback to maintain consistency and prevent common issues across the codebase.

---

## 1. Use Existing Infrastructure

### Logging: Use JLog

Use the existing JLog system for all logging needs. Do not create separate logging systems.

**Good:**
```cpp
JLog(LOG_LEVEL_INFO, true, "Player moved to (%d,%d)\n", x, y);
JLog(LOG_LEVEL_AI, false, "Combat:Player->%s Hit:true Dmg:%d\n", name, damage);
```

**Bad:**
```cpp
// Don't create separate logging systems
printf("Player moved\n");
fprintf(customLogFile, "...");
```

**Log Levels:**
| Level | Purpose |
|-------|---------|
| `LOG_LEVEL_NOISE` | Verbose debugging |
| `LOG_LEVEL_DEBUG` | Debugging info |
| `LOG_LEVEL_INFO` | General info |
| `LOG_LEVEL_WARN` | Warnings |
| `LOG_LEVEL_ERROR` | Errors |
| `LOG_LEVEL_AI` | AI observability (to file) |

### Geometry: Use JVector and JRect

Use existing vector and rectangle classes for positions and bounds.

**Good:**
```cpp
JVector playerPos = player->m_vPos;
JIVector gridPos((int)pos.x, (int)pos.y);

JRect viewBounds(minX, minY, maxX - minX, maxY - minY);
if (viewBounds.Contains(targetPos)) { ... }
```

**Bad:**
```cpp
// Don't use manual coordinate pairs
int px = (int)player->m_vPos.x;
int py = (int)player->m_vPos.y;

// Don't use manual bounds checking
int viewMinX = px - RADIUS;
int viewMaxX = px + RADIUS;
if (x >= viewMinX && x <= viewMaxX) { ... }
```

### Bounds Checking: Use Existing Methods

**Good:**
```cpp
if (pos.IsInWorld()) { ... }
if (rect.Contains(point)) { ... }
```

**Bad:**
```cpp
// Don't write manual bounds checks
if (x >= 0 && x < DUNG_WIDTH && y >= 0 && y < DUNG_HEIGHT) { ... }
```

---

## 2. Utility Functions

### Minimal External Dependencies

JMoria avoids external library dependencies. Do not use standard library headers like `<stdio.h>`, `<stdlib.h>`, `<cmath>`, or `<string.h>` directly. Instead, use the project's own implementations.

### Prefer Util.h Functions

Check `src/Util.h` for existing utility functions before writing new ones.

**Available utilities:**
- `Util::jstrcpy()`, `Util::jstrcmp()` - String operations (use instead of `<string.h>`)
- `Util::Nearby()` - Check if two positions are adjacent
- `Util::WithinRadius()` - Check distance between positions
- `Util::GetTickCount()` - Platform-independent time

### Prefer Class Methods

Use existing class methods instead of reimplementing functionality.

**Good:**
```cpp
JVector pos = monster->GetPos();
const char* name = item->GetName();
vec.printvec("label");  // Debug output: void printvec( const char *label )
```

**Bad:**
```cpp
// Don't access members directly when getters exist
JVector pos = monster->m_pBrain->m_vPos;

// Don't manually format vectors
sprintf(buf, "(%d,%d)", (int)pos.x, (int)pos.y);
```

---

## 3. Code Organization

### Separate Logic from Rendering

**Game logic files** (`Dungeon.cpp`, `Monster.cpp`, `Player.cpp`):
- Handle game state, rules, and mechanics
- Should not know about graphical representation

**Render files** (`Render.cpp`, `Tileset.cpp`):
- Handle display and visual representation
- Map encoding, RLE compression, ASCII conversion

**Example violation:**
```cpp
// Bad: RLE encoding in Dungeon.cpp
// This belongs in render/display code
static int RLEEncodeRow(const char* row, ...) { ... }
```

### Accessors Over Externs

Use accessor methods instead of extern declarations to access other module's data.

**Good:**
```cpp
// Use existing accessors
int monsterChar = monster->GetDisplayChar();
```

**Bad:**
```cpp
// Don't extern arrays from other modules
extern unsigned char MonIDs[];
extern unsigned char ItemIDs[];
char c = MonIDs[monster->m_md->m_dwIndex];
```

---

## 4. Performance Guidelines

### Avoid Per-Frame Allocations

Don't allocate large buffers on the stack every frame.

**Bad:**
```cpp
void Update() {
    char buffer[20000];  // Large stack allocation every frame
    char map[21][22];    // Every frame
    // ...
}
```

**Better:**
- Use member variables for persistent buffers
- Pre-allocate once during initialization
- Keep frame-critical paths simple

### Pre-calculate When Possible

Cache calculated values instead of recomputing each access.

**Good:**
```cpp
class CAIBrain {
    const char* m_szState;  // Updated when state changes

    void SetState(eBrainState state) {
        m_eBrainState = state;
        m_szState = StateToString(state);  // Cache the string
    }
};
```

**Bad:**
```cpp
// Don't calculate strings every time they're needed
const char* GetStateString() {
    switch(m_eBrainState) {  // This runs every call
        case BRAINSTATE_SEEK: return "seek";
        // ...
    }
}
```

---

## 5. Output Formats

### Prefer Plain Text Over JSON

For logging and debugging output, use simple plain text instead of manual JSON building.

**Good:**
```cpp
JLog(LOG_LEVEL_AI, false, "Monster:%s:(%d,%d) HP:%d\n",
     name, (int)pos.x, (int)pos.y, (int)hp);
```

**Bad:**
```cpp
// Don't manually build JSON strings
sprintf(buffer, "{\"name\":\"%s\",\"x\":%d,\"y\":%d,\"hp\":%d}",
        name, (int)pos.x, (int)pos.y, (int)hp);
```

**Why plain text?**
- No escape character issues
- Human readable for debugging
- Easy to grep and parse with regex
- No JSON library dependency

---

## 6. Don't Duplicate Infrastructure

Before adding new code, check if similar functionality exists:

| Need | Use |
|------|-----|
| Logging | `JLog()` in `JLog.h` |
| Positions | `JVector`, `JIVector` in `JVector.h` |
| Bounds | `JRect` in `JRect.h` |
| String ops | `Util::jstrcpy()`, etc. in `Util.h` |
| Time | `Util::GetTickCount()` in `Util.h` |
| Turn count | `g_pGame->GetITime()` |

---

## 7. Naming Conventions

### Member Variables
- Prefix with `m_` (e.g., `m_vPos`, `m_fHP`)
- Type prefix: `dw` (DWORD/int), `f` (float), `sz` (string), `p` (pointer), `v` (vector)

### Functions
- PascalCase (e.g., `GetPosition()`, `TakeDamage()`)

### Constants
- ALL_CAPS with underscores (e.g., `DUNG_IDX_FLOOR`, `LOG_LEVEL_AI`)

### Files
- PascalCase matching class name (e.g., `Player.cpp` for `CPlayer`)

---

## 8. Class Structure Patterns

### Class Naming

All classes use the `C` prefix:
- `CGame`, `CPlayer`, `CMonster`, `CDungeon`
- `CStateBase`, `CCmdState`, `CRestState`
- `CItem`, `CItemDef`, `CEffect`, `CAttack`

### Header File Organization

```cpp
#ifndef __CLASSNAME_H__
#define __CLASSNAME_H__

// Forward declarations
class COtherClass;

// Includes
#include "Dependency.h"

// Class definition
class CClassName
{
public:
    // Public interface
protected:
    // Virtual methods for subclasses
private:
    // Implementation details
};

#endif // __CLASSNAME_H__
```

### Member Variable Prefixes (Hungarian Notation)

| Prefix | Type | Example |
|--------|------|---------|
| `m_p` | pointer | `m_pDungeon`, `m_pPlayer`, `m_pBrain` |
| `m_dw` | DWORD/int/uint32 | `m_dwFlags`, `m_dwIndex`, `m_dwType` |
| `m_f` | float | `m_fGameTime`, `m_fHP`, `m_fSpeed` |
| `m_sz` | string (char*) | `m_szName`, `m_szDamage` |
| `m_b` | bool | `m_bIsRested`, `m_bDraw`, `m_bHasSpawned` |
| `m_v` | vector | `m_vPos`, `m_vVel`, `m_vDir` |
| `m_ll` | linked list | `m_llMonsters`, `m_llItems`, `m_llAttacks` |
| `m_e` | enum | `m_eCurState`, `m_eBrainState` |

### Constructor Pattern

Use initialization lists, heap allocations in body:

```cpp
CMonster::CMonster()
    : m_fHP( 0.0f ),
      m_fCurHP( 0.0f ),
      m_md( NULL ),
      m_pBrain( NULL )
{
    m_pBrain = new CAIBrain;
}
```

### Destructor Pattern

Always check, delete, and NULL:

```cpp
~CMonster()
{
    if( m_pBrain )
    {
        delete m_pBrain;
        m_pBrain = NULL;
    }
}
```

---

## 9. Utility Classes Quick Reference

### JVector / JIVector (`JVector.h`)

| Method | Purpose |
|--------|---------|
| `IsInWorld()` | Check bounds `[0,DUNG_WIDTH) x [0,DUNG_HEIGHT)` |
| `IsWithinWorld()` | Safe interior with 1-tile buffer |
| `Length()` | Squared length (avoids sqrt) |
| `Norm()` | Sign-based normalization |
| `printvec("label")` | Debug output to log |
| `VEC_EXPAND(v)` | Macro: expands to `v.x, v.y` for printf |

### JRect (`JRect.h`)

| Method | Purpose |
|--------|---------|
| `Contains(pos)` | Check if point is inside rectangle |
| `IsInWorld()` | Check if rectangle fits in dungeon |
| `IsInRect(other)` | Check if this rect is inside another |
| `Edges()` | Return rectangle expanded by 1 tile |
| `Width()`, `Height()` | Get dimensions |
| `RECT_EXPAND(r)` | Macro: expands to `l, t, r, b` for printf |

### JLinkList<T> (`JLinkList.h`)

| Method | Purpose |
|--------|---------|
| `Add(pData, index, ascending)` | Add element (sorted if index provided) |
| `GetHead()` | Get first element (NULL if empty) |
| `GetNext(pLink)` | Get next element (NULL at end) |
| `GetNthLink(n)` | Get element by index (0-based) |
| `Remove(pLink, bDelete)` | Remove element, optionally delete data |
| `Terminate()` | Clear all elements (**must call before delete**) |
| `length()` | Get element count |

### Util Namespace (`Util.h`)

| Function | Purpose |
|----------|---------|
| `GetRandom(lo, hi)` | Random int/float in range `[lo, hi)` |
| `Roll(dice, sides)` | Dice roll sum (e.g., `Roll(2, 6)` for 2d6) |
| `Roll("3d6")` | Parse dice notation string |
| `GetRandomPoint(rect)` | Random point within rectangle |
| `Nearby(pos, radius)` | Get JRect around position |
| `Taxicab(a, b, dist)` | Manhattan distance check |
| `WithinRadius(a, b, dist)` | Euclidean distance check (squared) |
| `Bresenham(src, dst, dist, walkFn)` | Line of sight with walkability check |
| `jstrcpy(dst, src)` | String copy (no bounds check!) |
| `jstrcmp(s1, s2)` | String compare |
| `jstrlen(s)` | String length |

---

## 10. Coding Patterns

### Error Handling: Return Codes (No Exceptions)

```cpp
JResult result = DoSomething();
if( result != JSUCCESS )
{
    return result;  // Fail fast
}
```

**Return codes** (from `JMDefs.h`):
- `JSUCCESS = 0` - Success
- `JQUITREQUEST = 1` - Quit requested
- `JRESETSTATE = 2` - Reset state
- `JCOMPLETESTATE = 3` - State complete
- `JBOGUSKEY = -1` - Invalid/bogus

### Memory Management: Manual (No Smart Pointers)

```cpp
// Allocation
m_pThing = new CThing;

// Cleanup pattern: check, delete, NULL
if( m_pThing )
{
    delete m_pThing;
    m_pThing = NULL;
}

// Linked lists: Terminate before delete
m_llItems->Terminate();
delete m_llItems;
m_llItems = NULL;
```

### State Machine Pattern

Inherit from `CStateBase` and override virtual methods:

```cpp
class CMyState : public CStateBase
{
protected:
    virtual int OnHandleKey( SDL_Keysym *keysym );
    virtual void OnUpdate( float fCurTime );
    virtual void ResetToState( int newstate );
};
```

Base class `HandleKey()` handles common keys, then calls `OnHandleKey()`.

### Two-Phase Initialization

Constructors set safe defaults; `Init()` does real setup:

```cpp
// Phase 1: Constructor sets safe defaults
CMonster* mon = new CMonster;

// Phase 2: Init() does real initialization
mon->Init( monsterDef );
mon->SpawnAt( position );
```

### Linked List Iteration

```cpp
CLink<CItem>* pLink = m_llItems->GetHead();
while( pLink != NULL )
{
    CItem* item = pLink->m_lpData;
    // ... process item ...
    pLink = m_llItems->GetNext( pLink );
}
```

### Global Access

Use `g_pGame` to access game systems:

```cpp
g_pGame->GetPlayer()
g_pGame->GetDungeon()
g_pGame->GetAIMgr()
g_pGame->GetMsgs()->Printf("...")
```

---

## 11. Anti-Patterns to Avoid

| Don't | Do Instead |
|-------|------------|
| Use C++ exceptions | Return `JResult` codes |
| Use smart pointers | Raw `new`/`delete` with NULL checks |
| Use STL containers | Use `JLinkList<T>` |
| Leave dangling pointers | Set to `NULL` after delete |
| Skip `Terminate()` on lists | Always call before deleting list |
| Inline large functions | Only inline trivial one-liners |
| Use `strcpy`/`strcat` | Use `Util::jstrcpy`/`jstrcat` |
| Access members directly | Use getter methods when available |
| Create separate log systems | Use `JLog()` with appropriate level |
| Use manual bounds checks | Use `IsInWorld()`, `Contains()` |

---

## 12. Testing Standards

### Framework

JMoria uses **Cucumber-cpp** (BDD) with **Google Test** for assertions.

### Directory Structure

```
test/
├── features/                    # BDD scenarios (.feature files)
│   ├── brains.feature          # AI Brain tests
│   ├── monsters.feature        # Monster tests
│   ├── game.feature            # Integration tests
│   ├── dungeonmap.feature      # Dungeon generation
│   └── step_definitions/       # C++ step implementations
│       ├── AllSteps.cpp        # Aggregates all steps
│       ├── BrainSteps.cpp      # Brain test steps
│       ├── TestContext.hpp     # Shared test state
│       └── ...
├── runtests.sh                 # Test runner script
└── Makefile                    # Test build config
```

### Running Tests

```bash
cd test && ./runtests.sh              # Run all tests
cd test && ./runtests.sh brains       # Run specific feature
cd test && ./runtests.sh --build      # Rebuild before running
```

### Writing a Feature Test

**1. Create scenario in `.feature` file:**
```gherkin
Scenario: Can set brain state
    Given I have a brain
    When I set the brain state to 2
    Then GetState returns 2
```

**2. Implement steps in `*Steps.cpp`:**
```cpp
GIVEN( "^I have a brain$" )
{
    ScenarioScope<TestCtx> context;
    context->brain = new CAIBrain;
}

WHEN( "^I set the brain state to ([0-9]+)$" )
{
    REGEX_PARAM( int, desired );
    ScenarioScope<TestCtx> context;
    context->brain->SetState( (eBrainState)desired );
}

THEN( "^GetState returns ([0-9]+)$" )
{
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( expected, context->brain->GetState() );
}
```

### Test Context

Tests share state via `TestContext.hpp`:
- `g_pGame` - Global pointer to the Game, shared by all tests
- `context->brain` - CAIBrain for AI tests
- `context->monster` - CMonster for monster tests
- `context->result_bool` - Store method return values

### TDD Workflow (Red/Green/Refactor)

1. **Red**: Write failing test first
2. **Green**: Write minimal code to pass
3. **Refactor**: Clean up while tests stay green

### When to Write Tests

| Situation | Action |
|-----------|--------|
| Adding new method | Write test first |
| Adding member variable with behavior | Test the behavior |
| Fixing a bug | Write test that reproduces bug first |
| Modifying existing behavior | Ensure existing tests pass, add new if needed |

### Skipping Tests

Use `@skip` tag for temporarily disabled tests:
```gherkin
@skip
Scenario: AI state changes work
    ...
```

Run `./runtests.sh` to see which tests are skipped.

---

## Summary Checklist

Before submitting code, verify:

- [ ] Classes use `C` prefix naming convention
- [ ] Member variables use Hungarian notation prefixes
- [ ] Using `JLog` for all logging needs
- [ ] Using `JVector`/`JRect` for geometry
- [ ] Using `JLinkList<T>` for collections (not STL)
- [ ] Using existing utility functions from `Util.h`
- [ ] Error handling via `JResult` return codes (no exceptions)
- [ ] Memory cleanup: check NULL, delete, set NULL
- [ ] Linked lists: call `Terminate()` before delete
- [ ] Game logic separated from rendering
- [ ] No large per-frame stack allocations
- [ ] Calculated values cached where appropriate
- [ ] Plain text format for logs (no manual JSON)
- [ ] No duplicate infrastructure
- [ ] Tests written for new functionality (TDD preferred)
- [ ] Existing tests still pass
