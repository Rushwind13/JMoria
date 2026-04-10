# AI Player Bot — Post-Pathfinding Roadmap

After PR#159 merges, the bot has: tmux harness, screen parser, A* pathfinding, goal-stack exploration, combat/loot/equip basics, persistent learning (knowledge.json), and death detection. This document tracks all remaining work.

---

## Open Issues

### Bot Behavior

- **[#213] Goal stack ordering — "happy wanderer" exploration**
  BFS target selection causes breadth-first wandering instead of committed depth-first exploration. Bot ping-pongs between frontiers, partially explores areas, skips doors at hallway ends. Needs DFS-style directional bias, frontier clustering, or door proximity boost.

- **[#200] Bot leaves parts of dark rooms unexplored**
  Tile-by-tile discovery in dark rooms misses interior sections. May need systematic sweep pattern or room-awareness to prioritize local unexplored tiles.

- **[#207] Reveal bot agent's thinking with low-noise visualization**
  Add visual overlay or log mode showing bot decision state without cluttering gameplay view.

- **[#206] Bot crawler log depth parameter is incorrect**
  Depth reporting in bot logs doesn't match actual dungeon depth.

- **[#204] Do not use wizard mode in the AI player bot**
  Bot should play without wizard-mode commands for realistic soak testing.

- **[#188] Bot parser likely overcounts visible monsters/items in ASCII viewport**
  Noisy entity counts affect threat scoring and decision heuristics.

### Game Engine (affecting bot)

- **[#215] Items visible through walls and closed doors**
  Items render on tiles the player has no LOS to, giving bot (and player) information they shouldn't have. Bot paths toward unreachable items.

- **[#214] Dungeon gen: isolated hallway segments with no exits**
  Player can spawn in a sealed hallway surrounded entirely by walls with no doors. Level is unplayable.

- **[#196] Dungeon gen: two hallways "sidle" creating double-wide corridors**
  Two adjacent hallways share a wall, creating double-wide corridors with floating doorways. Related to #214.

- **[#167] Monsters don't draw / draw as invisible / draw as holes**
  `DrawDungeon()` skips base tile when monster present; ASCII mode creates holes if `DrawMonsters()` also fails.

- **[#198] Yellow Icky Thing rendered in green**
  Color mismatch in monster rendering.

- **[#173] Items disappear at wrong view distance during gameplay**
  Item visibility drops off at incorrect distance.

---

## Room Exploration & Navigation

- [ ] **Room-loop-then-exit strategy** — When wall-follow enters a room, complete one full perimeter loop, then take the first unvisited hallway/door exit.
- [ ] **Room interior fill ("paint stripes")** — After room perimeter is mapped, walk horizontal stripes across the interior to reveal every floor tile. Track fill status per room.
- [ ] **Restore core survival behaviors** — Pick up loot near the path, auto-equip best gear, flee from dangerous monsters at low HP, pathfind to staircases after room is explored, descend when ready.
- [ ] **Level 2+ readiness** — Track XP and player level. Don't descend until character level 2. Reset exploration state on depth change.

---

## Parser & Telemetry

- [ ] Validate parser reject counters across multiple long runs and tune thresholds.
- [ ] Fix depth reporting in bot logs (#206).
- [ ] Reduce entity overcounting in ASCII viewport (#188).

---

## Knowledge & Persistent Learning

- [ ] Item slot semantics + equip compatibility memory (what can be worn/wielded where).
- [ ] Light economy memory (torch/lantern uptime, low-light risk windows).
- [ ] Time-to-kill and hit-difficulty memory by monster.
- [ ] Equipment outcome memory from stat deltas (AC/To-hit/To-dam impact).
- [ ] Per-depth map notes (chokes, dead-ends, stair positions, danger zones).
- [ ] Refine weapon ranking using real item damage metadata from Items.txt.

---

## Combat Strategy

- [ ] Monster threat assessment — calculate expected damage vs. player HP; decide engage, kite, or flee.
- [ ] Tactical positioning — use doorways for 1v1 fights; retreat to stairs if overwhelmed.
- [ ] Item usage in combat — quaff potions when HP < 30%, read scrolls for crowd control, zap wands at ranged enemies.

---

## Inventory Management

- [ ] Parse weapon damage (e.g. "2d8") from item names for auto-wield comparison.
- [ ] Auto-equip best armor (shields, helms, cloaks) and track AC improvements.
- [ ] Drop low-value items when inventory is full; keep consumables.

---

## Multi-Level Strategy

- [ ] Descend when: HP > 70%, good equipment, level explored. Stay when: low HP, unexplored areas, dangerous monsters.
- [ ] Ascend when: low resources, need to retreat and heal.
- [ ] Track consumable usage and return to earlier levels to restock if needed.
- [ ] Maintain full 100×100 explored map per level.

---

## Remove Wizard Mode Dependency (#204)

- [ ] Bot should play without `^t`, `^f`, `^i`, `^s` or `JMORIA_SHOW_PLAYER_POS=1`.
- [ ] Infer player world position from viewport-local position + scroll offset, or track movement deltas.
- [ ] Verify all bot features work in non-wizard play.

---

## Monitoring & Batch Testing (Phase 5)

- [ ] Track per-run metrics: deepest level, turns survived, monsters killed, items collected, cause of death.
- [ ] Log decision rationale (why each action was chosen, alternatives considered).
- [ ] `--runs N` flag for batch soak testing with statistics collection.
- [ ] Run 100+ consecutive games without hang or crash.
- [ ] Automated regression testing against baseline metrics.

---

## Bot Visualization (#207)

- [ ] Visual overlay or log mode showing goal stack, pathfinding target, and decision state.
- [ ] Optional web dashboard: real-time map, HP/XP graph, action history (Flask/FastAPI + WebSocket).

---

## LLM Integration (Phase 4 — Optional/Future)

- [ ] Prompt template with ASCII map + stats context for Claude/GPT-4.
- [ ] Structured function/tool definition for `jmoria_send_command`.
- [ ] Context management: limit prompt size, summarize earlier turns, cache static info.
- [ ] Local LLM support (llama.cpp, ollama).

---

## Far-Future / Stretch

- [ ] Multi-agent coordination — run multiple AI players, share knowledge, evolve strategies.
- [ ] Save/load state management — checkpoint before risky actions, reload on death for learning.
- [ ] Vision-based AI — read game state from SDL screen via computer vision.
- [ ] Reach dungeon level 100 (5000 ft depth).

---

## Milestones

| Milestone | Criteria | Status |
|-----------|----------|--------|
| 1. Survive the Floor | Bot navigates level 1 without dying | Mostly done (still dies to early monsters) |
| 2. Survival AI | Bot survives to level 3 | Not started |
| 3. Strategic Play | Bot reaches level 10 | Not started |
| 4. LLM Integration | LLM bot reaches level 5 | Not started |
| 5. Batch Testing | 100 consecutive runs, no hang/crash | Not started |
| Stretch | Bot reaches level 100 (5000 ft) | Not started |

---
*Created from WORKLIST_ai_player.md after PR#159 (Pathfinding) merge. Last updated: 2026-04-09.*
