# Phase 3d: Shiny - Prioritized Roadmap

**Repo**: `Rushwind13/JMoria`
**Build**: `make build`
**Test**: `make bdd`
**Status**: Roadmap for upcoming phase 
**Last Updated**: May 25, 2026 (session 2)
**Total Issues**: 12


---

## Overview

Phase 3d: Shiny focuses on visual polish, effect refinements, and bug fixes for core gameplay systems.

---

## Priority 1: Critical Bugs (P0)

### ~~[#173](https://github.com/Rushwind13/JMoria/issues/173) - Items disappear at wrong view distance during gameplay~~ ✅ DONE
**Type**: Bug | **Severity**: High | **Components**: Rendering, View Distance  
**Resolution**: Coordinate-truncation fix (`roundf` → `(int)` cast) in `RenderASCII::MapX/MapY` was already applied. Added BDD regression tests in `test/features/item_visibility.feature` (20 scenarios covering all 8 adjacent directions, distances 2–5, and the 5-step Chebyshev boundary). Issue closed May 25, 2026.

### ~~[#317](https://github.com/Rushwind13/JMoria/issues/317) - Melee "touch" attacks are being emitted at range~~ ✅ DONE
**Type**: Bug | **Severity**: High | **Components**: Combat, Attacks  
**Resolution**: Introduced a three-tier attack range classification in `BuildEligibleAttacks()` and `UpdateAttack()` in `src/AIMgr.cpp`: Range=0 → GoToDest (requires `CollideWithPlayer`), Range=1 → melee (fires in-place, requires adjacency), Range>1 → ranged (fires in-place at LOS distance). Previously, `bRanged = fRange > 0.0f` caused Range=1 "Cold Touch" and "Physical Hit" effects to be treated as ranged, firing without physical adjacency. Fix: `bRanged = fRange > 1.0f` in eligibility check; `bRanged = fRange >= 1.0f` in attack dispatch. Issue closed May 25, 2026.

### ~~[#266](https://github.com/Rushwind13/JMoria/issues/266) - Town level spawn warnings~~ ✅ NOT REPRODUCIBLE
**Type**: Bug | **Severity**: Medium | **Components**: Item/Monster Spawning  
**Resolution**: Not reproducible. `ChooseItemForDepth` / `ChooseMonsterForDepth` use `windowed_bell` with `sigma=10.0`, giving positive weights at depth 0 for level-1 items/monsters (`|delta|=1 < sigma=10` → weight ≈ 0.98). No "invalid item/monster" warnings appear in any of the 227 passing BDD scenarios. Issue closed May 25, 2026.

### ~~[#309](https://github.com/Rushwind13/JMoria/issues/309) - Intrinsic effects seem backwards~~ ✅ DONE
**Type**: Bug | **Severity**: Medium | **Components**: Effects, Status Display  
**Resolution**: `DoIntrinsicEffects` was not checking `EFFECT_MOD_RESIST`, so "Resist Fear" (`EFFECT_FLAG_AFRAID + EFFECT_MOD_RESIST`) fell into the same case as the afraid debuff, printing "You are afraid!" and setting the AFRAID bit. Stats display then showed "Afraid" instead of "Res: Fear". Fixed by: (1) adding `EFFECT_MOD_TIMED` to `Resist Fear` and `Resist Poison` in `Effects.txt` so they expire correctly and are stored in `m_llActiveEffects`; (2) checking `EFFECT_MOD_RESIST` in `DoIntrinsicEffects` and `UndoIntrinsicEffects` for AFRAID/POISON cases; (3) adding `HasActiveResistFor()` helper and using it in the stats display to show "Res: Fear" / "Res: Poison" vs "Afraid" / "Poisoned". Issue closed May 25, 2026.

---

## Priority 2: Code Cleanup & Refactoring (P1)

### ~~[#310](https://github.com/Rushwind13/JMoria/issues/310) - Refactor "noticeable effect" identification~~ ✅ DONE
**Type**: Cleanup | **Severity**: Medium | **Components**: Effects, UseState  
**Resolution**: Extracted the duplicate identify-on-notice block from `Quaff()`, `Read()`, `Zap()`, and `UseStaff()` into a single `DoEffectsAndMaybeIdentify(CLink<CItem>*)` helper on `CPlayer`. Each caller now delegates to that helper. Issue closed May 25, 2026.

### ~~[#315](https://github.com/Rushwind13/JMoria/issues/315) - Refactor "Player" Effects~~ ✅ DONE
**Type**: Cleanup | **Severity**: Medium | **Components**: Player, Effects  
**Resolution**: Moved dungeon-specific and effect-specific logic out of `CPlayer`:
- `DoLightArea()` → `CDungeon::LightAreaAt(JVector vPos)` (Dungeon.h/.cpp)
- `DoMagicMapping()` eliminated — logic inlined into `DoCreateEffects()` using existing `CDungeon::RevealMap()`
- `DoSummonMonsters()` → `CEffect::SummonMonsters(JVector vOrigin)` (Effect.h/.cpp)
- `DoRemoveCurse()` / `DoApplyCurse()` eliminated — inlined into `DoDestroyEffects()` using existing `CItem::SetCursed(bool)`

Net result: 5 private methods removed from `CPlayer`. Issue closed May 25, 2026.

**Follow-on cleanup (session 2, May 25 2026)**:
- `CDungeon::RevealMap()` refactored from `(int, int, int, int)` to `(JRect)` — both call sites in `DoCreateEffects` updated (area mapping uses `Util::Nearby`, full-dungeon uses `JRect(0,0,DUNG_WIDTH-1,DUNG_HEIGHT-1)`)
- Lit-room monster-wake loop removed from `CDungeon::LightAreaAt()` — `CAIBrain::UpdateRest()` already reads `pRoom->HasFlags(DUNG_FLAG_LIT)` per-tick via `SLEEP_LIT_ROOM_PENALTY`; `SLEEP_LIGHT_WAKE_CHANCE` constant deleted
- `IsDrinkable`, `IsReadable`, `IsZappable`, `IsStaff`, `IsFireable`, `IsWieldable`, `IsFuel` moved from `CPlayer` to `CItem` as inline methods; all call sites in `Player.cpp`, `UseState.cpp`, `RangedState.cpp` updated to call directly on `CItem`

### ~~[#311](https://github.com/Rushwind13/JMoria/issues/311) - Stone to Mud should melt doors~~ ✅ DONE
**Type**: Enhancement/Cleanup | **Severity**: Low | **Components**: Effects  
**Resolution**: Extended `CEffect::StoneToMud()` to handle three cases beyond walls: closed doors (`DUNG_IDX_DOOR`) and secret doors (`DUNG_IDX_SECRET_DOOR`) are converted to floor; open/broken doors are unaffected (naturally, since they're already passable). Added `MON_FLAG_ROCK` (0x00004000) for stone/rock monsters; `StoneToMud` now deals `3d8` damage to any `MON_FLAG_ROCK` monster on the target tile, printing "The X cracks!" or "The X crumbles to dust!". Tagged `Stone Golem` and `Stone Giant` in `Monsters.txt` with `MON_FLAG_ROCK`. Issue closed May 25, 2026.

### ~~[#316](https://github.com/Rushwind13/JMoria/issues/316) - Strings Table / Messages wrap~~ ✅ DONE
**Type**: Enhancement/Cleanup | **Severity**: Medium | **Components**: UI, Strings, DisplayText  
**Description**: Move all hardcoded strings visible to player to a string table with lookups. Support proper message wrapping:
- Interpret `\n` as "emit '-more-' prompt and pause for user input" rather than wrapping to next line
- Maximize usage of visible text box footprint
- Standardize strings tied to EFFECT_FLAG constants
**Benefits**: Easier localization, consistent player messaging, better UI space usage
**Related Code**: All state classes, Monster.cpp, Item.cpp, DisplayText.cpp

---

## Priority 3: Visual Polish & Effects (P2)

### [#289](https://github.com/Rushwind13/JMoria/issues/289) - Multicolor beam effects for wands
**Type**: Enhancement | **Severity**: Low | **Components**: Rendering, Effects, Ranged  
**Description**: Extend single-color beam support (added April 27, 2026) to support multicolor beams for visual distinction:
- **Fire ray**: red, orange, yellow-red (wWw pattern, use alternating normal and bold colors)
- **Cold ray**: blue, light blue, white (~, ^, *)
- **Acid ray**: black, chartreuse, purple (*, o, *)
- **Lightning ray**: yellow, white, yellow-orange (*, |, *)

Alternative: non-trail beam with flickering/pulsing effect.

**Implementation**: 
- Add optional beam color properties to CEffectDef (m_szBeamChars, m_llBeamColors)
- Update Resources/Effects.txt with beam color sequences
- Extend DoTrajectory() in RangedState.cpp to use effect color data
**Related Code**: [src/RangedState.cpp](src/RangedState.cpp#L527), [src/Effect.h](src/Effect.h), Resources/Effects.txt

### [#312](https://github.com/Rushwind13/JMoria/issues/312) - AoE effect animation
**Type**: Enhancement | **Severity**: Low | **Components**: Rendering, Effects, Animation  
**Description**: Add visual animations for area-of-effect effects:
- **Wand of *Balls**: Expanding animated ring of beam tiles in multicolor to show explosion radius
- **Staff of Mass Sleep**: Clock-time-based animation coloring all affected mobs grey momentarily before returning to normal color (e.g., "The Staff of Mass Sleep turns all mobs grey for a moment, to show which ones are asleep")

**Benefits**: Clearer visual feedback for AoE damage/effects
**Related Code**: Effect.cpp, Render.cpp, RangedState.cpp

### [#314](https://github.com/Rushwind13/JMoria/issues/314) - MON_AI_SEEKRANGED
**Type**: Enhancement | **Severity**: Low | **Components**: AI, Combat  
**Description**: Add new AI brain for ranged monsters: "stay at range from player"
- Monster seeks player with SeekPlayer() until reaching internally-configured m_fRange
- Once at desired range, switches to "attack if player within range" / "move away if player approaches"
- Implements min/max range desire: keep player within maximum range > minimum range > 0 distance

**Use Case**: Ranged monsters (warlocks, archers, etc.) that want to maintain distance
**Related Code**: AIMgr.cpp, add MON_AI_SEEKRANGED to Constants.h

### [#305](https://github.com/Rushwind13/JMoria/issues/305) - Display inventory upon player death ✅ DONE
**Type**: Enhancement | **Severity**: Low | **Components**: End Game, UI  
**Description**: Improve post-death sequence:
- Upon death, all inventory and equipment become fully identified
- Display inventory and equipment on new screen between map and score list
- Use 2 columns if possible, or 2 screens if needed to fit all items
**Related Code**: EndGameState.cpp, DisplayText.cpp

---

## Implementation

### Suggested Ordering
1. **#173** should be addressed first - blocks core gameplay UX
2. **#317** and **#266** are blockers for clean test suite
3. **#309** should follow after #317 to ensure combat effects are working correctly
4. **#310** and **#315** can be done in parallel with higher-priority work
5. **#316** is foundational for future feature work but not blocking
6. **#289**, **#312**, **#314**, **#305** are polish items that can be done after core fixes


### Testing Strategy
- Unit tests in test/features/ for critical bugs (#173, #317, #309)
- Integration testing for refactored code (#310, #315)
- Visual validation for new animations (#289, #312)
- Manual gameplay testing for new AI behavior (#314)

---

## Labels Reference
- **milestone1**: Part of Phase 3d: Shiny
- **shiny**: Core visual/effect polish work
- **bug**: Critical or blocking issues
- **enhancement**: Feature additions
- **cleanup**: Code quality & refactoring
- **rendering**: Display/visual rendering
- **view-distance**: Visibility system related
