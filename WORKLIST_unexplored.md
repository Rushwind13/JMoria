# WORKLIST: Fix Unexplored Tile Count in Lit Rooms (#221)

## Problem
The bot's `unexplored_tiles` count shows ~135 unexplored tiles remaining after walking through a fully lit room. In a lit room every floor tile is visible on screen, yet `_update_exploration_sets()` never marks those tiles as explored because `_update_known_map()` only blits tiles within the tmux viewport — not the full visible room extent.

## Root Cause Analysis
1. **`_update_known_map()`** (decision.py ~L674-705) copies only tiles visible in the tmux capture-pane viewport to `known_map`. The viewport is roughly 75×35 characters — smaller than some dungeon rooms plus their lit surroundings.
2. **`_update_exploration_sets()`** (decision.py ~L707-751) marks tiles as frontier (unexplored) if they border unknown (`" "`) in `known_map`. Tiles the game engine reveals (because the room is lit) but that fall outside the viewport remain `" "` in `known_map` forever — creating persistent phantom frontier tiles.
3. **`_room_is_visible()`** exists as a static method (decision.py ~L1140-1154) but is **dead code** — never called. It was likely intended to detect lit rooms but was never wired in.

## Work Items

### P0 — Core fix
- [ ] **W1: Expand known_map coverage for lit rooms**
  In `_update_known_map()`, after blitting the viewport, detect whether the player is in a lit room (reuse/adapt `_room_is_visible()`). If so, flood-fill or boundary-scan the room on `known_map` to mark all contiguous floor/door tiles as explored (`.`) even if they're outside the current viewport. This eliminates phantom frontier tiles inside known rooms.

### P1 — Supporting changes
- [ ] **W2: Wire in `_room_is_visible()` or replace it**
  The existing heuristic counts floor tiles 2-3 steps away; `far_floor >= 3` indicates a lit room. Decide whether to use this heuristic on the viewport grid, or infer lighting from the game engine (e.g., if the room walls are visible on screen at distance > 1 from player, the room is lit). Connect the chosen approach to W1.

- [ ] **W3: Flood-fill room extent on `known_map`**
  When a lit room is detected, flood-fill from the player's world position on `known_map` outward through `.`, `'`, `<`, `>` tiles (stopping at `#` walls and `" "` unknown). Mark all reached tiles as explored. This prevents frontier tiles from appearing inside the room perimeter.

### P2 — Edge cases & validation
- [ ] **W4: Handle room re-entry after scroll**
  If the player leaves a lit room and returns, the viewport may have shifted. Ensure previously flood-filled room tiles in `known_map` survive viewport shifts (they should, since `known_map` is persistent — verify no overwrite-with-blank logic erases them).

- [ ] **W5: Dark room behavior unchanged**
  Verify dark rooms (where only adjacent tiles are visible) are unaffected — `_room_is_visible()` should return `False` and no flood-fill occurs. Dark room exploration should continue to use the existing tile-by-tile reveal.

- [ ] **W6: Bot test validation**
  Run the bot crawler and confirm:
  - `Unexplored` count drops to near-zero after entering a lit room
  - Bot still descends stairs when exploration is complete
  - No regressions in dark room exploration or corridor following

### P3 — Cleanup
- [ ] **W7: Remove dead code or integrate**
  If `_room_is_visible()` is superseded by the new approach, remove it. If reused, remove the dead-code comment and add a call site.

## Files to Modify
- `scripts/bot/decision.py` — `_update_known_map()`, `_update_exploration_sets()`, `_room_is_visible()`

## Related Issues
- #200 Bot leaves parts of dark rooms unexplored
- #207 Bot visualization
