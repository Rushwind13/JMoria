# JMoria New Systems Design Documentation

This document captures the detailed design for two new systems that will be formalized as GitHub issues once initial designs are validated.

---

## #245 - Monster Color Effects & Standardized Threat Recognition

### Problem
Currently, monsters use various colors for visual distinction. Need a **standardized system** where color → threat type (stat damage, elemental, status effect) so player learns: "any red creature = CON/Fire threat".

### Design Philosophy
- **Learning curve**: Players should recognize monster threat by color after several encounters
- **Consistent mapping**: Same color always means same attack type across all monsters
- **Balance**: Threat type matches dungeon depth (shallow = Poison, deep = Stat drain + Elemental)

### Threat Categories & Color Mapping

#### 1. Stat-Damaging Attacks
| Color | Stat Drained | Example Monsters |
|---------|---------|---------|
| **Yellow** | Wisdom (fear/confusion) | Yellow Worm Mass, Ochre Jelly, Giant Bee |
| **Red** | Constitution (life levels) | Vampire, Wraith, Mummy (also Fire elemental) |
| **Blue** | All stats equally | Master Vampire, Lich (also Cold elemental) |
| **Green** | Poison damage + dexterity | Poison Snake, Hydra, Giant Spider |
| **Magenta** | Random stat | Demon, Devil (chaos-themed) |
| **Cyan** | Temporary stat reduction | Ghost, Spectre (ethereal, fading) |

#### 2. Elemental Attacks (Damage Type)
| Color | Element | Interactions |
|---------|---------|---------|
| **Red** | Fire | Scrolls burn, Potions evaporate; Fire resistance mitigates |
| **Blue** | Cold | Scrolls freeze, Potions freeze; Cold resistance mitigates |
| **Green** | Poison | Poison immunity blocks, POT_ANTIDOTE resists |
| **Yellow** | Electricity | Rare, high-level threat; Earth/Insulation resists |
| **Orange** | Acid | Potions melt/corrode, Metal equipment pits; Acid immunity blocks |

#### 3. Status Effect Attacks
| Color | Status | Example Monsters |
|---------|---------|---------|
| **Brown** | Fear/Cause Fear | Brown Bear, Brown Dragon, Brown Pudding |
| **Black** | Sleep (hard control, deadly) | Shadow, Black Hydra, Black Dragon |
| **White** | Paralyze + Slow | White Pudding, White Dragon |
| **Grey** | Confusion/Stunning | Grey Beast, Grey Mist (disorientation) |

#### 4. Special Attack Types
| Color | Threat | Notes |
|---------|---------|---------|
| **Pink/Magenta** | Mana Drain (spellcasters) | Mana-focused threat, Mage-specific danger |
| **Copper/Bronze** | Curse / Curse Item | Applies curse intrinsic, debuff-based | 
| **Silver** | Teleport / Blink Away | Erratic threat, hard to pin down |
| **Gold** | Teleport YOU (player) | Hard control, devastating |

### Implementation Details

#### Data Model
```cpp
struct MonColor {
  uint32_t color_rgb;
  AttackType attack_type;      // STAT_DRAIN, ELEMENTAL, STATUS, SPECIAL
  StatType stat_type;           // (if STAT_DRAIN) WIS, CON, STR, etc.
  ElementType element_type;     // (if ELEMENTAL) FIRE, COLD, POISON, ACID
  StatusType status_type;       // (if STATUS) FEAR, SLEEP, PARALYZE, etc.
  int threat_level_min;         // Earliest dungeon depth threat appears
};
```

#### Standardized Color Palette
```cpp
enum MonsterColorId {
  MCOLOR_YELLOW = 0xFFFF00,     // Stat drain (WIS)
  MCOLOR_RED = 0xFF0000,        // CON drain / Fire
  MCOLOR_BLUE = 0x0000FF,       // All stats / Cold  
  MCOLOR_GREEN = 0x00FF00,      // Poison
  MCOLOR_MAGENTA = 0xFF00FF,    // Random stat / Chaos
  MCOLOR_CYAN = 0x00FFFF,       // Temporary debuff
  MCOLOR_BROWN = 0x8B4513,      // Fear
  MCOLOR_BLACK = 0x000000,      // Sleep / Control
  MCOLOR_WHITE = 0xFFFFFF,      // Paralyze
  MCOLOR_GREY = 0x808080,       // Confusion
  MCOLOR_PINK = 0xFFC0CB,       // Mana drain
  MCOLOR_COPPER = 0xB87333,     // Curse
  MCOLOR_SILVER = 0xC0C0C0,     // Teleport away
  MCOLOR_GOLD = 0xFFD700,       // Teleport player
};
```

### Dungeon Progression Threats

#### Shallow (1-20 depth)
- **Yellow** Worm Mass (Wisdom drain, minor threat)
- **Green** Giant Spider (Poison, treatable)
- **Brown** Novice Rogue (Steal coins, fear)
- ✗ No Blue/Black/White/Magenta (save for depth progression)

#### Mid (20-40 depth)
- **Green** Hydra (Poison, higher damage)
- **Red** Vampire (CON drain, life levels, Fire attacks)
- **White** Giant Scorpion (Paralyze venom)
- **Grey** Wraith (Confusion + stat drain)
- **Cyan** Ghost (Temporary stat debuff, lesser threat)

#### Deep (40+ depth)
- **Blue** Master Vampire (All-stat drain, Cold elemental)
- **Blue** Ancient Lich (Blue, Cold, stat drain, Mana burn)
- **Red** Ancient Red Dragon (Fire + CON drain)
- **Black** Shadow (Sleep, one-shot threat)
- **Magenta** Demon Lord (Random stat + Chaos effects)
- **Gold** Ancient Balrog (Teleport player = hard control, devastating)

### Success Criteria

1. **Color → Threat Recognition**: Player can predict monster attack type from color
2. **Consistent Mapping**: No monster color used for two different threat types
3. **Progression**: Threat complexity increases with depth (Stat drain → Elemental → Status effects)
4. **Balance**: No single color dominates dungeon (variety per level feels fair)
5. **Documentation**: Monster definitions (Resources/Monsters.txt) updated with color/threat data

### Dependencies
- #77 (Effects Framework) — defines stat drain, elemental, status effect types
- #244 (Intrinsics & Status Effects) — intrinsic representation and UI display
- #242 (Speed System) — affects evade/survival of stat-draining attacks
- #197 (Stats) — stat damage formulas and Restore interactions

### Related Issues
- #42 (Balrog) — Gold threat with unique mechanic
- #46 (Monster Behavior) — threat recognition affects AI target selection
- #114 (Item ID) — player learns color → threat through identification

---

## #246 - Item Durability & Elemental Weakness Mechanics

### Problem
Currently, items are indestructible. Need a **durability system** where items degrade or break when exposed to elemental hazards (fire, cold, acid, lightning). This creates:
- Risk/reward in inventory choices (do I carry Books in a Fire dungeon level?)
- Economic sink (need replacement items, consume consumables faster)
- Realism (books burn, potions boil, metal corrodes)

### Design Philosophy
- **Weakness-based**: Items take damage only from specific elemental attacks
- **Non-permanent**: Can't permanently destroy unique items (prevents soft-locking)
- **Consumable impact**: Scrolls/Potions most vulnerable; Equipment hardier
- **Player choice**: Deliberate item drops for specific level types

### Item Vulnerability by Type

#### Scrolls (Paper-Based) — Most Vulnerable
| Threat | Effect | Damage |
|---------|---------|---------|
| Fire/Heat | Scroll burns, ink fades | 50-100% durability loss per hit |
| Cold/Freeze | Scroll becomes brittle, tears | 25-50% durability loss |
| Water (future) | Ink bleeds, illegible | 75% durability loss |
| Acid | Paper dissolves | 100% destroyed (unretrievable) |
| Lightning | Charred, crumbly | 50% durability loss |

**Result of Destruction**: Scroll becomes "Illegible Scroll" (unreadable, unsellable, ~0gp)

#### Potions (Glass Bottles) — Vulnerable
| Threat | Effect | Damage |
|---------|---------|---------|
| Fire | Bottle boils over, seals pop, potion evaporates | 50-100%, contents lost |
| Cold | Potion freezes or flask cracks | Bottle shatters, 100% loss |
| Acid | Flask corrodes, leaks | 50-75% loss, drips on adjacent items |
| Lightning | Cork flies, contents explode outward | 75% loss, scattered everywhere |
| Crushing (future) | Bottle breaks | 100% loss |

**Result of Destruction**: Potion bottle becomes empty/useless (~0gp), adjacent items take splash damage

#### Books (Heavy Paper/Binding) — Moderate Vulnerability
| Threat | Effect | Damage |
|---------|---------|---------|
| Fire | Pages burn, binding chars | 25-50% durability, may become unreadable |
| Cold | Binding warps, pages stick | 15-30% durability |
| Acid | Cover corrodes, pages dissolve | 40-60% durability |
| Water (future) | Pages swell, ink bleeds | 50% durability, readability reduced |
| Lightning | Spine cracks, pages singe | 25% durability |

**Result of Destruction**: Book becomes "Damaged Tome" (unreadable, unusable, ~10% value)

#### Equipment (Armor/Weapons) — Hardiest
| Threat | Effect | Damage |
|---------|---------|---------|
| Fire | Metal oxidizes, wooden handles scorch | 5-15% durability, AC reduced by 1-2 |
| Cold | Metal becomes brittle | 5-10% durability, Risk of weapon breaking on critical hits |
| Acid | Metal pits, corrodes joints | 10-20% durability, AC/damage degraded |
| Lightning | Metal arcs and warps | 10-15% durability, enchantments disrupted |
| Crushing (future) | Dents, bends | 5% durability |

**Result of Destruction**: Equipment becomes "Corroded/Damaged [item name]" (functional but penalized AC/damage by 1-3 points)

#### Rings & Amulets — Resistant
| Threat | Effect | Damage |
|---------|---------|---------|
| Fire/Cold/Acid | Minimal effect | 1-5% durability loss |
| Lightning | Intrinsic surges, minor damage | 2-5% durability loss |

**Result**: Rings rarely destroyed, mostly superficial (flavor text: "ring glows briefly, unharmed")

### Implementation Details

#### Data Model
```cpp
struct ItemDurability {
  uint16_t current_durability;      // 0-100%
  uint16_t max_durability;          // Usually 100
  ElementType weakness[4];          // Up to 4 elemental weaknesses (FIRE, COLD, ACID, LIGHTNING)
  uint16_t weakness_damage[4];      // % damage per hit from each element (50-100%)
  bool can_be_destroyed;            // Scrolls/Potions: true, Equipment: false
  bool is_destroyed;                // If destroyed, item becomes non-functional
};

enum ItemDestructionState {
  ITEM_DURABILITY_OK,
  ITEM_DURABILITY_WARN,    // <50% durability, show indicator
  ITEM_DURABILITY_CRITICAL, // <25% durability, warning
  ITEM_DURABILITY_DESTROYED  // 0% durability, item non-functional
};
```

#### Damage Application
```cpp
void ApplyElementalDamage(Item *item, ElementType element, int damage) {
  if (item == nullptr) return;
  
  // Check if item has this weakness
  int weakness_idx = -1;
  for (int i = 0; i < 4; i++) {
    if (item->durability.weakness[i] == element) {
      weakness_idx = i;
      break;
    }
  }
  if (weakness_idx == -1) return; // No weakness, immune
  
  // Calculate damage
  int damage_pct = item->durability.weakness_damage[weakness_idx];
  int durability_loss = (damage * damage_pct) / 100;
  
  // Apply
  item->durability.current_durability -= durability_loss;
  if (item->durability.current_durability <= 0) {
    item->durability.is_destroyed = true;
    item->durability.current_durability = 0;
    // Message: "Your Spell Book has been destroyed by fire!"
  }
  
  // Warning
  if (item->durability.current_durability <= 25) {
    DisplayWarning("Your " + item->name + " is nearly destroyed!");
  }
}
```

### Dungeon Level Elemental Threats

#### Fire-Themed Levels (Red dragons, Demons)
- Red Dragon attacks (Fire breath, Burning touch)
- Fire Elemental spawns
- Floor traps: Lava pits

**Impact**: Scrolls/Potions especially vulnerable; Books reduced 25-50% durability
**Player Adaptation**: Carry fewer scrolls, more direct-action gear (potions for healing instead)

#### Cold-Themed Levels (Ice mages, Blue dragons)
- Blue Dragon attacks (Cold breath)
- Ice Elemental spawns
- Floor traps: Frozen ground (slowing)

**Impact**: Potions freeze/shatter, Cold resistance needed
**Player Adaptation**: Use cold resistance intrinsic, careful potion management

#### Acid-Themed Levels (Gelatinous creatures, Acid Puddings)
- Ooze attacks (Acid spray)
- Acid Elemental spawns
- Floor traps: Acid pools

**Impact**: Equipment heavily damaged (AC penalties), Items destroyed
**Player Adaptation**: Acid resistance + Proof [Acid] amulet, or avoid dungeon

#### Lightning-Themed Levels (Storm Mages, Djinn, High-Tech Ruins)
- Storm Mage casts Lightning Bolt
- Electric Elemental spawns

**Impact**: Equipment slightly damaged, enchantments disrupted temporarily
**Player Adaptation**: Insulation intrinsic, Lightning resistance

### Inventory Management UX

#### Item Icon Indicators
- **OK**: Normal item sprite (e.g., leather-bound book = solid color)
- **Warn**: Item sprite with small damage symbol (crack, scorch mark) = <50% durability
- **Critical**: Item sprite with heavy damage symbol (multiple cracks/burns) = <25% durability
- **Destroyed**: Item sprite greyed out or with "X" overlay = destroyed item

#### Inventory Popup Tooltip
```
Spell Book (Damaged)
Durability: [████░░] 45%
Weaknesses: Fire (50% damage), Acid (100% destroyed)
Weight: 5 lb
Value: 50 gp (normally 200 gp)
```

#### Dungeon Messages
```
> Red Dragon's Fire Breath hits you!
You take 25 damage.
Your Spell Book is damaged! (Durability: 75% → 45%)
Your spellcasting ability is compromised...

> Ooze's Acid Spray touches your Pack!
Your Potion of Cure Wounds has been destroyed!
```

### Special Cases

#### Unique Items
- Unique items cannot be permanently destroyed (prevents save-scumming mechanics)
- But durability still degrades, inflicting penalties (AC, Damage) at <50%
- Unique items restore to 100% durability when character advances 1 level or rests
- **Design rationale**: Hardcore mode needs risk without hard-locks

#### Cursed Items
- Cursed items degrade **faster** (weakness damage +50%)
- Represents spite of curse + vulnerability
- e.g., "Cursed Spell Book" takes 75% damage from Fire (normally 50%)

#### Identify & Pricing
- Destroyed/Damaged items identified as such ("Illegible Scroll", "Damaged Longsword")
- Price reduced: If destroyed = ~5% value, If <50% durability = 40-60% value
- NPCs won't buy destroyed consumables ("These scrolls are ruined. Go to the rubbish heap!")

### Success Criteria

1. **Risk/Reward**: Elemental damage threatens inventory management
2. **Player Agency**: Deliberate item dropping/carrying choices per level type
3. **Economic Impact**: 10-30% more resource consumption in elemental-heavy levels
4. **Realism**: Books burn, potions boil, metal corrodes (expected behavior)
5. **No Soft-Locks**: Unique items remain usable (penalized but not broken)
6. **UI Clarity**: Durability status visible at a glance

### Dependencies
- #77 (Effects Framework) — defines elemental damage types
- #128 (Magic Items) — unique item mechanics, damage penalties
- #245 (Monster Color Effects) — elemental attacks identified by monster color
- #242 (Speed System) — affects evade/mitigation of elemental attacks

### Implementation Phasing

#### Phase 1: Core System
- Data model for durability (current/max, weaknesses, destruction flag)
- Damage application logic (elemental damage → durability loss)
- Basic UI indicators (simple greyscale overlay for destroyed items)

#### Phase 2: Content
- Assign weaknesses to all item types (scrolls/potions/books/equipment)
- Calibrate damage percentages per item type/element
- Add destruction messages and warnings

#### Phase 3: Dungeon Balance
- Assign elemental threats to dungeon levels (fire dragons, acid oozes, etc.)
- Test resource consumption (should it increase ~15-20%? Adjust cap)
- Polish unique item restoration mechanics

#### Phase 4: Polish
- Durability UI icons and tooltips
- Character voice lines (e.g., "My book is burning!")
- Hardcore mode unique item handling (restoration on level-up?)
