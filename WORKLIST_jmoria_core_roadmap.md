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

### #39 - Implement Ranged Attacks
**Status**: In progress (PR #144)
**Description**: Add bow-based ranged combat with targeting, ammunition management, and line-of-sight mechanics.
**Impact**: Opens alternative playstyle, extends combat depth.
**Dependencies**: None blocking
**Effort**: High

### #197 - Player Stats (STR/CON/DEX/INT/WIS/CHA)
**Status**: Not started
**Description**: Implement 6-attribute character system affecting combat, defense, and special abilities.
- STR: melee hit/damage, carry capacity
- CON: HP pool, health recovery
- DEX: AC, ranged hit, initiative
- INT/WIS: spellcaster prep (for future magic system)
- CHA: NPC interaction (for future shop system)
**Impact**: Foundation for all character progression and balance tuning.
**Dependencies**: None blocking
**Effort**: Medium

### #42 - JMoria Scoring and Win Condition
**Status**: Not started
**Description**: Implement high score system and boss encounter to provide clear win/loss conditions and competitive replayability.
- Points formula: depth × player_level + experience + monster_kills - penalties
- Killing boss grants +40 levels
- Max level: 40 (normal) → 80 (with boss kill)
**Impact**: Defines endgame goal, provides performance metric for bot testing.
**Dependencies**: None blocking
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

### #121 - Fuel for Lanterns & Wand Charges
**Status**: Not started
**Description**: Add consumable resource management for light sources and wands.
- Lanterns: Fill with Flask of Oil (give 5000 turns, max 15000)
- Wands: Scroll of Recharging adds charges (amount varies by wand/user/depth)
- Wand risk: recharging at high depth can cause wand to explode
**Impact**: Strategic resource planning, depth-based risk/reward.
**Dependencies**: Pairs with #72 (light economy)
**Effort**: Low-Medium

### #117 - Search Command & Secret Door Detection
**Status**: Not started
**Description**: Implement active search mechanic for finding hidden doors and traps.
- Passive: 5% chance to detect secret doors within 1 square
- Active search: 60% chance within 1 square, 20% at 2 squares (acts like rest)
- Ring of Searching: adds base % to both passive and active
- Scroll of Detect Doors/Stairs: reveals all secret doors in radius 30
- Scroll of Detect Traps: reveals all traps in radius 30
**Impact**: Enables hidden area discovery, buffs stealth playstyle, blocks #214 (dungeon accessibility).
**Dependencies**: None blocking
**Effort**: Medium
**Note**: **Blocks** #214 (dungeon gen isolated hallways)

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

### #111 - Use Commands: Inventory Filtered by Type
**Status**: Not started
**Description**: When using a command (quaff, read, zap, eat), show only types applicable to that action.
- Quaff: potions only
- Read: scrolls only
- Zap: wands/staffs only
- Eat: food items only
**Impact**: UX improvement, reduces player confusion.
**Dependencies**: None blocking
**Effort**: Low

### #110 - Monsters Don't Move During Use Commands
**Status**: Not started
**Description**: Prevent AI from moving between player selecting a use-command and selecting the target/item.
- Check `m_bReadyForUpdate` flag during use command flow.
**Impact**: Removes unfair surprise attacks during inventory management.
**Dependencies**: None blocking
**Effort**: Low

### #43 - Lady Teldra: Unique Item Personality
**Status**: Not started
**Description**: Implement named, unique weapons with personality and special properties.
- Example: Lady Teldra won't attack peaceful creatures ("refuses to leave her sheath")
- Unique items can persist across difficulty/resets or be one-per-game
- Flavor text varies by wielder/item relationship (e.g., "Lady Teldra scoffs at such simple magicks")
**Impact**: Added character/story, deeper item systems.
**Dependencies**: #128 (item tiers)
**Effort**: High

### #45 - MON_AI_SEEK: Target Position vs Player
**Status**: Not started
**Description**: Allow AI to seek arbitrary target positions, not just the player.
- Tigers attack other creatures (rabbits, kobolds, player)
- Shamans path to altars for summon casting
- Flock/school/pack behaviors
- Future: Player "run toward target" command
**Impact**: More sophisticated monster AI, emergent creature interactions.
**Dependencies**: None blocking
**Effort**: Medium

### #180 - Dump Full Dungeon Map on Death
**Status**: Not started
**Description**: Display the full 100×100 dungeon map on the death screen, revealing all unexplored areas.
- Uses canonical TileIDs character set
- Shown between tombstone and score list
- Player can advance with keypress
**Impact**: Satisfying death screen UX, reveals dungeon scale.
**Dependencies**: DumpMap() utility (already exists in DungeonMap)
**Effort**: Low

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
```

---

## Suggested Implementation Order

1. **Foundation Sprint** (Stats, Scoring, Identification)
   - #197 → #42 → #114
   - Enables progression and loot meaning

2. **Combat Expansion** (Ranged, Magic Items)
   - #39 (already in progress) → #128
   - Adds playstyle variety

3. **Economy & Depth** (Light, Resources, Search)
   - #72 → #121 → #117
   - Fixes #214 as fallout, enriches gameplay

4. **Quality of Life** (Overflow fixes + UX)
   - #111, #110, #112, #180
   - Polish and accessibility

5. **Advanced Systems** (Effects, Personality, AI)
   - #77, #43, #45
   - Sets stage for future expansion

---

## Metrics & Success Criteria

- **Replayability**: Different character stats + item diversity should yield ~100+ unique playstyles
- **Win Rate**: Competent player should have ~60% first-character win rate by depth 50
- **Score Variance**: High-score list should have 50%+ turnover per 10 games (indicates balance tuning needed)
- **Bot Benchmark**: Reference bot should reach depth 20 with consistent survival rate (pairs with bot roadmap)

---

## Open Questions

1. Is there interest in player classes (Warrior/Mage/Priest) or just stats-based builds?
2. Should unique items be one-per-game or reset per playthrough?
3. Is there a planned max depth, or is it "infinite until boss"?
4. Does the game support multiplayer or is it single-player only?

