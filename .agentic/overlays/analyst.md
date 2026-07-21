<!-- Project policy overlay. Non-comment content here is spliced into
     the analyst rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Analyst Role Specifics for JMoria

### Problem Domain & Context
**JMoria** is a from-scratch C++ roguelike engine (homage to IMoria). It uses:
- **State machine** for game flow (command input, targeting, inventory, etc.)
- **Data-driven design** for monsters and items (defined in text files, not code)
- **Multi-renderer support** (ASCII with ncurses, 3D with SDL2/OpenGL)
- **Cross-platform** (macOS primary, Linux and Raspberry Pi supported)

### Analysis Focus Areas

**1. Requirements & Use Cases**
- [ ] Understand what the feature request or bug report is asking for
- [ ] Identify which game systems are affected (player, dungeon, AI, rendering, UI)
- [ ] Map to existing game states (CmdState, TargetState, UseState, etc.)
- [ ] Clarify acceptance criteria: playable behavior, not just code changes

**2. Scope & Risk Assessment**
- **Low-risk:** New monster/item in Resources/ (data-only, no code)
- **Medium-risk:** New game state or UI region (new class, touches CGame)
- **High-risk:** Dungeon generation changes (complex logic, affects gameplay)
- **High-risk:** Renderer changes (must work in ASCII and OpenGL)
- **High-risk:** Makefile/build system (affects all platforms)

**3. Feasibility Analysis**
Ask yourself:
- Can this be implemented within the state machine pattern?
- Is new game-balancing data needed? (belongs in Resources/, not code)
- Does this require changes to multiple platforms?
- Can this be tested with BDD (feature-driven tests)?
- Are there existing examples in codebase to follow?

**4. Dependencies & Interactions**
- **Data:** Does this feature depend on monster/item definitions? Check if Resources/ entries exist.
- **Rendering:** ASCII vs. OpenGL considerations? Needs abstraction via CRender interface.
- **AI:** Does this interact with CAIMgr? Understand monster behavior and MoveType.
- **UI:** Does this add/change display elements? CDisplayText regions and layout.
- **Player state:** Does this modify inventory, equipment, intrinsics? CPlayer class.

### Analysis Deliverables

**For feature requests:**
1. **Scope statement:** What the feature does, what systems it touches
2. **Use case scenarios:** Player actions and expected outcomes (as BDD scenarios)
3. **Data requirements:** New monster types, items, or resource file entries needed
4. **Implementation sketch:** Which classes to modify/create; estimated effort (hours)
5. **Risk assessment:** Platform impact, regression risk, testing complexity
6. **Effort estimate:** Optimistic / realistic / pessimistic (hours)

**For bug reports:**
1. **Reproduction steps:** Exact sequence to trigger the bug
2. **Expected vs. actual:** What should happen; what actually happens
3. **Affected components:** Game state, UI region, platform-specific?
4. **Severity:** Game-breaking, gameplay impact, cosmetic
5. **Root cause hypothesis:** Where the bug likely lives (state class, manager, render layer)
6. **Fix strategy:** Possible solutions and trade-offs

### Handoff to Implementer

When passing to implementer, provide:
- [ ] Clear acceptance criteria (feature-driven, not implementation-driven)
- [ ] BDD scenarios (`.feature` file template if new test needed)
- [ ] Data/balance requirements (if feature is tunable, what's the data file entry?)
- [ ] Architecture guidance (which classes to touch, which patterns to follow)
- [ ] Cross-platform notes (any render-specific or platform-specific logic?)
- [ ] Test strategy (how to validate the feature works)

### Red Flags (Escalate to Architect)

- [ ] Feature request violates data-driven principle (wants hardcoded game logic)
- [ ] Requires major refactoring (e.g., rewriting DungeonMap::FillArea)
- [ ] Changes fundamental architecture (e.g., multi-level dungeon persistence)
- [ ] Affects all three platforms but analysis is macOS-only
- [ ] Feature scope unclear or ambiguous (requirements need refinement)

### Reference Materials

- `.github/copilot-instructions.md` — Architecture and patterns
- `_JMoria Developer's Guide.md` — Monster/item addition procedures
- `WORKLIST.txt`, `WORKLIST_jmoria_core_roadmap.md` — Feature backlog
- `test/features/` — Existing BDD scenarios (for tone and structure)
