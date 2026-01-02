# AI Analysis Scripts

Shell scripts for analyzing JMoria game sessions from AI logs.

## Prerequisites

- `jq` - JSON processor (install: `apt install jq` or `brew install jq`)
- `bash` 4.0+

## Scripts

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
session-001-2026-01-02T15-10-51.jsonl           553     21K 1,2,3
session-2026-01-02T15-05-30.jsonl                64    141K 1
```

### summarize-session.sh

Get a full summary of a game session.

```bash
./summarize-session.sh                    # most recent session
./summarize-session.sh session-001        # partial name match
./summarize-session.sh path/to/file.jsonl # specific file
```

Output includes:
- Duration (turns)
- Levels explored
- Items collected
- Combat encounters
- Monsters defeated
- Final player state
- Outcome (survived/died)

### show-state.sh

Show the current game state (most recent log entry).

```bash
./show-state.sh                    # most recent session
./show-state.sh session-001        # specific session
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
./monitor-live.sh              # all events (pretty JSON)
./monitor-live.sh brief        # one-line summaries
./monitor-live.sh combat       # only combat events
./monitor-live.sh move         # all movement
./monitor-live.sh player       # player movement only
./monitor-live.sh monster      # monster movement only
./monitor-live.sh dungeon      # dungeon state (compact)
./monitor-live.sh item         # item interactions
./monitor-live.sh "Icky Thing" # custom grep filter
```

**Brief mode output:**
```
--- Turn 42 ---
Player -> (45,37) (floor)
  Yellow Icky Thing -> (35,44)
COMBAT: Player hits Opal Icky Thing for 2.44 dmg (HP: 8.64)
ITEM: pickup Dagger
```

## Quick One-Liners

```bash
# Count turns in most recent session
grep -c '"type":"turn"' ai-logs/session-*.jsonl | tail -1

# Watch player HP over time
grep '"type":"dungeon"' ai-logs/session-*.jsonl | jq '.player.hp'

# List all monsters encountered
grep '"type":"combat"' ai-logs/session-*.jsonl | jq -r '.defender' | sort -u

# Count deaths by monster type
grep '"defender_hp":0' ai-logs/session-*.jsonl | jq -r '.defender' | sort | uniq -c

# Get items on current level
grep '"type":"dungeon"' ai-logs/session-*.jsonl | tail -1 | jq '.items[].name'

# Find when player took damage
grep '"defender":"Player"' ai-logs/session-*.jsonl | jq '{turn: .turn, damage, attacker}'
```

## For AI Assistants

When helping a user play JMoria:

1. **Start monitoring:** `./monitor-live.sh brief`
2. **Check current state:** `./show-state.sh`
3. **After session ends:** `./summarize-session.sh`

The logs use JSON Lines format - each line is valid JSON. Key event types:
- `turn` - new turn started
- `dungeon` - full game state (map, monsters, items)
- `player_move` / `monster_move` - movement
- `combat` - attacks (check `defender_hp` for kills, not `killed` flag due to Bug #156)
- `item` - pickup/drop/use
