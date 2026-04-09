# JMoria Bot Architecture

## Overview

The bot is a turn-based decision engine driven by screen-scraping. Each tick, `crawler.py` captures the tmux pane, parses it into a `GameState`, and calls `DecisionEngine.decide(state)` which returns a single keypress to send back to the game.

## Module Summary

| File | Role |
|---|---|
| `crawler.py` | Main loop: capture screen → parse → decide → send key |
| `screen.py` | Raw tmux pane capture and character grid extraction |
| `state.py` | `GameState` dataclass; parses screen into structured fields |
| `decision.py` | Decision engine (this document) |
| `pathfinding.py` | A\* pathfinding and BFS utilities on the known map |
| `cmd.py` | Low-level key sending via `tmux send-keys` |

---

## `decision.py` Process Flow

`DecisionEngine.decide(state)` is called once per game tick and returns a single keypress string.

### 1. Housekeeping

At the top of every tick:

- Decrements cooldown counters: `wield_cooldown`, `use_cooldown`, `staircase_attempt_cooldown`
- Updates `visited_world`, `known_map`, `door_graph`, `unexplored_tiles` from visible screen
- On dungeon depth change: clears all per-level state (map, goal stack, cached path, door graph)
- Detects stuck condition (same `last_pos` multiple turns in a row) → increments `stuck_turns`

### 2. Learning from Last Message

Each tick the game's last message is inspected:

- **`_learn_from_wield_feedback`** — records which item categories can't be wielded; updates gear scores on successful wield
- **`_learn_from_monster_feedback`** — tallies hits/misses/kills/damage per monster name into `monster_knowledge`
- **`_learn_from_scroll_feedback`** — tracks scroll labels awaiting effect resolution
- **`_learn_from_consumable_feedback`** — diffs state snapshots before/after quaff or read to tag effects (heals, harms, teleports, etc.) into `consumable_knowledge`
- **`_queue_pickup_equip_from_message`** — when a pickup message fires ("you have a ..."), queues that inventory slot for wield evaluation

### 3. Pending Multi-Turn Action Drain

Several actions require multiple ticks (send command, await prompt, send slot). These are checked in order and return early if active:

1. **`pending_wield_slot`** — waits for "wield which item?" prompt, then sends the slot key
2. **`pending_use_slot`** — waits for "quaff/read which item?" prompt, then sends the slot key
3. **Escape stale prompts** — if an unexpected inventory prompt appears with nothing pending, sends `ESC`
4. **`pending_pickup_equip_slot`** — evaluates freshly-picked-up gear via `_should_equip_item`; initiates wield if untested or strictly better by score
5. **`pending_reequip_kind`** — after testing a new item, re-equips the known-best for that slot
6. **`pending_keys`** — drains a queued key sequence (e.g. `o` + direction for door opening)

### 4. Reactive Door/Wall Handling

- "bumped into a door" → queues `o <last_direction>` (open command + direction)
- "bumped into a wall" with an adjacent monster visible → attacks in that direction (corner breakout)

### 5. Priority Decision Ladder

Evaluated in order; the first match wins and returns immediately:

| Priority | Condition | Action |
|---|---|---|
| 1 | HP < 50% and no visible monsters | `R` — rest |
| 2 | Consumable opportunity (`_consider_consumable_use`) | `q` quaff or `r` read |
| 3 | Adjacent monster (non-phantom) | Bump-attack in monster's direction |
| 4 | Post-combat, HP < 70%, no visible monsters | `R` — rest |
| 5 | Otherwise | `_pursue_goals` |

**Phantom suppression**: if stuck ≥ 4 turns attacking the same spot with no combat feedback, that world position is added to `phantom_positions` and the apparent monster is ignored.

**Consumable policy** (`_consider_consumable_use`):
- HP < 50%: use known healing potion if available
- HP < 60%, nothing adjacent: try any unidentified potion (learning)
- HP > 70%, nothing adjacent: try any unidentified scroll (learning)

### 6. Goal-Based Exploration: `_pursue_goals`

Maintains a **LIFO goal stack** of `(type, world_rc)` entries.

**Goal types** (lowest to highest effective priority when pushed):
- `"door"` — an unexplored door tile
- `"unexplored"` — a floor tile not yet stood on
- `"staircase"` — the `>` tile to descend
- `"item"` — a visible item on the floor

**Each tick:**

1. If standing on an item not yet scheduled → push `item` goal
2. If `door_momentum` is set (just opened a door) → push one step forward through it
3. **Initial phase** (`explore_phase == "head_east"`) — walk east until blocked, then switch to normal goal-stack mode
4. `_update_goal_stack` — scan visible tiles, push newly-seen doors (low priority) and items (high priority)
5. Clean stale goals from stack top (already-reached, already-explored)
6. If stack empty:
   - Find nearest unexplored tile via BFS on `known_map` (`_nearest_unexplored_tile`)
   - If none, and cooldown elapsed, find nearest `>` staircase (`_find_staircase`)
7. Opportunistic item promotion — if any item goal is exactly 1 step away, promote it to top
8. Follow **cached A\* path** to the top goal; invalidate cache if target changed or bot moved off-path
9. Door encountered in path → emit `o <dir>`, set `door_momentum`; clear cache
10. A\* fails → pop goal, retry up to 10 times on remaining stack; if all fail, walk any available direction; last resort: `.` (wait)

### 7. Staircase Descent Gate

`_can_safely_descend_stairs` enforces the progression policy before using `>`:

- All `unexplored_tiles` must be empty (level fully swept)
- HP ≥ 70%
- No adjacent monsters
- No recent combat (`turns_since_combat_feedback >= 3`)

See `PROGRESSION_STRATEGY.md` for rationale.

---

## Persistent Knowledge

`DecisionEngine` accumulates learning across runs via `load_knowledge` / `save_knowledge` (JSON):

| Field | Contents |
|---|---|
| `monster_knowledge` | Per-monster hit/miss/kill/damage tallies and attack types |
| `item_knowledge` | Gear AC/damage observations, best-by-slot tracking |
| `consumable_knowledge` | Effect tags observed per consumable category |
| `door_knowledge` | Lock pick success/fail counts |
| `learned_non_wieldable_categories` | Item categories the game rejected wield for |
| `flavor_map` | Per-run: potion/scroll flavor → observed effect label |

Knowledge is written to disk whenever `knowledge_dirty` is set (after any learning event).
