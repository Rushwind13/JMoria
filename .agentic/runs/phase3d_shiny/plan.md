# Plan: Phase 3d Shiny – Visual Polish & Effects

## Approach

This plan breaks three visual polish features into parallel, independent work items. Each requirement maps to a single code-change unit with minimal overlap.

**Architecture:**
- **R1 (Multicolor Beams)**: Beam rendering logic in `RangedState::DoTrajectory()` + `CRender` / `CRenderASCII` effect color dispatch
- **R2 (AoE Animation)**: Transient effect queue added to `CRender`; animation state machine in effect system
- **R3 (Ranged AI)**: New AI brain enum in `Constants.h`, logic in `CAIBrain::UpdateSeek()`, Monsters.txt data entries

All three can be implemented in parallel — no shared code changes except:
- `Constants.h`: 1 line (add `MON_AI_SEEKRANGED`)
- `Resources/Monsters.txt`: Monster type updates (non-conflicting)

## Interface Contracts

### R1: Multicolor Beams
**Input**: `CEffectDef::m_dwFlags` (e.g., `EFFECT_FLAG_FIRE`, `EFFECT_FLAG_COLD`, etc.)
**Output**: RGB color tuple or ASCII character determined at render time
**Location**: 
- Wand effect enum → color lookup in `RangedState.cpp` / `RenderASCII.cpp`
- No API change; color applied during `CRender::DrawTile()` for beam tile

**Color map** (hardcoded, not in data):
- Fire: RGB(255, 128, 0) / ASCII `*`
- Cold: RGB(100, 200, 255) / ASCII `~`
- Acid: RGB(0, 200, 0) / ASCII `#`
- Electricity: RGB(255, 255, 100) / ASCII `+`

### R2: AoE Animation
**Input**: Effect definition + target center + radius
**Output**: Expanding rings (OpenGL) or ASCII art (terminal)
**Location**:
- `CEffect` subclass `CAoEEffect` with duration/frame counter
- `CRender::m_llTransientEffects` queue (new member)
- `CRender::UpdateEffects()` per-frame step/render

**Lifecycle**: Effect queued in `Effect.cpp::ApplyEffect()` when ball/mass detected; render loop consumes & cleans up after duration.

### R3: Ranged AI
**Input**: Monster definition flags + distance to player
**Output**: Movement command (toward/away/strafe/hold)
**Location**:
- `Constants.h`: Add `MON_AI_SEEKRANGED = 0x10000000` (next available bit)
- `CAIBrain::UpdateSeek()`: Branch on brain type; if seekranged, check distance vs. optimal (5–8 tiles)
- `Resources/Monsters.txt`: Monsters that should use ranged AI get flag

**Decision Logic**:
- If `distance < 5`: Retreat (move away, perpendicular if blocked)
- If `distance > 8`: Advance (move toward)
- If `5 <= distance <= 8`: Attack (stay put, attack when in range)

## Requirement → Task Mapping

| Requirement | Task | File Contact | Parallel? |
|-------------|------|-------------|-----------|
| R1 | 01-multicolor-beams | RangedState, RenderASCII, RenderOpenGL, Effect, Constants | Yes (after Constants updated) |
| R2 | 02-aoe-animation | Render, RenderASCII, RenderOpenGL, Effect | Yes (after Constants updated) |
| R3 | 03-ranged-ai-seekranged | Constants, AIMgr, CAIBrain, Monsters.txt | Yes (after Constants updated) |
| Test | 04-test-beams-aoe-ranged | test/features/*, step_definitions/ | Parallel with impl |

**Dependency**: All tasks depend on Constants.h `MON_AI_SEEKRANGED` enum addition (5-line change). After that, all tasks run in parallel.

## ADRs (Architectural Decision Records)

### ADR1: Beam colors hardcoded, not parsed
**Context**: `Resources/Items.txt` has no `Color` field; effect type (Fire, Cold, etc.) maps to color.
**Decision**: Hardcode color lookup in `RangedState.cpp` and `RenderASCII.cpp` as a switch on `CEffectDef::m_dwFlags`.
**Rejected**: Parse from Items.txt (breaks existing data format; adds complexity for post-release tuning anyway).
**Consequence**: Beam colors cannot be changed per-item without code recompile. Acceptable for Phase 3d; data-driven color system can be added later.

### ADR2: AoE animation runs in render loop, not game logic
**Context**: Animation is purely visual; no game state mutation during ring expansion.
**Decision**: Queue transient effects in render system (`CRender::m_llTransientEffects`); render loop steps animation each frame.
**Rejected**: Add to game state / AI manager (adds complexity; couples rendering to game logic).
**Consequence**: Animation is framerate-dependent but decoupled from turn-based logic. Ring expansion speed controlled by frame delta.

### ADR3: Optimal range for MON_AI_SEEKRANGED hardcoded (5–8 tiles)
**Context**: `Resources/Monsters.txt` has no range field; cannot tune per-monster without parser change.
**Decision**: Use hardcoded constant `#define RANGED_AI_OPTIMAL_MIN 5` and `RANGED_AI_OPTIMAL_MAX 8` in Constants.h.
**Rejected**: Parse from Monsters.txt (adds column; future work).
**Consequence**: All ranged monsters use same distance threshold. Per-monster tuning deferred to Phase 3e.

### ADR4: MON_AI_SEEKRANGED uses existing pathfinding
**Context**: Monster movement already implemented in `CAIBrain::GotoDest()`; only distance heuristic changes.
**Decision**: Reuse `CAIBrain::GotoDest()` for movement; `CAIBrain::UpdateSeek()` just changes the target selection logic (toward/away based on distance).
**Rejected**: New pathfinding algorithm (unnecessary complexity).
**Consequence**: Ranged AI behavior depends entirely on existing movement code. If movement has bugs, ranged AI inherits them (low risk; movement is proven).

## Risks & Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| Beam color clashes in terminal (ASCII) | Medium | Unreadable combat output | Test with white/black terminal; use high-contrast ASCII chars (`*`, `#`, `+`, `~`) |
| AoE animation causes framerate drop | Low | Gameplay stutter | Profile render loop; cap animation frames (e.g., max 30 rings) if needed |
| Ranged AI pathfinding gets stuck | Medium | AI appears broken | Add strafing fallback in `CAIBrain::UpdateSeek()` (perpendicular movement if blocked for 3+ turns) |
| $50 budget exhausted mid-implementation | Medium | Tasks incomplete | Prioritize: beams (2 hrs) > AoE (3 hrs) > ranged AI (2 hrs); cut ranged AI if budget tight |

## BDD Test Strategy

**Feature files** (one per requirement, in `test/features/`):
- `beams_multicolor.feature` — beam color rendering (OpenGL + ASCII scenarios)
- `aoe_animation.feature` — expanding rings / mass flash (OpenGL + ASCII scenarios)
- `ranged_ai_seekranged.feature` — ranged monster movement (advance/retreat/hold scenarios)

**Step definitions** (C++ in `test/features/step_definitions/`):
- Render verification: Sample pixel color (OpenGL) or ASCII char at beam position
- Animation verification: Check ring count and expansion each frame
- Monster movement: Track position delta; verify toward/away movement

**Setup context**: Reuse existing `TestContext`; spawn known monsters at measured distances.

## Build & Verification Plan

1. **Baseline**: `make ascii && make opengl && make test` — verify clean build
2. **Per-task**: Edit feature files first (test stubs); implement code; `make test` to verify
3. **Final**: `make clean && make && make test` on both macOS (required) and simulated Linux (Makefile cross-check)
4. **Code style**: `clang-format -i <modified_files>` before staging

## Acceptance Gate Checklist

- [ ] All three feature files + step definitions in place
- [ ] Beam colors render in both OpenGL and ASCII modes
- [ ] AoE rings expand smoothly without framerate drop
- [ ] Ranged monsters advance/retreat correctly per distance
- [ ] `clang-format` passes on all modified `.cpp` / `.h` files
- [ ] `make ascii`, `make opengl`, `make` all build cleanly
- [ ] `make test` passes all new BDD scenarios
- [ ] No pre-existing tests broken

## Known Architectural Constraints

- **Dungeon generation**: Untouched (out of scope)
- **Combat system**: No changes to damage calculation or turn timing
- **Monsters/items**: Only existing types used for testing; no new types added
- **Resource files**: Only data entries added (Monsters.txt beam/AI flags), no format changes
- **Dependencies**: No new packages added; use SDL2, OpenGL, ncurses, boost (already in Makefile)

## Session Budget Impact

- **Total**: $50 USD budgeted
- **Architecture review** (this phase): ~2 USD tokens
- **Implementation** (3 tasks): ~35–40 USD (depends on iteration depth)
- **Buffer**: ~8–13 USD for debug/review cycles

**Burn rate check**: If any single task exceeds 20 USD, pause and escalate for guidance.
