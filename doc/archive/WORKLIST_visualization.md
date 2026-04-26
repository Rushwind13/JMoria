# Bot Visualization — Issue #207

Reveal the bot agent's thinking with low-noise, human-readable visualization. The bot's decision-making is currently opaque outside `--verbose` text logs. This worklist breaks the effort into incremental, shippable pieces.

Ref: https://github.com/Rushwind13/JMoria/issues/207

---

## Phase 1 — Thought Bubble (Status Line in tmux)

**Goal**: Show a one-line "thought bubble" in a dedicated tmux pane so a human observer can see what the bot is doing without reading raw logs.

- [ ] **1.1 Add `think` summary to DecisionEngine** — Ensure every `decide()` exit path sets a short human-readable string (already partially done via `think=` in verbose logs). Normalize to ≤60 chars.
- [ ] **1.2 Create tmux status pane** — Split a small (3-row) pane below the game pane in `crawler.py` launch sequence. Write the thought bubble there each tick.
- [ ] **1.3 Show goal stack top-3** — Below the thought line, show the top 3 goals from the goal stack with type and world position (e.g., `[1] unexplored (45,22) [2] door (30,18) [3] staircase (60,55)`).
- [ ] **1.4 Show key vitals alongside** — HP, depth, turn count, stuck counter, # unexplored tiles remaining — one line.
- [ ] **1.5 Toggle via `--think` flag** — Off by default. When enabled, creates the status pane; when disabled, no extra pane or overhead.

## Phase 2 — Structured Decision Log

**Goal**: Replace free-form verbose logging with structured, parseable output for post-run analysis.

- [ ] **2.1 Emit JSON-lines decision log** — One JSON object per turn written to a `.jsonl` file: `{turn, hp, depth, wpos, think, action, goal_stack_size, top_goal, monsters_visible, items_visible, stuck_counter}`.
- [ ] **2.2 Add learning events to log** — When monster/item/consumable knowledge updates, emit a `learning` event line with before/after values.
- [ ] **2.3 Add depth-transition summaries** — On depth change, emit a summary: tiles explored, doors found, monsters killed, items collected, time on level.
- [ ] **2.4 Log file rotation** — `--log-dir` flag. Default `scripts/bot/logs/`. One file per run with timestamp.

## Phase 3 — ASCII Map Overlay

**Goal**: Render the bot's internal known_map with annotations to a file or pane, showing what the bot "sees" vs. what's unexplored.

- [ ] **3.1 Dump known_map to file** — Periodic (every N turns or on demand) ASCII dump of the 100×100 known_map with legend: `#` wall, `.` floor, `?` unexplored, `@` player, `*` current goal target, `G` goal stack targets.
- [ ] **3.2 Show path overlay** — Draw the current A* cached_path on the map as `+` characters so you can see where the bot is heading.
- [ ] **3.3 Show explored vs. visited** — Use distinct characters for "explored but not visited" vs. "visited" tiles.
- [ ] **3.4 Trigger via signal or turn interval** — `SIGUSR1` dumps map snapshot, or `--map-interval N` dumps every N turns.

## Phase 4 — Periodic Snapshot Summary

**Goal**: Every N turns (default 500), print a concise multi-line summary of bot state to stdout or log.

- [ ] **4.1 Snapshot formatter** — Summary includes: turn, depth, HP, XP/level, inventory highlights (best weapon, armor AC, potion count), kills this level, tiles explored %, goal stack depth.
- [ ] **4.2 Death summary** — On death, print a final snapshot with cause-of-death context: last 5 actions, monster that killed, HP trend.
- [ ] **4.3 Run summary** — On clean exit or death, print aggregate stats: total turns, deepest level, total kills, items collected, knowledge entries gained.

## Phase 5 — Web Dashboard (Stretch)

**Goal**: Optional real-time web UI for watching the bot play remotely.

- [ ] **5.1 WebSocket event emitter** — Lightweight FastAPI/Flask server that replays the JSON-lines log in real-time via WebSocket.
- [ ] **5.2 Map canvas** — HTML5 canvas rendering the known_map with color-coded tiles, goal markers, path overlay.
- [ ] **5.3 Stats sidebar** — Live HP/XP/depth/inventory panel.
- [ ] **5.4 Action timeline** — Scrolling list of recent decisions with reasoning.
- [ ] **5.5 Standalone mode** — Can also load a completed `.jsonl` log file for post-mortem replay.

---

## Priority Order

| Priority | Item | Rationale |
|----------|------|-----------|
| **P0** | 1.1–1.5 Thought bubble | Immediate observability with minimal code; unblocks debugging |
| **P1** | 4.1–4.3 Periodic snapshots | Useful for batch runs where you can't watch live |
| **P1** | 2.1 JSON-lines log | Foundation for all post-run analysis |
| **P2** | 3.1–3.2 Map + path overlay | Critical for debugging pathfinding and exploration |
| **P2** | 2.2–2.4 Learning events + rotation | Enriches the structured log |
| **P3** | 3.3–3.4 Explored/visited + triggers | Polish for map overlay |
| **P3** | 5.x Web dashboard | Nice-to-have; only after core visualization is solid |

---

## Integration Notes

- **Existing hooks**: `DecisionEngine.decide()` already returns `(action, think_reason)`. The `think=` field in verbose logs is the basis for 1.1.
- **tmux pane split**: `crawler.py` already manages tmux session lifecycle; adding a status pane is a natural extension.
- **Performance**: All visualization is write-only (no game state queries). JSON-lines is append-only. Map dump is periodic. Minimal impact on tick rate.
- **Toggle flags**: `--think` (Phase 1), `--jsonl` (Phase 2), `--map-interval N` (Phase 3), `--web` (Phase 5). All off by default.

---
*Created for issue #207. Last updated: 2026-04-09.*
