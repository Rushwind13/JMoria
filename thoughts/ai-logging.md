# AI Logging Design Notes

## Purpose

Enable AI assistants (like Claude) to observe and understand game state during play sessions. This supports:
- Debugging game issues
- Understanding game mechanics
- Following along with player actions
- Analyzing game balance

## Design Decisions

### Why Separate from JLog?

The existing `JLog` system is designed for human-readable console output:
- Printf-style formatting
- No structured format
- Stdout only
- Filtered by log level

AI logging needs:
- Structured JSON format (machine-parseable)
- File output (persistent, can be read after game ends)
- Every-turn dungeon state dumps
- Consistent event schema

**Decision**: Create new `AILog` system, keep `JLog` unchanged.

### Why Layer 2 (ASCII)?

The game's 3-layer architecture (see `architecture.md`) offers three options:

| Layer | Format | Pros | Cons |
|-------|--------|------|------|
| 1 | `DUNG_IDX_WALL` | Precise | Not human-readable |
| 2 | `#` | Human + AI readable | Good balance |
| 3 | Tileset index | Rendering detail | Implementation detail |

**Decision**: Use Layer 2 (ASCII) because:
- Human-readable (`.` = floor, `#` = wall, `@` = player)
- Compact (1 character per tile)
- Matches roguelike tradition
- AI can easily understand dungeon layout

### Why JSON Lines?

Options considered:
1. **Plain text**: Easy to read, hard to parse programmatically
2. **Full JSON**: One big object per session - hard to stream
3. **JSON Lines**: One JSON object per line - best of both worlds

**Decision**: JSON Lines (`.jsonl`) because:
- Each line is independent, valid JSON
- Easy to stream/tail during gameplay
- Easy to parse with standard tools
- Can be loaded into data analysis tools

### Log File Naming

Format: `ai-logs/session-<timestamp>.jsonl`

Example: `ai-logs/session-2026-01-02T12-34-56.jsonl`

New file per game session. Old sessions preserved for analysis.

## Event Types

### Core Events

| Type | When | Key Data |
|------|------|----------|
| `session_start` | Game init | timestamp, version |
| `session_end` | Game quit | timestamp, reason |
| `turn` | Each player action | turn number |
| `dungeon` | Each turn | ASCII map, player/monster/item positions |

### Movement Events

| Type | When | Key Data |
|------|------|----------|
| `player_move` | Player moves | from, to, direction, result |
| `monster_move` | Monster moves | monster name, from, to, AI state |

### Combat Events

| Type | When | Key Data |
|------|------|----------|
| `combat` | Attack occurs | attacker, defender, roll, hit, damage |
| `death` | Creature dies | creature, killer, position |

### Interaction Events

| Type | When | Key Data |
|------|------|----------|
| `item` | Item interaction | action (pickup/drop/use), item, position |
| `door` | Door interaction | action (open/close), position |
| `stairs` | Level change | direction (up/down), new depth |

### State Events

| Type | When | Key Data |
|------|------|----------|
| `state_change` | Game state changes | from_state, to_state |
| `level_enter` | Enter new level | depth, rooms, hallways |

## JSON Schema Examples

### Session Start
```json
{"type":"session_start","timestamp":"2026-01-02T12:34:56Z","version":"0.1.0"}
```

### Dungeon State
```json
{
  "type": "dungeon",
  "turn": 42,
  "level": 1,
  "depth_ft": 50,
  "width": 100,
  "height": 100,
  "map": [
    "##########",
    "#........#",
    "#..@..o..#",
    "#....!...#",
    "##########"
  ],
  "player": {"x": 5, "y": 2, "hp": 20, "max_hp": 25},
  "monsters": [
    {"name": "Orc", "char": "o", "x": 8, "y": 2, "hp": 10}
  ],
  "items": [
    {"name": "Potion", "char": "!", "x": 5, "y": 3}
  ]
}
```

### Player Movement
```json
{
  "type": "player_move",
  "turn": 42,
  "from": {"x": 5, "y": 2},
  "to": {"x": 6, "y": 2},
  "direction": "east",
  "result": "floor"
}
```

### Combat
```json
{
  "type": "combat",
  "turn": 43,
  "attacker": "Player",
  "defender": "Orc",
  "attack_roll": 75,
  "hit": true,
  "damage": 12,
  "defender_hp_remaining": 0,
  "kill": true
}
```

## Implementation Notes

### New Files
- `src/AILog.h` - API declarations
- `src/AILog.cpp` - Implementation

### Key Functions
```c
void AILog_Init(const char* basedir);    // Open log file
void AILog_Term();                        // Close log file
void AILog_SessionStart();                // Log session start
void AILog_Turn(uint32 turn);             // Log turn marker
void AILog_Dungeon(CDungeon* dungeon);    // Log dungeon state
void AILog_PlayerMove(...);               // Log player movement
void AILog_MonsterMove(...);              // Log monster movement
void AILog_Combat(...);                   // Log combat
void AILog_Item(...);                     // Log item interaction
```

### Integration Points
- `main.cpp`: Init/Term
- `Game.cpp`: Turn counter, state changes
- `Dungeon.cpp`: Dungeon dump
- `Player.cpp`: Movement, combat, items
- `AIMgr.cpp`: Monster movement, attacks

## Future Enhancements

- Command-line flag to enable/disable AI logging
- Log compression for long sessions
- Real-time log streaming via named pipe
- Log viewer tool
