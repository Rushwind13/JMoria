# Spec: Phase 3d Shiny – Visual Polish & Effects

## Summary
Complete three critical visual polish enhancements to finalize Phase 3d:
1. **Multicolor beam effects for wands** (#289)
2. **AoE effect animation** (#312)  
3. **MON_AI_SEEKRANGED** – ranged monster AI behavior (#314)

All prior Phase 3d work (12 issues) is complete and merged; these three enhancements are blockers for phase release. Budget: $50 USD.

---

## Context

### Project Facts (Observed)
- **Codebase**: C++ roguelike (homage to IMoria) with state-machine architecture
- **Rendering**: Dual-mode (ASCII ncurses + OpenGL SDL2); effects must render in both
- **Wand system**: Located in `src/Wand.cpp`; wands fire beams with hardcoded visual representation
- **Monsters**: Data-driven from `Resources/Monsters.txt`; AI controlled by `CAIMgr` in `src/AIManager.cpp`
- **Testing**: Cucumber-CPP BDD framework in `test/features/`; step definitions in C++
- **Constants**: Game constants in `src/Constants.h`; monster AI flags include `MON_AI_*` enum

### Brief Recap
- Phase 3c is merged; Phase 3d's 12 core issues complete
- Three polish features remain: beam effects, AoE animation, ranged AI
- These finalize the visual/gameplay polish for Phase 3d release
- All work must pass BDD tests or have documented acceptance criteria
- Hard $50 budget cap

---

## Requirements

### R1: Multicolor Beam Effects for Wands (#289)
Wands shall emit color-coded beams matching their effect type to improve combat legibility.

**Acceptance Criteria:**
- [ ] Fire wands emit red/orange gradient beam in OpenGL mode
- [ ] Cold wands emit blue/white gradient beam in OpenGL mode
- [ ] Acid wands emit green beam in OpenGL mode
- [ ] Electric wands emit yellow/white beam in OpenGL mode
- [ ] ASCII mode renders beams with distinct characters per type (e.g., `*` red, `~` blue, `#` green)
- [ ] Beam color matches wand's `Effect` type from `Resources/Items.txt`
- [ ] Existing beam trajectory/range mechanics unchanged
- [ ] BDD test: "When fire wand fires, beam is red" (verify pixel color or ASCII char)

---

### R2: AoE Effect Animation (#312)
Ball spells and mass effects shall animate with expanding rings (balls) or color flashes (mass), improving visual feedback.

**Acceptance Criteria:**
- [ ] Ball spells (radius-based damage) render as expanding concentric rings in OpenGL
- [ ] Mass spells (all monsters) render as a full-screen color flash
- [ ] Animation duration: 500–1000ms configurable
- [ ] Ring expansion speed is smooth and proportional to ball radius
- [ ] ASCII mode renders expanding rings with ASCII art (e.g., concentric `@`, `*`, `o`)
- [ ] Mass effect ASCII: full screen inverted/color-inverted text flash
- [ ] Animation does not block gameplay; runs asynchronously
- [ ] BDD test: "When ball spell cast, rings expand to spell radius" (verify animation frames)

---

### R3: MON_AI_SEEKRANGED – Ranged Monster AI (#314)
Ranged monsters shall maintain an optimal attack distance (e.g., 5–8 tiles), advancing if prey is too close and retreating if too far, improving tactical gameplay.

**Acceptance Criteria:**
- [ ] New AI brain `MON_AI_SEEKRANGED` added to `src/Constants.h`
- [ ] Ranged monsters (e.g., orc archer, frost mage) use this brain in `Resources/Monsters.txt`
- [ ] Pathfinding: monster moves toward player if range < optimal, away if range > optimal
- [ ] Optimal range: 5–8 tiles; configurable per-monster via `Resources/Monsters.txt`
- [ ] When blocked, monster strafes perpendicular to player or waits for line-of-sight
- [ ] Monster still attacks when in range (no range-finding bug prevents ranged attacks)
- [ ] BDD test: "When ranged monster sees player within close range, it retreats" (verify movement)
- [ ] BDD test: "When ranged monster sees player too far, it advances" (verify movement)

---

## Out of Scope
- Dungeon generation changes
- Core combat system overhaul
- New monsters/items beyond those needed for testing ranged AI (variants of existing types only)
- Graphics asset import beyond current beam/animation rendering

---

## Assumptions

**ASSUMPTION: Beam colors are RGB hardcoded, not parsed from `Resources/Items.txt`**
→ Rationale: No color field exists in Items.txt format; colors are tied to `Effect` enum. Implementer will add a color map in wand rendering code (e.g., `Fire → RGB(255,128,0)`).

**ASSUMPTION: AoE animation runs in the render loop, not game logic**
→ Rationale: Animation is purely visual; no game state mutation. Render system adds a transient effect queue.

**ASSUMPTION: "Optimal range" for MON_AI_SEEKRANGED is 5–8 tiles as a hardcoded default**
→ Rationale: Resources/Monsters.txt has no range field. Implementer will use a constant; per-monster tuning can be added post-release if needed.

**ASSUMPTION: Pathfinding uses the existing `CAIMgr` movement logic**
→ Rationale: Monster movement already implemented; AI brain just changes distance heuristic, not movement code.

**ASSUMPTION: All three features must have BDD test stubs before implementation**
→ Rationale: Testing requirement stated in brief; tests do not need to pass initially, only exist and outline acceptance.

---

## Test Plan

- **BDD Feature files** (one per requirement, in `test/features/`):
  - `beams_multicolor.feature` — beam color rendering (OpenGL + ASCII)
  - `aoe_animation.feature` — expanding rings / mass flash (OpenGL + ASCII)
  - `ranged_ai_seekranged.feature` — ranged monster movement (advance/retreat)

- **Step definitions** (C++ in `test/features/step_definitions/`):
  - Render verification (pixel color sampling or ASCII char inspection)
  - Monster movement tracking (verify position delta per turn)

- **Setup context**: Use existing `TestContext` in test suite; mock or spawn test dungeon with known monster placements.

---

## Gates

| Gate | Owner | Status | Notes |
|------|-------|--------|-------|
| **G0** | Jimbo | ⏳ Decision pending | Code style & clang-format compliance |
| **G1** | — | ⏳ Pending G0 | Framework integrity (no .agentic/ edits) |
| **G2** | — | ⏳ Pending G0 | Build & platform compatibility (macOS, Linux, Raspberry Pi) |
| **G3** | — | ⏳ Pending G0 | Verification & merge approval |

---

## Risk & Mitigation

| Risk | Likelihood | Mitigation |
|------|------------|-----------|
| Beam colors clash with terminal themes (ASCII mode) | Medium | Test on black/white terminal; use neutral chars if needed |
| AoE animation causes framerate drop | Low | Profile animation loop; cap frame count if needed |
| Ranged AI pathfinding gets stuck on obstacles | Medium | Use existing pathfind logic; add strafing fallback |
| $50 budget exhausted mid-implementation | Medium | Prioritize: beams > AoE > ranged AI if needed |

---

## Deliverables

1. **Code changes**: Wand effects, AoE animation, MON_AI_SEEKRANGED + related monster data
2. **Tests**: Three BDD feature files + passing step definitions
3. **Docs**: Update `doc/_JMoria Developer's Guide.md` with new AI brain + effect system notes
4. **Commit**: All changes in branch `feat/phase3d_shiny`; ready for merge to `develop`

---

## Questions for G0

None at this time; brief is clear and aligned with observed repo state.
