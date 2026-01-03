# AI Player System - Work List

## Overview

This work list defines the enhancements needed to enable automated gameplay in JMoria. The foundation is **PR#157** (AI Observability Logging System), which provides JSON Lines logging of game state. This document outlines the next steps to create an autonomous AI player that can read logs, make decisions, and send commands to reach level 100 in the dungeon.

**Goal:** Create an AI agent that can play JMoria autonomously by parsing AI logs and sending keystroke commands, gaining experience, collecting items, and reaching dungeon level 100 (5000 ft depth).

---

## Foundation: PR#157 - AI Observability Logging

### Status: Open (ready for merge)
**Branch:** `Rushwind13/feat/ai-observability`

### What PR#157 Provides:
- ✅ JSON Lines logging to `ai-logs/session-NNN-TIMESTAMP.jsonl`
- ✅ Complete dungeon state every turn (map, monsters, items, player)
- ✅ Player movement logging (position, result)
- ✅ Monster movement logging (position, AI state)
- ✅ Combat logging (attacker, defender, damage, HP)
- ✅ Item interaction logging (pickup, drop events)
- ✅ Size optimizations (RLE compression, viewport clipping, ~2.2 KB/turn)
- ✅ Session management (sequential numbering, auto-archiving)
- ✅ Analysis shell scripts (`monitor-live.sh`, `summarize-session.sh`, etc.)
- ✅ Comprehensive documentation (`thoughts/ai-logging.md`)

### Key Event Types:
| Event Type | Information Provided |
|-----------|---------------------|
| `session_start` | Game launched |
| `session_end` | Game terminated |
| `turn` | Turn counter increment |
| `dungeon` | Complete game state (21x21 viewport, RLE-encoded map) |
| `player_move` | Player position change, collision result |
| `monster_move` | Monster position change, AI state |
| `combat` | Attack details (hit/miss, damage, HP) |
| `item` | Item pickup/drop with position |

### Known Issues from PR#157:
- ⚠️ **Bug #156:** Combat `killed` flag reports `false` when monster HP = 0
  - Workaround: Check `defender_hp == 0` instead of `killed` flag

---

## Architecture Options

### Option A: External Script Player (Recommended First)
An external Python/Bash script that:
1. Launches `jmoria` as a subprocess
2. Tails the AI log file (`ai-logs/session-*.jsonl`)
3. Parses dungeon state on each turn
4. Makes decisions using rule-based or LLM logic
5. Sends keystrokes to the game via stdin or automation tools

**Pros:**
- No changes to game code
- Easy to iterate and test
- Can use any language/framework
- Can integrate with LLMs easily

**Cons:**
- Requires keystroke injection (tmux, xdotool, etc.)
- Slight delay between log output and command input

### Option B: LLM Chat Function
A structured function/tool that LLMs (Claude, GPT-4, etc.) can call:
1. Function provides current game state from log
2. LLM analyzes state and returns command
3. Function sends command to game
4. Repeat

**Pros:**
- Leverages LLM reasoning capabilities
- Natural language decision-making
- Can explain decisions

**Cons:**
- Slower (API latency)
- Requires API integration
- Token costs for long games

### Option C: Embedded AI Player Mode
Add a new game mode that runs AI logic inside the game loop:
1. Add `--ai-player` command-line flag
2. AI decision function called each turn in `CmdState`
3. Generates keystroke events internally
4. Still logs to AI logs for observability

**Pros:**
- Fastest execution
- No external coordination needed
- Full access to game internals

**Cons:**
- Requires C++ changes to game
- Harder to iterate on AI logic
- Couples AI to game codebase

---

## Phase 1: Command Injection Infrastructure

**Goal:** Enable external scripts to send commands to a running `jmoria` instance.

### Tasks:

#### 1.1 Research Command Input Methods
- [ ] **Document current keyboard input handling**
  - Location: `src/CmdState.cpp`, `src/Game.cpp`
  - Input path: SDL events → `CGame::HandleEvents()` → `CStateBase::HandleKey()`
- [ ] **Evaluate command injection approaches:**
  - [ ] Option 1: Named pipe (FIFO) for command input
  - [ ] Option 2: Stdin command mode (read from cin if not a TTY)
  - [ ] Option 3: Socket/network interface (TCP or Unix domain socket)
  - [ ] Option 4: Shared memory + signal
  - [ ] Option 5: External automation (xdotool, tmux send-keys)

#### 1.2 Implement Command Input Channel (Choose One)
**Recommended: Named Pipe (FIFO)**
- [ ] Create `ai-commands/commands.fifo` at startup
- [ ] Add non-blocking read from FIFO in `CGame::HandleEvents()`
- [ ] Parse single-character commands from pipe
- [ ] Convert to SDL key events internally
- [ ] Add `--ai-mode` flag to enable this behavior
- [ ] Test: `echo "hjkl" > ai-commands/commands.fifo` moves player

**Files to modify:**
- `src/main.cpp` - Initialize FIFO, parse `--ai-mode` flag
- `src/Game.cpp` - Add FIFO reading to `HandleEvents()`
- `src/Game.h` - Add FIFO file handle member

#### 1.3 Command Format Specification
- [ ] **Define command protocol:**
  - Single-character commands (matches keyboard: `h`, `j`, `k`, `l`, etc.)
  - Optional JSON format for complex commands: `{"cmd": "move", "dir": "north"}`
- [ ] **Document all valid commands in AI mode:**
  - Movement: `hjklyubn` (vi-keys), `HJKLYUBN` (run)
  - Actions: `o` (open), `c` (close), `T` (tunnel), `<>` (stairs)
  - Items: `w` (wield), `t` (take off), `d` (drop), `q` (quaff), `r` (read), `z` (zap)
  - Targeting: `*` (target), `:` (look)
  - Rest: `.` (rest one turn), `R` (rest until full HP)
  - Wizard: `^t` (teleport), `^i` (create item), `^s` (summon monster)
- [ ] **Add command validation and error logging**

---

## Phase 2: Basic AI Decision Engine

**Goal:** Create a simple rule-based AI that can navigate dungeons and survive.

### Tasks:

#### 2.1 AI Log Parser
- [ ] **Create Python module `ai_player/log_parser.py`**
  - [ ] Parse JSON Lines format
  - [ ] Tail log file in real-time
  - [ ] Maintain current game state (player HP, position, level)
  - [ ] Track visible monsters (name, position, HP)
  - [ ] Track visible items (name, position)
  - [ ] Decode RLE-encoded map to 2D array
- [ ] **Create state representation class:**
  ```python
  class GameState:
      turn: int
      level: int
      player_pos: tuple[int, int]
      player_hp: int
      player_max_hp: int
      map: list[list[str]]  # 21x21 decoded map
      monsters: list[Monster]
      items: list[Item]
      last_action_result: str
  ```

#### 2.2 Pathfinding Module
- [ ] **Implement A\* pathfinding on decoded map**
  - [ ] Walkable terrain detection (`.`, `'`, `<`, `>`)
  - [ ] Obstacle avoidance (`#`, `+`, `:`)
  - [ ] Monster-aware pathing (avoid or target)
- [ ] **Utility functions:**
  - [ ] `find_nearest_item(state) -> Item | None`
  - [ ] `find_nearest_stairs(state, direction='down') -> tuple[int,int] | None`
  - [ ] `find_safe_position(state) -> tuple[int,int]` (away from monsters)
  - [ ] `is_path_clear(state, from_pos, to_pos) -> bool`

#### 2.3 Basic Decision Logic
- [ ] **Create decision tree for survival:**
  ```python
  def decide_action(state: GameState) -> str:
      # Priority 1: Survive
      if state.player_hp < state.player_max_hp * 0.3:
          return rest_or_flee(state)
      
      # Priority 2: Combat if necessary
      if nearby_monsters(state):
          return engage_or_flee(state)
      
      # Priority 3: Explore
      if unexplored_areas(state):
          return explore(state)
      
      # Priority 4: Descend
      if can_find_stairs(state):
          return go_to_stairs(state)
      
      # Default: Rest
      return '.'
  ```

- [ ] **Implement sub-decision functions:**
  - [ ] `rest_or_flee(state)` - Heal when low HP
  - [ ] `engage_or_flee(state)` - Fight or run based on monster strength
  - [ ] `explore(state)` - Move toward unexplored areas
  - [ ] `go_to_stairs(state)` - Navigate to stairs down
  - [ ] `pickup_items(state)` - Collect nearby valuable items

#### 2.4 Command Sender
- [ ] **Create Python module `ai_player/command_sender.py`**
  - [ ] Open FIFO for writing: `ai-commands/commands.fifo`
  - [ ] Send single-character commands
  - [ ] Handle FIFO errors gracefully
  - [ ] Add command rate limiting (max 10/second)

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
├── ai-commands/
│   └── commands.fifo           # Named pipe for AI commands
├── ai-logs/                     # JSON game state logs (from PR#157)
│   └── session-*.jsonl
├── ai-player/
│   ├── __init__.py
│   ├── log_parser.py           # Parse AI logs
│   ├── command_sender.py       # Send commands via FIFO
│   ├── pathfinding.py          # A* pathfinding
│   ├── decision_engine.py      # Rule-based AI logic
│   ├── llm_player.py           # LLM-based decision making
│   ├── game_state.py           # State representation
│   ├── metrics.py              # Performance tracking
│   └── config.py               # AI configuration
├── ai-scripts/                  # Analysis scripts (from PR#157)
│   ├── monitor-live.sh
│   ├── summarize-session.sh
│   └── show-state.sh
└── src/                         # Game source code
    ├── main.cpp                 # Add --ai-mode flag
    ├── Game.cpp                 # Add FIFO reading
    ├── CmdState.cpp             # Command handling
    └── AILog.cpp                # Logging (from PR#157)
```

### Entry Point Scripts:
- `scripts/run_ai_player.py` - Launch game + AI player loop
- `scripts/run_llm_player.py` - Launch game + LLM decision loop
- `scripts/benchmark_ai.py` - Run automated testing

---

## Implementation Priorities

### Milestone 1: Basic Automation (Weeks 1-2)
- ✅ Merge PR#157 (AI logging)
- [ ] Implement FIFO command input (Phase 1)
- [ ] Create basic AI log parser (Phase 2.1)
- [ ] Implement simple movement AI (Phase 2.3 - exploration only)
- [ ] **Success criteria:** AI can navigate level 1 dungeon without dying

### Milestone 2: Survival AI (Weeks 3-4)
- [ ] Add pathfinding (Phase 2.2)
- [ ] Implement combat logic (Phase 2.3 - engage/flee)
- [ ] Add HP management (rest when low)
- [ ] **Success criteria:** AI can survive to level 3

### Milestone 3: Strategic Play (Weeks 5-6)
- [ ] Implement inventory management (Phase 3.3)
- [ ] Add multi-level strategy (Phase 3.4)
- [ ] Track explored areas (Phase 3.1)
- [ ] **Success criteria:** AI can reach level 10

### Milestone 4: LLM Integration (Weeks 7-8)
- [ ] Create LLM prompt template (Phase 4.1)
- [ ] Integrate Claude/GPT-4 API (Phase 4.1)
- [ ] Test LLM decision quality
- [ ] **Success criteria:** LLM AI reaches level 5

### Milestone 5: Optimization (Weeks 9-10)
- [ ] Add monitoring dashboard (Phase 5.2)
- [ ] Run batch testing (Phase 5.3)
- [ ] Tune AI parameters
- [ ] **Success criteria:** AI reaches level 20+ consistently

### Stretch Goal: Level 100 (Weeks 11+)
- [ ] Advanced combat tactics
- [ ] Perfect resource management
- [ ] Adaptive learning from failures
- [ ] **Success criteria:** AI reaches level 100 (5000 ft depth)

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
