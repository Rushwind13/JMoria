# AI Player System - Work List

## Overview

This work list defines the implementation of an autonomous dungeon crawler bot for JMoria. See also **Issue #182** for the design rationale.

**Foundation:** The ASCII renderer (`src/RenderASCII.cpp`, merged in PR#155) renders the game as plain text via ncurses. Running the game inside a `tmux` session lets an external script read screen state with `tmux capture-pane` and send commands with `tmux send-keys`. No changes to the game executable are required.

**Goal:** Create a Python/Bash bot script in `scripts/` that plays JMoria autonomously — navigating, fighting, looting, and descending toward level 100 (5000 ft depth). Primary use: soak testing, balance data collection, crash detection, and regression coverage.

---

## Foundation: ASCII Renderer + tmux

### What We Already Have:
- ✅ ASCII renderer (`src/RenderASCII.cpp`) renders dungeon, monsters, items, stats, and messages as plain text
- ✅ ncurses-based 80x24 (or larger) terminal layout with all game regions visible
- ✅ `tmux new-session` launches the game headlessly inside a terminal session
- ✅ `tmux capture-pane -p` captures current screen contents as plain text
- ✅ `tmux send-keys` injects single keystrokes into the game
- ✅ `SDL_VIDEODRIVER=dummy` suppresses graphics window (only ncurses output needed)

### Running the Game for Bot Use:
```bash
# Build ASCII-only executable (no SDL/OpenGL required at runtime):
make ascii

# Launch in tmux:
tmux new-session -d -s crawler -x 125 -y 40 './jmoria 2>/tmp/crawler.log'
tmux capture-pane -t crawler -p   # read screen
tmux send-keys -t crawler 'h'     # send keystroke (no Enter needed for single-char commands)
```

### Watching The Bot Live (Visual):
- [x] Attach to the running bot session: `tmux attach -t crawler`
- [x] Detach without stopping it: `Ctrl-b` then `d`
- [x] One-shot watch without attaching: `tmux capture-pane -t crawler -p | tail -40`
- [x] Run with verbose telemetry in another terminal:
  - `python3 scripts/crawler.py --verbose`
  - shows bot reasoning (`think=...`) and message persistence (`<same for N turns>`)

### Screen Layout (from `ASCIILayout::CreateForSize`):
| Region | Content | Parse Strategy |
|--------|---------|----------------|
| Top 5 rows | Messages | Last game message text |
| Left ~25 cols | Stats (HP, AC, XP, level, depth) | Regex on labeled values |
| Center (remaining) | Dungeon map | 2D char grid; `@`=player, letters=monsters, symbols=items |
| Right ~25 cols | Inventory / Equipment | Item list with slot letters |
| Center overlay | Use menu (when active) | Detect and respond to prompts |

### No Dependency on PR#157:
PR#157 adds structured JSON logging and is valuable but is not required here. The ASCII screen is sufficient for a bot: all visible game state (map, HP, messages, monsters) is present in the terminal output captured by `tmux capture-pane`.

---

## Architecture: External Script Bot (tmux-based)

A Python script that:
1. Launches `jmoria` in a tmux session with the ASCII renderer
2. Reads screen state by capturing the tmux pane
3. Parses the ASCII dungeon, stats, and messages from the captured text
4. Decides the next action via rule-based logic (and optionally LLM)
5. Sends the command as a keystroke via `tmux send-keys`
6. Repeats until death or goal reached

**Pros:**
- No game code changes needed
- Easy to iterate
- Platform-independent (works on macOS, Linux, Pi)
- Compatible with how Copilot already interacts with the game (user memory)

---

## Phase 1: Bot Infrastructure (tmux-based)

**Goal:** Wire up the harness that launches the game and drives it via tmux.

### Tasks:

#### 1.1 Launcher Script
- [x] **Create `scripts/crawler.py` (or `scripts/crawler.sh`)** — entry point
  - Build ASCII executable first: `make ascii`
  - Launch: `tmux new-session -d -s crawler -x 125 -y 40 './jmoria 2>/tmp/crawler.log'`
  - Wait for the game to reach the intro screen before sending any keys
  - Tear down: `tmux kill-session -t crawler` on exit/death

#### 1.2 Screen Reader
- [x] **Create `scripts/bot/screen.py`** — reads and parses the tmux pane
  - `tmux capture-pane -t crawler -p` → raw 125×40 string
  - Parse message region (top 5 rows) → `last_message: str`
  - Parse stats region (left ~25 cols) → `hp`, `max_hp`, `ac`, `level`, `depth`
  - Parse dungeon region (center) → `map: list[list[str]]` (2D char grid)
  - Find `@` in map → `player_pos: tuple[int,int]`
  - Find monster chars (letters a-z, A-Z per `MonIDs`) → `monsters: list[MonsterSighting]`
  - Find item chars (symbols per `ItemIDs`) → `items: list[ItemSighting]`

#### 1.3 Command Sender
- [x] **Create `scripts/bot/cmd.py`** — sends keystrokes
  - `send(key: str)` → `tmux send-keys -t crawler '<key>'`  (no Enter for single-char game commands)
  - `send_ctrl(key: str)` → `tmux send-keys -t crawler 'C-<key>'`
  - Rate limiting: short sleep between commands to avoid flooding SDL event queue
  - Command reference (from `doc/Player Docs.txt`):
    - Movement: `hjklyubn` (vi-keys), `HJKLYUBN` (run)
    - Actions: `o` (open), `c` (close), `T` (tunnel), `<` / `>` (stairs)
    - Items: `w` (wield), `t` (take off), `d` (drop), `q` (quaff), `r` (read), `z` (zap)
    - Rest: `.` (rest one turn), `R` (rest until healed)
    - Wizard (debug): `^t` teleport, `^i` create item, `^s` summon monster

---

## Phase 2: Basic AI Decision Engine

**Goal:** Create a simple rule-based AI that can navigate dungeons and survive.

### Tasks:

#### 2.1 Game State Representation
- [x] **Create `scripts/bot/state.py`** — data classes for parsed game state
  ```python
  @dataclass
  class GameState:
      turn: int
      depth: int          # e.g. 50 ft = level 1
      player_pos: tuple[int, int]
      player_hp: int
      player_max_hp: int
      player_ac: int
      player_level: int
      map: list[list[str]]    # 2D char grid from dungeon region
      monsters: list[tuple[int,int,str]]   # (row, col, char)
      items: list[tuple[int,int,str]]      # (row, col, char)
      last_message: str
  ```

#### 2.2 Pathfinding Module
- [x] **Create `scripts/bot/pathfinding.py`** — A* on the parsed map
  - Walkable chars: `.`, `'`, `<`, `>`, `+` (door - open it)
  - Obstacle chars: `#`, `:`, ` ` (void)
  - Utility functions:
    - `find_nearest(map, from_pos, target_chars) -> tuple[int,int] | None`
    - `path_to(map, from_pos, to_pos) -> list[tuple[int,int]]`
    - `direction_key(from_pos, to_pos) -> str`  (returns `h`/`j`/`k`/`l`/`y`/`u`/`b`/`n`)

#### 2.3 Basic Decision Logic
- [x] **Create `scripts/bot/decision.py`** — priority-based action selection
  ```python
  def decide(state: GameState) -> str:
      if state.player_hp < state.player_max_hp * 0.3:
          return 'R'              # rest until healed
      if monsters_adjacent(state):
          return attack_dir(state)  # bump-attack nearest monster
      if visible_items(state):
          return move_toward(state, nearest_item(state))
      if can_see_stairs_down(state):
          return move_toward(state, stairs_pos(state))
      return explore(state)       # move toward unexplored edge
  ```

---

## Phase 3: Advanced AI Capabilities

**Goal:** Improve decision-making with memory, strategy, and learning.

### Tasks:

#### 3.1 Memory & State Tracking
- [ ] **Track explored tiles across all levels**
  - [ ] Maintain full 100x100 map for each level
  - [ ] Mark tiles as `explored`, `unexplored`, `dangerous`
- [ ] **Monster database:**
  - [ ] Record monster types encountered
  - [ ] Track difficulty (damage dealt, HP)
  - [ ] Prioritize dangerous monsters
- [ ] **Item value database:**
  - [ ] Rank items by usefulness (weapons > armor > consumables)
  - [ ] Avoid picking up junk when inventory is full

---

## Current PR Focus (Active)

- [x] Verbose logs now prioritize bot reasoning (`think=`) over raw message text
- [x] Verbose logs no longer show viewport-local player position
- [x] Repeated message text is marked as persistence (`<same for N turns>`) instead of event multiplicity
- [x] Stuck detection switched to world position when available
- [~] Task 1 parser hardening in progress:
  - [x] Use canonical `MonIDs` / `ItemIDs` from source
  - [x] Filter text-like/overlay-like glyphs in dungeon parsing
  - [x] Add parser reject counters for debug telemetry
  - [ ] Validate reject counters across multiple long runs and tune thresholds
- [~] Task 2: further reduce combat-zone patrol loops (#186)
  - [x] Prioritize door exploration (`path_to_door`, `open_adjacent_door`) before generic frontier roam
  - [ ] Add long wall-bump-chain breaker (force broader escape after repeated `wall_bump_pivot`)
- [ ] Task 3: auto-wield/equip best available weapon

#### 3.2 Combat Strategy
- [ ] **Monster threat assessment:**
  - [ ] Calculate expected damage vs. player HP
  - [ ] Decide: engage, kite, or flee
- [ ] **Tactical positioning:**
  - [ ] Use doorways for 1v1 fights (avoid being surrounded)
  - [ ] Retreat to stairs if overwhelmed
- [ ] **Item usage in combat:**
  - [ ] Use potions when HP < 30%
  - [ ] Use scrolls for crowd control
  - [ ] Zap wands at ranged enemies

#### 3.3 Inventory Management
- [ ] **Auto-wield best weapon**
  - [ ] Parse weapon damage (e.g., "2d8") from item name
  - [ ] Compare current vs. new weapon
  - [ ] Send `w` command + item selection
- [ ] **Auto-equip best armor**
  - [ ] Track AC improvements
  - [ ] Equip shields, helms, cloaks, etc.
- [ ] **Drop low-value items when full**
  - [ ] Keep consumables (scrolls, potions, wands)
  - [ ] Drop duplicate weapons/armor

#### 3.4 Multi-Level Strategy
- [ ] **Level progression logic:**
  - [ ] Descend when: player HP > 70%, good equipment, level explored
  - [ ] Stay when: low HP, unexplored areas, dangerous monsters nearby
  - [ ] Ascend when: low resources, need to retreat and heal
- [ ] **Resource management:**
  - [ ] Track consumable item usage
  - [ ] Return to earlier levels to restock if needed

---

## Phase 4: LLM Integration (Optional)

**Goal:** Enable LLM-based decision-making for adaptive gameplay.

### Tasks:

#### 4.1 LLM Decision Function
- [ ] **Create prompt template:**
  ```
  You are an AI playing JMoria, a roguelike dungeon crawler.
  
  Current game state:
  - Turn: {turn}
  - Level: {level} ({depth} ft depth)
  - HP: {hp}/{max_hp}
  - Position: ({x}, {y})
  
  Visible map (21x21, '@' is you, letters are monsters, symbols are items):
  {decoded_map}
  
  Monsters:
  {monster_list}
  
  Items:
  {item_list}
  
  Recent events:
  {last_5_events}
  
  Goal: Reach dungeon level 100 (5000 ft) while gaining XP and items.
  
  What command should you send next? Reply with a single character.
  Options: h/j/k/l (move), w (wield), d (drop), < (upstairs), > (downstairs), etc.
  ```

- [ ] **Send prompt to LLM API:**
  - [ ] OpenAI GPT-4
  - [ ] Anthropic Claude
  - [ ] Local LLM (llama.cpp, ollama)
- [ ] **Parse LLM response:**
  - [ ] Extract single-character command
  - [ ] Validate against allowed commands
  - [ ] Fall back to rule-based AI if invalid

#### 4.2 LLM Function/Tool Definition
- [ ] **Define structured function for LLM tool use:**
  ```json
  {
    "name": "jmoria_send_command",
    "description": "Send a command to JMoria game",
    "parameters": {
      "command": {
        "type": "string",
        "description": "Single-character game command (h/j/k/l/w/d/etc.)"
      },
      "reasoning": {
        "type": "string",
        "description": "Why you chose this action"
      }
    }
  }
  ```

- [ ] **Implement function handler:**
  - [ ] Accept LLM function call
  - [ ] Send command to FIFO
  - [ ] Wait for next turn
  - [ ] Return updated game state

#### 4.3 LLM Context Management
- [ ] **Limit prompt size:**
  - [ ] Only include last 10 turns of events
  - [ ] Summarize earlier turns (e.g., "Explored level 1, killed 5 orcs")
- [ ] **Cache static information:**
  - [ ] Game rules
  - [ ] Command reference
  - [ ] Monster bestiary

---

## Phase 5: Monitoring & Evaluation

**Goal:** Track AI performance and improve over time.

### Tasks:

#### 5.1 Metrics Collection
- [ ] **Track AI player statistics:**
  - [ ] Deepest level reached
  - [ ] Turns survived
  - [ ] Monsters killed
  - [ ] Items collected
  - [ ] Cause of death
- [ ] **Log decision rationale:**
  - [ ] Why AI chose each action
  - [ ] Alternative actions considered
  - [ ] Expected vs. actual outcome

#### 5.2 Visualization Dashboard
- [ ] **Create web dashboard for live monitoring:**
  - [ ] Real-time map display
  - [ ] HP/XP graph over time
  - [ ] Action history log
  - [ ] Monster encounter list
- [ ] **Technologies:**
  - [ ] Flask/FastAPI backend
  - [ ] WebSocket for real-time updates
  - [ ] HTML Canvas or terminal-in-browser for map

#### 5.3 Automated Testing
- [ ] **Run AI player in headless mode:**
  - [ ] Add `--headless` flag to disable SDL rendering
  - [ ] Run game at maximum speed
- [ ] **Batch testing:**
  - [ ] Run 100 games overnight
  - [ ] Collect statistics
  - [ ] Identify failure patterns
- [ ] **Regression testing:**
  - [ ] Ensure AI improvements don't break basic functionality
  - [ ] Compare metrics against baseline

---

## Phase 6: Advanced Features

**Goal:** Enable sophisticated gameplay strategies.

### Tasks:

#### 6.1 Multi-Agent Coordination (Future)
- [ ] Run multiple AI players in parallel
- [ ] Share learned knowledge between agents
- [ ] Evolve strategies via genetic algorithms

#### 6.2 Save/Load State Management
- [ ] Implement game save/load commands
- [ ] Checkpoint before risky actions
- [ ] Reload on death for learning

#### 6.3 Vision-Based AI (Future)
- [ ] Read game state from SDL screen instead of logs
- [ ] Use computer vision to parse dungeon display
- [ ] Enable AI to play other roguelikes

---

## Code Structure

### Recommended Directory Layout:
```
JMoria/
├── scripts/
│   ├── crawler.py              # Entry point: launch game + run bot loop
│   └── bot/
│       ├── __init__.py
│       ├── screen.py           # tmux capture-pane parser
│       ├── cmd.py              # tmux send-keys wrapper
│       ├── state.py            # GameState dataclasses
│       ├── pathfinding.py      # A* on ASCII map
│       ├── decision.py         # Rule-based AI logic
│       ├── llm_player.py       # (Phase 4) LLM-based decision making
│       └── metrics.py          # (Phase 5) Performance tracking
└── src/                        # Game source (no changes needed for Phase 1-2)
    ├── RenderASCII.cpp         # ASCII renderer (already merged)
    └── ...
```

### Entry Points:
- `python3 scripts/crawler.py` — launch and run the rule-based bot
- `python3 scripts/crawler.py --llm` — (Phase 4) use LLM for decisions
- `python3 scripts/crawler.py --runs 100` — (Phase 5) batch soak testing

---

## Implementation Priorities

### Milestone 1: Basic Automation
- ✅ ASCII renderer merged (PR#155)
- ✅ tmux interaction pattern established (see user memory / Issue #182)
- [ ] Bot harness: launcher, screen reader, command sender (Phase 1)
- [ ] Simple exploration AI: move toward unexplored tiles (Phase 2)
- [ ] **Success criteria:** Bot can navigate level 1 dungeon without dying

### Milestone 2: Survival AI
- [ ] A* pathfinding (Phase 2.2)
- [ ] Combat logic: bump-attack adjacent monsters, flee if low HP (Phase 2.3)
- [ ] HP management: rest when below 30% max HP
- [ ] **Success criteria:** Bot survives to level 3

### Milestone 3: Strategic Play
- [ ] Inventory management: auto-wield/equip (Phase 3.3)
- [ ] Multi-level strategy: when to descend (Phase 3.4)
- [ ] Exploration memory: track visited tiles per level (Phase 3.1)
- [ ] **Success criteria:** Bot reaches level 10

### Milestone 4: LLM Integration (Optional)
- [ ] LLM prompt template with ASCII map + stats context (Phase 4.1)
- [ ] Claude/GPT-4 API integration
- [ ] **Success criteria:** LLM bot reaches level 5

### Milestone 5: Batch / Soak Testing
- [ ] `--runs N` flag for repeated games with statistics
- [ ] Collect: deepest level, monsters killed, cause of death, turns survived
- [ ] **Success criteria:** 100 consecutive runs without hang or crash

### Stretch Goal: Level 100 (5000 ft)
- [ ] Advanced tactics: doorway fighting, kiting, item usage
- [ ] Perfect resource management
- [ ] **Success criteria:** Bot reaches dungeon level 100

---

## Technical Requirements

### Dependencies:
- **Python 3.8+** for AI player scripts
- **jq** for JSON parsing in shell scripts (already required by PR#157)
- **asyncio** for real-time log tailing
- **numpy** (optional) for map processing
- **openai** or **anthropic** SDK for LLM integration (optional)

### System Requirements:
- Linux/macOS with FIFO support
- SDL2, SDL2_image, OpenGL (already required)
- Named pipe support (POSIX)

### Testing Environment:
- Automated testing rig
- Headless mode for fast iteration
- Metric collection database (SQLite)

---

## Open Questions & Decisions Needed

1. **Command Input Method:** FIFO vs. stdin vs. socket?
   - **Recommendation:** Named FIFO (clean, non-invasive)

2. **AI Engine:** Rule-based vs. LLM vs. hybrid?
   - **Recommendation:** Start with rule-based, add LLM later

3. **Headless Mode:** Modify game to run without SDL window?
   - **Recommendation:** Yes, add `--headless` flag for batch testing

4. **Turn Speed:** Should AI mode run at full speed or match human pace?
   - **Recommendation:** Configurable via `--ai-speed` flag

5. **Logging Verbosity:** Should AI add its own decision logs?
   - **Recommendation:** Yes, create `ai-player/decisions-*.log`

---

## References

- **PR#157:** https://github.com/Rushwind13/JMoria/pull/157
- **AI Logging Documentation:** `thoughts/ai-logging.md` (from PR#157)
- **Architecture Documentation:** `thoughts/architecture.md` (from PR#157)
- **Developer Guide:** `_JMoria Developer's Guide.md`
- **Keyboard Commands:** `README.md`

---

## Notes

- This work list assumes PR#157 is merged to `develop` branch first
- All enhancements build upon the AI logging foundation
- External script approach is recommended for rapid iteration
- LLM integration is optional but enables more adaptive gameplay
- Ultimate goal is reaching level 100, but intermediate milestones provide value

---

## Contributing

When working on AI player features:
1. Follow branching strategy in Developer Guide (`username/feat/feature-name`)
2. Test with existing analysis scripts from PR#157
3. Document AI decision logic in comments
4. Add metrics for evaluating performance
5. Create PRs for each milestone

**Questions?** See Developer's Guide or open a GitHub issue.
