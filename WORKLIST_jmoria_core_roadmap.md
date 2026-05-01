# JMoria Core Roadmap

This roadmap tracks core game features that are essential to JMoria's gameplay experience. These are distinct from the AI bot roadmap and focus on player-facing systems, mechanics, and content.

## Philosophy

The core roadmap prioritizes:
1. **Player agency** - meaningful choices, progression, customization
2. **Strategic depth** - character building, item variety, tactical combat
3. **Replayability** - diverse builds, randomized loot, multiple playstyles
4. **Quality of life** - clear feedback, discoverable mechanics, intuitive UI

---

## 📋 Recent Updates (Integration of Issues #243-275)

**Latest Integration (Issues #243-275)**:
- **Priority 4 (Major Post-Phase-3 Systems)**: 8 new issues integrated:
  - #243 (Town System) — shop economy, NPC interaction, safe hub
  - #244 (Intrinsics & Status Effects) — unified 70+ effect framework
  - #245 (Monster Color Effects) — standardized threat recognition
  - #246 (Item Durability) — elemental destruction, inventory strategy
  - #247 (XP & Level Progression) — character 1-40 levels, depth scaling
  - #248 (Guard Timer UI) — crime system display
  - #249 (Loot Tables) — monster drops, shop economy calibration

- **Priority 5 (Advanced Systems & Polish)**: 7 new issues + 2 known issues:
  - #268 (Unique Monsters) — named one-per-game encounters
  - #269 (Shrieker Mushroom) — aggravate/sleep mechanic
  - #274 (Trap System) — dungeon traps, search, disarm
  - #275 (Pickpocket Monsters) — thief-type monster content
  - #267 (GetMonsterDef shadowing) — known limitation
  - #266 (Town spawn warnings) — minor console warnings

---

## 🎯 Priority 1: Core Mechanics & Foundation

### #39 - Implement Ranged Attacks
**Status**: In progress (PR #144)
**Description**: Add bow-based ranged combat with targeting, ammunition management, and line-of-sight mechanics.
**Impact**: Opens alternative playstyle, extends combat depth.
**Dependencies**: None blocking
**Effort**: High

---

## 🔧 Priority 2: Major Systems
COMPLETE

---

## 📦 Priority 3: Expansions & Refinement

### #112 - Add Races
**Status**: Not started
**Description**: Allow character race selection with inherent abilities.
- Examples: Elf (infravision), Dwarf (stoneform), Human (bonus feat/skill)
- Display on character creation and sheets
**Impact**: Extends character build variety.
**Dependencies**: None blocking
**Effort**: Low-Medium

### #239 - Add Classes (Warrior, Rogue, Mage, Priest, Druid, Shaman, Monk, Ranger)
**Status**: Not started
**Description**: Class system for diverse playstyles beyond Warrior (currently the only class).
- 8 classes with distinct mechanics: Warrior, Rogue, Ranger, Mage, Priest, Druid, Shaman, Monk
- Spellcasters learn spells from **Books** (not scrolls): 4 books per class, progressively rare (Books 1-2 purchasable, Books 3-4 dungeon-only)
- Books don't degrade; spell distribution follows ZAngband model (spectrum of levels in each book, not strict ranges)
- Stats are implemented incrementally: CON → STR → DEX → INT/WIS → CHA (unlocks class features)
**Impact**: Massive replayability, class-specific stat priorities, emergent build combinations.
**Dependencies**: #197 (Stats), #77 (Item Effects for spell mechanics), #121 (mana/resource systems)
**Effort**: Very High
**Phasing**: Phase 1: Warrior only (current) → Phase 2: Rogue/Ranger → Phase 3: Spellcasters + Book system
**See Also**: GitHub issue #239 for full spec and spell book mechanics

### #197 - Player Stats (STR/CON/DEX/INT/WIS/CHA)
**Status**: Not started
**Description**: Implement 6-attribute character system affecting combat, defense, and special abilities.
- **Existence**: Stats exist from character creation and persist (rolled on character start)
- **Base Effects**: STR (melee hit/damage, carry capacity), CON (HP pool, health recovery), DEX (AC, ranged hit, speed), INT/WIS (spell casting, magic resistance), CHA (shop prices, NPC interaction)
- **Progression Mechanics** (core to winning the game):
  - Monsters can permanently **damage** stats (Yellow Worm Mass -WIS, Ghosts -CON/life levels, Vampires drain life levels)
  - **Restore [Stat] Potions** undo stat damage (mid-level items)
  - **Gain [Stat] Potions** permanently increase stats (high-level items)
  - Most winning characters will grind Gain potions to max all stats (6x9 = 54 potions needed for godlike character)
- **Stat Cap**: Reasonable maximum (18? 20?) per stat
**Impact**: Core progression and difficulty scaling; stat damage/restoration becomes survival strategy.
**Dependencies**: None blocking
**Effort**: Medium-High

### #42 - JMoria Scoring and Boss Encounter (Balrog)
**Status**: Not started
**Description**: Implement high score system and Balrog boss encounter to provide clear win/loss conditions and competitive replayability.
- **Scoring Formula**: depth × player_level + experience + monster_kills - death_penalties
- **Balrog (Boss)**:
  - Location: Depth 5000' (dungeon level 100), no down stairs (final level)
  - Can appear: Anywhere below 4500' (depth 90), but guaranteed at 5000'
  - Attacks: (Bite, Claw, Claw), (Breathe Fire or Poison), (Whip or Flaming Sword)
  - HP: Massive (~8000, rolled as 100d8 maxed)
  - AC: Very high
  - Speed: 4-5 (moves multiple times per player turn, deadly if out-sped)
  - Resistances: Fire, Frost, and other elemental types
  - Requires high-speed character to fight on fair terms (see Speed/Difficulty issue)
- **Victory**: Killing Balrog grants +40 levels and triggers win state
- **Max Level**: 40 (normal) → 80 (with boss kill)
**Impact**: Clear endgame goal, requires multi-system mastery (stat buildup, speed rings, resistances).
**Dependencies**: #197 (Stats), Speed/Difficulty system
**Effort**: Medium

### #128 - Magic Weapons: Item Tiers (Normal/Cursed/Magic/Ego/Legendary/Unique)
**Status**: Not started
**Description**: Implement comprehensive item classification system providing replayability through randomized loot properties.
- Normal: +0 bonuses
- Cursed: negative bonuses, special downsides
- Magic: positive bonuses (added via enchant scrolls)
- Ego: positive bonuses + one thematic intrinsic (e.g., Flametongue, Resist Lightning)
- Legendary: fixed constellation of intrinsics (e.g., Longsword of Westernesse)
- Unique: named one-per-game items with personality (Lady Teldra won't fight innocents)
**Impact**: Infinite replayability through item variety; item identification becomes strategic knowledge.
**Dependencies**: #114 (item identification system)
**Effort**: High

### #43 - Lady Teldra: Unique Item Personality
**Status**: Not started
**Description**: Implement named, unique weapons with personality and special properties.
- Example: Lady Teldra won't attack peaceful creatures ("refuses to leave her sheath")
- Unique items can persist across difficulty/resets or be one-per-game
- Flavor text varies by wielder/item relationship (e.g., "Lady Teldra scoffs at such simple magicks")
**Impact**: Added character/story, deeper item systems.
**Dependencies**: #128 (item tiers)
**Effort**: High

---

## � Priority 4: Major Post-Phase-3 Systems

### #243 - Town System (Shop Infrastructure & NPC Interaction)
**Status**: Not started (Design Complete)
**Description**: Implement Moria-style single town hub with shops, NPCs, and safe rest area.
- **5 Shop Types**: General Store, Weaponsmith/Armorer, Temple, Magic Shop, Player Home
- **Shop Economics**: Base markup formula, CHA modifier, item generation by type
- **NPCs**: Thieves steal coins, Guards patrol with decay timer, Quest-givers (future), flavor NPCs
- **Word of Recall**: Consumable scroll + treasure Staff for teleporting to/from town
- **Day/Night Cycle**: Foundation for future Vampire PCs and stealth gameplay
**Impact**: Enables shop economy, character progression via loot flow, NPC interaction
**Dependencies**: #197 (Stats) for CHA-based pricing; #114 (Item ID) for shop learning; #128 (Magic Items) for loot
**Effort**: Very High
**See Also**: GitHub issue #243 for full design, shop economics, NPC behaviors

### #244 - Intrinsics & Status Effects System (Temp/Perm Buffs/Debuffs)
**Status**: Not started (Design Complete)
**Description**: Unified framework for character buffs/debuffs (70+ potential effects) covering temporary effects, permanent effects, and negative effects.
- **4 Variants per Effect**: Perm buff (equipment), temp buff (potion), perm debuff (curse), temp debuff (trap/monster)
- **No Hard Cap**: Characters can stack unlimited positive intrinsics if obtained
- **Display**: Character Stats screen (active temp buffs) + Intrinsics Grid (equipment × effect matrix)
- **Trap Integration**: Traps trigger intrinsic-related effects (fire trap, teleport trap, summon trap, etc.)
- **Sustain Mechanics**: Sustain [Stat] restores stat to max achieved + prevents damage to that stat
- **70+ Effects**: Resistances (RF, RC, RL, RA, RP), Immunities (Hold Life, Free Action, Sustain), Abilities (Regen, Telepathy, ESP), Status (Afraid, Confused, Blind, Hold), Stat Modifiers
**Impact**: Unified effect system, enables complex interactions, support for Sustain mechanic
**Dependencies**: #77 (Effects Framework) defines types; #197 (Stats); #246 (item durability intrinsics)
**Effort**: High
**See Also**: GitHub issue #244 for comprehensive effect categories and data model

### #245 - Monster Color Effects & Standardized Threat Recognition
**Status**: Not started (Design Complete)
**Description**: Standardized system where monster color → threat type (stat damage, elemental, status) for player learning.
- **Color → Threat Mapping**: Yellow=Wisdom drain, Red=CON/Fire, Blue=All-stats/Cold, Green=Poison, Brown=Fear, Black=Sleep, White=Paralyze, etc.
- **13 Standardized Colors**: Consistent mapping across all monsters
- **Threat Progression**: Shallow levels = stat drain only; Deep levels = stat drain + elemental + status effects
- **Learning Curve**: Players recognize threats by color after repeated encounters
**Impact**: Enables threat recognition at-a-glance, improves player learning curve, supports difficulty scaling
**Dependencies**: #77 (Effects Framework); #244 (Intrinsics)
**Effort**: Medium
**See Also**: GitHub issue #245 for color mappings and depth-based threat progression

### #246 - Item Durability & Elemental Weakness Mechanics
**Status**: Not started (Design Complete)
**Description**: Item degradation/destruction when exposed to elemental hazards (fire, cold, acid, lightning).
- **Vulnerability by Type**: Scrolls (50-100%), Potions (50-100%), Books (15-50%), Equipment (5-20%), Rings (1-5%)
- **Elemental Threats**: Fire damages scrolls/potions, Cold shatters potions, Acid destroys all, Lightning damages equipment
- **Unique Items**: Degrade with penalties but never permanently destroyed
- **Dungeon Hazards**: Fire-themed levels destroy scrolls; Cold levels freeze potions; forces strategic inventory planning
**Impact**: Risk/reward in inventory choices, economic sink, realism, strategic planning
**Dependencies**: #77 (Item Effects); #128 (Magic Items); #245 (Monster Color attacks)
**Effort**: High
**See Also**: GitHub issue #246 for damage models and dungeon level threats

### #247 - XP & Level Progression System
**Status**: Not started (Design Complete)
**Description**: Formalized XP progression and leveling system tying character progression to dungeon depth.
- **AD&D 1e Extended**: Levels 1-30 standard XP progression, Levels 31-40 capped at +200k per level
- **Depth Multipliers**: Base 1.0x at shallow depths → 2.0x at deep depths (2000'+ )
- **Level 40 Cap**: Character reaches max level at 3,500,000 total XP
- **Balrog Victory**: Massive XP bonus triggers when Balrog defeated
**Impact**: Core progression mechanic, enables character power curve, stat gating system
**Dependencies**: #197 (Stats) for HP/stat bonuses; #42 (Scoring) XP factors into score
**Effort**: Medium
**See Also**: GitHub issue #247 for XP table and depth scaling

### #248 - Guard Timer UI & Crime System Display
**Status**: Not started (Design Complete)
**Description**: Clear UI display for town crime system's guard timer status.
- **Wanted Status Display**: RED text on stats screen showing hours remaining (0-24 range)
- **Escalating Messages**: Narrative feedback as timer decreases (12+ hrs → 6-12 hrs → <1 hr)
- **Town Entry Messages**: "The guards are watching you closely!" when wanted status active
- **Guard Behavior**: Guards spawn and attack during day if player wanted
- **Timer Decay**: ~5-7 in-game days to clear wanted status
**Impact**: Transparency of crime consequences, gameplay tension, NPC interaction feedback
**Dependencies**: #243 (Town System) crime system; #197 (Stats) for display
**Effort**: Low-Medium
**See Also**: GitHub issue #248 for UI mockups and decay logic

### #249 - Loot Tables & Monster Item Drops
**Status**: Not started (Design Complete)
**Description**: Comprehensive loot table system correlating monster difficulty/level to item/coin rewards.
- **5 Tier System**: Weak (L1-10), Minor (L11-20), Major (L21-40), Elite (L41-60), Boss (L70+)
- **Item Scaling**: Normal → Cursed → Magic → Ego → Legendary → Unique by tier
- **Special Drops**: Stat-damaging monsters drop Restoratives; Elemental monsters drop Resistances
- **Coin Economy**: ~20,000-50,000 gp per 40-level run maintains shop economy
- **Drop Rates**: Normal items ~40-60, Magic ~20-30, Ego ~2-3, Legendary ~0-1, Unique ~0-1 per run
**Impact**: Loot meaningfulness, economic progression, incentivizes deeper exploration
**Dependencies**: #197 (Stats), #128 (Magic Items), #245 (Monster Color level correlation)
**Effort**: High
**See Also**: GitHub issue #249 for full loot tier table and coin economy

---

## 🎪 Priority 5: Advanced Systems & Polish

### #268 - Unique Monster System
**Status**: Not started (Design Complete)
**Description**: System for named, one-of-a-kind monsters beyond normal spawn tables.
- **One-Per-Game**: Only one instance should exist at a time
- **Special Properties**: Unkillable, respawns, or special death events; loot tables; flavor text
- **Fixed Depth**: Always appears on specific level or range
- **Example**: Sethra Lavode (HP 30d8, Speed 2, Confuse + Paralyze attacks, Level 95)
**Impact**: Memorable encounters, story elements, unique boss fights
**Dependencies**: None blocking (pairs with #128, #245)
**Effort**: High
**See Also**: GitHub issue #268 for Sethra Lavode stats and design questions

### #269 - Shrieker Mushroom Patch Monster (Sleep & Aggravate Mechanic)
**Status**: Not started (Design Complete)
**Description**: Stationary monster that wakes sleeping monsters in range when triggered.
- **Type**: Shrieker (`,` tile, camouflage as food)
- **Behavior**: When player adjacent, wails to wake sleeping monsters and aggravate them
- **Effects**: Aggravate mechanic for area-of-effect monster state change
- **Blocked By**: Monster sleep state system needs implementation
**Impact**: Emergent monster interaction, risk/reward for careless exploration
**Dependencies**: Monster sleep state (#244 Intrinsics); Aggravate mechanic
**Effort**: Medium
**See Also**: GitHub issue #269 for sleep/aggravate system design

### #274 - Trap System (Dungeon Traps, Search, Disarm, Trap Scrolls)
**Status**: Not started (Design Complete)
**Description**: Complete trap system for dungeon exploration with invisible placement, search detection, and disarm mechanics.
- **Trap Types**: Siren, Flashbang, Confuse, Paralyze (early); Fire, Cold, Lightning, Acid, Water, Teleport, Trap Door, Chute, Spiked Pit, Summon, Cave-In (later/deep)
- **Mechanics**: Invisible when created (`.` tile), revealed as `^` on search, triggered on step (doesn't disarm), disarm removes trap
- **Search/Disarm**: `s)earch` reveals traps (same mechanics as secret doors), `d)isarm` removes traps with XP reward
- **Scrolls**: Scroll of Detect Traps (done), Scroll of Destroy Doors/Traps (planned), Scroll of Create Traps/Create Traps * (planned)
**Impact**: Core dungeon hazard, incentivizes Detect Traps items, risk/reward in exploration
**Dependencies**: Elemental trap types use #77 (Effects); Stats for stat damage traps; Trap system state (DUNG_FLAG_TRAP)
**Effort**: Very High
**See Also**: GitHub issue #274 for full trap type table and implementation notes

### #275 - Create Pickpocket Monsters (Steal Effect)
**Status**: Not started (Design Complete)
**Description**: Add thief-type monsters with Pickpocket/Steal effect.
- **Effect**: Named EFFECT_FLAG_STEAL / MON_FLAG_PICKPOCKET on touch
- **Monsters**: Novice Rogue, Hobbit, Singing Happy Drunk, etc. (previously noted in WORKLIST)
- **Behavior**: Steal coins from player on hit
- **Integration**: Requires named Steal effect in Effects.txt
**Impact**: Thief-themed monsters, economic consequence in combat
**Dependencies**: #77 (Effects System) for named Steal effect
**Effort**: Low
**See Also**: GitHub issue #275 in Issues | See Monster-Design.md for Rogue line progression

### #267 - GetMonsterDef(name) Shadows Duplicate Monster Names
**Status**: Known Issue (Low Priority)
**Description**: By-name monster lookup returns first match, shadowing duplicates with same name.
- **Affected**: Wizard mode `^s` summon-by-name, cucumber tests using GetMonsterDef(name)
- **Not Affected**: Normal gameplay uses index-based lookup
- **Current Duplicates**: Teckla (rat + person), Lyorn (canine + person), Tiassa (fly + person)
- **Decision**: Accept as known limitation; fix if name-based lookup becomes important
**Impact**: Low - wizard mode only, gameplay unaffected
**Dependencies**: None blocking
**Effort**: Low (fix: add GetMonsterDef(name, type) overload)
**See Also**: GitHub issue #267 for duplicate name details

### #266 - Town Level Spawn Warnings (Word of Recall from Town)
**Status**: Known Issue (Minor)
**Description**: Warnings when spawning items/monsters at depth 0 (town level).
- **Scenario**: Reading Word of Recall from town takes player to dungeon
- **Warning**: "Couldn't find a suitable item for this depth. got an invalid item: -1"
- **Root Cause**: Loot table generation expects dungeon depth > 0
- **Fix**: Add town-specific loot tables or skip for depth 0
**Impact**: Minor - doesn't break gameplay, just console warnings
**Dependencies**: #249 (Loot Tables)
**Effort**: Low
**See Also**: GitHub issue #266 in Issues

---

## �🐛 Known Blocking Issues

### #214 - Dungeon Gen: Isolated Hallway Segments (Blocked by #117)
**Status**: Not started
**Description**: Some levels generate with hallway pockets that only have secret doors as exits, making levels unplayable without search.
**Impact**: Critical blocker for playability at high depths.
**Fix**: Implement #117 (search) as immediate workaround, then fix generation logic.

---

## Dependency Graph

```
✅ #42 (Scoring)
   └─ No deps

✅ #197 (Stats)
   └─ No deps

✅ #39 (Ranged Attacks) — PR #144 in progress
   └─ No deps (pairs with targeting system)

✅ #110 (Use command UX)
   └─ No deps
   
#128 (Magic Items)
   └─ #114 (Item Identification) — recommended but not required
   └─ Enables #43 (Unique items)

#43 (Unique Items)
   └─ #128 (Magic Items)

#121 (Lantern Fuel)
   └─ Pairs with #72 (Fog of War)

#117 (Search Command)
   └─ No deps
   └─ **Blocks** #214 (Dungeon gen)

#112 (Races)
   └─ No hard deps
   ├─ Best after #197 (Stats)

#239 (Add Classes)
   ├─ #197 (Stats) — enables full stat priority system
   ├─ #77 (Item Effects) — enables spell mechanics
   ├─ #121 (Lantern Fuel) — enables mana/resource management
   └─ Phase 1 (Warrior only); Phase 2 (Rogue/Ranger); Phase 3 (Spellcasters)

#243 (Town System)
   ├─ #197 (Stats) — for stat-based shopping interactions
   └─ Enables #241 (Class Quests), economy integration

#244 (Intrinsics & Status Effects)
   ├─ #77 (Item Effects) — defines effect types
   ├─ #197 (Stats) — for stat modifiers and Sustain mechanics
   └─ Enables #245 (Monster Color) and #246 (Item Durability)

#245 (Monster Color Effects)
   ├─ #77 (Item Effects) — stat drain, elemental, status effect types
   ├─ #244 (Intrinsics) — intrinsic representation and UI
   └─ Supports #42 (Balrog uses Gold threat)

#246 (Item Durability & Elemental Weakness)
   ├─ #128 (Magic Items) — unique item mechanics
   ├─ #245 (Monster Color) — elemental attacks from colors

#247 (XP & Level Progression)
   ├─ #197 (Stats) — HP/stat bonuses per level
   ├─ #42 (Scoring) — XP factors into score
   └─ Enables character progression arc

#248 (Guard Timer UI)
   ├─ #243 (Town System) — crime system, guard NPC behavior
   └─ #197 (Stats pane) — display integration

#249 (Loot Tables)
   ├─ #197 (Stats) — stat potion drops
   ├─ #128 (Magic Items) — item tier spawning
   ├─ #245 (Monster Color) — level correlation via color
   └─ Enables #243 (Town shop economy)

#268 (Unique Monsters)
   ├─ #128 (Magic Items) — loot tables for uniques
   └─ #245 (Monster Color) — threat recognition

#269 (Shrieker Mushroom)
   ├─ #244 (Intrinsics) — monster sleep state
   └─ Aggravate mechanic (new)

#274 (Trap System)
   ├─ #77 (Item Effects) — elemental trap types
   ├─ #197 (Stats) — stat damage traps
   ├─ #117 (Search) — search also detects traps
   └─ Scroll interactions (#114, #249)

#275 (Pickpocket Monsters)
   └─ #77 (Item Effects) — named Steal effect; #245 (threat type)

#267 (GetMonsterDef duplicate names)
   └─ Low priority; wizard mode only

#266 (Town spawn warnings)
   └─ #249 (Loot Tables) — town-specific tables needed
```

---

## Suggested Implementation Order

Following the re-prioritized P1/P2/P3 structure:

### **Phase 1: Quick Wins & Accessibility** ✅ COMPLETE
Low-effort, immediate player-facing improvements:
- ✅ #111 (Filtered inventory) — branch: phase1-quick-wins
- ✅ #110 (Monsters don't move during use commands) — branch: phase1-quick-wins
- ✅ #180 (Death screen full map dump) — branch: phase1-quick-wins

### **Phase 2: Gameplay Expansion & Exploration** ✅ COMPLETE
Core P1 features that extend playstyle and unlock dungeon accessibility:
- #39 (Ranged attacks) — *separate branch* (issue/39-ranged-weapon-system)
- ✅ #117 (Search command) — branch: phase2-gameplay-expansion
- ✅ #121 (Lantern fuel) — branch: phase2-gameplay-expansion (wand recharging TBD)
- ✅ #45 (MON_AI_SEEK target positions) — branch: phase2-gameplay-expansion (foundation)

### **Phase 3: Deep Systems Foundation** ✅ COMPLETE
Complex P2 systems that enable everything downstream:
- ✅ #72 (Fog of War & lighting) — atmosphere, depth progression (foundation done, some light items moved to Phase 4)
- ✅ #77 (Item Effects system) — foundation complete; 150 items, 98 effects, all monster attacks migrated
- 🚧 #114 (Item Identification) — core system done, feeling tiers and class-specific feelings moved to Phase 4

### **Phase 4: Progression & Economy** (Starting)
Major mechanics that define end-to-end player experience and enable late-game systems:

**Core Progressions** (Do first, enables others):
- **#247 (XP & Level Progression)** — Character level system 1-40; AD&D 1e progression; depth multipliers
- **#249 (Loot Tables)** — Monster item drops by tier; 80/20 dungeon/shop loot split; coin economy calibration

**Town & NPC Hub** (Enables economy, quests, NPC interaction):
- **#243 (Town System)** — 5 shop types, NPCs, Word of Recall, day/night cycle
- **#248 (Guard Timer UI)** — Crime system display, wanted status tracking

**Item & Monster Enhancement** (Supports difficulty):
- **#245 (Monster Color Effects)** — Standardized color → threat mapping (Yellow=WIS, Red=CON/Fire, Blue=All/Cold, etc.)
- **#246 (Item Durability & Elemental Weakness)** — Item destruction by elemental attacks, strategic inventory planning
- **#244 (Intrinsics & Status Effects)** — 70+ effects system, Sustain mechanics, resistance stacking

**Monster Content**:
- **#268 (Unique Monster System)** — Named one-per-game uniques (e.g., Sethra Lavode)
- **#269 (Shrieker Mushroom)** — Monster aggravate/waking mechanic; sleep state system
- **#275 (Pickpocket Monsters)** — Thief-type monsters with Steal effect
- **#274 (Trap System)** — Dungeon traps with detect/disarm; trap scrolls

### **Phase 5: Character Progression & Replayability** (Later)
P3 character building systems, phased to unlock incrementally:

**Stat Progression** (existing from start, enhanced by this phase):
- Stat damage from monsters becomes strategic concern (must carry Restore potions)
- Gain [Stat] potions become mid/late-game goals
- Maxed stats + Speed tuning = Balrog readiness

**Item Tiers & Uniqueness:**
- #128 (Magic item tiers) — loot meaningfulness via shops and dungeon drops
- #43 (Unique item personality) — flavor, character, stakes (singleton model)

**Character Diversity (phased):**
- #239 Phase 1 (Warrior only) — current state
- #239 Phase 2 (Rogue/Ranger) — DEX-based builds, after #197-DEX
- #239 Phase 3 (Spellcasters + Books) — INT/WIS builds, requires #77/#121
- #112 (Races) — build diversity multiplier
- #241 (Class Quests) — class flavor, story arcs

**Result**: 8 classes × diverse stat configurations × 100+ items × 500+ viable builds

### **Phase 5b: Polish & Endgame** (Final Polish)
- **#44 (High Score List)** — persistent score tracking, ranked displays
- **#114 (Item ID)** — refinements and class-specific feelings
- **Balrog Encounter** (#42) — final tuning and death messages
- **Hardcore Mode Options** — permadeath toggles, unique item rules

---

## System Interconnections

The roadmap is designed so systems reinforce each other:

**Foundation (Phases 1-3)**:
- **Stats (#197) + Speed (#242) + Equipment**: High-level character requires high stats (Gain potions) + speed gear (rings, boots) to survive Balrog's damage output and action rate
- **Speed (#242) + Monster Colors (#245)**: Monster speed determines encounter danger; Player speed creates progression arc; Color coding enables threat recognition
- **Town (#243) + Items (#128) + Shops**: Players gather magical items and speed equipment from shops, creating equipment progression arc
- **Stat Damage (monsters) + Restoration (potions + town temples)**: Stat damage becomes meaningful challenge; town provides access to Restoration potions
- **Classes (#239) + Stats (#197) + Town (#243)**: Each class has different stat priorities; town shops provide class-specific equipment (Mage seeks +INT, Rogue seeks +DEX or AC, etc.)

**Advanced (Phase 4+)**:
- **Item Durability (#246) + Elemental Hazards**: Fire-themed levels destroy scrolls; Cold levels freeze potions; forces strategic inventory planning
- **Speed (#242) + Item Durability (#246)**: High-speed character also needs defensive intrinsics (fire/cold resistance) to protect items; speed alone insufficient for survival
- **Town (#243) + Item Durability (#246)**: Player House storage becomes critical for protecting valuable items between levels; encourages frequent town visits
- **Loot Tables (#249) + Town Economy (#243)**: Monster drops calibrate to maintain shop economy; 80/20 dungeon/shop loot split
- **XP Progression (#247) + Monster Level (#245) + Loot Drops (#249)**: Higher-level monsters (colored by threat) give more XP and drop better loot; incentivizes descending deeper
- **Intrinsics (#244) + Status Effects**: Unified framework for buffs/debuffs; supports Sustain mechanics and resistances stacking
- **Color Threats (#245) + Intrinsics (#244)**: Monsters' threat type (color) matches intrinsics needed to survive (Fire resistance for Red, etc.)
- **Trap System (#274) + Search (#117)**: Traps found via search; Detect Traps scrolls enable exploration
- **Trap Effects (#274) + Intrinsics (#244)**: Traps trigger intrinsic effects (fire trap = temp weak to fire)

---

## Metrics & Success Criteria

- **Replayability**: 8 classes × (6 stats configurations) × (100+ item combinations) × (multiple unique monsters) should yield ~500+ distinct viable builds
- **Win Rate**: Competent player should have a 1-in-100 win rate (depends on Balrog tuning via #242)
- **Speed Tuning**: Balrog should be impossible without speed ~25-30; achievable with speed ~30-40 (achieves endgame goal)
- **Score Variance**: High-score list should have 50%+ turnover per 10 games (indicates balance tuning)
- **Bot Benchmark**: Reference bot should reach depth 20 with consistent survival rate (pairs with bot roadmap)
- **Class Balance**: No single class should dominate win rates; victory distribution relatively even across 8 classes
- **Economy**: 20,000-50,000 gp per 40-level run maintains sustainable shop economy (#249)
- **Loot Satisfaction**: Players find meaningful loot progression without feeling flooded or starved (#249)
- **Trap Challenge**: Traps present meaningful risk (~10-15% character death rate from traps on deep levels) (#274)
- **Color Learning**: New players intuitively recognize threats by color after 3-5 encounters (#245)

---

## Design Decisions (Answered)

### Unique Items & Hardcore Mode
**Decision**: Each unique item is a **singleton per character save**.
- **Setting**: Hardcore Mode is a character creation choice, saved to save file
- **Standard Mode**: Unique items respawn on dungeon levels if not collected before leaving 
- **Hardcore Mode**: Unique items not found before leaving a level are gone forever (increases challenge)
- **Future**: Once character save serialization works, players can enable/disable Hardcore for their next character
- See #43 and #128 for implementation details

### Dungeon Depth & Endgame
**Design**: 
- Balrog (final boss) lives at **5000' (depth 100)**
- Boss can appear anywhere below 4500' (depth 90), guaranteed at 5000'
- Level 5000' has **no down staircases** — final level
- Killing boss = victory condition (triggers #42 scoring system)
- Progression arc: descend → survive → gather stat buffs → speed up → reach floor 100 → defeat Balrog on equal terms
- Balrog is designed as gear/stat check: cannot be beaten by low-speed characters (requires rings, potions, items)
- See Speed/Difficulty issue for interaction with character speed

### Stat Progression as Core Mechanic
**Design**: Stats exist from character creation and form a core progression path:
- Stats can be **damaged** by monsters (Worm Masses -WIS, Ghosts -life levels, Vampires drain levels)
- **Restore [Stat] Potions** undo damage (mid-level rewards)
- **Gain [Stat] Potions** permanently +1 to stat (high-level rewards)
- Winning strategy involves grinding Gain potions to max stats, enabling Balrog fight
- Stat system is as important as gear for character power

### Multi-Classing
**Decision**: **No multi-classing** in current design. Magic system follows Moria model (fixed, disciplined) rather than ZAngband (choosing multiple schools). Hybrid classes (Shaman, Druid) provide **class-level** diversity without **spell-selection** flexibility.

## Created Companion Issues

### Major Systems (Now in Issues)

**#240 - Multiplayer System** (deep future)
- Cooperative or competitive dungeon crawling
- Out-of-scope for v1.0; requires single-player systems to stabilize first

**#241 - Class-Specific Quests** (post-classes feature)
- Story arcs and character flavor per class
- Dependency: #239 (Classes)

**#243 - Town System**
- Safe hub with 5 shop types (General Store, Weaponsmith, Temple, Magic Shop, Black Market)
- NPC system: thieves steal coins, guards patrol, quest-givers
- Day/night cycle with guard availability
- Word of Recall: 30-50 turn delay, 85-150gp cost, recalls to last-explored dungeon level
- Player House for persistent item storage
- Shop markup formula: Base Price × Store Markup × Shopkeeper Modifier × Race Modifier
- **See GitHub issue #243 for full design, shop economics, and NPC behaviors**

**#244 - Intrinsics & Status Effects System**
- Framework for all temp/perm buffs/debuffs (4 versions per effect)
- 70+ potential effects: resistances, immunities, movement, status, sensory, special
- **No hard cap**: Characters can stack unlimited positive intrinsics if obtained
- **UI Strategy**: Temp buffs shown on stats screen with countdown; full Intrinsics Grid shows equipment × effect matrix
- **Sustain Mechanics**: Restores stat to max achieved value + blocks all damage to that stat (e.g., Sustain WIS restores and blocks -WIS damage)
- **Trap Integration**: Wilderness traps trigger intrinsic effects (fire trap, teleport trap, summon trap, trapdoor, stat damage, curse)
- **Passive Tracking**: Some effects (Blindness, Hallucination, Confusion) may track internally without UI; implement as discovered
- **See GitHub issue #244 for comprehensive effect categories and data model**

**#245 - Monster Color Effects & Standardized Threat Recognition**
- Standardized color → threat type mapping for player learning curve
- 13 standardized colors × 4 threat categories (stat drain, elemental, status, special)
- Yellow (Wisdom drain), Red (CON drain + Fire), Blue (All-stat drain + Cold), Green (Poison), Magenta (Chaos/Random stat), Brown (Fear), Black (Sleep), White (Paralyze), Grey (Confusion), Pink (Mana drain), Copper (Curse), Silver (Teleport away), Gold (Teleport player)
- Threat progression: Shallow (stat drain only) → Mid (elemental added) → Deep (all threat types)
- Enables consistent UI feedback and monster threat recognition at a glance
- **See GitHub issue #245 for color mappings and depth-based threat progression**

**#246 - Item Durability & Elemental Weakness Mechanics**
- Items degrade/destroy when exposed to elemental hazards (fire, cold, acid, lightning)
- Vulnerability by type: Scrolls (50-100% destroyed), Potions (50-100%), Books (15-50%), Equipment (5-20%), Rings (1-5%)
- Destroyed items become non-functional (scrolls illegible, potions empty, equipment corroded)
- Unique items cannot be permanently destroyed; degrade with penalties at <50%, restore on level-up
- Cursed items degrade 50% faster (vulnerability × 1.5)
- Forces strategic inventory planning per elemental-heavy dungeon levels
- **See GitHub issue #246 for damage models, UI indicators, and dungeon level threats**

