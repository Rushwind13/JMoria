Focusing on #181 (dead-end hallways) and #196 (hallway sidling).
PR: https://github.com/Rushwind13/JMoria/pull/216

## Build & Run Commands
- Build: `make clean && make ascii`
- Run: `tmux new-session -d -s crawler -x 200 -y 50 './jmoria 2>/tmp/jmoria_map.txt'`
- Capture: `tmux capture-pane -t crawler -p`
- Kill: `tmux kill-session -t crawler`
- View map: `grep -A 100 "\[MAP\]" jmoria.log | head -105`
- View diagnostics: `grep "DUNGEN" jmoria.log`

## Completed Work

### Task 1: Hallway-to-room fallback directions (P0) ✅
Already existed in develop as `TryCreateRoomWithFallback()` — tries primary + both perpendicular directions.

### Task 2: TruncateHallway (P0) ✅
Already existed in develop as `TruncateHallway()` — shortens hallways that collide with existing geometry and places connecting door.

### Task 3: ConnectAdjacentStructures for hallways (#196) (P1) ✅
Removed room-only gate so `ConnectAdjacentStructures()` runs for both rooms AND hallways. Fixes sidling (#196).

### Additional: MAX_TRIES 2→10 ✅
Rooms/hallways now get 10 placement attempts instead of 2.

### Additional: Hallway fallback branching ✅
When `TryCreateRoomWithFallback()` fails, branch 2-3 child hallways instead of dead-ending.

## Remaining Work

### Task 4: Dead-end pruning pass (P1) — NOT STARTED
Post-generation scan: find hallway tiles with only one open neighbor (dead ends), erase them back to wall, walking back until hitting a junction or door.

### Task 5: Anti-sidle hallway placement check (P2) — NOT STARTED
In `CheckBorder()`, reject hallway placement if border scan finds FLOOR tiles (not just WALL/DOOR). Prevents parallel hallways from being placed.

### Task 6: Force terminal rooms (P2) — NOT STARTED
At `MAX_RECURDEPTH`, try placing a small (2×2 or 3×3) room instead of giving up entirely.