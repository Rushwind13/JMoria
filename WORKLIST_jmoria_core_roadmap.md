# JMoria Core Roadmap

This roadmap tracks core game features that are essential to JMoria's gameplay experience. These are distinct from the AI bot roadmap and focus on player-facing systems, mechanics, and content.

## Philosophy

The core roadmap prioritizes:
1. **Player agency** - meaningful choices, progression, customization
2. **Strategic depth** - character building, item variety, tactical combat
3. **Replayability** - diverse builds, randomized loot, multiple playstyles
4. **Quality of life** - clear feedback, discoverable mechanics, intuitive UI

---

## 🎯 Priority 1: Core Mechanics & Foundation

### #121 - Fuel for Lanterns & Wand Charges
**Status**: ✅ Lantern fuel done (phase2-gameplay-expansion). Wand charges/recharging not yet implemented.
**Description**: Add consumable resource management for light sources and wands.
- ✅ Lanterns: Fill with Flask of Oil (Shift+F), max 15000 fuel, consumes flask
- Wands: Scroll of Recharging adds charges (amount varies by wand/user/depth)
- Wand risk: recharging at high depth can cause wand to explode
**Impact**: Strategic resource planning, depth-based risk/reward.
**Dependencies**: Pairs with #72 (light economy) blocks #39 (ranged attacks)
**Effort**: Low-Medium

### #39 - Implement Ranged Attacks
**Status**: In progress (PR #144)
**Description**: Add bow-based ranged combat with targeting, ammunition management, and line-of-sight mechanics.
**Impact**: Opens alternative playstyle, extends combat depth.
**Dependencies**: None blocking
**Effort**: High


### #117 - Search Command & Secret Door Detection
**Status**: ✅ Core search done (phase2-gameplay-expansion). Scrolls/rings not yet implemented.
**Description**: Implement active search mechanic for finding hidden doors and traps.
- ✅ Passive: 5% chance to detect secret doors within 1 square (CHANCE_SEARCH_PASSIVE)
- ✅ Active search: 25% chance within 1 square via 's' key (CHANCE_SEARCH_ACTIVE)
- ✅ Configurable chance constants for all door/search interactions in Constants.h
- Ring of Searching: adds base % to both passive and active
- Scroll of Detect Doors/Stairs: reveals all secret doors in radius 30
- Scroll of Detect Traps: reveals all traps in radius 30
**Impact**: Enables hidden area discovery, buffs stealth playstyle, blocks #214 (dungeon accessibility).
**Dependencies**: None blocking
**Effort**: Medium
**Note**: **Blocks** #214 (dungeon gen isolated hallways)

### #180 - Dump Full Dungeon Map on Death
**Status**: ✅ Done (phase1-quick-wins)
**Description**: Display the full 100×100 dungeon map on the death screen, revealing all unexplored areas.
- Uses canonical TileIDs character set
- Shown between tombstone and score list
- Player can advance with keypress
**Impact**: Satisfying death screen UX, reveals dungeon scale.
**Dependencies**: DumpMap() utility (already exists in DungeonMap)
**Effort**: Low

### #111 - Use Commands: Inventory Filtered by Type
**Status**: ✅ Done (phase1-quick-wins)
**Description**: When using a command (quaff, read, zap, eat), show only types applicable to that action.
- Quaff: potions only
- Read: scrolls only
- Zap: wands/staffs only
- Eat: food items only
**Impact**: UX improvement, reduces player confusion.
**Dependencies**: None blocking
**Effort**: Low

### #110 - Monsters Don't Move During Use Commands
**Status**: ✅ Done (phase1-quick-wins)
**Description**: Prevent AI from moving between player selecting a use-command and selecting the target/item.
- Check `m_bReadyForUpdate` flag during use command flow.
**Impact**: Removes unfair surprise attacks during inventory management.
**Dependencies**: None blocking
**Effort**: Low

### #45 - MON_AI_SEEK: Target Position vs Player
**Status**: ✅ Foundation done (phase2-gameplay-expansion). Higher-level behaviors not yet implemented.
**Description**: Allow AI to seek arbitrary target positions, not just the player.
- ✅ m_vTargetPos + SetTargetPos() on CAIBrain, WalkSeek() uses arbitrary target
- ✅ MON_AI_SEEKPLAYER sets target to player pos before seeking
- Tigers attack other creatures (rabbits, kobolds, player)
- Shamans path to altars for summon casting
- Flock/school/pack behaviors
- Future: Player "run toward target" command
**Impact**: More sophisticated monster AI, emergent creature interactions.
**Dependencies**: None blocking
**Effort**: Medium
---

## 🔧 Priority 2: Major Systems

### #114 - Identify Spell & Item Knowledge System
**Status**: Not started
**Description**: Implement item identification mechanics with learned properties and class-specific "feelings."
- Players learn item properties through: trying to remove, using, time-based discovery
- Scroll of Identify reveals most properties; Scroll of *Identify* reveals all
- Class feelings: Warriors sense weapon curses; Mages sense magic items; Priests sense blessings
- Known intrinsics display in inventory; unknown ones remain hidden
**Impact**: Enables strategic item management and knowledge-based gameplay.
**Dependencies**: None blocking (but pairs with #128)
**Effort**: Medium

### #72 - Fog of War & Sight Distance Expansion
**Status**: Partially done (some features in)
**Description**: Expand visibility/lighting systems with depth-based darkness, multiple light sources, and special sight modes.
- Rooms spawn lit/dark based on depth (lit chance → 0 at depth 50)
- Torches: 3000-turn fuel, radius 3
- Lanterns: refueling with oil (+5000 per can), radius 5, max 15000 turns
- Infravision: race intrinsic, sees warm creatures in dark, radius 8
- ESP: race intrinsic, detects brains in dark, radius 8 (not undead/worms)
- Scroll of Light: light current room radius 10
- Wand of Light: shoot line of light, damage light-weak creatures (blue light hurts orcs, vampires, worm masses)
**Impact**: Emergent light economy, depth-based atmosphere, utility item variety.
**Dependencies**: None blocking
**Effort**: High

### #77 - Item Effects System
**Status**: Not started
**Description**: Implement comprehensive effect system for items and spells:
- EFFECT_FLAG: fire, cold, acid, lightning, poison, identify, AC, etc.
- EFFECT_MOD: weak/resistant/immune/gain/lose/restore for each effect
- EFFECT_TYPE: determines target and persistence (instant, timed, permanent)
**Impact**: Unified system for all magical item/spell effects, enables complex interactions.
**Documentation**: /doc/ItemEffectSystem.md (proposed)
**Dependencies**: None blocking
**Effort**: High


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

## 🐛 Known Blocking Issues

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
   
✅ #111 (Filtered inventory)
   └─ No deps

#114 (Item Identification)
   └─ No hard deps
   └─ Enhances #128

#128 (Magic Items)
   └─ #114 (Item Identification) — recommended but not required
   └─ Enables #43 (Unique items)

#43 (Unique Items)
   └─ #128 (Magic Items)

#72 (Fog of War)
   └─ No hard deps
   └─ Pairs with #121 (Lanterns)

#121 (Lantern Fuel)
   └─ Pairs with #72 (Fog of War)

#77 (Item Effects)
   └─ No hard deps
   └─ Future enabler for all magical items

#117 (Search Command)
   └─ No deps
   └─ **Blocks** #214 (Dungeon gen)

#112 (Races)
   └─ No hard deps
   ├─ Best after #197 (Stats)

#45 (MON_AI_SEEK)
   └─ No hard deps

#180 (Death Screen Map)
   └─ DumpMap() exists; low effort

#239 (Add Classes)
   ├─ #197 (Stats) — enables full stat priority system
   ├─ #77 (Item Effects) — enables spell mechanics
   ├─ #121 (Lantern Fuel) — enables mana/resource management
   └─ Phase 1 (Warrior only); Phase 2 (Rogue/Ranger); Phase 3 (Spellcasters)

#242 (Speed System & Difficulty Tuning)
   ├─ #197 (Stats) — for DEX interactions
   └─ Enables #42 (Balrog) difficulty tuning

#243 (Town System)
   ├─ #114 (Item Identification) — recommended
   ├─ #197 (Stats) — for stat-based shopping interactions
   └─ Enables #241 (Class Quests), economy integration

#244 (Intrinsics & Status Effects)
   ├─ #77 (Item Effects) — defines effect types
   ├─ #197 (Stats) — for stat modifiers and Sustain mechanics
   ├─ #242 (Speed System) — Speed intrinsics interact with action economy
   └─ Enables #245 (Monster Color) and #246 (Item Durability)

#245 (Monster Color Effects)
   ├─ #77 (Item Effects) — stat drain, elemental, status effect types
   ├─ #244 (Intrinsics) — intrinsic representation and UI
   ├─ #242 (Speed System) — affects evade/survival of attacks
   └─ Supports #42 (Balrog uses Gold threat)

#246 (Item Durability & Elemental Weakness)
   ├─ #77 (Item Effects) — elemental damage types
   ├─ #128 (Magic Items) — unique item mechanics
   ├─ #245 (Monster Color) — elemental attacks from colors
   └─ #242 (Speed System) — affects evade of elemental attacks
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

### **Phase 3: Deep Systems Foundation** (Weeks/Sprint)
Complex P2 systems that enable everything downstream:
- #72 (Fog of War & lighting) — atmosphere, depth progression
- #77 (Item Effects system) — foundation for all magical interactions
- #114 (Item Identification) — strategic knowledge, enables item tiers

### **Phase 3b: Difficulty & Progression** (Concurrent/Early P4)
Major mechanics that define end-to-end player experience:
- **#242 (Speed System & Difficulty Tuning)** — ACTION ECONOMY, **primary Balrog difficulty lever**, equipment goals (speed rings/boots)
  - Equipment limits: 2 rings, 1 boots, 1 gloves = +40 base speed ceiling
  - Monster speed calibration: Bats 2 → Balrog 4-5 creates difficulty curve
  - Success metric: Balrog unwinnable at ~15 speed, winnable at ~30+ speed
- Ties together: Monster design (speed ratings), Equipment rewards, Stat synergy

### **Phase 4: Town & Shop Hub** (Early P4)
Enables all mid-to-late-game systems:
- **#243 (Town System)** — SAFE HUB, shop economy, NPC interaction, Word of Recall
  - 5 shop types with unified markup formula
  - Thieves steal coins, Guards patrol with decay timer, Quest-givers for flavor
  - Player House: persistent storage for item management
  - WoR: 30-50 turn delay enables frequent town visits without soft-locking
- Prerequisites: Item ID (#114), Stats (#197), Magic Items (#128)
- Enables: Quest givers (#241), Class flavor, Gold sink for economy

### **Phase 4b: Item & Effect Systems** (Concurrent P4)
- **#246 (Item Durability & Elemental Weakness)** — Strategic inventory management
  - Scrolls/Potions destroyed by elemental hazards; Equipment degraded
  - Forces thoughtful item carrying (e.g., fewer scrolls in fire-themed levels)
  - Unique items degrade but never destroyed (no save-scumming)
- **#245 (Monster Color Effects)** — Standardized threat recognition
  - Yellow = WIS/Confusion, Red = CON/Fire, Blue = All-stats/Cold, etc.
  - Creates learning curve for recognizing monster threat types
  - 80+ monsters mapped to 13 standardized colors

### **Phase 5: Character Progression & Replayability** (Later P5)
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

- **Stats (#197) + Speed (#242) + Equipment**: High-level character requires high stats (Gain potions) + speed gear (rings, boots) to survive Balrog's damage output and action rate
- **Speed (#242) + Monster Colors (#245)**: Monster speed determines encounter danger; Player speed creates progression arc; Color coding enables threat recognition
- **Town (#243) + Items (#128) + Shops**: Players gather magical items and speed equipment from shops, creating equipment progression arc
- **Stat Damage (monsters) + Restoration (potions + town temples)**: Stat damage becomes meaningful challenge; town provides access to Restoration potions
- **Classes (#239) + Stats (#197) + Town (#243)**: Each class has different stat priorities; town shops provide class-specific equipment (Mage seeks +INT, Rogue seeks +DEX or AC, etc.)
- **Item Durability (#246) + Elemental Hazards**: Fire-themed levels destroy scrolls; Cold levels freeze potions; forces strategic inventory planning
- **Speed (#242) + Item Durability (#246)**: High-speed character also needs defensive intrinsics (fire/cold resistance) to protect items; speed alone insufficient for survival
- **Town (#243) + Item Durability (#246)**: Player House storage becomes critical for protecting valuable items between levels; encourages frequent town visits

---

## Metrics & Success Criteria

- **Replayability**: 8 classes × (6 stats configurations) × (100+ item combinations) should yield ~500+ distinct viable builds
- **Win Rate**: Competent player should have ~40-60% first-character win rate (depends on Balrog tuning)
- **Score Variance**: High-score list should have 50%+ turnover per 10 games (indicates balance tuning needed)
- **Bot Benchmark**: Reference bot should reach depth 20 with consistent survival rate (pairs with bot roadmap)
- **Class Balance**: No single class should dominate win rates; victory distribution relatively even across classes
- **Speed Tuning**: Balrog should be impossible without speed ~25-30; achievable with speed ~30-40

---

## Design Decisions (Answered)

### Unique Items & Hardcore Mode
**Decision**: Each unique item is a **singleton per character save**.
- **Setting**: Hardcore Mode is a character creation choice, saved to save file
- **Standard Mode**: Unique items not found before leaving a level are gone forever
- **Hardcore Mode**: Unique items respawn on dungeon levels if not collected before leaving (increases challenge)
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

**#242 - Speed System & Difficulty Tuning**
- Action economy as primary difficulty lever
- Equipment slot limits: 2 rings, 1 boots, 1 gloves = +40 max base speed
- Potions stack temporarily (+10 per Speed potion)
- Monster speeds calibrated: Bats 2, Vampire Lords 3-4, Balrog 4-5
- Balrog unwinnable at low speed (~15), winnable at high speed (~30+)
- **See GitHub issue #242 for full design and monster speed table**

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

