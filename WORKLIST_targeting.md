# JMoria Targeting & Line-of-Sight — Work List

PR: https://github.com/Rushwind13/JMoria/pull/144

## Build & Run Commands
- Build: `make clean && make ascii`
- Run: `tmux new-session -d -s crawler -x 200 -y 50 './jmoria 2>/tmp/jmoria_map.txt'`
- Capture: `tmux capture-pane -t crawler -p`
- Kill: `tmux kill-session -t crawler`
- Bot test: `bash /tmp/run_bot_test.sh`

## Key Files
- `src/TargetState.h` / `src/TargetState.cpp` — target selection state machine
- `src/RangedState.h` / `src/RangedState.cpp` — ranged attack (fire/zap) state machine
- `src/Dungeon.cpp` — `PlayerCanSee()`, `CanSeeEachOther()`, `SightCollisionTest()` callback
- `src/Util.cpp` — `Bresenham()` line-of-sight / trajectory generator
- `src/Player.h` — `SIGHT_DISTANCE_PLAYER` (5 tiles)
- `src/CmdState.cpp` — targeting entry point (Shift+8 = `*`)

## Current Architecture

### Target Selection (`CTargetState`)
`DoInit()` builds a list of targetable monsters by iterating the monster list and calling `CDungeon::PlayerCanSee()` for each. Targets are stored as **list-position indices** (`uint32`) in a `JLinkList<uint32>`. The player cycles targets with `*` and confirms with `.`.

### Line-of-Sight (`CDungeon::CanSeeEachOther`)
Three-layer visibility check:
1. **Same-room shortcut** — if both entities are in the same lit room, immediate pass
2. **Special senses** — ESP (15 tiles, non-empty-mind) and Infravision (10 tiles, warm bodies)
3. **Bresenham LOS** — range-limited (`SIGHT_DISTANCE_PLAYER` = 5), uses `SightCollisionTest` callback

`SightCollisionTest` blocks on: walls, rubble, closed/secret doors. Open doors and floors are transparent.

### Ranged Attacks (`CRangedState`)
State-machine flow: `RANGED_INIT` → fire(`f`)/zap(`z`) → item selection → target/direction → trajectory animation → hit resolution.
- Trajectory built via `Util::Bresenham()` with `NoCollisionCheck` (projectiles fly full range)
- Hardcoded `PROJECTILE_RANGE` = 8 tiles, animated at 10ms intervals
- Auto-target: if player already has a target from `*`, ranged commands reuse it
- `RANGED_LAUNCH` state defined but unimplemented (dead code)

### Bresenham (`Util::Bresenham`)
Standard implementation with error-term tracking and diagonal gap checking. Supports distance limit and optional line collection for trajectory visualization. Mixed responsibilities: line generation, collision checking, logging, and list collection all in one function.

### Wizard Mode
`PlayerCanSee()` returns `true` unconditionally if wizard mode is active, bypassing all LOS checks.

## Existing Test Coverage
- `test/features/light.feature` — tests lighting (affects visibility indirectly)
- `test/features/dungeonmap.feature` — room connectivity (affects same-room shortcut)
- **No dedicated targeting or ranged feature tests exist**

## Bot / Crawler Status
- Bot (`scripts/bot/decision.py`) has **no ranged combat logic** — melee only
- No `fire`, `zap`, or targeting commands in the bot decision engine
- Bot cannot exercise targeting paths; manual testing or new bot capabilities needed

---

## Priority Worklist

### [P0] Target index staleness — store stable references
- **Bug**: `DoInit()` stores monster list-position indices. If monsters die, spawn, or the list mutates before confirm, indices become invalid or point to wrong monster.
- **Fix**: Store `CMonster*` or a stable unique ID in the target list. On confirm (`.`), re-validate `IsAlive()` and `PlayerCanSee()` before committing.
- **Test**: Add BDD scenario — select target, monster dies before confirm, verify graceful handling.

### [P0] Re-validate LOS on confirm
- **Bug**: LOS depends on map state via `isWalkable` callback. Doors opening/closing or tunneling between selection and confirm silently change outcome.
- **Fix**: Re-run `PlayerCanSee()` at confirm (`.`). If target is no longer visible, display message and refresh target list.
- **Test**: Add BDD scenario — select target, close door, confirm, verify rejection message.

### [P1] Add `test/features/targeting.feature`
- Dedicated acceptance tests for CTargetState lifecycle:
  - Visible monster appears in target list
  - Non-visible monster excluded from target list
  - Target cycling wraps around
  - Confirm on valid target transitions to command/ranged
  - Cancel (ESC) returns to command state
  - Target dies before confirm — handled gracefully

### [P1] Add `test/features/ranged.feature`
- Acceptance tests for CRangedState:
  - Fire weapon at visible target — projectile hits
  - Zap wand at visible target — effect applies
  - Fire at out-of-range target — rejected
  - Auto-target reuse from previous `*` selection
  - Projectile trajectory animation renders correctly

### [P1] Unit tests for `Util::Bresenham()`
- Extract pure line-generation function (no callbacks, no logging)
- Test cases: horizontal, vertical, diagonal, near-diagonal, obstacle blocking, max distance cap, diagonal gap checking
- Verify against canonical Bresenham output

### [P2] Split Bresenham responsibilities
- Make LOS computation pure: separate line-generation from collision checks
- New function: `Util::GenerateLine(start, end, distance)` → returns line as `JLinkList<JVector>`
- Collision check becomes a separate pass over the generated line
- Enables deterministic unit testing without callback mocking

### [P2] Cached visible-set
- Maintain a set of visible monsters, recomputed when player moves or world state changes (door open/close, monster move/die)
- `CTargetState::DoInit()` reads from cache instead of re-scanning entire monster list
- Consistent target lists; avoids redundant LOS computation

### [P3] Target mark / persistent tracking
- Assign stable unique IDs to monsters (consider reusing `CItem`-style instance IDs)
- Target "mark" persists across list reordering and state transitions
- Enables "last target" recall for repeated attacks

### [P3] UI feedback — draw LOS line while targeting
- Render computed Bresenham line on-screen while player is in `TARGET_TARGET` state
- Show line in distinct color (green = clear, red = blocked)
- Always re-validate on confirm regardless of visual feedback

### [P3] Clean up `RANGED_LAUNCH` dead code
- `RANGED_LAUNCH` state modifier defined in `RangedState.h` but never wired to a handler
- Remove or implement depending on design intent (thrown weapons?)

### [P3] Bot ranged combat support
- Add targeting/fire/zap commands to `scripts/bot/decision.py`
- Bot should evaluate ranged vs melee based on distance and available items
- Enables automated testing of targeting paths via `crawl_metric.sh`
