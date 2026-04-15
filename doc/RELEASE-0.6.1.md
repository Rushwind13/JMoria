# JMoria Release 0.6.1 - Targeting!

**Release Date**: April 15, 2026

## Overview

Release 0.6.1 delivers a complete targeting and line-of-sight system with full Bresenham ray-casting, ranged combat fixes, and significant architectural cleanup following post-release code review.

## Major Features

### Targeting & Line-of-Sight — Full Implementation (#144)
- Target cycling sorted by taxicab distance (nearest-first)
- Bresenham LOS line drawn in bright cyan from player to current target
- LOS re-validation on confirm before accepting target
- Stable target tracking via monster instance IDs (survives add/remove)
- ASCII renderer key mapping: `*` (Shift+8) for target, digit keys for directional input

### Lit-Room Field of View
- Player can see into lit rooms from hallways at extended range (28 tiles — max room diagonal)
- Bresenham LOS through doorways creates natural cone-shaped FOV
- Dark hallway sight unchanged at 5 tiles

### Extended Projectile Range
- Wand range increased from 8 to 20 tiles (full room width)
- `PROJECTILE_RANGE` and `SIGHT_DISTANCE_LIT` auto-derived from room size constants

### Bresenham API Refactor
- Split into composable `GenerateLine()` + `CheckLineCollision()` functions
- `TargetState` and `RangedState` call `GenerateLine()` directly

## Bug Fixes

- **Reversed projectile trajectory**: projectile now flies source→target (was backwards)
- **Zap retval bug**: `IsZapCommand` returned `JBOGUSKEY` (-1) instead of 0, causing spurious 'Unrecognized command' on every zap
- **Dangling m_pTarget**: cleared in `RemoveMonster()` when removed monster is player's target
- **Target index bounds checking**: NULL guards in `CTargetState` cycling
- **StringInputState crash**: NULL guard for bad monster/item names in wizard commands
- **Enter key in ASCII**: exclude `\n`/`\r` from Ctrl+letter branch (was mapping to 'j')
- **Fire/zap rejection**: `OnHandleZap`/`OnHandleFire` now `ResetToState` on wrong item type

## New Commands

- **g)ain**: pick up items at player's feet
- **^w**: toggle wizard mode off (password: 'xyzzy')

## Gameplay Enhancements

- Visible monster cache moved from `CDungeon` to `CPlayer` (per-observer pattern)
- `DUNG_MAXDEPTH` corrected to 100

## Architectural Cleanup (#250, #252)

- Eliminated `m_llTargets` from `TargetState` — uses `GetVisibleMonsters()` directly
- JLinkList ownership model: `m_bOwnsData` flag, non-owning lists via `JLinkList<T>(false)`
- Visible monsters changed from `JLinkList<uint32>` to `JLinkList<CMonster>(false)`
- Removed `GetMonster()`/`FindMonsterByInstanceId()` — zero callers remain
- Renderer separation: `PollEvent` → `IRenderBackend`, `ConfigureDisplayRegions` extracted
- Zoom ownership moved from `CDungeon` to `CRender`
- `PopulateLevel(depth)` extracted; `SpawnPlayer` folded in
- Panel toggles → `CmdState::IsToggleCommand()` with `JHANDLED_NOTURN`
- Unified `Draw()` for both renderers
- stdlib/include cleanup across multiple files

## Testing

- **24 new BDD scenarios** across targeting, ranged, and Bresenham features
- All scenarios passing (127+ total)
- Test fixtures: `map42.txt` reference map for seed-42 deterministic dungeon

## Bot Enhancements

- Ranged immunity tracking: `(wand, monster)` pairs learned from 'is unaffected' feedback
- Empty wand tracking from 'nothing happens' messages
- `--init-keys`, `--init-file`, `--max-turns` CLI options for scenario-driven testing
- Scenario files for targeted bot testing

## Version Info

- **Version**: 0.6.1
- **Game Engine**: SDL2/OpenGL with ASCII fallback
- **Language**: C++17
- **Platform Support**: macOS, Linux, Raspberry Pi OS
