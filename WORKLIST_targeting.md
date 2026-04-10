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
- `RANGED_LAUNCH` state wired in preparation for full ranged pipeline (thrown weapons, etc.)

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

### [P0] ✅ IMPLEMENTED — Dangling target pointer — targeted monster dies
- **Bug**: `CPlayer::m_pTarget` is a raw `CMonster*` pointer. If the targeted monster is killed by another monster (or despawns), the pointer dangles → use-after-free crash. `OnKillMonster()` clears `m_pTarget` only when the PLAYER kills it; no cleanup path exists for other death causes.
- **Fix applied**: `CDungeon::RemoveMonster()` now checks if the removed monster is the player's current target, and calls `SetTarget(NULL)` to clear it before removing from the list.
- **Test**: Add BDD scenario — target monster, another monster kills it, player tries to zap → verify no crash, graceful "target lost" message.

### [P0] ✅ IMPLEMENTED — Target index staleness in CTargetState — bounds & NULL guards
- **Bug**: `DoInit()` stores monster list-position indices (`uint32`) in `JLinkList<uint32>`. If monsters die, spawn, or the list mutates between `DoInit()` and confirm (`.`), the index can point to the wrong monster or go out of bounds.
- **Evidence**: `OnBaseHandleKey()` retrieves `m_llMonsters->GetNthLink(*dwTarget)` without bounds checking or identity validation.
- **Fix applied**: Added bounds checking on the index against current monster list length, and NULL guards on the retrieved link/monster pointer in `OnBaseHandleKey()` target cycling. Stale indices now skip gracefully instead of crashing.
- **Future**: Consider storing `CMonster*` pointers directly instead of indices for full robustness.
- **Test**: Add BDD scenario — select target, monster dies before confirm, verify graceful handling.

### [P0] ✅ IMPLEMENTED — Re-validate LOS on confirm
- **Bug**: LOS depends on map state via `isWalkable` callback. Doors opening/closing or tunneling between selection and confirm silently change outcome.
- **Fix applied**: Confirm (`.`) in `CTargetState::OnBaseHandleKey()` now re-runs `PlayerCanSee()` before accepting the target. If the target is no longer visible, displays "You can no longer see that target." and clears the target.
- **Test**: Add BDD scenario — select target, close door, confirm, verify rejection message.

### [P0] AIMgr timing during ranged — fragile design
- **Status**: Currently NOT a runtime bug in TURN_BASED mode. `DoLaunch()` calls `SetReadyForUpdate(false)`, which gates `AIMgr::Update()` for the entire trajectory animation. Monsters do not move during projectile flight.
- **Risk**: This safety depends entirely on the `m_bReadyForUpdate` flag in the `TURN_BASED` code path. Removing TURN_BASED or restructuring `CGame::Update()` would immediately create a race where AIMgr moves monsters while the projectile follows a stale pre-built path.
- **Position capture**: `UsePlayerTarget()` reads `GetTarget()->GetPos()` from the live `CMonster*` pointer, so the position IS current at capture time. The trajectory is pre-computed from this snapshot and never re-read.
- **Recommendation**: Document this coupling. Consider resolving hit at trajectory-build time (make animation purely cosmetic) to decouple from update ordering. Add an assertion/guard in `DoTrajectory()` that `m_bReadyForUpdate == false`.

### [P1] Skip OpenGL tests — get targeting working in ASCII Renderer (#225)
- ✅ **FIXED**: `HandleEventsASCII` now maps `'*'` (Shift+8) to `JKEY_8 + JMOD_SHIFT`
- ✅ **FIXED**: `IsDirectional`/`GetDir` now accept plain digit keys `1`-`9` as directional input (ASCII renderer has no numpad); shifted digits excluded to preserve `*` target command
- ✅ **FIXED**: Enter key was mapping to `'j'` (move down) — excluded `\n`/`\r` from Ctrl+letter branch
- ✅ **FIXED**: `StringInputState::OnHandleItem` NULL guard — `CreateItem(NULL, ...)` segfaulted on unknown item names
- All targeting and ranged test development should use ASCII renderer (`make ascii`)
- Skip or gate any OpenGL-dependent test paths
- Ensure `CTargetState` and `CRangedState` work correctly with `CRenderASCII`
- Validate trajectory rendering in terminal mode

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

### [P1] Cached visible-set + distance-sorted target list
- ✅ **IMPLEMENTED**: Target list now sorted by taxicab distance (nearest first) using `JLinkList::Add(pData, dist)` sorted insert
- ✅ **IMPLEMENTED**: Initial target set to nearest visible monster
- Maintain a set of visible monsters, recomputed when player moves or world state changes (door open/close, monster move/die)
- `CTargetState::DoInit()` reads from cache instead of re-scanning entire monster list
- Consistent target lists; avoids redundant LOS computation

### [P1] ✅ IMPLEMENTED — UI feedback — draw LOS line while targeting
- ✅ Bresenham LOS line from player to current target rendered in bright cyan (A_BOLD in ASCII)
- ✅ Line updates when cycling targets with `*`
- ✅ Line cleared on ESC or confirm (`.`)
- ✅ Projectile tile color bumped to bright yellow (A_BOLD in ASCII)
- ✅ `CDungeon::m_llLOSLine` with `SetLOSLine()`/`ClearLOSLine()`/`IsOnLOSLine()` API
- Future: Show line in red when blocked vs green when clear

### [P2] Split Bresenham responsibilities
- Make LOS computation pure: separate line-generation from collision checks
- New function: `Util::GenerateLine(start, end, distance)` → returns line as `JLinkList<JVector>`
- Collision check becomes a separate pass over the generated line
- Enables deterministic unit testing without callback mocking

### [P3] Target mark / persistent tracking
- Assign stable unique IDs to monsters (consider reusing `CItem`-style instance IDs)
- Target "mark" persists across list reordering and state transitions
- Enables "last target" recall for repeated attacks

### [P3] Bot ranged combat support
- Add targeting/fire/zap commands to `scripts/bot/decision.py`
- Bot should evaluate ranged vs melee based on distance and available items
- Enables automated testing of targeting paths via `crawl_metric.sh`
