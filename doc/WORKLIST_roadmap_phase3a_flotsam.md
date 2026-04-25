# Phase 3a: Flotsam — Post-Foundation Polish & Difficulty Tuning

**Repository:** `Rushwind13/JMoria`  
**Branch:** `feat/phase3a_flotsam`  
**Status:** Planning Phase  
**Created:** April 24, 2026

## Overview

Phase 3a: Flotsam covers small-to-medium post-Phase-3 systems that enhance difficulty tuning, UI polish, and combat mechanics. These are **not blocking** other features but significantly improve player experience and enable final Phase 4 preparation.

**Focus Areas:**
1. **Speed System & Difficulty Tuning** (#242) — Action economy, primary difficulty lever for Balrog
2. **Combat Enhancements** (#265) — Breath weapon scaling by monster HP
3. **Effect System Polish** (#264) — EFFECT_MOD_MAX flag for dice roll control
4. **UI Improvements** (#272) — Visible Monsters pane for detection spells

---

## Phase 3a Issues

### #242 — Speed System & Difficulty Tuning (Action Economy Mechanic)

**Priority**: 🔴 CRITICAL (Primary difficulty lever)  
**Status**: Not started (Design Complete)  
**Effort**: High  
**Dependency Chain**: Enables #42 (Balrog endgame goal)

**Description**: 
Implement speed/action economy system that creates difficulty scaling and character differentiation. Speed is the **primary difficulty lever** for the entire game—determines survivability against Balrog and other threats.

**Design Specification**:
- **Base Speed**: 10 (represents normal speed = 1 action per turn)
- **Speed Formula**: Base 10 + Equipment mods + Stat mods + Temporary effects
- **Equipment Modifiers**:
  - Rings: +1 to +20 per ring (2 ring slots) → max +40 from rings
  - Boots: +10 (1 boot slot)
  - Gloves: +5 (1 glove slot)
  - **Equipment Cap**: 2 rings + 1 boots + 1 gloves = +55 max base speed
- **Stat Modifiers**: DEX contributes to speed (details from #197)
- **Potion Effects**: Potion of Speed grants +10 temporary (stacks, tracked with duration)
- **Monster Speed Calibration**:
  - Slow (1-2): Bats, Rats, Ants
  - Normal (2-3): Most dungeon monsters
  - Fast (3-4): Vampire Lords, Shamans, Dragons
  - **Balrog (4-5)**: Moves 4-5 times per player turn; deadly if out-sped
- **Balrog Tuning (Critical)**:
  - **Unwinnable**: Player speed ~15 or lower
  - **Winnable**: Player speed ~25-30 with optimization
  - **Comfortable**: Player speed ~35+
  - **Design Intent**: Balrog speed advantage forces character to stack speed gear + potions

**Implementation Strategy**:
1. Add `m_nSpeed` field to `CPlayer` (base 10)
2. Modify combat loop: Actions per turn = Speed / 10 (rounded down, min 1)
3. Monster speed affects AI action frequency (same scaling)
4. Equipment speed bonuses applied during item equip
5. Timed speed from potions tracked with duration
6. Combat queue/turn system updated to respect speed differences

**Related Issues**:
- #197 (Player Stats) — DEX affects speed
- #242 specifically connects to #42 (Balrog)
- Balrog designed as speed check: gear/potion grind required

**Success Criteria**:
- [ ] Balrog is unbeatable below speed 25
- [ ] Balrog is winnable but challenging at speed 30
- [ ] Ring/boots/gloves speed bonuses distributed in loot tables (#249)
- [ ] Potion of Speed crafted at Level 35+ (#247 XP progression)
- [ ] Combat loop respects speed differences (actions per turn scale correctly)

---

### #265 — Breath Weapon Damage Scales with Monster HP

**Priority**: 🟡 HIGH (Improves monster difficulty curve)  
**Status**: Not started (Design Complete)  
**Effort**: Medium  
**Dependency Chain**: Integrates with #77 (Item Effects) combat system

**Description**: 
Breath attacks do damage based on monster **current HP** (not max HP). As monsters get hurt, their breath becomes progressively less deadly, allowing player comeback scenarios and improving perceived difficulty curve.

**Design Specification**:
- **Current Mechanic**: Breath damage is typically `XdY` (e.g., Red Dragon: 8d8 fire)
- **New Mechanic**: Breath damage scales by `(current_HP / max_HP)` ratio
- **Example**:
  - Red Dragon at 100% HP: 8d8 fire damage
  - Red Dragon at 50% HP: 4d8 fire damage
  - Red Dragon at 25% HP: 2d8 fire damage
- **Calculation**: `damage = base_damage * (current_HP / max_HP)`, rounded down (min 1d2)
- **Combat Psychology**: Players perceive victory is possible if they can chip away at breath damage
- **Balance Interaction**: Pairs with #242 (Speed System) — high-speed character can survive initial breath, then whittle down monster

**Implementation Strategy**:
1. Modify breath attack effect calculation in combat
2. Add monster current HP to breath damage formula
3. Ensure minimum damage (1d2 or 1d3) to prevent 0 damage
4. Apply to all breath-capable monsters (Dragons, Balorag, Demons, etc.)
5. Test scaling at various breakpoints (100%, 75%, 50%, 25%, <10% HP)

**Related Issues**:
- #77 (Item Effects) — combat system integration
- #245 (Monster Color Effects) — breath monsters are high-threat (Red)
- #246 (Item Durability) — breath attacks destroy items; scaling affects item danger

**Success Criteria**:
- [ ] Dragon breath damage reduces proportionally as HP decreases
- [ ] Damage never goes below 1d2 (always meaningful threat)
- [ ] Balrog breath damage scales with HP (enhances comeback opportunity)
- [ ] Combat feels fairer: "I can win if I don't get one-shot"

---

### #272 — Visible Monsters UI Pane (Detect Monsters Targeting)

**Priority**: 🟢 MEDIUM (Improves detection spell UX)  
**Status**: Not started (Design Complete)  
**Effort**: Low-Medium  
**Dependency Chain**: No hard blockers; integrates with existing #114 (Item ID) visibility system

**Description**: 
Dedicated UI pane for listing detected monsters from Scroll of Detect Monsters. Shows monster names and distances during detection effect, improving targeting clarity and spell feedback.

**Design Specification**:
- **Display Region**: Similar to Inv/Equip sidebars (DisplayText pane)
- **Trigger**: When Scroll of Detect Monsters or spell cast
- **Content**:
  - Monster name (unidentified format if applicable)
  - Direction (N, NE, E, etc. or numeric offset)
  - Distance in tiles (or "far" for long range)
- **Duration**: Shows detected monsters for effect duration, then reverts
- **Example Output**:
  ```
  DETECTED MONSTERS
  - Orc (E, 3 tiles)
  - Giant Spider (SW, 8 tiles)
  - Vampire Lord (far N, 25+ tiles)
  ```
- **Integration**: `m_llVisibleMonsters` already tracks detected monsters in code; UI just needs to surface it

**Implementation Strategy**:
1. Add `CDisplayText` region for detected monsters (similar to Msgs/Stats/Inv)
2. During detection effect: populate visible monsters list
3. Display monsters with direction + distance calculation
4. Revert to normal visibility next turn or when effect expires
5. Handle unidentified monster names (use generic types)

**Related Issues**:
- #114 (Item Identification) — uses visibility system; unidentified names
- #77 (Item Effects) — Detect Monsters effect framework
- #243 (Town System) — future detection-based NPC interaction

**Success Criteria**:
- [ ] Detect Monsters scroll shows list of nearby monsters
- [ ] List persists for effect duration
- [ ] Distances and directions accurate
- [ ] UI doesn't break existing Inv/Equip display
- [ ] Works with multiple detected monsters (scaling UI)

---

### #264 — EFFECT_MOD_MAX Flag (Modifier for Max Dice Rolls)

**Priority**: 🟢 MEDIUM (Balrog tuning tool)  
**Status**: Not started (Design Complete)  
**Effort**: Low  
**Dependency Chain**: Integrates with #77 (Item Effects) framework

**Description**: 
New EFFECT_MOD flag where dice rolls always return maximum value. Enables deterministic high damage and tuning for boss-level threats (primarily Balrog).

**Design Specification**:
- **Effect Modifier**: `EFFECT_MOD_MAX`
- **Behavior**: When flag set, dice rolls return max value
- **Examples**:
  - `3d6` → 18
  - `1d100` → 100
  - `1d20` → 20
  - `8d8` → 64
- **Use Cases**:
  - Monster HD (Ancient Dragons: 100d8 max → 800 HP guaranteed)
  - Breath weapon damage (predictable high damage)
  - Balrog attacks (ensures one-shot threat)
  - Boss-level guaranteed minimum damage

**Implementation Strategy**:
1. Add `EFFECT_MOD_MAX` to Effect system vocabulary (Constants.h)
2. Modify dice roller: check for EFFECT_MOD_MAX flag
3. If flag set: return `num_dice * dice_sides` (skip random)
4. Apply to Balrog and other boss monsters
5. Use in high-threat monster definitions (Monsters.txt)

**Related Issues**:
- #77 (Item Effects) — effect system integration
- #42 (Balrog) — uses EFFECT_MOD_MAX for guaranteed damage
- #265 (Breath Scaling) — pairs with this for tuning

**Success Criteria**:
- [ ] EFFECT_MOD_MAX dice rolls deterministic (no RNG)
- [ ] Balrog uses MAX for at least 1 attack type
- [ ] No dice rolls accidentally hit MAX when not intended
- [ ] Works with multi-dice (3d6, 100d8, etc.)

---

### #273 — [Issue Not Found in Roadmap]

**Status**: ⚠️ NOT IN CURRENT ROADMAP  
**Note**: Issue #273 is referenced but not present in `WORKLIST_jmoria_core_roadmap.md`. Please clarify:
- Is this a new issue to create?
- Is it a known issue that should be added to Phase 3a scope?
- Should it replace one of the above issues?

---

## Phase 3a Execution Priority

### Tier 1: Difficulty Infrastructure (Must do first)
1. **#242 (Speed System)** — Primary difficulty lever; enables Balrog tuning
   - Estimated effort: 5-7 days
   - Blocks: Balrog difficulty calibration
   - Unblocks: Entire Phase 4 difficulty progression

### Tier 2: Combat Polish (Do after speed is working)
2. **#265 (Breath Scaling)** — Improves monster combat balance
   - Estimated effort: 2-3 days
   - Enhances: #242 speed system (stacking HP-based damage reduction)
   - Pairs with: Balrog combat feel

3. **#264 (EFFECT_MOD_MAX)** — Boss monster tuning
   - Estimated effort: 1 day
   - Dependency: #77 (Item Effects) framework
   - Usage: Balrog, Ancient Dragons, unique monsters

### Tier 3: UI/UX Polish (Parallel or after Tier 2)
4. **#272 (Visible Monsters UI)** — Detection spell clarity
   - Estimated effort: 2-3 days
   - Non-blocking: Can start parallel to Tier 1
   - Enhances: #114 (Item ID) detection mechanics

---

## Phase 3a Dependencies & Blockers

**No hard blockers** — Phase 3a is designed to work independently after Phase 3 foundation.

**Soft dependencies**:
- **#242 requires**: #197 (Stats) for DEX-to-speed, or can hardcode DEX contribution
- **#265 requires**: #77 (Item Effects) combat integration, or can patch directly into breath calculation
- **#264 requires**: #77 (Item Effects) vocabulary (or add to existing)
- **#272 requires**: Existing visibility tracking (`m_llVisibleMonsters`)

**Enables downstream**:
- #242 enables #42 (Balrog) difficulty tuning
- #265 + #264 enable Balrog as achievable goal
- #272 enhances #243 (Town System) detection feedback

---

## Suggested Implementation Order

### Week 1: Speed System (#242)
- [ ] Add `m_nSpeed` to CPlayer (base 10)
- [ ] Equipment speed modifiers (rings +1-20, boots +10, gloves +5)
- [ ] Combat loop: actions per turn = Speed / 10
- [ ] Potion of Speed (+10 timed)
- [ ] Monster speed calibration (Bats 1-2, Balrog 4-5)
- [ ] Test: Confirm Balrog unwinnable at speed 15, winnable at 30+

### Week 2: Combat Polish (#265, #264)
- [ ] Breath damage scales by current HP (Dragon: 100% → 50% → 25% progression)
- [ ] Add EFFECT_MOD_MAX to effect vocabulary
- [ ] Balrog uses EFFECT_MOD_MAX for guaranteed damage
- [ ] Test: Breath damage reduces as monster HP decreases
- [ ] Test: Balrog damage is deterministic and threatening

### Week 2 (Parallel): Visible Monsters UI (#272)
- [ ] Create DisplayText pane for detected monsters
- [ ] Populate with `m_llVisibleMonsters` list
- [ ] Calculate distance + direction
- [ ] Revert UI after effect expires
- [ ] Test: Detect Monsters shows list, clearing works

### Week 3: Integration & Testing
- [ ] Speed + Breath scaling interact correctly (faster player avoids more breath damage over time)
- [ ] Balrog uses all three systems (#242 + #265 + #264)
- [ ] UI doesn't break with large monster counts
- [ ] Commit to `feat/phase3a_flotsam`

---

## Success Metrics

- **Speed System**: Balrog tuning achieved; 30+ speed feels challenging but winnable
- **Breath Scaling**: Monster HP → damage scaling provides comeback opportunity
- **EFFECT_MOD_MAX**: Balrog guaranteed damage creates one-shot threat at low speed
- **Visible Monsters**: Detection spell feedback clear; monster list readable
- **Integration**: All three systems work together (speed allows survival, breath scales down, Balrog still threatening)

---

## Known Issues & Caveats

### Missing #273
- #273 referenced but not in roadmap
- Clarification needed before Phase 3a scope finalization

### Speed System Complexity
- Action economy (Speed / 10) affects entire combat system
- Requires comprehensive testing with monster AI
- May need balance tuning post-Phase-3a

### Breath Damage Minimum
- Scaling can produce very low damage late-fight
- Need minimum threshold (1d2?) to keep threat meaningful

### Visible Monsters Performance
- Large dungeons with many detected monsters could cause UI lag
- May need pagination or scrolling UI

---

## References

- **Core Roadmap**: [WORKLIST_jmoria_core_roadmap.md](WORKLIST_jmoria_core_roadmap.md)
- **Phase 3 Completion**: [doc/WORKLIST_roadmap_phase3.md](doc/WORKLIST_roadmap_phase3.md)
- **Item Effects Framework**: [doc/Effects-Design.md](doc/Effects-Design.md)
- **Balrog Design**: [doc/WORKLIST_jmoria_core_roadmap.md](doc/WORKLIST_jmoria_core_roadmap.md#42---jmoria-scoring-and-boss-encounter-balrog)
