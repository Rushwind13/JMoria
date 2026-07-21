<!-- Project policy overlay. Non-comment content here is spliced into
     the architect rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Architect Role Specifics for JMoria

### Core Architectural Pattern: State Machine
JMoria uses a **hierarchical state machine** for game flow. All game modes (command input, targeting, inventory, rest, look, etc.) are separate `CStateBase` subclasses managed by `CGame::SetState()`.

**Benefits of this pattern:**
- Clean separation of concerns (each state owns its input handling and update logic)
- Easy to add new modes without touching existing states
- Testable in isolation (each state's behavior is independent)
- Prevents state explosion (vs. large switch statements in a single input handler)

**Design constraints:**
- States must not directly modify peer states' internals
- State transitions routed through `CGame::SetState()`, not direct calls
- Shared state (player, dungeon, etc.) accessed via `CGame` reference

### Module Architecture & Dependencies
```
CGame (coordinator)
  ├─ CDungeon (tile grid, generation)
  ├─ CPlayer (state, inventory, equipment)
  ├─ CAIMgr (monster controller)
  ├─ CRender (renderer abstraction)
  ├─ CDisplayText (UI layout)
  └─ CStateBase* (current state)
```

**Dependency flow:**
1. `CStateBase` subclasses query `CGame` for references to managers
2. States update player/dungeon via manager interfaces
3. Render layer called by game loop (not by states directly)
4. No bidirectional dependencies; managers don't call back into states

### Design Patterns in Use

**1. Strategy Pattern (Renderer Selection)**
- `CRender` base class with `RenderASCII` and `RenderOpenGL` implementations
- Renderer selected at runtime via `--renderer` flag
- New renderer support: create `RenderFoo.cpp`, inherit from `CRender`, implement abstracted draw methods

**2. Data-Driven Design (Monster & Item Definitions)**
- Game objects defined in text files, not compiled code
- `CDataFile` class reads and caches definitions
- Extensibility: new monster types require only `.txt` entry + new index in `Constants.h`
- Benefit: Non-programmers can balance and extend content

**3. Manager Pattern (AI, Dungeon, Display)**
- `CAIMgr` manages all active monsters and their behaviors
- `CDungeon` owns the tile map and generation logic
- `CDisplayText` manages UI regions and text rendering
- Coordinator pattern: `CGame` holds all managers, states access them via `CGame`

**4. Command Pattern (Potentially)**
- Future: Player actions (move, attack, cast) could be commands with undo/replay
- Currently: Direct state modifications; consider command pattern for save/replay features

### Scalability & Future Direction

**Current bottleneck:** DungeonMap::FillArea() is complex stepwise calculation (2017 implementation); well-documented but fragile  
**Recommendation:** Consider dungeon generation refactor if adding new room types or biomes  

**Extensibility points (low-effort, high-impact):**
- New monster types: Edit `Resources/Monsters.txt`, add index to `Constants.h`
- New item effects: Add effect type to `Resources/Items.txt`, implement handler in `Effect.cpp`
- New UI regions: Add to `CDisplayText`, route rendering in game loop
- New game states: Subclass `CStateBase`, call `CGame::SetState()` to activate

**High-effort directions (estimate 40+ hours each):**
- Wizard mode graphical editor for dungeon layout
- Procedural skill/spell system (vs. fixed attack types)
- Multi-level dungeon persistence (currently per-level)
- Network multiplayer (major architectural refactor)

### Cross-Platform Architecture Considerations
- Renderer abstraction (`CRender` interface) shields platform-specific graphics code
- Build system detects platform at make-time; Makefile conditionals set platform-specific flags
- No `#ifdef` guards in game logic; platform-specific code isolated to renderer and build system
- **Design rule:** If you need platform-specific code, it belongs in a separate implementation file (e.g., `Render_MacOS.cpp`), not scattered in headers

### Test Architecture for Design Validation
- BDD test structure validates game behaviors at a high level (vs. unit tests)
- Feature scenarios describe player actions and expected outcomes
- Step definitions tie scenarios to game state queries/modifications
- **Design implication:** Features should map cleanly to game states and manager operations; if a feature requires complex glue code, it signals a design issue

### Integration Points with Agent Framework
- **Implementer** will receive detailed coding standards and dependency rules
- **Reviewer** needs architecture overview to assess PRs for pattern consistency
- **Verifier** needs test architecture to understand what BDD tests validate
- **This document** serves as reference architecture for all downstream agents
