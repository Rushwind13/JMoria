# Issue #288: ITEM_IDX_STAFF Implementation — Complete Work Breakdown

**Repository**: Rushwind13/JMoria  
**Issue**: https://github.com/Rushwind13/JMoria/issues/288  
**Labels**: milestone1, cleanup  
**Status**: IN PROGRESS — Part 5 High Priority complete, Part 3 medium priority pending

---

## Executive Summary

Issue #288 requires:
1. **Keybind verification**: Staves should use "Z" (uppercase), wands use "z" (lowercase)
2. **Effect mapping audit**: Verify all 42 wands/staves have Effects defined in Effects.txt
3. **Implementation verification**: Confirm which effect implementations work vs. need coding
4. **Documentation**: Create comprehensive list of each wand/staff with in-game effect description and implementation status

**Total items to audit**: 42 (18 wands + 24 staves)

---

## PART 1: KEYBINDING IMPLEMENTATION

### Current State
- `IsZapCommand()` in [src/CmdState.cpp](src/CmdState.cpp#L422) only checks for lowercase 'z'
- No uppercase 'Z' keybind exists for staves
- **Design rule**: Wands always target a specific tile/monster (directed). Staves always affect the player directly or an area centered on the player (never targeted). This is the fundamental difference.

### Work Items

**1.0 — Add EFFECT_MOD_AREA handler** (NEW FUNCTIONALITY)
- Files: [src/Player.cpp](src/Player.cpp), [src/Monster.cpp](src/Monster.cpp)
- Update `CPlayer::DoHitEffects( CEffect *pEffect )` function to handle EFFECT_MOD_AREA
- Update `CMonster::AttackEffect()` function to handle EFFECT_MOD_AREA
- When an Effect has EFFECT_MOD_AREA, it affects all targets within m_fRange of the caster, possibly with diminishing effect as range increases.
- This is the way that ITEM_IDX_STAFF will affect AoE, a radius around the staff user (could be player, could be monster, once monsters can use items (not implemented yet))

**1.1 — Add EFFECT_MOD_BALL handler** (NEW FUNCTIONALITY)
- Files: [src/Player.cpp](src/Player.cpp), [src/Monster.cpp](src/Monster.cpp)
- Update `CPlayer::DoHitEffects( CEffect *pEffect )` function to handle EFFECT_MOD_BALL
- Update `CMonster::AttackEffect()` function to handle EFFECT_MOD_BALL
- When an Effect has EFFECT_MOD_BALL, it acts like a EFFECT_MOD_LINE projectile for targeting and animation. When the (single tile) projectile hits a target (or max range) it explodes to affect all targets within m_fRange of the final projectile position.
- This is the way that ITEM_IDX_WAND will affect AoE, a projectile with a blast radius around the target.

**1.2 — Add Z keybind detection** (NEW FUNCTION)
- File: [src/CmdState.cpp](src/CmdState.cpp)
- Create `IsStaffCommand(JKeysym *keysym)` function
- Check for 'Z' (uppercase, no modifiers)
- Add case in main `OnHandleKey()` switch — routes to STATE_RANGED with 'Z' stored as command

**1.3 — Bypass targeting for staves in RangedState** (MODIFY)
- File: [src/RangedState.cpp](src/RangedState.cpp)
- When `m_cCommand == JKEY_Z`: skip item selection, skip trajectory, skip target selection
- Flow: Z pressed → select staff from inventory → **immediately fire effects at player position** → done
- No trajectory is built; `m_vRangedHitPosition` is set to player position
- HEAL/INTRINSIC effects apply to player; AREA effects radiate from player position
- Wands (z): unchanged — require explicit target (current behavior)

**1.4 — Update Player Docs** (DOCUMENTATION)
- File: [doc/Player Docs.txt](doc/Player%20Docs.txt)
- Add Z keybind to command help
- Clarify staff vs wand usage

---

## PART 2: EFFECT MAPPING AUDIT

### Wands (18 total) — All Effects Exist in Effects.txt ✅

| Wand | Effect Name | EFFECT_TYPE | Status |
|---|---|---|---|
| Wand of Heal Monster | Heal Monster | HIT + EFFECT_FLAG_HP | ❌ NOT IMPLEMENTED |
| Wand of Light | Light Ray | HIT + EFFECT_FLAG_LIGHT | ✅ WORKS |
| Wand of Stone to Mud | Stone to Mud | HIT + EFFECT_FLAG_STONE_TO_MUD | ✅ IMPLEMENTED |
| Wand of Sleep | Cause Sleep | HIT + EFFECT_FLAG_SLEEP | ✅ IMPLEMENTED (DoStatusHit) |
| Wand of Confusion | Cause Confusion | HIT + EFFECT_FLAG_CONFUSE | ✅ IMPLEMENTED (DoStatusHit) |
| Wand of Firebolts | Firebolt | HIT + EFFECT_FLAG_FIRE + MOD_LINE | ✅ WORKS |
| Wand of Fear | Cause Fear | HIT + EFFECT_FLAG_AFRAID | ✅ IMPLEMENTED (DoStatusHit) |
| Wand of Frost | Frost Bolt | HIT + EFFECT_FLAG_COLD + MOD_LINE | ✅ WORKS |
| Wand of Lightning | Lightning Bolt | HIT + EFFECT_FLAG_ELECTRICITY + MOD_LINE | ✅ WORKS |
| Wand of Summoning | Summon Monsters | CREATE + EFFECT_FLAG_SUMMON | ✅ WORKS |
| Wand of Teleport Away | Teleport Away | HIT + EFFECT_FLAG_TELEPORT | ✅ IMPLEMENTED (DoTeleportAway) |
| Wand of Acid | Acid Bolt | HIT + EFFECT_FLAG_ACID + MOD_LINE | ✅ WORKS |
| Wand of Fireballs | Fireball | HIT + EFFECT_FLAG_FIRE + MOD_BALL | ✅ IMPLEMENTED (DoBallHit) |
| Wand of Frost Balls | Frost Ball | HIT + EFFECT_FLAG_COLD + MOD_BALL | ✅ IMPLEMENTED (DoBallHit) |
| Wand of Lightning Balls | Lightning Ball | HIT + EFFECT_FLAG_ELECTRICITY + MOD_BALL | ✅ IMPLEMENTED (DoBallHit) |
| Wand of Paralyze | Cause Paralysis | HIT + EFFECT_FLAG_PARALYZE | ✅ IMPLEMENTED (DoStatusHit) |
| Wand of Probing | Probe | HIT + EFFECT_FLAG_IDENTIFY | ❌ MISSING |
| Wand of Acid Balls | Acid Ball | HIT + EFFECT_FLAG_ACID + MOD_BALL | ✅ IMPLEMENTED (DoBallHit) |

**Wands Summary**: 16 working ✅, 1 needs implementation ❌ (Wand of Heal Monster, Wand of Probing)

---

### Staves (24 total) — All Effects Exist in Effects.txt ✅

| Staff | Effect(s) | EFFECT_TYPE | Status |
|---|---|---|---|
| Staff of Cure Light Wounds | CLW + Cure Poison + Cure Blindness | HEAL | ✅ WORKS |
| Staff of Light | Light Area | CREATE + EFFECT_FLAG_LIGHT | ✅ WORKS |
| Staff of Healing | Minor Healing + Cure Poison | HEAL | ✅ WORKS |
| Staff of Mapping | Partial Mapping | CREATE + EFFECT_FLAG_MAPPING + MOD_AREA | ✅ WORKS |
| Staff of Treasure Detection | Detect Treasure | CREATE + EFFECT_FLAG_TREASURE (player-centered) | ❓ NOT FOUND IN CODE |
| Staff of Sleep | Cause Sleep | HIT + EFFECT_FLAG_SLEEP + MOD_AREA (player-centered) | ✅ IMPLEMENTED (DoAreaHit) |
| Staff of Acid Resistance | Timed Resist Acid | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Cold Resistance | Timed Resist Cold | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Electricity Resistance | Timed Resist Electricity | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Fear | Mass Fear | HIT + EFFECT_FLAG_AFRAID + MOD_AREA (player-centered) | ✅ WORKS |
| Staff of Fire Resistance | Timed Resist Fire | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Protection | Timed Blessing | HIT + EFFECT_FLAG_AC + MOD_TIMED (player-centered) | ❌ NEEDS AREA HANDLER |
| Staff of Starlight | Light Ray + Light Area | HIT/CREATE + EFFECT_FLAG_LIGHT | ✅ WORKS |
| Staff of Teleportation | Teleport Self | CREATE + EFFECT_FLAG_TELEPORT | ✅ WORKS |
| Staff of Paralysis | Mass Paralyze | HIT + EFFECT_FLAG_PARALYZE + MOD_AREA (player-centered) | ✅ IMPLEMENTED (DoAreaHit) |
| Staff of Perception | Identify | RESTORE + EFFECT_FLAG_IDENTIFY | ❌ "emits an Identify" should choose an item to ID |
| Staff of Summoning | Summon Monsters | CREATE + EFFECT_FLAG_SUMMON | ✅ WORKS |
| Staff of Telepathy | Timed ESP | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Mass Sleep | Mass Sleep | HIT + EFFECT_FLAG_SLEEP + MOD_AREA (player-centered, large radius) | ✅ IMPLEMENTED (DoAreaHit) |
| Staff of Resistance | All 4 resistances | INTRINSIC + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Word of Recall | Recall | CREATE + EFFECT_FLAG_RECALL | ✅ WORKS |
| Staff of *Resistance* | All 4 immunities | INTRINSIC + EFFECT_MOD_IMMUNE + EFFECT_MOD_TIMED | ✅ WORKS |
| Staff of Cure Serious Wounds | CSW + Cure Poison + Blindness + Confusion | HEAL | ✅ WORKS |
| Staff of Greater Healing | Greater Healing + 4 cures | HEAL | ✅ WORKS |

**Staves Summary**: 22 working ✅, 1 unknown ❓ (Detect Treasure), 1 pending ❌ (Staff of Protection)

> **Note**: All staves apply effects at player position. HEAL/INTRINSIC/CREATE/RESTORE staves already work because DoEffects() handles those types directly. The 8 ❌ staves require the EFFECT_MOD_AREA handler (task 1.0) — they have no targeting gap, only missing AoE dispatch.

---

## PART 3: MISSING EFFECT IMPLEMENTATIONS

### Critical Missing Single-Target HIT Effects (Wands Only)

These effects are needed by wands (targeted at a specific monster). Staves with the same flags use EFFECT_MOD_AREA and are handled by task 1.0 — they do **not** need these single-target functions.

**3.1 — Implement EFFECT_FLAG_PARALYZE (single-target wand)**
- **Used by**: Wand of Paralyze
- **Required behavior**: Target monster becomes paralyzed for N turns
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to modify**: Add case to `DoHitEffects()` → new function `DoParalyzeHit(pEffect)`

**3.2 — Implement EFFECT_FLAG_AFRAID (single-target wand)**
- **Used by**: Wand of Fear
- **Required behavior**: Target monster becomes afraid for N turns
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to modify**: Add case to `DoHitEffects()` → new function `DoFearHit(pEffect)`

**3.3 — Implement EFFECT_FLAG_SLEEP (single-target wand)**
- **Used by**: Wand of Sleep *(Staff of Sleep uses AREA handler — covered by task 1.0)*
- **Required behavior**: Target monster falls asleep for N turns
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to modify**: Add case to `DoHitEffects()` → new function `DoSleepHit(pEffect)`

**3.4 — Implement EFFECT_FLAG_CONFUSE (single-target wand)**
- **Used by**: Wand of Confusion
- **Required behavior**: Target monster becomes confused for N turns
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to modify**: Add case to `DoHitEffects()` → new function `DoConfuseHit(pEffect)`

### Complex Effects Needing Implementation

**3.5 — Implement EFFECT_FLAG_STONE_TO_MUD**
- **Used by**: Wand of Stone to Mud
- **Effect description**: "Destroy rock wall at target location"
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to add**: `DoStoneMudHit(pEffect)`
- **Logic**: Check if target tile is DUNG_IDX_WALL, replace with DUNG_IDX_FLOOR
- **Reference**: Partial reference in Effects.txt definition

**3.6 — Implement EFFECT_FLAG_TELEPORT (single-target HIT)**
- **Used by**: Wand of Teleport Away
- **Issue**: CREATE type exists but HIT type missing (need to teleport TARGET away, not player)
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to add**: `DoTeleportAwayHit(pEffect)` 
- **Logic**: Random teleport for monster at ranged hit position
- **Reference**: `DoTeleport()` exists for self, but need reverse logic for monsters

**3.7 — Implement EFFECT_FLAG_IDENTIFY (single-target HIT, wand only)**
- **Used by**: Wand of Probing *(Staff of Perception uses RESTORE+IDENTIFY — already works)*
- **Effect description**: "Reveal details about target monster"
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to add**: `DoProbeHit(pEffect)`
- **Logic**: Reveal full monster stats/HP in message, mark monster as "probed"
- **Reference**: `MonsterRecall` system may already exist

**3.8 — Implement EFFECT_FLAG_HP (HIT type - Heal Monster buff)**
- **Used by**: Wand of Heal Monster
- **Issue**: This is a BUFF spell (heals monster), unusual for wand
- **Decision needed**: It is intentional to have some wands be dangerous; player learning is the key gameplay mechanic.

### Missing Detect Effect

**3.9 — Verify/Implement Detect Treasure**
- **Used by**: Staff of Treasure Detection
- **Status**: Effect defined in Effects.txt but NOT found in DoCreateEffects() code
- **Implementation file**: [src/Player.cpp](src/Player.cpp)
- **Function to add**: `DoDetectTreasure(pEffect)` in DoCreateEffects()
- **Logic**: Find all items within reasonable range (15) and reveal positions (light them)
- **Reference**: Similar to Magic Mapping but for items instead of walls

### Area Effect Behaviors to Verify

**3.10 — Test Mass-Effect Behaviors** (RANGED)
These exist in code but need verification that they work in ranged context:
- Mass Sleep: Staff of Mass Sleep (HIT + AREA + SLEEP)
- Mass Fear: Staff of Fear (HIT + AREA + AFRAID)
- Mass Paralyze: Staff of Paralysis (HIT + AREA + PARALYZE)
- Timed Blessing: Staff of Protection (HIT + TIMED + AC)

---

## PART 4: TESTING & DOCUMENTATION

### 4.1 — Build Test Harness
- Create test feature file: `test/features/staves_wands.feature`
- Test scenarios:
  - EFFECT_MOD_AREA effects apply to all monsters within radius of **player position**
  - EFFECT_MOD_BALL projectiles explode at impact with AoE around **target position**
  - z keybind (wand): prompts for item → prompts for target → fires
  - Z keybind (staff): prompts for item → **immediately fires**, no targeting step
  - Each wand effect fires at a targeted monster and produces expected message
  - Each staff effect fires from player (self or AoE) and produces expected message
  - Verify charge consumption for both wands and staves
  - Verify staves never prompt for a target

### 4.2 — Manual Testing Checklist
- [ ] Test all 18 wands in-game
- [ ] Test all 24 staves in-game
- [ ] Verify Z vs z keybinding works correctly
- [ ] Document any immunities or unexpected behavior
- [ ] Note any visual inconsistencies

### 4.3 — Documentation Output
Create comprehensive table: [doc/ITEM_EFFECT_STATUS.md](doc/ITEM_EFFECT_STATUS.md)
- Columns: Item Name | Effect | EFFECT_TYPE | Implementation Status | In-Game Result | Notes
- Rows: All 42 items (18 wands + 24 staves)

---

## PART 5: IMPLEMENTATION PRIORITY & EFFORT

### High Priority (Core Functionality)
| Task | Effort | Status |
|---|---|---|
| Implement EFFECT_MOD_AREA handler | 2-3 hours | ✅ DONE — `DoAreaHit()` in Player.cpp |
| Implement EFFECT_MOD_BALL handler | 2-3 hours | ✅ DONE — `DoBallHit()` in Player.cpp |
| Implement EFFECT_MOD_LINE dispatch | 1 hour | ✅ DONE — `DoLineHit()` in Player.cpp |
| Add Z keybind | 1 hour | ✅ DONE — `IsStaffCommand()` + `RANGED_STAFF` |
| Implement single-target status HIT effects (PARALYZE/AFRAID/SLEEP/CONFUSE) | 2-3 hours | ✅ DONE — `DoStatusHit()` handles all 4 |
| Implement STONE_TO_MUD | 1 hour | ✅ DONE — `DoStoneToMud()` in Player.cpp |
| Implement TELEPORT_AWAY HIT | 1 hour | ✅ DONE — `DoTeleportAway()` in Player.cpp |
| AI: sleep/paralyze skip turn in UpdateRest | 1 hour | ✅ DONE — `m_nEffectTurns` tick-down |
| AI: AFRAID flee all move types + mind immunity table | 1 hour | ✅ DONE — `IsImmuneToEffect()` + UpdateSeek() |

### Medium Priority (Completeness)
| Task | Effort | Why |
|---|---|---|
| Implement IDENTIFY/Probe HIT | 1 hour | Used by 2 items |
| Verify Detect Treasure | 1 hour | Staff-specific effect |
| Test area effects (Mass Sleep/Fear/Paralyze) | 1 hour | Verify existing code works |

### Low Priority (Documentation)
| Task | Effort | Why |
|---|---|---|
| Create comprehensive status table | 1 hour | Reference doc for testing |
| Update Player Docs | 30 min | Help system |
| Create test suite | 2-3 hours | Ongoing regression testing |

**Total Estimated Effort**: 15-19 hours

---

## PART 6: CODE LOCATIONS REFERENCE

### Files to Modify
- [src/Player.cpp](src/Player.cpp) — Add EFFECT_MOD_AREA & EFFECT_MOD_BALL handlers, effect implementations (lines 1599-1897)
- [src/Monster.cpp](src/Monster.cpp) — Add EFFECT_MOD_AREA & EFFECT_MOD_BALL handlers in AttackEffect()
- [src/CmdState.cpp](src/CmdState.cpp) — Add Z keybind detection (line ~422)
- [src/RangedState.cpp](src/RangedState.cpp) — Distinguish wand/staff targeting (line ~300-400)
- [doc/Player Docs.txt](doc/Player%20Docs.txt) — Document Z keybind

### Reference Data Files
- [Resources/Items.txt](Resources/Items.txt) — Wand/staff definitions (lines 1410-1912)
- [Resources/Effects.txt](Resources/Effects.txt) — Effect definitions
- [src/Constants.h](src/Constants.h) — EFFECT_FLAG_* constants

### Related Architecture
- [src/Effect.h](src/Effect.h) — CEffect class definition
- [src/Item.h](src/Item.h) — CItem class definition
- [doc/Effects-Design.md](doc/Effects-Design.md) — Effect system design doc
- [doc/Item-Design.md](doc/Item-Design.md) — Item system design doc

---

## DELIVERABLES CHECKLIST

- [x] **Issue Analysis Complete**
- [x] EFFECT_MOD_AREA handler implemented — `DoAreaHit()` in Player.cpp
- [x] EFFECT_MOD_BALL handler implemented — `DoBallHit()` in Player.cpp
- [x] EFFECT_MOD_LINE handler implemented — `DoLineHit()` in Player.cpp
- [x] Keybind (Z) implemented — `IsStaffCommand()`, `RANGED_STAFF`, `OnHandleStaff()`
- [x] Single-target status effects implemented — `DoStatusHit()` (SLEEP/PARALYZE/AFRAID/CONFUSE)
- [x] STONE_TO_MUD implemented — `DoStoneToMud()`
- [x] TELEPORT_AWAY HIT implemented — `DoTeleportAway()` using `GetSpawnPoint()`
- [x] AI sleep/paralyze: moved to `UpdateRest()` with `m_nEffectTurns` tick-down
- [x] AI AFRAID: all move types flee; `IsImmuneToEffect()` immunity table (EMPTY_MIND)
- [ ] IDENTIFY/Probe HIT implemented (Wand of Probing)
- [ ] Detect Treasure verified/implemented (Staff of Treasure Detection)
- [ ] Staff of Protection (EFFECT_FLAG_AC + MOD_TIMED)
- [ ] Area effects tested in-game
- [ ] Comprehensive status table created
- [ ] All 42 items tested in-game
- [ ] PR ready with full test coverage

