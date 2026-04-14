# AI Player Bot Roadmap

This roadmap tracks the development of an autonomous dungeon-crawler bot (`scripts/bot/`) for automated playtesting, gameplay research, and future LLM integration. The bot complements the core game roadmap by providing a testbed for balance, dungeon generation quality, and emergent gameplay.

## Philosophy

The bot roadmap prioritizes:
1. **Survival & Learning** - bot progresses deeper, adapts to threats
2. **Fidelity** - bot plays without unfair advantages (no wizard mode, no cheating)
3. **Observability** - logging, decision tracing, replay analysis
4. **Scalability** - batch testing (100+ runs), statistical analysis

---

## 🎯 Priority 1: Core Infrastructure

### #218 - Bot Roadmap: Post-Pathfinding Work (Master Epic)
**Status**: Active reference
**Description**: Comprehensive tracking of all bot work remaining after PR #159 (core tmux harness + A* pathfinding).
**Impact**: Central roadmap for all bot development; sub-issues feed into this epic.
**Current Scope**: See #218 for full breakdown (exploration quality, parser tuning, combat strategy, multi-level play, wizard-mode removal, monitoring, visualization, LLM integration).
**Related Files**: 
- `WORKLIST_ai_player_roadmap.md` (older planning doc)
- `scripts/bot/BOT_ARCHITECTURE.md` (system design)
- `scripts/bot/PROGRESSION_STRATEGY.md` (strategy docs)

### #230 - Create Unit Tests for the Bot
**Status**: Not started
**Description**: Build standalone unit test suite for bot modules (screen parser, pathfinding, decision engine, state machine) separate from game BDD tests.
- Tests should mock tmux interaction (no live game required)
- Use pytest or unittest (Python standard lib)
- Test modules: `screen.py`, `decision.py`, `pathfinding.py`, `state.py`, `cmd.py`
**Impact**: Regression safety, enables rapid iteration on bot logic.
**Dependencies**: None (orthogonal to game tests)
**Effort**: Medium
**Files Affected**: `test/runtests.sh` (integrate bot tests), new `scripts/bot/tests/`

### #204 - Do Not Use Wizard Mode in the AI Player Bot
**Status**: Top priority (prerequisite for real testing)
**Description**: Remove dependence on wizard mode cheats; bot must infer player position from movement deltas instead of using `JMORIA_SHOW_PLAYER_POS=1`.
**Impact**: Bot plays "fair"; results are meaningful for balance/generation testing.
**Challenge**: Need reliable dead-reckoning from relative viewport movement.
**Dependencies**: None blocking
**Effort**: Medium
**Sub-tasks**:
- Remove `--wizard` flag from bot startup script
- Implement position tracking via movement history
- Validate with 50+ game runs to ensure accuracy

---

## 🏃 Priority 2: Exploration & Movement

### #232 - Bot: Learn "Breeds Explosively" Intrinsic
**Status**: Not started
**Description**: Bot must detect and handle creatures that multiply (worm masses spawn more worms).
- Recognize "breeds explosively" as high threat
- Prioritize fleeing or clearing with area effects
- Remember which monsters have this trait
- Lock doors behind self when retreating
**Impact**: Prevents getting trapped by spawn spam; teaches bot threat assessment.
**Dependencies**: #200 (dark room exploration) ideally done first
**Effort**: Low-Medium
**Related Issues**: #218 item "combat & inventory strategy"

### #200 - Bot Leaves Parts of Dark Rooms Unexplored
**Status**: Not started
**Description**: Bot enters dark rooms, explores partially, exits without full coverage. Need systematic sweep pattern.
- Implement horizontal stripe passes or zigzag sweep
- Only exit room once all interior tiles marked explored
- Or: after reaching door, check remaining unexplored in current room and prioritize those
**Impact**: Improves exploration quality, reduces "stumbled through" inefficiency.
**Dependencies**: None blocking
**Effort**: Low-Medium
**Related**: goal-stack ordering (#213 in #218 epic)

### #224 - Player Bot: Stepwise Turn-Based Mode for Debugging
**Status**: Not started
**Description**: Add `--think` or similar flag to pause bot after each turn, showing parsed state, decision logic, and chosen action before pressing forward.
- Display: current goal stack, parsed screen (monsters/items), decision reasoning
- Advance via keypress or auto-resume after timeout
- Useful for: debugging stuck states, understanding decision flow, watching high-stakes moments
**Impact**: Critical debugging tool for understanding bot behavior without live screen watching.
**Dependencies**: None blocking
**Effort**: Low
**Usage**: 
```bash
python3 scripts/crawler.py --think
```

---

## 🎯 Priority 2b: Parser & Telemetry

### #188 - Bot Parser Overcounts Visible Monsters/Items in ASCII
**Status**: Not started
**Description**: Parser.py reports implausibly high entity counts (100+ visible monsters in tight viewport). Likely counting non-dungeon glyphs or stale UI artifacts.
- Add strict dungeon-region masking using known box borders and panel bounds
- Exclude glyphs appearing in messages/UI/title bar
- Cross-check entity tiles against walkable/seen semantics
- Validate against raw tmux screen capture
**Impact**: Fixes decision noise; combat/threat scoring becomes reliable.
**Dependencies**: None blocking
**Effort**: Low
**Testing**:
```bash
python3 scripts/crawler.py --verbose | grep -E "(monsters|items)="
```

---

## 🛠️ Priority 3: Operational & Polish

### #233 - Bot: --think Needs Taller Window
**Status**: Not started
**Description**: The `--think` mode bottom pane (decision display) causes equipment window cutoff on 50-line terminals. Need 55+ lines or dynamic pane sizing.
**Impact**: UX improvement for stepwise debugging.
**Dependencies**: #224 (stepwise mode)
**Effort**: Low

### #182 - Create Dungeon Crawler Bot Script (Reference)
**Status**: Mostly done (crawler.py exists)
**Description**: This issue documents the original bot design concept. Implementation already exists; issue is a reference.
**Current**: `scripts/crawler.py`, `scripts/crawl.sh`, `scripts/crawl_metric.sh`
**Use**: 
```bash
./scripts/crawl.sh           # Loop runner
./scripts/crawl_metric.sh    # Single run with summary
./scripts/watch.sh           # Attach to live game
```

---

## 🎓 Priority 3b: Learning & Knowledge

### Meta: Game Engine Bugs (Affecting Bot)
These are game bugs, not bot bugs, but they materially affect bot testing:

**From #218 epic:**
- **#214** - Isolated hallway segments (bot gets stuck)
  - **Fix**: Implement #117 (search command) as workaround
- **#215** - Items visible through walls (bot chases unreachable loot)
  - **Fix**: Add visibility check filter to item parser
- **#167** - Monsters don't draw / draw as invisible (parser confusion)
  - **Fix**: Detect missing tile, treat as floor until move there
- **#173** - Items disappear at wrong view distance (dropped items vanish)
  - **Fix**: Add item dropping logic (not yet in game)

---

## 📊 Performance & Batch Testing

### Metrics (from #218)

**Per-run analytics:**
- Deepest level reached
- Turns survived
- Monsters killed
- Cause of death
- Items collected
- Equipment changes

**Aggregate (100+ runs):**
- Mean depth, std dev
- Death rate % per level
- Most common killers
- Loot distribution
- Progression curve

**Commands:**
```bash
# Single run with output
./scripts/crawl_metric.sh

# Loop 100 runs (watch progression live)
for i in {1..100}; do ./scripts/crawl_metric.sh; done

# Verbose run with decision trace
python3 scripts/crawler.py --verbose --think
```

---

## 🎬 Future Phases

### Phase 4: Advanced Learning (Post-#218)
- **Per-depth map memory**: 100×100 explored map per level, persisted to knowledge.json
- **Threat assessment**: time-to-kill by monster type, expected damage vs HP
- **Combat items**: auto-quaff potions at low HP, cast scrolls for crowd control
- **Equipment outcome memory**: stat delta tracking (AC improvements, to-hit gains)
- **Consumable management**: auto-restock from known item locations

### Phase 5: Strategic Play (Post-multievel work from #218)
- Level 10+ survival
- Descend/ascend decision logic (HP threshold, gear quality, exploration %)
- Emergent strategies (door-locking tactics, mob farming, gear trading)

### Phase 6: Visual & Monitoring (Post-#207)
- Real-time decision overlay (goal stack, pathfinding heat map)
- Web dashboard (map view, HP/XP graph, action history)
- Replay system for post-game analysis

### Phase 7: LLM Integration (Far future, from #218 item 10)
- Structured prompts: ASCII map + stats + decision context
- Claude/GPT-4 API calls for high-stakes decisions
- Local LLM support (llama.cpp, mistral)
- Context management (token budgets, relevance ranking)

---

## Dependencies: Bot ↔ Game Roadmap

**Bot needs from Core Roadmap:**

| Bot Need | Provided By | Reason |
|----------|-------------|--------|
| Searchable dungeon | #117 (Search) | Escape isolated hallways (#214) |
| Fair item visibility | Fix to #215 | Don't chase through-walls loot |
| Visible monsters | Fix to #167 | Parser reliability |
| Dropped items visible | Fix to #173 | Inventory management |
| Combat system | #39 (Ranged) | Tactical variety, testing |
| Item tiers | #128 (Magic Items) | Loot meaning, equipment decisions |
| Stat system | #197 (Stats) | Build diversity, balance testing |
| Light economy | #72 (Fog of War) | Resource management testing |
| Search mechanics | #117 (Search) | Exploration strategy testing |

**Game needs from Bot:**

| Game Need | From Bot | Reason |
|-----------|----------|--------|
| Balance feedback | Batch test runs | Reveal overpowered/underpowered monsters |
| Dungeon quality | 100+ runs analytics | Identify generation issues (#214, #196, #213) |
| Item distribution | Loot telemetry | Spot missing/common items |
| AI reference | Bot decision trace | Validate game AI feels reasonable |
| Crash detection | Soak testing | Catch segfaults, hangs, infinite loops |

---

## Dependency Graph

```
Foundation:
✅ #182 (Crawler exists)
✅ #218 (Master roadmap / reference)

Blocking Chains:
#204 (No wizard mode)
   └─ Remove wizard dependency (#218 item 6)

#230 (Unit tests)
   └─ Independent; recommended early

#224 (Stepwise mode)
   └─ No deps
   └─ Enables debugging, pairs with #232/200

#233 (Taller window)
   └─ #224 (Stepwise mode)

#232 (Breeds explosively)
   └─ #200 (Dark rooms) — recommended order

#200 (Dark room exploration)
   └─ No hard deps

#188 (Parser overcounting)
   └─ No hard deps
   └─ Quick fix with big payoff

Critical Game Bugs to Fix:
#214 (Isolated hallways)
   ← Blocked by #117 (Search command) [Game roadmap]
   
#215 (Items visible through walls)
#167 (Monsters don't draw)
#173 (Items disappear at wrong distance)
```

---

## Current Status (April 14, 2026)

**Completed (from PR #159):**
- Tmux harness (send keys, read screen)
- Screen parser (monster/item detection, player position tracking)
- A* pathfinding engine
- Goal stack (exploration, loot, combat)
- Combat basics (melee approach, simple evasion)
- Death detection
- Knowledge persistence via knowledge.json

**In Progress:**
- Refinement and bug fixes
- Parser accuracy improvements

**Blocked:**
- Multi-level play (need descent/ascend logic)
- Advanced combat (need threat assessment, item usage)
- Fair testing (need wizard-mode removal)

**Next Steps:**
1. Fix parser (low-hanging fruit)
2. Implement #204 (remove wizard mode)
3. Add #230 (unit tests)
4. Add #224 (stepwise debugging)
5. Tackle #200 (dark rooms)

---

## Testing Workflow

**Development Loop:**
```bash
# Terminal 1: Development
cd /Users/jimbo/Documents/code/tmp/JMoria
vim scripts/bot/decision.py  # Make changes

# Terminal 2: Testing
tmux send-keys -t crawler "C-c"  # Kill current game
# New game starts automatically (crawl.sh loop)

# Terminal 3: Watching
./scripts/watch.sh  # Attach to live game
```

**Batch Analysis:**
```bash
# Run 50 games, collect metrics
for i in {1..50}; do ./scripts/crawl_metric.sh >> bot_results.txt; done

# Parse results
grep "Level\\|Depth\\|Cause" bot_results.txt | sort | uniq -c
```

**Unit Testing:**
```bash
cd scripts/bot
python3 -m pytest tests/ -v
```

---

## References

- [Bot Architecture](scripts/bot/BOT_ARCHITECTURE.md) — system design
- [Progression Strategy](scripts/bot/PROGRESSION_STRATEGY.md) — decision logic
- [Core Roadmap](WORKLIST_jmoria_core_roadmap.md) — game features the bot relies on
- [Master Epic #218](https://github.com/Rushwind13/JMoria/issues/218) — comprehensive post-pathfinding breakdown

