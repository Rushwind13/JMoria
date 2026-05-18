# Phase 3d: Shiny - Prioritized Roadmap

**Status**: Roadmap for upcoming phase following Phase 3c Polish (PR#306 merged into develop)  
**Last Updated**: May 18, 2026  
**Total Issues**: 12

---

## Overview

Phase 3d: Shiny focuses on visual polish, effect refinements, and bug fixes for core gameplay systems. This roadmap assumes Phase 3c Polish (PR#306) has been fully integrated into `develop`.

---

## Priority 1: Critical Bugs (P0)

### [#173](https://github.com/Rushwind13/JMoria/issues/173) - Items disappear at wrong view distance during gameplay
**Type**: Bug | **Severity**: High | **Components**: Rendering, View Distance  
**Description**: Items are disappearing from view at incorrect distances. Items visible 2 steps away become invisible when adjacent, only reappearing when stepped on. Dropped items don't render until player stands on them.
**Impact**: Core gameplay UX - players cannot see items they're trying to pick up
**Acceptance Criteria**:
- Items remain visible in all adjacent tiles (full 8-direction visibility)
- Dropped items render immediately when placed
- Item visibility matches movement range
**Related Code**: RenderASCII.cpp, Render.cpp, possibly DungeonMap.cpp or Draw.h

### [#317](https://github.com/Rushwind13/JMoria/issues/317) - Melee "touch" attacks are being emitted at range
**Type**: Bug | **Severity**: High | **Components**: Combat, Attacks  
**Description**: Non-adjacent, non-moving monsters (e.g., White Jelly, Gas Spore) are landing melee "touch" attacks from a distance with no damage, creating confusing combat feedback.
**Impact**: Combat is unclear and misleading to player
**Related Code**: Attack resolution in Monster.cpp or AIMgr.cpp

### [#266](https://github.com/Rushwind13/JMoria/issues/266) - Town level spawn warnings
**Type**: Bug | **Severity**: Medium | **Components**: Item/Monster Spawning  
**Description**: When spawning items or monsters at town depth (depth 0), warnings are logged: "Couldn't find a suitable item for this depth" and "got an invalid item: -1". Should have town-appropriate spawning.
**Impact**: Test suite warnings; may indicate logic issues in depth-appropriate spawning
**Related Code**: Dungeon.cpp monster/item spawn logic

### [#309](https://github.com/Rushwind13/JMoria/issues/309) - Intrinsic effects seem backwards
**Type**: Bug | **Severity**: Medium | **Components**: Effects, Status Display  
**Description**: 
- Quaff Potion of Courage shows "afraid" on Character Stats (should show resistance to fear); player can still attack
- Potion of Slow Poison shows "poisoned" intrinsic but no HP loss occurs
**Impact**: Player confusion about active effects and status
**Acceptance Criteria**: Intrinsic display matches actual effect state

---

## Priority 2: Code Cleanup & Refactoring (P1)

### [#310](https://github.com/Rushwind13/JMoria/issues/310) - Refactor "noticeable effect" identification
**Type**: Cleanup | **Severity**: Medium | **Components**: Effects, UseState  
**Description**: The same identification code block exists in at least 4 places:
```cpp
JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );
if( !(pItem->IsIdentified()) && m_bLastEffectNoticed )
{
    pItem->Identify();
    g_pGame->GetMsgs()->Printf( "You recognize it as a %s.\n", pItem->GetName() );
}
return retval;
```
Should consolidate into one location (likely near where quaff/read/use paths are chosen) or move inside `Identify()`.
**Related Code**: Multiple locations in Player.cpp, UseState.cpp, and effect handling

### [#315](https://github.com/Rushwind13/JMoria/issues/315) - Refactor "Player" Effects
**Type**: Cleanup | **Severity**: Medium | **Components**: Player, Effects  
**Description**: Many `Do*` functions on `CPlayer` belong elsewhere:
- `DoSummonMonsters` → `CEffect`
- `DoRemove/ApplyCurse` → `CItem`
- `DoLightArea` / `DoMagicMapping` → `CDungeon`
- `NeedsItemChoice()` → `CEffect` (UseState cycle context)
- `FindNeedsChoiceEffect()` → `CItemDef`

**Outcome**: Better separation of concerns, cleaner CPlayer interface

### [#311](https://github.com/Rushwind13/JMoria/issues/311) - Stone to Mud should melt doors
**Type**: Enhancement/Cleanup | **Severity**: Low | **Components**: Effects  
**Description**: Stone to Mud effect currently melts stone walls. Should also:
- Melt doors and secret doors (but not open doors)
- Leave treasure behind (when treasure system exists)
- Damage "rock" type monsters (stone golems, stone giants, etc.)
**Related Code**: Effect execution in Effect.cpp; may need CEffectFlag additions

### [#316](https://github.com/Rushwind13/JMoria/issues/316) - Strings Table / Messages wrap
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
- **Fire ray**: red, orange, yellow-red (*, r, R pattern)
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

### [#305](https://github.com/Rushwind13/JMoria/issues/305) - Display inventory upon player death
**Type**: Enhancement | **Severity**: Low | **Components**: End Game, UI  
**Description**: Improve post-death sequence:
- Upon death, all inventory and equipment become fully identified
- Display inventory and equipment on new screen between map and score list
- Use 2 columns if possible, or 2 screens if needed to fit all items
**Related Code**: EndGameState.cpp, DisplayText.cpp

---

## Implementation Notes

### Dependencies & Ordering
1. **#173** should be addressed first - blocks core gameplay UX
2. **#317** and **#266** are blockers for clean test suite
3. **#309** should follow after #317 to ensure combat effects are working correctly
4. **#310** and **#315** can be done in parallel with higher-priority work
5. **#316** is foundational for future feature work but not blocking
6. **#289**, **#312**, **#314**, **#305** are polish items that can be done after core fixes

### Phase 3c Polish Assumptions
- PR#306 has been merged to `develop`
- All Phase 3c polish features are available for use in Phase 3d
- Equipment system uses ITEM_FLAG_EQUIPMENT (data-driven)
- Basic effect system is stable

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
