# WORKLIST: Ranged Attack Systems

**Status**: In Progress
**Related Issue**: #39 - Implement ranged attacks
**Repo**: `Rushwind13/JMoria`
**Branch**: `issue/39-ranged-weapon-system`
**Build**: `make clean ascii test`
**Test**: `cd test; ./runtests.sh`
**Areas**: Item system, fire command, secondary weapon slots, ammo mechanics

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
  `MAIN_HAND + OFF_HAND` (active) ↔ `2ND_MAIN + 2ND_OFF` (stowed, `ITEM_FLAG_STOWED`).
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
- [ ] **BUG**: Long Bow `Damage <1d8>` — must be `<1d2>` (see B2)
- [ ] **BUG**: Short Bow `Damage <1d5>` — must be `<1d2>` (see B2)
- [ ] **MISSING**: No Arrow item definitions in Items.txt at all

---

## KNOWN BUGS (must fix before f)ire is usable)

### B1 — Fire selection flow is misdesigned for bow+arrow two-piece mechanic

The f)ire path was written to mirror z)ap, but bow+arrow is different:

| Current behavior | Correct behavior |
|---|---|
| `DisplayEquipment(INV_FIRE)` shows bows from equipment | Show **inventory** filtered to ammo compatible with equipped primary weapon |
| `IsFireable(pLink)` checks if `pLink` is BOW/XBOW | Check if the **equipped primary weapon** (MAIN_HAND slot) is a ranged type |
| `GetResponse(RANGED_FIRE)` returns an inventory item by index (unfiltered) | Return an inventory item filtered to arrows/bolts matching the equipped bow type |
| `m_pSelected` tested with `TestFire()` then passed to `Fire()` | `m_pSelected` = the chosen **ammo** item; bow is read separately from MAIN_HAND slot |

### B2 — Bow damage values wrong in Items.txt

All ranged weapons must be `Damage <1d2>` (= unarmed). Current values:
- Long Bow: `1d8` → must be `1d2`
- Short Bow: `1d5` → must be `1d2`
- Any future crossbow/sling entries must also use `1d2`

---

## PRIORITIZED IMPLEMENTATION TASKS

### P0 — Wire up the f)ire command (these together make fire work end-to-end)

- [ ] **[src/CmdState.cpp]** Add `IsFireCommand()` method — plain `f` key, no modifier.
      Pattern is identical to `IsZapCommand()` (which checks plain `z`). Add an `else if` branch
      in `OnHandleKey()` routing to `STATE_RANGED`. `HandleKey()` on the new state will need to
      distinguish fire vs. zap (the `m_eCurModifier` enum already has `RANGED_FIRE`).

- [ ] **[src/CmdState.h]** Declare `IsFireCommand()`.

- [ ] **[src/RangedState.cpp / Player.cpp]** Fix Bug B1 — redesign fire selection flow:
  - In `OnHandleFire()`: read primary weapon from `EQUIP_IDX_MAIN_HAND` equipment slot.
    If slot is empty or item is not ranged (`ITEM_FLAG_NEEDSAMMO` not set): print
    "You have nothing to fire with." and abort back to `STATE_CMD`.
  - Replace `DisplayEquipment(INV_FIRE)` call with `DisplayInventory(INV_FIRE, PLACEMENT_USE)`.
  - `Player::DisplayInventory(INV_FIRE)` filters inventory to items whose `EquipType()` is
    `EQUIP_IDX_AMMO` **and** whose ammo type is compatible with the equipped bow
    (BOW → ITEM_IDX_ARROW; XBOW → ITEM_IDX_BOLT).
  - `GetResponse(RANGED_FIRE)` must return from the filtered inventory, not unfiltered.
  - If no compatible ammo found: print "You have nothing to fire." and abort.
  - If exactly one ammo type: auto-select (no prompt).
  - If multiple ammo types: show selection UI, player presses letter.
  - `m_pSelected` = the chosen ammo `CLink<CItem>*`. `TestFire()` re-validates it.
    `Fire()` consumes one unit of that ammo. Bow bonuses applied separately (see P1).

- [ ] **[Resources/Items.txt]** Fix Bug B2: set `Damage <1d2>` on Short Bow and Long Bow.

- [ ] **[Resources/Items.txt]** Add arrow item definitions. Minimum entry needed to test
 (matches actual Items.txt format — curly-brace block, no
  empty fields, spacing consistent with surrounding entries):
  ```
  Item <Flight Arrow>
  {
      Plural      <Flight Arrows>
      Type        <ITEM_IDX_ARROW>
      Value       5.0
      Flags       <ITEM_FLAG_STACKABLE>
      Level       1
      Damage      <1d8>
      Weight      0.1
      Speed       0.0
      Color       <200,200,200,255>
  }
  ```
  Additional arrows to add after testing:
  - `Arrow of Wounding` — `Damage <1d8>`, bleed effect (N turns of DoT)
  - `Fire Arrow` — `Damage <1d6>`, fire elemental hit effect
  - `Poison Arrow` — `Damage <1d6>`, poison intrinsic effect

### P1 — x)change command (core mechanic for this PR)

- [ ] **[src/Constants.h]** Add new equipment slot indices and renumber AMMO to keep it last.
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

- [ ] **[src/Constants.h]** Add `ITEM_FLAG_STOWED` item flag (use next available bit).

- [ ] **[src/CmdState.cpp / CmdState.h]** Add `IsXchangeCommand()` for plain `x` key.
      In `OnHandleKey()`, add routing branch that calls `Player::XchangeWeapons()` directly
      (no new state needed — it's a single atomic operation, like PickUp).

- [ ] **[src/Player.cpp]** Implement `Player::XchangeWeapons()`:
  - Look up the four slot nodes: `GetLink(EQUIP_IDX_MAIN_HAND)`, `GetLink(EQUIP_IDX_OFF_HAND)`,
    `GetLink(EQUIP_IDX_2ND_MAIN)`, `GetLink(EQUIP_IDX_2ND_OFF)` (any may be null/empty).
  - Call `m_llEquipment->SwapData(pMain, p2ndMain)` and
    `m_llEquipment->SwapData(pOff, p2ndOff)` to exchange slot contents.
  - Recalculate `m_fArmorClass`, `m_szDamage`, `m_fToHitModifier`, `m_fDamageModifier`,
    `m_fSpeed` by removing the old active set's contributions and adding the new set's.
  - Print "You switch to your <new primary weapon name>."
  - If new primary slot is empty: "You switch to your bare hands."

- [ ] **[src/Player.h]** Declare `XchangeWeapons()`.

- [ ] **[src/Player.cpp / DisplayEquipment]** No display logic changes needed beyond adding
      labels for the two new slot indices. `DisplayEquipment()` already iterates the full
      `m_llEquipment` list in slot-index order. Because `XchangeWeapons()` physically moves
      items between slots in the list (no flag needed), the display is always correct by
      construction. Add label strings for `EQUIP_IDX_2ND_MAIN` ("Secondary Weapon") and
      `EQUIP_IDX_2ND_OFF` ("Secondary Off-Hand") to match the format of existing slot labels.

- [ ] **[src/Player.cpp / Wield()]** Enforce that 2-handed weapons (`ITEM_FLAG_2HANDED`) clear
      the off-hand of the **same weapon set** (already partly done; verify it works for 2ND slots).

### P1 — Bow+arrow combined combat math

- [ ] **[src/RangedState.cpp]** At fire time, read the equipped bow's `m_fBonusToHit` from
      `EQUIP_IDX_MAIN_HAND` and add it to the to-hit roll used in `DoFire()` / `DoHitEffects()`.
      Arrow's own `m_fBonusToHit` and `m_fBonusToDamage` come from `m_pSelected` (the ammo item).

### P2 — Arrow ground behavior

- [ ] **[src/RangedState.cpp]** On trajectory end (wall collision, range exceeded, or miss past
      target): attempt to drop one arrow at the final trajectory tile via `Dungeon::Drop()`.
  - If tile is occupied by a non-arrow item: try orthogonally adjacent tiles (scatter up to 1
    step). Each adjacent tile tried in random order.
  - If the tile holds another arrow of the same type: stack them (`m_dwCount++` on existing tile
    item, do not call `Drop()`).
  - If no valid placement: arrow breaks and vanishes.
- [ ] **[src/RangedState.cpp]** Standard break chance on landing (e.g., 1-in-3). Guaranteed break
      on critical miss (define as to-hit roll < 5 or similar). Use a named constant.

### P2 — BDD tests

- [ ] Scenario: player with bow equipped fires flight arrow at monster — monster takes damage,
      arrow count decrements by 1
- [ ] Scenario: player without ranged weapon presses f)ire — "You have nothing to fire with."
- [ ] Scenario: player with bow but no arrows presses f)ire — "You have nothing to fire."
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
- [x] Ammo consumption (`Fire()` decrements stack)
- [ ] f)ire command wired in `CmdState`
- [ ] Arrow item definitions in Items.txt
- [ ] Fire selection flow redesigned for bow+ammo two-piece model (Bug B1 fixed)
- [ ] Bow damage corrected to 1d2 (Bug B2 fixed)
- [ ] x)change command — swap active ↔ stowed weapon sets
- [ ] Arrows drop to ground on miss/wall/range-exceed (with break chance)
- [ ] Arrow recovery (pick up from dungeon floor)

Optional:
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
| [src/Constants.h](src/Constants.h) | Add `EQUIP_IDX_2ND_MAIN/OFF`, `ITEM_FLAG_STOWED` |
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
