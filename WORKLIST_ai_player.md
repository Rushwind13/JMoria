# AI Player System - Work List

## Overview

This work list defines the implementation of an autonomous dungeon crawler bot for JMoria. See also **Issue #182** for the design rationale.

---
## 2026-04-09: Current AI Player Issue Priorities

### Closed Issues

**[#194] Refactor: Remove dead or redundant code in bot Python scripts (DONE)**
- Deep clean completed: removed 311 lines of dead code from decision.py.

**[#203] Refactor decision.py — remove dead and redundant code (post-goal-stack) (DONE)**
- Full cleanup pass: removed 755 net lines (all unused wall-follow remnants, dead helpers, duplicate logic).
- Added `scripts/bot/BOT_ARCHITECTURE.md` documenting the full decision flow.
- Commit: `5be993e`.

**[#191] Bot gets stuck in CCW loop and can't traverse doors (DONE)**
- Fixed: door priority in wall-follow scan + door_momentum flag.

**[#192] AI bot should prioritize doors as exits to escape rooms (DONE)**
- Addressed by #191 fix.

**[#193] AI bot should avoid re-entering doorways and tiles (DONE)**
- Lap-aware exploration with world-coordinate visited tracking.

**[#195] Bot should explore room interiors and return to wall-following (DONE)**
- Interior item seeking + wield prompt detection.

**[#190] Combat and wield actions corrupt tmux/curses screen (DONE)**
- ASCII renderer DrawTextBoundingBox() fills interiors with spaces.

**[#189] 1st level characters can start with 1 HP (DONE)**
- FIXED in commit c2bcdc4. Closed.

**[#186] Bot behavior: exploration redesign — goal-based pathfinding (DONE)**
- Old wall-follow + lap counting replaced with LIFO goal-stack + A* pathfinding.
- Commits: `50e5661` (landmark), `54676d0` (direction-commit + item promotion + bravery).
- Results: 1079 turns, 534 unique positions. No oscillation.

**[#185] Bot bug: death not reliably detected (DONE)**
- Fixed: `lost_player_turns` counter, tombstone detection, HP=0 detection. Reliable in all tests.

**[#209] Bot: recalculate exploration frontier when Scroll of Light is read (DONE)**
- Stale unexplored goals pruned in `_update_exploration_sets()` when frontier rebuilds.
- Commit: `6c8dff6`.

**[#210] Bot: hallway item pickup causes unnecessary backtracking (DONE)**
- Stale unexplored goal under item now chains forward in travel direction instead of silently discarding.
- Commit: `6c8dff6`.

**[#211] Bot: lower priority of opening closed doors until ready to path through (DONE)**
- Removed first-class "door" goals. Closed doors remain in exploration frontier as targets.
- Path-following code opens doors en route. `+` moved from SOLID to passable-but-expensive (DOOR_COST=3).
- Commit: `6c8dff6`.

**[#212] Bot: goal stack should never be empty (DONE)**
- Staircase always pushed when no unexplored tiles remain (safety gate at arrival only).
- Falls back to wander toward center, then stuck_fallback.
- Commit: `6c8dff6`.

**[#199] Bot walks past items on the floor instead of picking them up (DONE)**
- Closed by owner.

**[#202] Deep refactor knowledge.json — fix errors and clean stale data (DONE)**
- Closed by owner.

**[#201] Bot goal stack ordering causes inefficient backtracking (CLOSED)**
- Superseded by #213.

### Open Issues

**[#213] Bot: goal stack needs better ordering — "happy wanderer" exploration**
- BFS target selection causes breadth-first wandering instead of depth-first committed exploration.
- Bot ping-pongs between frontiers, partially explores areas, skips doors at hallway ends.
- Possible fixes: DFS-style directional bias, frontier clustering, door proximity boost.

**[#215] Items visible through walls and closed doors**
- Items render on tiles the player has no LOS to, giving the bot (and player) information they shouldn't have.
- Bot paths toward items through expensive unknown territory or walls it can't reach.

**[#214] Dungeon gen: isolated hallway segments with no exits**
- Player can spawn in a sealed 1x6 hallway surrounded entirely by walls with no doors.
- Level is unplayable — no way to explore, fight, or descend.

**[#200] Bot leaves parts of dark rooms unexplored**
- Tile-by-tile discovery in dark rooms misses interior sections.
- May need systematic sweep pattern or room-awareness to prioritize local unexplored tiles.

**[#207] Reveal bot agent's thinking with low-noise visualization**
- Add visual overlay or log mode showing bot decision state without cluttering gameplay view.

**[#206] Bot crawler log depth parameter is incorrect**
- Depth reporting in bot logs doesn't match actual dungeon depth.

**[#204] Do not use wizard mode in the AI player bot**
- Bot should play without wizard-mode commands for realistic soak testing.

**[#188] Bot parser likely overcounts visible monsters/items in ASCII viewport**
- Noisy entity counts affect threat scoring and decision heuristics.

**[#196] Dungeon gen: two hallways can "sidle" creating double-wide corridors**
- Two hallways placed adjacent share a wall, creating double-wide corridors with floating doorways.
- Related to #214 — both are dungeon generation connectivity issues.

**[#167] Monsters don't draw / draw as invisible / draw as holes**
- `DrawDungeon()` skips base tile when monster present; ASCII mode creates holes if `DrawMonsters()` also fails.

**[#198] Yellow Icky Thing rendered in green**
- Color mismatch in monster rendering.

**[#173] Items disappear at wrong view distance during gameplay**
- Item visibility drops off at incorrect distance.

### Non-bot open issues (game engine)

- **[#197]** Player stats
- **[#181]** Reduce dead-end hallways in dungeon generation
- **[#180]** Dump full dungeon map to screen after tombstone
- **[#177]** Conditionalize test linker flags by RENDER_MODE
- **[#176]** Move srand/time seed from main.cpp into Game::Init()
- **[#175]** Replace strcmp with jstrcmp in main.cpp CLI parsing
- **[#172]** Death screen tombstone: long monster names overflow the border
- **[#171]** No in-game hint when bumping into closed doors
- **[#170]** Wield command accepts non-weapon items without restriction
- **[#166]** Wand of Light does not light tiles
- **[#165]** Telepathy lasts forever
- **[#156]** AI log combat 'killed' flag incorrect when monster HP reaches 0
- **[#128]** Magic Weapons — normal, cursed, magic, ego, unique
- **[#121]** Fuel for lanterns, charges for wands
- **[#117]** Search command and search ability
- **[#114]** Identify spell
- **[#112]** Add races

---

### #186 Exploration Redesign: Goal-Based Pathfinding (COMPLETED)

**Implementation**: LIFO goal stack with A* pathfinding. Commits `50e5661` + `54676d0`.

**Goal hierarchy** (constant, every level):
1. See the whole map (visit every reachable tile)
2. Kill monsters (or flee if outmatched)
3. Collect and evaluate items/equipment
4. Find and take the down staircase

**Room exploration** depends on lighting:

**Dark room:**
- Wall-follow the perimeter to reveal room boundaries, doors, and walls
- Stripe the interior in rows/columns to reveal every floor tile
- Now you can see monsters, items, and exits — handle them

**Lit room:**
- You already see everything. Skip perimeter/stripe entirely
- Pathfind directly to monsters, items, and doors

**After a room is fully revealed:**
- Fight all visible monsters (pathfind to them)
- Pick up all items (pathfind to them)
- Evaluate gear: wield/wear upgrades, identify unknowns by use
- Room is "cleared"

**Choosing where to go next:**
- Look at all known exits (doors, open doorways, hallway openings)
- Prefer exits leading to unvisited territory
- If multiple choices are equal, use a consistent tiebreaker (e.g. always pick left/clockwise)
- Pathfind directly to the chosen exit — no wall-following in known territory

**In a hallway:**
- Follow it to the next room. Pathfind to the far end.

**Overall map strategy:**
- The bot desires to walk every tile on the map
- `visited_world` tracks where it's been
- Always pathfind toward the nearest unvisited tile when there's no higher-priority task (monster, item)
- Wall-follow is a tool for dark-room perimeter revelation only, not the core navigation strategy
- In a perfect game, the bot would never visit any square twice

**Key principles:**
- Pathfind in known/lit areas; wall-follow only in the dark
- When choosing between equal paths, pick consistently (e.g. leftmost) to disambiguate
- `_is_hallway()` helper: checks 3x3 around a tile; ≤2 open neighbours = corridor

**Implementation tasks:**
1. Revert current wall-follow patch complexity (lap counting, breakout, stale timers)
2. Add `_is_hallway()` and room-vs-hallway detection
3. Replace explore phases: `head_east` → `reveal_room` → `clear_room` → `choose_exit` → `navigate`
4. `reveal_room`: if dark, wall-follow perimeter then stripe interior; if lit, skip
5. `clear_room`: pathfind to monsters, then items, then evaluate gear
6. `choose_exit`: scan for unvisited exits, pathfind to best one
7. `navigate`: A* to target exit/tile, follow hallways to next room

---
### Notes
- This list is up to date with current code and open issues as of 2026-04-09.
- See commit c2bcdc4 for the fix to #189 (closed).
- #191 and #192 done in commit 941ec0b (closed).
- #193 done in commit e84bf5d (closed). #194 closed.
- #195 done in commit ce3d4c5 (closed).
- #190 done in commit 140d0c6 (closed).
- #186 done in commits 50e5661 + 54676d0 (closed). Goal-stack exploration with A* pathfinding.
- #185 done: death detection reliable via lost_player_turns + tombstone + HP=0 (closed).
- #203 done in commit 5be993e (closed). decision.py stripped to active behavior only; BOT_ARCHITECTURE.md added.
- #209, #210, #211, #212 done in commit 6c8dff6 (closed). Exploration model fixes: stale goals, door lifecycle, empty stack.
- #201 superseded by #213 (closed).
- Best test result: 1079 turns, 534 unique positions, depth 1.
- #199 closed by owner. #202 closed by owner.
- Next priorities: #213 (goal ordering), #215 (items through walls), #214 (isolated hallways), #200 (dark rooms), #188 (parser overcounts).
- Last updated: 2026-04-09.


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

## Current PR Focus (Active) — Milestone 1: Survive the Floor

### Completed: Wall-Follow Exploration Rewrite (fc2b245)
- [x] Remove all circular movement: BFS frontier, right-hand cardinal wall-follow, patrol cycle / oscillation detection, pivot/escape mechanisms, tile scoring, 15+ state variables
- [x] New two-phase exploration: head east → clockwise left-hand-rule wall-follow (8 directions)
- [x] Wall-follow opens doors on wall side, searches on lap completion
- [x] Bot reliably explores dungeon perimeter/hallways, fights, picks up items, equips gear

### Task 1: Deep Clean of Legacy Decision Code (DONE)
- [x] Audit `decision.py` for dead code left over from old pathfinding (removed methods still referenced, unused state vars, orphan helpers)
- [x] Remove `_is_blocked_dir`, `_blocked_dirs_by_world`, `wall_bump_chain`, `follow_open_dir_turns`, `avoid_key`/`avoid_key_turns`, `cycle_escalation` — no longer used by wall-follow
- [x] Flatten decide() — removed dead variable assignments for removed mechanisms
- [x] Verify all remaining helper methods are actually called; deleted the rest
- Removed 18 dead methods/functions and 7 dead instance variables (311 lines net)

### Task 2: Room-Loop-Then-Exit Strategy
- [ ] When wall-follow enters a room (detected by >2 walkable neighbors on multiple sides), complete one full perimeter loop to see all walls
- [ ] Track room entry point (first doorway/hallway tile entered)
- [ ] On lap completion (return to entry point), take the first *unvisited* hallway/door exit
- [ ] If all exits visited, continue wall-follow through the most recently opened exit

### Task 3: Room Interior Fill ("Paint Stripes")
- [ ] After room perimeter is mapped (Task 2 loop), switch to interior fill mode
- [ ] Walk 3-row-wide horizontal stripes across the room (east→west, step 3 south, west→east, etc.)
- [ ] Stripe walking reveals items, stairs, and monsters in the room center
- [ ] Return to wall-follow after fill is complete
- [ ] Track fill status per room so revisits skip already-filled rooms

### Task 4: Restore Core Survival Behaviors
- [ ] Pick up loot: walk to visible items on or near the wall-follow path
- [ ] Wield/equip: auto-equip best weapon, armor, and light source from inventory (already partially working)
- [ ] Avoid monsters: flee from unknown uppercase-glyph monsters when HP < 80%; engage known-weak monsters
- [ ] Find staircases: when `>` is visible, pathfind to it after current room is explored
- [ ] Descend: step on `>` and press `>` to go down when ready (HP > 70%, gear equipped)

### Task 5: Level 2 Readiness (100' depth)
- [ ] Track XP and player level from stats panel
- [ ] Don't descend until player has reached character level 2 (enough XP from combat on level 1)
- [ ] Reset wall-follow state on depth change (explore_phase → head_east, clear lap tracking)
- [ ] Verify bot survives depth transition and resumes wall-follow on level 2

---

### Earlier PR Milestones (Completed)

- [x] Verbose logs now prioritize bot reasoning (`think=`) over raw message text
- [x] Verbose logs no longer show viewport-local player position
- [x] Repeated message text is marked as persistence (`<same for N turns>`) instead of event multiplicity
- [x] Stuck detection switched to world position when available
- [x] Bot now auto-enforces panel visibility (`i`, `e`, `C`) when parsing detects missing panels
- [~] Task 1 parser hardening in progress:
  - [x] Use canonical `MonIDs` / `ItemIDs` from source
  - [x] Filter text-like/overlay-like glyphs in dungeon parsing
  - [x] Add parser reject counters for debug telemetry
  - [ ] Validate reject counters across multiple long runs and tune thresholds
- [x] Task 2: further reduce combat-zone patrol loops (#186)
  - [x] Prioritize door exploration (`path_to_door`, `open_adjacent_door`) before generic frontier roam
  - [x] Add long wall-bump-chain breaker (force broader escape after repeated `wall_bump_pivot`)
  - [x] Blacklist unreachable item goals after 3 wall-bump failures (commit a2582a4)
- [x] Task 3: auto-wield/equip best available weapon
  - [x] Parse inventory slot-letter entries from right panel
  - [x] Auto-issue `w` + slot for all equip-candidate carried items (not just one weapon)
  - [x] Treat torches/lanterns as high-priority utility equipment
  - [x] Learn non-wieldable items from feedback (e.g., "You can't wield a ...") and avoid retries
  - [x] Persist learned non-wieldable item knowledge to JSON config across runs
  - [x] Persist monster observations (hit/miss/kill/attack-type) across runs
  - [x] Persist scroll label -> observed effect notes across runs
  - [x] Category-based wieldability + consumable usage (commit e2c3fe9)
  - [x] Gear comparison with swap-back mechanism (commit 925570d)
  - [ ] Refine weapon ranking using real item damage metadata

## Knowledge Roadmap (Persistent Learning)

- [x] Monster empirical danger scoring from observed HP loss and attack pressure
- [x] Use learned monster danger + confidence directly in flee/engage decisions
- [ ] Item slot semantics + equip compatibility memory (what can be worn/wielded where)
- [ ] Light economy memory (torch/lantern uptime, low-light risk windows)
- [x] Door/lock memory (pick success/failure stats and retry profile)
- [ ] Time-to-kill and hit-difficulty memory by monster
- [x] Scroll label confidence memory (label -> observed effect notes)
- [ ] Equipment outcome memory from stat deltas (AC/To-hit/To-dam impact)
- [ ] Per-depth map notes (chokes, dead-ends, stair positions, danger zones)

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
- ✅ Bot harness: launcher, screen reader, command sender (Phase 1)
- ✅ Simple exploration AI: move toward unexplored tiles (Phase 2)
- [~] **Success criteria:** Bot can navigate level 1 dungeon without dying
  - Bot explores, picks up items, opens doors, and fights — but still dies to early monsters

### Milestone 2: Survival AI
- ✅ A* pathfinding (Phase 2.2)
- ✅ Combat logic: bump-attack adjacent monsters, flee if low HP (Phase 2.3)
- ✅ HP management: rest when below 30% max HP
- [ ] **Success criteria:** Bot survives to level 3

### Milestone 3: Strategic Play
- ✅ Inventory management: auto-wield/equip (Phase 3.3)
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
