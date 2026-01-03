# AI Analysis Scripts

Shell scripts and Python tools for analyzing JMoria game sessions from AI logs.

## Prerequisites

- `bash` 4.0+
- `python3` (for deep-dive.py)

## Log Format

JMoria uses a human-readable text format (`.log` files):

```
SESSION_START 2026-01-03T10:44:41

TURN 1

DUNGEON turn:1 level:1 depth:50ft
  VIEW 57,0 21x19
  PLAYER 67,8 hp:9/9
  MAP
    0: #9@.3#7
    7: #5'#8
  ENDMAP
  ROOMS 4
  HALLS 6
  MON Green_Jelly J 70,21 hp:6
  ITEM Dagger ( 54,23
ENDDUNGEON

MOVE player from:67,8 to:68,8 result:floor

COMBAT attacker:Player defender:Orc hit:true damage:3 hp:4 killed:false

ITEM pickup name:Dagger count:1 pos:45,24

DEATH killed_by:Clear_Icky_Thing level:2 pos:82,14

SESSION_END 2026-01-03T10:45:22
```

## Scripts

### deep-dive.py (Recommended)

Full session analysis with detailed statistics.

```bash
./deep-dive.py                    # most recent session
./deep-dive.py session-015        # partial name match
./deep-dive.py --map              # include map snapshots
./deep-dive.py --timeline         # include event timeline
```

Output includes:
- Event counts overview
- Level progression
- Dungeon layout (rooms, hallways)
- Items collected
- Monster activity and kills
- Combat statistics
- Player HP tracking
- Outcome (survived/died)

### list-sessions.sh

List all game sessions with basic stats.

```bash
./list-sessions.sh
```

Output:
```
=== GAME SESSIONS ===

Session                                       Turns     Size Levels
-------                                       -----     ---- ------
session-015-2026-01-03T10-44-41.log            553     21K 1,2,3
session-014-2026-01-03T09-30-22.log             64    141K 1
```

### summarize-session.sh

Get a summary of a game session.

```bash
./summarize-session.sh                    # most recent session
./summarize-session.sh session-015        # partial name match
./summarize-session.sh path/to/file.log   # specific file
```

### show-state.sh

Show the current game state (most recent dungeon snapshot).

```bash
./show-state.sh                    # most recent session
./show-state.sh session-015        # specific session
```

Output includes:
- Turn number and dungeon level
- Player position and HP
- ASCII map (decoded from RLE)
- Visible monsters with HP
- Items on ground

### monitor-live.sh

Watch a game session in real-time.

```bash
./monitor-live.sh              # all events
./monitor-live.sh brief        # one-line summaries
./monitor-live.sh combat       # only combat events
./monitor-live.sh move         # all movement
./monitor-live.sh player       # player movement only
./monitor-live.sh monster      # monster movement only
./monitor-live.sh dungeon      # dungeon headers
./monitor-live.sh item         # item interactions
./monitor-live.sh "Icky Thing" # custom grep filter
```

**Brief mode output:**
```
--- Turn 42 ---
Player -> (45,37) (floor)
  Yellow Icky Thing -> (35,44)
COMBAT: Player hits Opal Icky Thing for 2 dmg (HP: 8)
ITEM: pickup Dagger
```

## Quick One-Liners

```bash
# Count turns in most recent session
grep -c '^TURN ' ai-logs/session-*.log | tail -1

# Watch player HP over time
grep '  PLAYER' ai-logs/session-*.log | grep -o 'hp:[0-9]*/[0-9]*'

# List all monsters encountered
grep '^COMBAT ' ai-logs/session-*.log | sed 's/.*defender:\([^ ]*\).*/\1/' | tr '_' ' ' | sort -u

# Count kills by monster type
grep 'killed:true' ai-logs/session-*.log | sed 's/.*defender:\([^ ]*\).*/\1/' | sort | uniq -c

# Get items on current level
grep '  ITEM ' ai-logs/session-*.log | tail -10

# Find when player took damage
grep 'defender:Player.*hit:true' ai-logs/session-*.log
```

## For AI Assistants

When helping a user play JMoria:

1. **Start monitoring:** `./monitor-live.sh brief`
2. **Check current state:** `./show-state.sh`
3. **Deep analysis:** `./deep-dive.py --map`
4. **After session ends:** `./summarize-session.sh`

### Event Types

| Event | Description |
|-------|-------------|
| `SESSION_START/END` | Session boundaries |
| `TURN` | New turn started |
| `DUNGEON ... ENDDUNGEON` | Full game state (multi-line block) |
| `MOVE player/monster` | Movement events |
| `COMBAT` | Attack with hit/damage/killed |
| `ITEM` | Pickup/drop/use |
| `LEVEL_CHANGE` | Stairs used |
| `DEATH` | Player died |

### Key Fields

- `killed:true` in COMBAT = monster was killed
- `hp:X/Y` in PLAYER = current/max HP
- Map uses RLE encoding: `#9` = 9 walls, `.3` = 3 floors
- Names use underscores: `Green_Jelly` (display as "Green Jelly")
