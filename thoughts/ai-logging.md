# AI Observability Logging

## Overview

JMoria includes a structured logging system designed for AI assistants (like Claude) to observe and understand game state during play sessions. Logs are written in JSON Lines format to `ai-logs/` directory.

**Quick Start for AIs:**
```bash
# Watch a game session in real-time
tail -f ai-logs/session-*.jsonl | jq .

# Get a quick summary of the last session
grep '"type":"turn"' ai-logs/session-001-*.jsonl | wc -l  # Turn count
grep '"action":"pickup"' ai-logs/session-001-*.jsonl | jq -r '.item'  # Items found
```

---

## Log File Format

### File Naming

```
ai-logs/session-NNN-TIMESTAMP.jsonl
```

- `NNN` - Sequential session number (001, 002, etc.) for easy sorting
- `TIMESTAMP` - ISO 8601 format: `2026-01-02T15-10-51`

Example: `session-001-2026-01-02T15-10-51.jsonl`

### JSON Lines Format

Each line is a complete, independent JSON object. This allows:
- Real-time tailing during gameplay
- Easy parsing with `jq` or any JSON library
- Line-by-line processing without loading entire file

---

## Event Types

### Session Events

| Event | Description |
|-------|-------------|
| `session_start` | Game launched |
| `session_end` | Game closed |

### Turn Events

| Event | Description |
|-------|-------------|
| `turn` | Marks start of a new turn (after player action) |
| `dungeon` | Complete dungeon state snapshot |

### Movement Events

| Event | Description |
|-------|-------------|
| `player_move` | Player moved to a new tile |
| `monster_move` | A monster moved (includes AI state) |

### Interaction Events

| Event | Description |
|-------|-------------|
| `combat` | Attack occurred (player or monster) |
| `item` | Item picked up, dropped, or used |

---

## Dungeon State Format

The `dungeon` event is logged every turn and contains the complete game state.

### Map Encoding

The map uses a **21x21 viewport** centered on the player, with **run-length encoding (RLE)** for compression.

**RLE Format:** Character followed by count (if > 1)
- `#5` = `#####` (5 walls)
- `.3` = `...` (3 floors)
- `@` = player (count of 1, no number)

**Map Array Format:** `[row_index, "RLE_encoded_row"]`
- Row indices are relative to viewport (0-20)
- All-wall rows are omitted entirely

### Example Dungeon Event

```json
{
  "type": "dungeon",
  "turn": 42,
  "level": 1,
  "depth_ft": 50,
  "view": {"x": 35, "y": 27, "w": 21, "h": 21},
  "map": [
    [7, "#10.4#7"],
    [8, "#10.4#7"],
    [9, "|#9.4#7"],
    [10, ".#9@.3#7"],
    [11, ".#9.4#7"],
    [12, ".'.7+.4#7"],
    [13, ".#9.4#7"]
  ],
  "rooms": 13,
  "hallways": 20,
  "player": {"x": 45, "y": 37, "hp": 8, "max_hp": 8},
  "monsters": [
    {"name": "Yellow Icky Thing", "char": "i", "x": 35, "y": 45, "hp": 7}
  ],
  "items": [
    {"name": "Long Sword", "char": "|", "x": 35, "y": 36}
  ]
}
```

### Decoding the Map

To reconstruct row 10 from `".#9@.3#7"`:
1. `.` = 1 floor
2. `#9` = 9 walls
3. `@` = player
4. `.3` = 3 floors
5. `#7` = 7 walls

Result: `.#########@...#######` (21 characters)

### ASCII Legend

| Char | Meaning |
|------|---------|
| `.` | Floor |
| `#` | Wall |
| `@` | Player |
| `+` | Closed door |
| `'` | Open door |
| `<` | Up stairs |
| `>` | Down stairs |
| `a-z, A-Z` | Monsters |
| `!?()[]{}/\|` | Items |

---

## Other Event Examples

### Player Move
```json
{
  "type": "player_move",
  "from": {"x": 45, "y": 37},
  "to": {"x": 46, "y": 37},
  "result": "floor"
}
```

### Monster Move
```json
{
  "type": "monster_move",
  "monster": "Yellow Icky Thing",
  "from": {"x": 35, "y": 45},
  "to": {"x": 35, "y": 44},
  "state": "gotodest"
}
```

### Combat
```json
{
  "type": "combat",
  "attacker": "Player",
  "defender": "Opal Icky Thing",
  "roll": 46.28,
  "hit": true,
  "damage": 2.44,
  "defender_hp": 8.64,
  "killed": false
}
```

### Item Pickup
```json
{
  "type": "item",
  "action": "pickup",
  "item": "Dagger",
  "count": 1,
  "position": {"x": 47, "y": 21}
}
```

---

## Example: Session Analysis

### Session 001 Summary

**Duration:** 553 turns

**Levels Explored:** 1, 2, 3 (down to 150 ft depth)

**Items Collected:**
| Count | Item |
|-------|------|
| 1 | Dagger |
| 1 | Cloak |
| 1 | Set of Gauntlets |
| 1 | Pair of Leather Gloves |
| 1 | Large Steel Shield |
| 1 | Steel Shield |
| 1 | Steel Cap |
| 1 | Shovel |
| 2 | Scrolls (unidentified) |

**Combat Encounters:**
| Attacks | Target |
|---------|--------|
| 9 | Giant Snake |
| 8 | White Icky Thing |
| 6 | Opal Icky Thing |
| 5 | Player (got hit) |

**Final State:**
- Level 1 (50 ft)
- HP: 9/9 (full health)
- Position: (72, 13)

**Outcome:** Survived! Explored down to level 3 and returned to level 1.

### Commands Used to Generate Summary

```bash
# Count total turns
grep -c '"type":"turn"' ai-logs/session-001-*.jsonl

# List levels visited
grep '"type":"dungeon"' ai-logs/session-001-*.jsonl | jq -r '.level' | sort -u

# Count items picked up
grep '"action":"pickup"' ai-logs/session-001-*.jsonl | jq -r '.item' | sort | uniq -c

# Count combat by target
grep '"type":"combat"' ai-logs/session-001-*.jsonl | jq -r '.defender' | sort | uniq -c

# Get final player state
grep '"type":"dungeon"' ai-logs/session-001-*.jsonl | tail -1 | jq '{level, player}'
```

---

## Implementation Details

### Source Files

| File | Purpose |
|------|---------|
| `src/AILog.h` | API declarations |
| `src/AILog.cpp` | JSON logging implementation |

### Key Functions

```cpp
void AILog_Init(const char* basedir);  // Initialize, open log file
void AILog_Term();                      // Close log file
bool AILog_IsActive();                  // Check if logging enabled
void AILog_Write(const char* json);     // Write raw JSON line
void AILog_Event(type, format, ...);    // Write formatted event
```

### Integration Points

| File | What's Logged |
|------|---------------|
| `main.cpp` | Session start/end |
| `Game.cpp` | Turn counter |
| `Dungeon.cpp` | Dungeon state dump |
| `Player.cpp` | Movement, combat, item pickup |
| `AIMgr.cpp` | Monster movement, monster attacks |

### Session Management

- New log file created each game session
- Sequential numbering (001, 002, ...) for easy identification
- Old sessions auto-archived with gzip after 10 sessions accumulate
- Archived files: `session-001-*.jsonl.gz`

---

## Size Optimizations

The logging system uses several techniques to reduce file size:

| Optimization | Reduction | Description |
|--------------|-----------|-------------|
| Viewport clipping | ~80% | Only log 21x21 area around player, not full 100x100 map |
| RLE compression | ~50% | Compress repeated characters (`#####` -> `#5`) |
| Wall row omission | ~20% | Skip rows that are entirely walls |

**Result:** ~2.2 KB per turn (down from ~11 KB)

---

## Tips for AI Assistants

### Real-time Monitoring

```bash
# Watch the latest log file
tail -f ai-logs/session-*-$(date +%Y-%m-%d)*.jsonl | jq .

# Filter to just combat events
tail -f ai-logs/session-*.jsonl | grep combat | jq .
```

### Quick Analysis

```bash
# How many monsters killed? (check for killed:true)
grep '"killed":true' ai-logs/session-*.jsonl | jq -r '.defender' | sort | uniq -c

# Player's HP over time
grep '"type":"dungeon"' ai-logs/session-*.jsonl | jq '.player.hp'

# What items exist on current level?
grep '"type":"dungeon"' ai-logs/session-*.jsonl | tail -1 | jq '.items[].name'
```

### Reconstructing the Map

To visualize the current dungeon from a log entry:

```python
import json
import re

def decode_rle(rle_string):
    result = []
    i = 0
    while i < len(rle_string):
        char = rle_string[i]
        i += 1
        count = ""
        while i < len(rle_string) and rle_string[i].isdigit():
            count += rle_string[i]
            i += 1
        result.append(char * (int(count) if count else 1))
    return "".join(result)

# Parse a dungeon event and print the map
event = json.loads(log_line)
for row_idx, rle_row in event["map"]:
    print(f"{row_idx:2}: {decode_rle(rle_row)}")
```

---

## Known Issues

- **Bug #156:** Combat `killed` flag reports `false` when monster HP reaches exactly 0.0
  - Workaround: Check if `defender_hp` is 0 instead of relying on `killed` flag

---

## Future Enhancements

- [ ] Command-line flag to enable/disable AI logging
- [ ] Real-time log streaming via WebSocket
- [ ] Log viewer/replay tool
- [ ] Compress logs in-place after session ends
