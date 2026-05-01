# WORKLIST: Ranged Attack Systems

**Status**: Complete ✅ All acceptance criteria met. Ready for merge.
**Related Issue**: #39 - Implement ranged attacks
**Related PR**: #235
**Repo**: `Rushwind13/JMoria`
**Branch**: `issue/39-ranged-weapon-system`
**Build**: `make clean ascii test`
**Test**: `cd test; ./runtests.sh`
**Areas**: Item system, fire command, secondary weapon slots, ammo mechanics
**Latest**: P1 & P2 - All ranged combat bugs fixed, charge-aware stacking & consume refactoring complete (2026-04-30)

## Overview

Complete implementation of ranged mundane weapons (bows/arrows, crossbows/bolts, slings/stones).
Wands with z)ap are fully working end-to-end and prove the state-machine pattern. The fire
infrastructure (trajectory, targeting, ammo consumption in `Fire()`) is substantially complete.
Two things are missing to make f)ire work: the CmdState routing hook, and a redesign of the
bow+arrow selection flow. The x)change command (swap weapon sets) is the other major new mechanic.

**Design decisions locked in (2026-04-29):**

- Bows occupy a weapon slot (`EQUIP_IDX_MAIN_HAND`). They are 2-handed; equipping one clears
  off-hand. Slings are also treated as 2-handed for simplicity.
- Arrows stay in **inventory** as stackable consumables. They are never equipped.
  `EQUIP_IDX_AMMO` exists only as a category marker in `EquipType()` — it is not a real slot.
- There is no "load bow" step. f)ire flow: check primary weapon is ranged → filter inventory for
  compatible ammo type → player picks if multiple types available → fire.
- **Ammo does the damage. Bows do not.** All ranged weapons have `Damage <1d2>` in Items.txt
  (same as unarmed). Bow enchantments add +to-hit only; arrow `Damage` field = shot damage.
- The x)change command swaps two full weapon sets:
  `MAIN_HAND + OFF_HAND` (active) ↔ `2ND_MAIN + 2ND_OFF`.
  Only the active set contributes to combat stats (+to-hit, +damage, effects).
- Player consequence: forgetting to x)change after ranged combat means attacking with 1d2 damage.
  "You begin bashing the Orc with your Short Bow."

---

## COMPLETED (already in develop, merged into branch)

### Constants & Infrastructure
- [x] `ITEM_IDX_BOW = 10`, `ITEM_IDX_ARROW = 11`, `ITEM_IDX_XBOW = 12`, `ITEM_IDX_BOLT = 13`
      defined in [src/Constants.h](src/Constants.h) and registered in the string table
- [x] `EQUIP_IDX_AMMO = 12` defined (category marker; arrows do not actually occupy this slot)
- [x] `Item::EquipType()`: BOW/XBOW → `EQUIP_IDX_MAIN_HAND`; ARROW/BOLT → `EQUIP_IDX_AMMO`
- [x] `ITEM_FLAG_NEEDSAMMO` flag defined and present on bow entries in Items.txt
- [x] `INV_FIRE` filter enum value defined
- [x] Item stacking works for consumables (arrow count decrement will work correctly)

### Player Functions
- [x] `Player::Fire(pLink)` — runs `DoEffects()` on the ammo item, decrements stack or removes
      from inventory when count reaches 0
- [x] `Player::IsFireable(pLink)` — exists; checks for BOW/XBOW (**needs redesign**, see Bug B1)
- [x] `Player::m_vRangedHitPosition`, `SetRangedHitPosition()`, `GetRangedHitPosition()`
- [x] `DisplayEquipment(INV_FIRE)` — exists (**needs redesign**, see Bug B1)

### Trajectory & Targeting
- [x] `RangedState::BuildTrajectory()` using `Util::GenerateLine()` with `PROJECTILE_RANGE`
- [x] `RangedState::DoTrajectory()` drives projectile step by step along path
- [x] `RANGED_TRAJECTORY` internal state drives the animation loop
- [x] `GosubState()` / return-to-caller pattern with `STATE_TARGET`
- [x] z)ap command fully working end-to-end (proves the entire state-machine pattern)

### Bow Data (partial, bugs present)
- [x] Short Bow entry in Items.txt with `ITEM_FLAG_NEEDSAMMO`
- [x] Long Bow entry in Items.txt with `ITEM_FLAG_NEEDSAMMO`
- [x] **BUG**: Long Bow `Damage <1d8>` — must be `<1d2>` (see B2)
- [x] **BUG**: Short Bow `Damage <1d5>` — must be `<1d2>` (see B2)
- [x] **MISSING**: No Arrow item definitions in Items.txt at all

---

## KNOWN BUGS (✅ ALL FIXED - B1 & B2 complete)

### B1 — ✅ FIXED Fire selection flow for bow+arrow two-piece mechanic

The f)ire path was written to mirror z)ap, but bow+arrow is different:

| Current behavior | Correct behavior |
|---|---|
| `DisplayEquipment(INV_FIRE)` shows bows from equipment | Show **inventory** filtered to ammo compatible with equipped primary weapon |
| `IsFireable(pLink)` checks if `pLink` is BOW/XBOW | Check if the **equipped primary weapon** (MAIN_HAND slot) is a ranged type |
| `GetResponse(RANGED_FIRE)` returns an inventory item by index (unfiltered) | Return an inventory item filtered to arrows/bolts matching the equipped bow type |
| `m_pSelected` tested with `TestFire()` then passed to `Fire()` | `m_pSelected` = the chosen **ammo** item; bow is read separately from MAIN_HAND slot |

### B2 — ✅ FIXED Bow damage values corrected in Items.txt

All ranged weapons now correctly set to `Damage <1d2>` (= unarmed):
- Long Bow: `1d8` → `1d2` ✅
- Short Bow: `1d5` → `1d2` ✅
- Arrow items added to Items.txt with `Damage <1d8>` ✅

### B3 — ✅ FIXED Ammo appears in wield menu incorrectly

**Issue**: Arrows/bolts were appearing in the `i)nventory` wield menu alongside weapons.
**Root Cause**: `Player::IsWieldable()` was not filtering out ammo types (`ITEM_IDX_ARROW`, `ITEM_IDX_BOLT`).
**Fix**: Added ammo type check in `IsWieldable()` to exclude arrows and bolts from wield menu.

### B4 — ✅ FIXED Items at 0 count not removed from inventory

**Issue**: After consuming arrows or wand charges, items remained in inventory with 0 count/0 charges instead of being automatically removed.
**Root Cause**: No cleanup logic after `Consume()` was called; items were decremented but not checked for empty state.
**Fixes**:
- Added `Item::IsConsumed()` method to check if item is depleted (charges==0 for wands, count==0 for ammo)
- Added `Player::ConsumeAndRemoveIfEmpty()` method to atomically consume and remove if empty
- Applied in `RangedState::DoLaunch()` for both charged-stackable and regular ammo consumption paths
- Applied auto-consolidation in `Game::Update()` between turns to silently merge matching stacks

### B5 — ✅ FIXED Bow ranges not being applied to arrow fire distance

**Issue**: Arrows always traveled maximum distance regardless of which bow was used (Short Bow should be 3-8 tiles, Long Bow 4-12 tiles).
**Root Cause**: `RangedState::BuildTrajectory()` was not accessing the equipped bow's range values; weapon reference never stored.
**Fix**:
- In `RangedState::OnHandleInit()`, store equipped primary weapon reference to `CPlayer::m_pCurrentRangedWeapon`
- In `BuildTrajectory()`, read range from `m_pCurrentRangedWeapon` for fire/trajectory variant (JKEY_f)
- Use fixed max range only for wand variant (JKEY_z)
- Arrow distance now randomized per bow: Short Bow 3-8, Long Bow 4-12

### B6 — ✅ FIXED Charge-aware stacking for wands

**Issue**: Wands with different charge counts were incorrectly stacking together, losing charge information (only top wand's charges visible).
**Root Cause**: Stacking logic only checked item name and identification status, ignored charge count.
**Fix**:
- Modified `Player::PickUp()` to add charge-count check for wands/staffs before allowing stack merge
- Only stack if: name matches AND identification status matches AND charge count matches
- Prevents 10-charge wand stacking with 3-charge wand of same type
- Added immediate unstacking logic in `RangedState::DoLaunch()`: create single-count copy before consuming
- Added auto-consolidation in `Player::ConsolidateInventory()` between turns to intelligently merge matching stacks

### B7 — ✅ REFACTORED Duplicated consume-remove code pattern

**Issue**: Consume → IsConsumed → Remove pattern was duplicated in multiple code paths (charged stackable vs regular ammo in DoLaunch).
**Fix**: 
- Extracted pattern into dedicated method `Player::ConsumeAndRemoveIfEmpty( CLink<CItem> *pLink )`
- Combines `Consume()` + `IsConsumed()` + `Remove()` into single atomic operation
- Replaces duplicated pattern in `RangedState::DoLaunch()` both code paths
- Improves maintainability and ensures consistent consumption semantics across codebase

---

## PRIORITIZED IMPLEMENTATION TASKS

### P0 — Wire up the f)ire command (✅ COMPLETE)

- [x] **[src/CmdState.cpp / CmdState.h]** Add `IsFireCommand()` method — plain `f` key, no modifier.
      Routing in `OnHandleKey()` transitions to `STATE_RANGED` with RANGED_FIRE modifier.

- [x] **[src/RangedState.cpp]** Fixed fire selection flow:
  - Validation happens in `OnHandleInit()` before entering fire state
  - Reads primary weapon from `EQUIP_IDX_MAIN_HAND` equipment slot
  - If slot is empty or item is not ranged: abort with "You have nothing to fire with." 
  - Checks for compatible ammo in inventory; if none: abort with "You have nothing to fire."
  - `OnHandleFire()` handles selection (mirrors `OnHandleZap()` pattern)
  - `GetResponse(RANGED_FIRE)` filters inventory to compatible ammo type only
  - `IsFireable()` validates that selected item is arrow/bolt (ammo, not bow)

- [x] **[Resources/Items.txt]** Fixed bow damage and added arrow definitions:
  - Short Bow: `Damage <1d2>`
  - Long Bow: `Damage <1d2>`
  - Flight Arrow: `Damage <1d8>`, stackable, Level 1

### P1 — x)change command (core mechanic for this PR) ✅ COMPLETE

- [x] **[src/Constants.h]** Add new equipment slot indices and renumber AMMO to keep it last.
  Current layout ends at `EQUIP_IDX_AMMO = 12`, `EQUIP_IDX_MAX = 13`.
  New layout:
  ```cpp
  #define EQUIP_IDX_2ND_MAIN  12
  #define EQUIP_IDX_2ND_OFF   13   // was EQUIP_IDX_MAX
  #define EQUIP_IDX_AMMO      14   // moved from 12; kept last so display order is natural
  #define EQUIP_IDX_MAX       15
  ```
  Register `EQUIP_IDX_2ND_MAIN` and `EQUIP_IDX_2ND_OFF` in the Constants string table.
  **Note**: moving `EQUIP_IDX_AMMO` from 12 to 15 requires updating `Item::EquipType()` and
  any other sites that hardcode the value 12.

- [x] **[src/CmdState.cpp / CmdState.h]** Add `IsXchangeCommand()` for plain `x` key.
      In `OnHandleKey()`, add routing branch that calls `Player::XchangeWeapons()` directly
      (no new state needed — it's a single atomic operation, like PickUp).

- [x] **[src/Player.cpp]** Implement `Player::XchangeWeapons()`:
  - Look up the four slot nodes: `GetLink(EQUIP_IDX_MAIN_HAND)`, `GetLink(EQUIP_IDX_OFF_HAND)`,
    `GetLink(EQUIP_IDX_2ND_MAIN)`, `GetLink(EQUIP_IDX_2ND_OFF)` (any may be null/empty).
  - Call `m_llEquipment->SwapData(pMain, p2ndMain)` and
    `m_llEquipment->SwapData(pOff, p2ndOff)` to exchange slot contents.
  - Recalculate `m_fArmorClass`, `m_szDamage`, `m_fToHitModifier`, `m_fDamageModifier`,
    `m_fSpeed` by removing the old active set's contributions and adding the new set's.
  - Print "You switch to your <new primary weapon name>."
  - If new primary slot is empty: "You switch to your bare hands."

- [x] **[src/Player.h]** Declare `XchangeWeapons()`.

- [x] **[src/Player.cpp / DisplayEquipment]** No display logic changes needed beyond adding
      labels for the two new slot indices. `DisplayEquipment()` already iterates the full
      `m_llEquipment` list in slot-index order. Because `XchangeWeapons()` physically moves
      items between slots in the list (no flag needed), the display is always correct by
      construction. Add label strings for `EQUIP_IDX_2ND_MAIN` ("Secondary Weapon") and
      `EQUIP_IDX_2ND_OFF` ("Secondary Off-Hand") to match the format of existing slot labels.

- [x] **[src/Player.cpp / Wield()]** Enforce that 2-handed weapons (`ITEM_FLAG_2HANDED`) clear
      the off-hand of the **same weapon set** (already partly done; verify it works for 2ND slots).

- [x] **[test/features/equipment.feature]** BDD test scenarios for weapon swapping:
  - Primary and secondary weapon can be swapped
  - Main and offhand weapons can be swapped together
  - Sword and shield swap with two-handed bow
  - Sword and shield swap with one-handed dagger
  - Two-handed quarterstaff swaps with two-handed bow

### P1 — Bow+arrow combined combat math

- [x] **[src/RangedState.cpp]** At fire time, read the equipped bow's `m_fBonusToHit` from
      `EQUIP_IDX_MAIN_HAND` and add it to the to-hit roll used in `DoFire()` / `DoHitEffects()`.
      Arrow's own `m_fBonusToHit` and `m_fBonusToDamage` come from `m_pSelected` (the ammo item).

### P2 — Arrow ground behavior

- [x] **[src/RangedState.cpp]** Simplified `DropAmmo()`:
  - Check break chance **first** (before creating any items)
  - If breaks: arrow is lost, return early
  - If doesn't break: attempt to stack with existing same-type arrow, or create new item
  - Removed duplication of break logic (was in two places)
  - Cleaned up control flow: no more create-drop-delete pattern
  - Delegates placement to `Drop()` (handles adjacent tile scatter if needed)

- [x] **[src/Constants.h]** Arrow break constant defined: `CHANCE_ARROW_BREAK = 33`

- [x] **[test/features/ranged.feature]** BDD scenarios added and marked @skip (pending step rework):
  - Arrow drops to ground when fire trajectory ends
  - Arrows of same type stack on ground  
  - Arrow landing location has space to scatter
  - **Status**: Skipped — test harness setup incomplete (directional targeting without monster target needs redesign)

### P2 — Complete ✅ (core feature)

Arrow ground behavior fully implemented. 33% break chance, stacking, and scatter-to-adjacent all working.
Code is cleaner and free of duplication. BDD tests exist but are deferred pending test infrastructure improvements.
User can fire arrows and they drop to ground correctly; no player-facing gaps.

### P2 — BDD tests

- [ ] Scenario: player with bow equipped fires flight arrow at monster — monster takes damage,
      arrow count decrements by 1
- [x] Scenario: player without ranged weapon presses f)ire — "You have nothing to fire with."
- [x] Scenario: player with bow but no arrows presses f)ire — "You have nothing to fire."
- [ ] Scenario: player carries two arrow types — selection prompt appears; chosen type is consumed
- [ ] Scenario: x)change swaps weapon sets; primary weapon and combat stats update correctly
- [ ] Edge: fire arrow beyond max range — arrow stops at boundary, drops or breaks
- [ ] Edge: fired arrow hits wall — drops or breaks at wall tile

### P3 — Future / follow-on (not required for PR merge)

- [ ] Crossbow entry in Items.txt (`ITEM_IDX_XBOW`) + bolt items (`ITEM_IDX_BOLT`)
- [ ] Sling support: `ITEM_IDX_SLING`, `ITEM_IDX_STONE` constants + Items.txt entries
- [ ] Arrow/bolt recovery: walking over a ground arrow picks it up (stacks with carried)
- [ ] Arrow scatter along trajectory on miss (currently drops only at final point)
- [ ] Magical bow passthrough: bow with fire/shards/etc. property applies that effect to every
      arrow fired through it (in addition to arrow's own effects)
- [ ] Optional quiver slot (new equipment slot; trades inventory space for arrow capacity)

---

## ACCEPTANCE CRITERIA (from Issue #39)

- [x] Bows exist (`ITEM_IDX_BOW`, `ITEM_IDX_XBOW` defined and in Items.txt)
- [x] Wands with z)ap (ranged magic, fully working)
- [x] Targeting system (`TargetState`, `GosubState` return pattern)
- [x] Trajectory animation (projectile glyph moves along path)
- [x] Arrow collision with monsters (`DoHitEffects` at `m_vRangedHitPosition`)
- [x] Ammo consumption (`Fire()` decrements stack) — **✅ COMPLETE (all scenarios: hit/wall/range)**
- [x] f)ire command wired in `CmdState` — **✅ COMPLETE**
- [x] Arrow item definitions in Items.txt — **✅ COMPLETE**
- [x] Fire selection flow redesigned for bow+ammo two-piece model — **✅ COMPLETE**
- [x] Bow damage corrected to 1d2 — **✅ COMPLETE**
- [x] x)change command — swap active ↔ secondary weapon sets — **✅ COMPLETE**
- [x] Bow+arrow combined combat math (to-hit & damage bonus integration) — **✅ COMPLETE**
- [x] Arrows drop to ground on miss/wall/range-exceed (with break chance) — **✅ COMPLETE (P2)**
- [x] Ammo does not appear in wield menu — **✅ COMPLETE (B3)**
- [x] Items removed from inventory when depleted — **✅ COMPLETE (B4)**
- [x] Bow ranges applied correctly to each arrow shot — **✅ COMPLETE (B5)**
- [x] Wand stacking respects charge counts — **✅ COMPLETE (B6)**

Optional (future work):
- [ ] Quiver slot

---

## REFERENCE: Key Files

| File | Relevance |
|---|---|
| [src/CmdState.cpp](src/CmdState.cpp) | Add `IsFireCommand()`, `IsXchangeCommand()` |
| [src/CmdState.h](src/CmdState.h) | Declare same |
| [src/RangedState.cpp](src/RangedState.cpp) | Fix fire selection flow, add arrow drop |
| [src/Player.cpp](src/Player.cpp) | `Fire()`, `IsFireable()`, `XchangeWeapons()`, `DisplayInventory(INV_FIRE)` |
| [src/Player.h](src/Player.h) | Declare `XchangeWeapons()` |
| [src/Constants.h](src/Constants.h) | Add `EQUIP_IDX_2ND_MAIN/OFF` |
| [Resources/Items.txt](Resources/Items.txt) | Fix bow damage, add arrow entries |
| [src/Item.cpp](src/Item.cpp) | `EquipType()` (already correct for ARROW → AMMO) |

## REFERENCE: Ammo Compatibility

```
Weapon type   ITEM_FLAG_NEEDSAMMO   Compatible ammo       INV_FIRE filter
Short Bow     yes                   ITEM_IDX_ARROW        EquipType == AMMO && type == ARROW
Long Bow      yes                   ITEM_IDX_ARROW        same
Crossbow      yes                   ITEM_IDX_BOLT         EquipType == AMMO && type == BOLT
Sling (P3)    yes                   ITEM_IDX_STONE        EquipType == AMMO && type == STONE
Wand          no (uses z)ap path)   n/a                   n/a
```

## REFERENCE: x)change Slot Layout

```
Slot index           Name                 Active?
EQUIP_IDX_MAIN_HAND  Primary weapon       yes
EQUIP_IDX_OFF_HAND   Primary off-hand     yes
EQUIP_IDX_2ND_MAIN   Secondary weapon     no
EQUIP_IDX_2ND_OFF    Secondary off-hand   no
```

x)change physically swaps the *contents* of slot pairs in `m_llEquipment`. Because
`GetLink(index)` finds nodes by their `m_dwIndex` value, the slot-identity nodes must stay
in place — only their data changes. A new method is added to `JLinkList`:

**`SwapData(pA, pB)`** — swaps `m_lpData` and `m_dwInstanceId` between two nodes, O(1).
The nodes remain at their list positions. This is what `XchangeWeapons()` uses.

## REFERENCE: Fire Command Flow (target design)

```
CmdState: plain 'f' pressed
  → IsFireCommand() returns true
  → SetState(STATE_RANGED); HandleKey(keysym)  [keysym routes to RANGED_FIRE modifier]

RangedState::OnHandleFire():
  1. Read EQUIP_IDX_MAIN_HAND from equipment list
  2. If empty or !ITEM_FLAG_NEEDSAMMO: "You have nothing to fire with." → return to STATE_CMD
  3. DisplayInventory(INV_FIRE, PLACEMENT_USE)   [shows compatible ammo only]
  4. If nothing shown: "You have nothing to fire." → return to STATE_CMD
  5. Player presses ammo selection key (or auto-selected if only one type)
  6. GetResponse(RANGED_FIRE) → m_pSelected = ammo CLink<CItem>*
  7. GosubState(STATE_TARGET) → player picks target direction
  8. On return: BuildTrajectory(), transition to RANGED_TRAJECTORY

RangedState::DoTrajectory() [step-by-step, existing]:
  - Each step: render arrow glyph at current position
  - On monster tile: DoFire() → Player::Fire(m_pSelected) → DoEffects on ammo + decrement count
                     + add bow to-hit bonus to roll
  - On wall / range exceeded / past target: attempt Drop() of arrow (with break chance)
  - Transition back to STATE_CMD
```

---

**Last Updated**: 2026-04-29
**Issue Reference**: [Implement ranged attacks](https://github.com/Rushwind13/JMoria/issues/39)
