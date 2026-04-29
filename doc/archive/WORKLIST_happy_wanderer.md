# Happy Wanderer Exploration — Work List

## Overview

This work list tracks the implementation of depth-first, committed exploration for the JMoria bot. See **Issue #213** for the full problem statement and design rationale.

The bot currently uses BFS (`_nearest_unexplored_tile()`) to pick the next exploration target, which produces breadth-first wandering — ping-ponging between frontiers, partially exploring areas, and skipping doors at hallway ends. The goal is to replace this with exploration that commits to clearing one area before moving to the next.

---

## Priority 1: Direction-biased target selection (core fix)

**[P1.1] Add directional continuation scoring to `_nearest_unexplored_tile()`**
- When scoring frontier tiles, add a bonus for tiles that continue in the bot's current travel direction.
- Use `last_action` (directional char) to determine the current heading.
- Tiles "ahead" of the bot should score better than equidistant tiles behind or to the side.
- This directly addresses mid-hallway turnarounds and hallway-end door avoidance.

**[P1.2] Prefer forward-reachable frontier over globally nearest**
- When the bot is in a hallway or corridor, bias strongly toward the frontier tile at the far end rather than a closer tile in a previously-visited area.
- Consider a two-pass approach: first check for reachable frontier tiles in the forward arc, then fall back to BFS if none exist.

**[P1.3] Reduce backtracking after door encounters**
- When the bot is adjacent to a closed door with unknown tiles behind it, that door should win over a slightly-closer unexplored tile behind the bot.
- The existing door scoring (score=0 for closed doors with unknown neighbors) may need to factor in distance-from-bot so nearby doors always beat far-away floor tiles.

## Priority 2: Frontier clustering (area commitment)

**[P2.1] Identify connected frontier clusters**
- Group contiguous unexplored frontier tiles into clusters (connected components).
- Each cluster represents an "area" the bot could commit to clearing.

**[P2.2] Commit to clearing current cluster before switching**
- Once the bot begins exploring a cluster, continue targeting tiles within that cluster until it's fully explored.
- Only switch clusters when the current one is exhausted or unreachable.
- This directly fixes the "explore half a dark room, leave, come back later" pattern.

**[P2.3] Score clusters by proximity and size**
- When choosing which cluster to explore next, prefer nearby clusters (minimize travel) and larger clusters (more efficient to clear).

## Priority 3: Door proximity boost

**[P3.1] Boost score for doors adjacent to current path**
- When the bot's A* path passes within 1-2 tiles of a closed door, boost that door's exploration priority.
- Prevents "walk past a door in a hallway without opening it" behavior.

**[P3.2] Opportunistic door opening during traversal**
- If the bot is pathfinding through a hallway and a closed door is directly adjacent (perpendicular), consider opening it as a low-cost side action before continuing.

## Priority 4: Measurement and validation

**[P4.1] Add exploration efficiency metrics to crawler output**
- Track: total turns to clear level, unique tiles visited vs total walkable, number of frontier switches, number of backtrack segments.
- Output to crawler log for before/after comparison.

**[P4.2] Baseline current exploration behavior**
- Run crawler with current code: record turns-to-clear, tile coverage, and frontier-switch count across multiple runs.
- Save as baseline for comparison.

**[P4.3] Validate improvements against baseline**
- After each priority tier is implemented, re-run crawler and compare against baseline.
- Target: fewer frontier switches, fewer total turns, more complete area coverage before moving on.

## Priority 5: Edge cases and polish

**[P5.1] Handle dark room exploration specifically**
- Dark rooms (no light source) require tile-by-tile discovery.
- When inside a dark room, switch to a systematic sweep pattern (e.g., spiral or row-scan) to avoid missing interior tiles.
- Related: Issue #200.

**[P5.2] Avoid oscillation between two equidistant frontiers**
- Add hysteresis: once the bot commits to a direction, require a meaningful reason to reverse (e.g., current frontier exhausted, new high-priority item visible).
- Track "committed direction" and apply a penalty for reversals.

**[P5.3] Graceful fallback when DFS-style gets stuck**
- If directional bias leads to a dead end with no forward frontier, fall back to global BFS cleanly.
- Ensure stuck_turns detection still works correctly with the new scoring.

---

## Implementation Notes

- All changes are in `scripts/bot/decision.py` and possibly `scripts/bot/pathfinding.py`.
- The goal stack structure (LIFO with type/position tuples) should remain unchanged.
- A* pathfinding and door momentum are working correctly — this work is about **which target to pick**, not how to get there.
- Test via `scripts/crawl.sh` and bot test harness; validate with before/after crawler metrics.
