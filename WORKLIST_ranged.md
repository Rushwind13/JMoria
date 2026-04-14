# WORKLIST: Ranged Attack Systems

**Status**: In Progress  
**Related Issue**: #39 - Implement ranged attacks  
**Areas**: Item system, Fire command, secondary weapons, ammo mechanics

## Overview

Complete implementation of ranged mundane weapons (bows/arrows, slings/stones, crossbows/bolts). Wands with z)ap are already implemented. The f)ire command and ammo-loading mechanics remain to be completed.

---

## TOP PRIORITIES

### 1. Fix Existing Longbow Configuration ⚠️ BLOCKER
- Longbows in Items.txt currently have damage set to 1d8 (incorrect)
- Should be 1d2 (reload time penalty for ranged weapons)
- Longbows must be in secondary weapon slot; cannot be wielded as melee weapons
- **Impact**: Breaks balance; prevents testing of fire command

### 2. Implement x)change Command (Primary → Secondary Swap)
- Allow swapping primary (melee) and secondary (ranged) weapons
- Must work from equipment menu or as direct command
- Enable carrying: sword/bow, sword/shovel, any weapon-slot combination
- **Dependency**: Must complete before ammo-loading UX

### 3. Complete Fire Command Implementation
- Add f)ire command fully (currently partial)
- Integrate with loaded ammo detection
- **Dependency**: Must complete ammo-loading (#4) first

### 4. Ammo System Architecture
- Add ITEM_IDX_ARROW type constant to [src/Constants.h](src/Constants.h)
- Add ITEM_IDX_ARROW entry to [Resources/Items.txt](Resources/Items.txt)
- Extend Items to support stacking/charges (already works for wands)
- Create secondary ammo types: ITEM_IDX_STONE, ITEM_IDX_BOLT
- **Deliverable**: Player can carry multiple ammo types simultaneously

---

## IMPLEMENTATION TASKS

### Data & Constants
- [ ] Fix Longbow damage: 1d8 → 1d2 in [Resources/Items.txt](Resources/Items.txt)
- [ ] Add ITEM_IDX_ARROW to [src/Constants.h](src/Constants.h)
- [ ] Add ITEM_IDX_ARROW entry to [Resources/Items.txt](Resources/Items.txt)
- [ ] Add ITEM_IDX_STONE (for slings)
- [ ] Add ITEM_IDX_BOLT (for crossbows)
- [ ] Mark weapons with ammo type compatibility (BOW→ARROW, SLING→STONE, CROSSBOW→BOLT)

### Weapon Exchange Command (x)change)
- [ ] Implement x)change keybinding in CmdState
- [ ] Create XchangeState or add logic to UseState/ModState
- [ ] Swap item pointers in Player equipment slots (primary ↔ secondary)
- [ ] Validate weapon-slot constraints
- [ ] Display feedback "You switch to <item>"

### Ammo Loading & Fire Command
- [ ] Extend CPlayer to track "loaded ammo" state
- [ ] Implement "load bow" action (prompt if carrying multiple compatible ammo types)
- [ ] Complete f)ire command in CmdState/RangedState:
  - Require loaded ammo
  - Handle "you aren't carrying any <arrows>" case
  - Trigger RangedState (similar to TargetState pattern)
- [ ] Detect ammo type when player carries only one type (auto-select)
- [ ] Present ammo choice UI when multiple types are compatible

### Arrow Physics & Ground Behavior
- [ ] Determine max range by ammo type:
  - Short bow → Short range (limited distance)
  - Long bow → Medium range
  - Composite bow → Long range
- [ ] Arrow trajectory: fired arrows move toward targeted position
- [ ] Arrow collision detection with monsters and walls
- [ ] Arrow ground drop when:
  - Misses target (short/past/wide)
  - Hits obstacle  
  - Exceeds max range
- [ ] Arrow breakage on impact (configurable chance; critical miss → guaranteed break)
- [ ] Arrow recovery on ground (player can collect dropped arrows)

### Item Stacking & Charges
- [ ] Verify stacking system works for arrow/stone/bolt items
- [ ] Implement charge decrement on fire
- [ ] Remove item from inventory when charges reach 0

---

## ACCEPTANCE CRITERIA (from Issue #39)

✓ Already Implemented:
- [x] Bows created (ITEM_IDX_LONGBOW exists)
- [x] Wands with z)ap (ranged magic)
- [x] Distance attacks (targeting system via TargetState)

- [ ] Creating arrows (ITEM_IDX_ARROW)
- [ ] Loading bow with different arrow types
- [ ] Swap primary/secondary weapons (x)change)
- [ ] Distance-limited projectile movement
- [ ] Arrow collision with monsters
- [ ] Arrows drop to ground
- [ ] Arrow breakage/recovery mechanics

Optional:
- [ ] Creating quivers (new equipment slot; inventory space trade-off)

---

## TECHNICAL NOTES

### Ammo Compatibility Table
```
Weapon Type    | Ammo Type      | Min Range | Max Range
Short bow      | Arrow          | 2         | 8
Long bow       | Arrow          | 2         | 15
Composite bow  | Arrow          | 2         | 20
Sling          | Stone          | 1         | 10
Crossbow       | Bolt           | 3         | 18
Wand (magic)   | Charge         | N/A       | 20+
```

### Implementation Strategy
1. **Phase 1** (Blocking): Fix Longbow; implement x)change
2. **Phase 2** (Core): Ammo system + arrow constants; complete fire command
3. **Phase 3** (Physics): Range/distance; collision; ground behavior
4. **Phase 4** (Polish): Breakage chance; recovery; optional quiver system

### Related Files
- [src/Constants.h](src/Constants.h) — ITEM_IDX_* definitions
- [Resources/Items.txt](Resources/Items.txt) — Item definitions
- [src/Item.cpp](src/Item.cpp) — Item class, stacking logic
- [src/CmdState.cpp](src/CmdState.cpp) — f)ire command hook
- [src/RangedState.cpp](src/RangedState.cpp) — Targeting + arrow movement
- [src/Player.cpp](src/Player.cpp) — Equipment slots, loaded ammo tracking
- [src/Render.cpp](src/Render.cpp) — Projectile rendering

### Known Issues
- Weapon secondary slot: clarify UI (is it part of inventory or separate "quip slot"?)
- Arrow rendering: use existing tileset or add new glyph?
- Physics: Do arrows move instantly or animated frame-by-frame?

---

## TESTING CONSIDERATIONS

- [ ] BDD scenario: Load bow, fire single arrow, verify charge count
- [ ] BDD scenario: Attempt fire with no ammo (error message)
- [ ] BDD scenario: Carry multiple ammo types, prompt for selection
- [ ] BDD scenario: Arrow hits monster, arrow drops, player recovers
- [ ] Edge case: Fire beyond max range (arrow stops at boundary)
- [ ] Edge case: Arrow breaks on critical miss or wall impact

---

**Last Updated**: 2026-04-13  
**Issue Reference**: [Implement ranged attacks](https://github.com/Rushwind13/JMoria/issues/39)
