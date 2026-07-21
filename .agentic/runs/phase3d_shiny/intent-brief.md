# Intent Brief: Phase 3d - Shiny

<!-- Contract: all four sections required. Author: a human. Consumer: Analyst.
     This is deliberately informal — it captures what you want, not a spec. -->

## Problem
Phase 3c Polish (infrastructure and core fixes) is complete and merged into develop. Phase 3d has addressed critical bugs and code cleanup (12 issues total), but three visual polish enhancements remain incomplete and blocking final phase release:

**Priority 3: Visual Polish & Effects (P2)** – In Progress:
- #289: Multicolor beam effects for wands (red/orange/yellow for Fire, blue/white for Cold, etc.)
- #312: AoE effect animation (expanding rings for balls; color-flash for mass effects)
- #314: MON_AI_SEEKRANGED (new AI brain enabling ranged monsters to maintain desired distance)

All Priority 1 bugs (P0: #173, #317, #266, #309) and Priority 2 cleanups (P1: #310, #315, #311, #316) plus #305 (inventory on death) are complete. These three enhancements provide the final visual polish layer needed for phase release.

## Motivation
Visual feedback is critical for roguelike gameplay clarity. Beam effects (#289) and animations (#312) make ranged combat legible; ranged AI (#314) makes monster behavior predictable. These enhancements finalize Phase 3d by completing the polish promised in the roadmap.

This work fits within the $50 USD budget allocation already underway.

## Constraints
- Hard budget limit of $50 USD; work pauses at exhaustion
- Must not break any of the 12 completed Phase 3d issues or core gameplay
- All three enhancements must have passing BDD tests or documented acceptance criteria
- Visual effects must render correctly in both ASCII and OpenGL modes

## Out of scope
- Graphics overhaul beyond beam/animation assets
- Dungeon generation or core combat system changes
- New monsters/items beyond those affected by MON_AI_SEEKRANGED (ranged monster variants)
