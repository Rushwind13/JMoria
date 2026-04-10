# Bot Downward Progression Strategy (Issue #205)

## Overview

This document defines and explains when the JMoria bot agent chooses to advance to the next dungeon level. The progression strategy ensures the bot explores each level systematically and safely before descending deeper into the dungeon.

## Goal Hierarchy (Issue #186)

The bot follows a clear priority hierarchy on each dungeon level:

1. **Explore All Reachable Territory**
   - Visit every walkable tile on the current level
   - Interact with doors to reveal room contents
   - Catalog the complete layout and contents

2. **Eliminate Visible Threats**
   - Engage and defeat any visible monsters
   - Prioritize adjacent threats via bump-attacks
   - Use combat feedback to learn monster behaviors

3. **Evaluate and Collect Items**
   - Pick up items discovered during exploration
   - Wield/wear equipment to test and identify
   - Manage inventory and identify unknown items

4. **Descend to Next Level**
   - Only after all above priorities exhausted
   - Subject to safety criteria (see below)

## Downward Progression Criteria (Issue #205)

Before descending via the down staircase ('>' command), the bot MUST satisfy ALL of these criteria:

### Criterion 1: Exploration Complete

**Description**: All reachable walkable tiles on the current level must be explored.

**Implementation**:
- `unexplored_tiles` set must be empty
- All discovered doors must have been stepped through
- Goal stack exhausted or only contains staircase goal

**Safety Rationale**: 
- Prevents missing items, monsters, or exits
- Ensures complete knowledge of current level layout
- Avoids premature descent leaving resources behind

**Telemetry**:
- If fails: `prog_descend_delay_exploring`

### Criterion 2: HP Recovery (≥70%)

**Description**: Player health must be at least 70% of maximum.

**Implementation**:
- Check: `state.player_hp / state.player_max_hp >= 0.70`
- Prior decision tier: at 50% HP, bot rests before combat
- Progression decision: requires 70% before descent

**Safety Rationale**:
- 20% safety margin above urgent-rest threshold (50%)
- Entering next level with full strength enables combat
- Prevents "debt spiral" where HP compounds downward

**Interactions**:
- Healing potions prioritized during exploration (Criterion 3)
- Post-combat recovery phase (rest to 70%) happens before descent
- Rest action ("R") only triggered when safe (no adjacent threats)

**Telemetry**:
- If fails: `prog_descend_delay_low_hp_<percent>`

### Criterion 3: Combat Stability

**Description**: No immediate or recent threats present.

**Implementation**:
- No visible adjacent monsters (`_adjacent_monster()` returns None)
- No recent attacker (`turns_since_combat_feedback >= 3`)
- `recent_attacker_name` cooldown respected

**Safety Rationale**:
- Prevents descending mid-combat
- Allows threat assessment before transition
- Ensures cleared room before moving down

**Telemetry**:
- If adjacent monster: `prog_descend_delay_combat_adjacent`
- If recent combat: `prog_descend_delay_combat_recent`

### Criterion 4: Weapon Readiness (Optional)

**Description**: At least basic melee weapon identified/wielded (not strictly enforced).

**Implementation**:
- Check: `self.current_wielded_weapon is not None`
- Warning level only; unarmed descent allowed but suboptimal

**Safety Rationale**:
- Combat readiness for early monsters
- Prevents defenseless entry into populated levels
- Encourages early weapon identification

**Telemetry**:
- If observed: `prog_descend_delay_unready` (future use)

## Progression Telemetry

All descent attempts are logged in `progression_telemetry` dict:

```python
{
    "depths_cleared": [
        (depth, reason, visited_count),
        # Example:
        # (1, "prog_descend_ready", 150),
        # (2, "prog_descend_ready", 220),
    ],
    "last_descent_reason": "prog_descend_ready",
    "turns_at_depth": 850,  # accumulated turns at current level
}
```

### Telemetry Signals

| Signal | Meaning |
|--------|---------|
| `prog_descend_ready` | All criteria satisfied; descending |
| `prog_descend_delay_exploring` | Unexplored tiles remain; continue exploration |
| `prog_descend_delay_low_hp_<pct>` | HP insufficient; seeking healing potions or resting |
| `prog_descend_delay_combat_adjacent` | Adjacent monster present; engage combat first |
| `prog_descend_delay_combat_recent` | Too soon after combat; cooldown active |
| `prog_descend_delay_unready` | No weapon identified; continuing search |

## Implementation Details

### Code Location
- Main logic: [scripts/bot/decision.py](decision.py#L927-L950)
  - Class method: `DecisionEngine._can_safely_descend_stairs(state)`
- Staircase descent handler: [scripts/bot/decision.py](decision.py#L800-L816)
  - In `_pursue_goals()`, staircase goal processing
- Telemetry tracking: [scripts/bot/decision.py](decision.py#L108-L113)
  - Instance variable: `progression_telemetry`

### State Tracking

**Exploration Sets**:
```python
self.unexplored_tiles  # walkable (row,col) seen but not cleared
self.explored_tiles    # walkable (row,col) we've cleared
self.goal_stack        # LIFO stack of (goal_type, (row,col))
```

**Health/Combat**:
```python
self.last_player_hp              # previous HP for damage detection
self.recent_attacker_name        # last monster that hit us
self.turns_since_combat_feedback # turn counter for combat cooldown
```

**Level Progression**:
```python
self.progression_telemetry       # descent history and reasons
self._current_depth              # current dungeon depth (1-based)
```

## Example Progression Flow

### Scenario 1: Normal Progression (All Criteria Met)

```
Depth 1: 500 turns
- Explored entire level (all tiles visited)
- HP: 95% (well above 70% threshold)
- No adjacent monsters
- Weapon: "dagger" wielded
→ Descend: prog_descend_ready (depth=1, turns=500, visited=147)

Depth 2: Starts with fresh exploration state
```

### Scenario 2: Delayed by Exploration (Criterion 1 Fails)

```
At staircase, all other criteria met, but:
- unexplored_tiles contains 12 tiles
→ Reject descent: prog_descend_delay_exploring
→ Pop staircase goal, continue exploration
→ Once all tiles explored, staircase goal re-added to stack
→ Next staircase encounter: re-evaluate all criteria
```

### Scenario 3: Delayed by Low HP (Criterion 2 Fails)

```
At staircase, exploration complete, but:
- HP: 45% (below 70% threshold)
→ Reject descent: prog_descend_delay_low_hp_45%
→ Pop staircase goal
→ Prior decision tier triggered rest ("R") or healing potion use
→ Once HP >= 70%, re-evaluate
→ Eventually pass all criteria and descend
```

### Scenario 4: Delayed by Recent Combat (Criterion 3 Fails)

```
At staircase, exploration complete, HP good, but:
- recent_attacker_name = "orc"
- turns_since_combat_feedback = 1
→ Reject descent: prog_descend_delay_combat_recent
→ Wait 2 more turns
→ On turn 3: re-evaluate, now passes all criteria
→ Descend: prog_descend_ready
```

## Testing and Verification

### Test Scenarios (in test/features/)

1. **Normal Progression** - All criteria met
   - Expected: Descend and increment depth
   - Telemetry: `prog_descend_ready`

2. **Blocked by Unexplored** - Tiles remain
   - Expected: Reject descent, continue exploring
   - Telemetry: `prog_descend_delay_exploring`

3. **Blocked by Low HP** - Below 70%
   - Expected: Reject descent, seek healing
   - Telemetry: `prog_descend_delay_low_hp_<pct>`

4. **Blocked by Combat** - Adjacent threat
   - Expected: Reject descent, engage combat
   - Telemetry: `prog_descend_delay_combat_adjacent`

### Verification Checklist

- [ ] Bot never descends with unexplored tiles remaining
- [ ] Bot never descends below 70% HP (except critical edge cases)
- [ ] Bot never descends with adjacent monsters
- [ ] Bot correctly logs progression telemetry for each descent
- [ ] Each descent reason matches actual criteria state
- [ ] Multi-level runs show increasing depth progression
- [ ] HP recovery phase works correctly between descent attempts

## References

- **Issue #205**: Confirm bot agent downward progression strategy
- **Issue #186**: Bot behavior - exploration redesign (goal-based pathfinding)
- **Issue #182**: AI Player System design rationale
- [WORKLIST_ai_player.md](../../WORKLIST_ai_player.md) - AI player development roadmap
