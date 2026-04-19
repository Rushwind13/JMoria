# Item Design — JMoria

Authoritative design document for the JMoria item system. Effect system (verbs, nouns, adverbs, named effects) is in [Effects-Design.md](Effects-Design.md). Raw source material in [WORKLIST_items_design.md](WORKLIST_items_design.md).

---

## 1. Item Data Model

### Items.txt Fields

Every item in [Resources/Items.txt](../Resources/Items.txt) is a named block with these fields:

| Field | Type | Required | Description |
|---|---|---|---|
| Plural | string | yes | Plural display name |
| Type | string | yes | ITEM_IDX_* type constant |
| Value | float | yes | Base cost in gold |
| Level | int | yes | Earliest dungeon level (0 = town) |
| Flags | string | — | Comma-delimited ITEM_FLAG_* flags |
| Color | RGBA | — | Display color |
| AC | float | — | Base AC when equipped |
| ACBonus | NdM | — | Magical AC bonus |
| Damage | NdM | — | Base damage dice |
| To-HitBonus | NdM | — | Magical to-hit bonus |
| To-DamBonus | NdM | — | Magical to-damage bonus |
| Speed | float | — | Attack speed |
| Weight | float | — | Weight in pounds |
| Effect | string,NdM | — | Effect type + amount/duration |

### Item Instance vs Definition

**CItemDef** is the template (parsed from Items.txt). **CItem** is a live instance with per-object state:

| CItemDef (template) | CItem (instance) |
|---|---|
| m_szName, m_szPlural | m_id → points to def |
| m_szUnidentifiedName, m_szFlavor | m_dwFlags (CURSED, IDENTIFIED, etc.) |
| m_dwFlags (STACKS, 2HANDED, etc.) | m_dwCharges |
| m_llEffects (linked list of CEffect) | m_dwCount (stack size) |
| m_dwLevel, m_fValue, m_fWeight | m_dwKnownProps (KNOWN_* bitmask) |
| m_fDuration, m_fRadius | m_fRemainingDuration |

### Effects

Each item can have one or more effects. Effects are reusable LEGO pieces — the same named effect can appear on multiple items. See [Effects-Design.md](Effects-Design.md) for the full effect data model, grammar, and catalog.

Items reference effects by name in Items.txt:

```
Effect <Firebolt>
Effect <Resist Acid>
```

Or inline (legacy):

```
Effect <EFFECT_TYPE_HIT>,<EFFECT_FLAG_FIRE>,<EFFECT_MOD_LINE>,<2d8>
```

---

## 2. Wands vs Staves

Wands and staves share the same effect nouns and adverbs but use different verbs. A wand is **directed at a target** (HIT); a staff is **self or area effect** (CREATE / RESTORE).

| Concept | Wand | Staff |
|---|---|---|
| Verb | HIT (at target) | CREATE or RESTORE (self/area) |
| Visual | Blue zap line to target | Effect on user or room |
| Targeting | Player aims at monster/tile | Automatic (self or AoE) |
| Example | Wand of Teleport Away → HIT + TELEPORT | Staff of Teleportation → CREATE + TELEPORT |
| Example | Wand of Probing → HIT + IDENTIFY | Staff of Perception → RESTORE + IDENTIFY |
| Charges | Yes (NdM, see §6) | Yes (NdM, see §6) |
| Rechargeable | Yes | Yes |
| Stackable | Yes (same flavor, ID'd, same charges) | Yes (same flavor, ID'd, same charges) |

### Known Wand Types

| Wand | Effect | Status |
|---|---|---|
| Wand of Light | Light Ray + Light Area | In Items.txt |
| Wand of Firebolts | Firebolt | In Items.txt |
| Wand of Fireballs | Fireball | In Items.txt |
| Wand of Frost | Frost Bolt | In Items.txt |
| Wand of Frost Balls | Frost Ball | In Items.txt |
| Wand of Lightning | Lightning Bolt | In Items.txt |
| Wand of Lightning Balls | Lightning Ball | In Items.txt |
| Wand of Acid | Acid Bolt | In Items.txt |
| Wand of Acid Balls | Acid Ball | In Items.txt |
| Wand of Sleep | Cause Sleep | In Items.txt |
| Wand of Confusion | Cause Confusion | In Items.txt |
| Wand of Paralyze | Cause Paralysis | In Items.txt |
| Wand of Stone to Mud | Stone to Mud | In Items.txt |
| Wand of Summoning | Summon Monsters | In Items.txt |
| Wand of Teleport Away | Teleport Away | In Items.txt |
| Wand of Heal Monster | Heal Monster | In Items.txt |
| Wand of Probing | Probe | In Items.txt |
| Wand of Fear | Cause Fear | In Items.txt |

### Known Staff Types

| Staff | Effect | Status |
|---|---|---|
| Staff of Light | Light Area | In Items.txt |
| Staff of Starlight | Light Ray + Light Area | In Items.txt |
| Staff of Healing | Minor Healing + Cure Poison | In Items.txt |
| Staff of Cure Light Wounds | CLW + Cure Poison + Cure Blindness | In Items.txt |
| Staff of Cure Serious Wounds | CSW + Cure Poison + Cure Blindness + Cure Confusion | In Items.txt |
| Staff of Greater Healing | Greater Healing + Cure Poison + Cure Blindness + Cure Confusion + Cure Fear | In Items.txt |
| Staff of Fire Resistance | Timed Resist Fire | In Items.txt |
| Staff of Cold Resistance | Timed Resist Cold | In Items.txt |
| Staff of Electricity Resistance | Timed Resist Electricity | In Items.txt |
| Staff of Acid Resistance | Timed Resist Acid | In Items.txt |
| Staff of Resistance | All 4 timed elemental resistances | In Items.txt |
| Staff of *Resistance* | All 4 timed elemental immunities | In Items.txt |
| Staff of Perception | Identify | In Items.txt |
| Staff of Teleportation | Teleport Self | In Items.txt |
| Staff of Word of Recall | Recall | In Items.txt |
| Staff of Mapping | Partial Mapping | In Items.txt |
| Staff of Telepathy | Timed ESP | In Items.txt |
| Staff of Treasure Detection | Detect Treasure | In Items.txt |
| Staff of Summoning | Summon Monsters | In Items.txt |
| Staff of Sleep | Cause Sleep | In Items.txt |
| Staff of Mass Sleep | Mass Sleep | In Items.txt |
| Staff of Fear | Mass Fear | In Items.txt |
| Staff of Paralysis | Mass Paralyze | In Items.txt |
| Staff of Protection | Timed Blessing | In Items.txt |

---

## 3. Item Quality Tiers

Six tiers of item quality. Quality is determined at spawn time.

| Tier | Bonuses | Rarity | Depth Req | Example |
|---|---|---|---|---|
| Normal | (+0, +0) | Common | Any | Leather Armor |
| Cursed | (-N, -N) | Uncommon | Any | Leather Armor (-2, -3) |
| Magic | (+N, +N) | Common magic | Any | Leather Armor (+2, +1) |
| Ego | Named suffix | Semi-rare (~1%) | Varies | Bastard Sword of Frost Brand |
| Legendary | Fixed constellation | Very rare (<0.01%) | Level 20+ | Holy Avenger |
| Unique | Named, one per game | Singleton | Carrier depth | Excalibur |

### Spawn Probability Chain

```
1. Spawn an item (level-appropriate base type)
2. Is it a weapon or armor?
3. Magical? (% based on dungeon level, item level, luck)
4. If magical → cursed? (random chance)
5. If magical and not cursed → ego? (~1%)
6. If ego → legendary? (~0.01%, depth 20+ only)
7. If legendary → unique? (mob-carried, per Uniques.txt)
```

### Blessed / Uncursed / Cursed Three-State

All items have one of three blessed states. Two versions of the same item can behave differently based on state:

| State | Behavior | Removable |
|---|---|---|
| Blessed | Enhanced bonuses | n/a (desirable) |
| Uncursed | Normal behavior | n/a |
| Cursed | Reduced/negative bonuses, cannot unequip | Scroll of Remove Curse |

---

## 4. Identification System

### Known Properties Bitmask (CItem::m_dwKnownProps)

| Flag | Hex | Revealed By |
|---|---|---|
| KNOWN_CURSED | 0x01 | Failed remove attempt |
| KNOWN_BONUSES | 0x02 | Scroll of Identify |
| KNOWN_CHARGES | 0x04 | Scroll of Identify |
| KNOWN_TRIED | 0x08 | Blind use without noticeable effect |

### Identification Methods

| Method | What It Reveals |
|---|---|
| **Scroll of Identify (?ID)** | Base name, bonuses, charges, cursed status |
| **Scroll of *Identify* (?*ID*)** | Full lore: ego name, intrinsics, all properties (level 30+ scroll) |
| **Failed remove** | KNOWN_CURSED |
| **Feeling tiers** (passive) | Per-turn low chance: {magical}, {excellent}, {special} |
| **Class feelings** | Warriors sense weapons, Mages sense magic, Priests sense curses |
| **Blind use** | Noticeable effects → auto-ID; non-noticeable → {tried} |
| **Shopkeeper pricing** | Selling reveals identity (good price = cursed, bad price = valuable) |
| **Mage spell (level 37)** | Equivalent to ?*ID* |

### Unidentified Display

Items start with randomized flavor names (e.g., "a Green Potion", "a Scroll titled WERGNA"). Flavors are randomized per game via FileParse. Using an item that doesn't produce a noticeable effect marks the flavor as {tried}.

### Pre-Identified Items

Some items are always identified on sight:
- Flask of Oil
- Potion of Apple Juice

---

## 5. Stacking Rules

### Core Principle

Two items stack when they are **the same thing with the same instance properties**. If an item type has no per-instance variation, identical items stack freely. If it has per-instance properties (charges, bonuses), those must match.

### Flavor-Learning (Wands, Staves, Rings, Amulets)

Wands, staves, rings, and amulets share the same identification behavior:
- **UnID'd**: Do not stack. Show as "a [Flavor] [Object]" in separate inventory slots.
- **ID one of a flavor** → all items of that flavor learn the type name (e.g., ID one Oak Staff → all Oak Staves become "Oak Staff of Light"). Per-instance properties (charges, bonuses) still need individual ?ID.
- **Stack when**: same flavor AND same instance properties (charges for wands/staves, bonus for rings). Items with no per-instance properties (some amulets, some rings) stack as soon as the flavor is learned.

### What Stacks

| Category | Stacks? | Condition |
|---|---|---|
| **Potions** | Always | Same color (ID or not, no instance properties) |
| **Scrolls** | Always | Same flavor (ID or not, no instance properties) |
| **Wands** | After ID | Same flavor + same charges |
| **Staves** | After ID | Same flavor + same charges |
| **Rings** | After ID | Same flavor + same bonus (some have no bonus → stack on flavor-learn) |
| **Amulets** | After ID | Same flavor + same bonus (some have no bonus → stack on flavor-learn) |
| **Books** | Always | Same title |
| **Food** | Always | Same type |
| **Arrows / Bolts** | Always | Same type |
| **Torches / Lanterns** | Conditional | Same type + identical duration remaining |
| **Oil (Fuel)** | Always | Same type |
| **Weapons / Armor** | Conditional | Identified as mundane (+0, +0) and same base type |

**If an item is not on this list, it does not stack.** Magical weapons and armor never stack.

### Example: Wands/Staves

1. Pick up Oak Staff, pick up another Oak Staff → 2 separate "Oak Staff" entries (unID'd, don't stack)
2. Read ?ID on first → "Oak Staff of Light (17 charges)" — second becomes "Oak Staff of Light" (type learned, charges unknown)
3. Read ?ID on second → "Oak Staff of Light (13 charges)" — 17 ≠ 13, don't stack
4. If both had 17 charges → they would stack

### Example: Rings (with bonus)

1. Pick up Silver Ring, pick up another Silver Ring → 2 separate "Silver Ring" entries (unID'd, don't stack)
2. Read ?ID on first → "Silver Ring of Protection (+3)" — second becomes "Silver Ring of Protection" (type learned, bonus unknown)
3. Read ?ID on second → "Silver Ring of Protection (+7)" — +3 ≠ +7, don't stack
4. If both were +5 → they would stack

### Example: Amulets (no bonus)

1. Pick up Muddy Amulet, pick up another Muddy Amulet → 2 separate "Muddy Amulet" entries (unID'd, don't stack)
2. Read ?ID on first → "Muddy Amulet of Slow Digestion" — flavor learned, second is also now identified → **"2 Muddy Amulets of Slow Digestion"** (auto-stacked)

### Cursed Stack Behavior

- Cursed items do not stack with uncursed items
- If a stacked item becomes cursed → **entire stack becomes cursed**
- One application of Scroll of Remove Curse → **uncurses the entire stack**

### Partial Stack Operations

When dropping or selling a stack, prompt: **"How many? (1-n)"** where n is the stack size. Player can drop/sell individual items from a stack. Arrows fire one at a time from a stack in the quiver.

### Ground Stacking

- All stackable items can stack on the ground (same rules as inventory)
- A single ground tile can hold a stack of identical items (e.g., 38 arrows)
- Monster loot: if multiple items drop and the tile is occupied, loot **explodes** to nearby open floor tiles
- Player can pick up an entire ground stack at once

---

## 6. Charges & Recharge

### Initial Charges

Wands and staves have charges defined as NdM per type (e.g., a particular wand type might have 3d4 initial charges). When spawned, the charges roll determines how many you find. Most have at least 1-2 charges; very rare wands can spawn with 0 charges.

### Lifetime Charge Limit

Each wand/staff **instance** has a hidden lifetime charge limit. This is the total number of charges the item can ever hold. Recharging past this limit causes explosion.

### Scroll of Recharging

- Works on both wands and staves
- Adds a **random** number of charges (not player-controlled)
- The wand/staff **accepts more charge when near empty** and **resists more when nearly full**
- Explosion risk = f(current charges, lifetime limit, dungeon depth)
  - **Higher risk** at shallow depth (town level is dangerous)
  - **Lower risk** at deep levels (3000'+)
  - Guaranteed explosion if you keep reading recharge scrolls at a full wand
- On explosion: wand/staff is **destroyed**, possible player damage

### Empty Wands/Staves

At 0 charges, the wand/staff becomes **inert** — stays in inventory, can be recharged, just does nothing when used. Not destroyed.

---

## 7. Item Destruction

When a monster hits the player with an elemental attack, an inventory scan checks for vulnerable items. Items can be destroyed based on their material and the element.

### Material Vulnerability Table

| Element | Destroys |
|---|---|
| **Fire** | Scrolls, Potions, Leather armor |
| **Cold** | Potions |
| **Acid** | Scrolls, Potions, Leather armor, Metal items |

*Requires ranged attack system (PR #235) to be in place before inventory scan can trigger.*

### Item Type to Material Mapping (TBD)

Items need a material property (or material is inferred from ITEM_IDX type):

| Material | Item Types |
|---|---|
| Paper | SCROLL, BOOK |
| Glass | POTION |
| Leather | ARMOR (some), GLOVES (some), BOOTS (some), BELT |
| Metal | SWORD, SHIELD, ARMOR (some), HELMET, RING, AMULET, etc. |
| Wood | STAFF, BOW, XBOW, ARROW, BOLT, SPEAR (shaft) |

Material is **implicit from ITEM_IDX type** (no explicit field needed). Ego and better items resist or are immune to elemental destruction. Books 3-4 resist destruction; books 1-2 are vulnerable (players learn to carry spares).

---

## 8. Ego Items

Ego items are magic items with a **named suffix** granting specific intrinsics. Ego type is restricted to certain ITEM_IDX types. Semi-rare: ~1% of magical item spawns, roughly 1-3 per 40-level run.

### Weapon Egos

| Ego Suffix | Code | Effect | Type Restriction |
|---|---|---|---|
| of Flame (Flametongue) | [FT] | 2× damage vs fire-weak | Swords, Daggers |
| of Frost (Frost Brand) | [FB] | Cold damage / cold resist | Swords, Daggers |
| of Demon Bane | [DB] | Bonus vs demons | Any weapon |
| of Slay Dragon | [SD] | Bonus vs dragons | Any weapon |
| of Slay Beast | — | 2× vs beasts | Any weapon |
| of Slay Demon | — | Bonus vs demons | Any weapon |
| of Extra Attacks | — | Additional attacks per round | Any weapon |

### Armor Egos

| Ego Suffix | Effect | Type Restriction |
|---|---|---|
| of Resist Lightning | Electricity resistance | Any armor |
| of Telepathy | ESP | Helmets only |

### Belt Egos

| Ego Suffix | Effect | Type Restriction |
|---|---|---|
| of Hill Giant Strength | STR bonus | Belts only |

### Ego Data Format (TBD)

Ego definitions need a data structure specifying:
- Ego name / suffix
- Which ITEM_IDX types it can appear on
- Intrinsics granted (as CEffect list)
- Rarity modifier
- Minimum depth

---

## 9. Legendary Items

Legendary items are pre-defined constellations of bonuses — fixed ego items with specific names and known properties. Very rare (<0.01% of magical spawns), dungeon level 20+ only.

| Item | Code | Base Type | Intrinsics | Source |
|---|---|---|---|---|
| Westernesse | [WB] | Weapon (any) | (constellation TBD) | #128 |
| Holy Avenger | [HA] | Mace | (constellation TBD) | #128 |
| Defender | [DF] | Halberd | (constellation TBD) | #128 |
| Amulet of the Magi | — | Amulet | (constellation TBD) | #128 |
| Morgul Blade | [MB] | Weapon (any) | Cursed legendary: holy weakness, life drain | #128 |

---

## 10. Unique Items

One instance per character save. Always a specific base item type. Typically carried by a specific mob.

### Persistence Rules

| Mode | Behavior |
|---|---|
| Standard | Not collected before leaving level → gone forever |
| Hardcore | Can respawn if not collected |

### Known Uniques

| Item | Base Type | Intrinsics | Carrier |
|---|---|---|---|
| Sting | Dagger | +2 speed, See Invisible, orc warning, orc glow, 2× orc damage | Floor drop |
| Menthir | Helm | (TBD) | (TBD) |
| Excalibur | Bastard Sword | (TBD) | King Arthur |
| Lady Teldra | Blade (unique type) | Spell absorption, peaceful creature refusal, personality text | (unique spawn) |

### Uniques.txt Data File (TBD)

Unique items need their own resource file. Format per entry:
- Unique name
- Base ITEM_IDX type + specific base item
- Intrinsics (CEffect list)
- Carrier mob name (or floor drop rules)
- Lore text

---

## 11. Enchant Scrolls

Enchant scrolls modify the bonuses on equipped items.

| Scroll | Effect | Amount |
|---|---|---|
| Scroll of Enchant Weapon to Hit | +to-hit on weapon | +1 |
| Scroll of Enchant Weapon Damage | +to-dam on weapon | +1 |
| Scroll of Enchant Armor | +AC on armor | +1 (or small intrinsic) |
| Scroll of *Enchant Weapon* | +to-hit AND +to-dam | +1d3 both |
| Scroll of *Enchant Armor* | +AC + random intrinsic | +1d3 AC |

### Enchantment Failure

Enchanting past **+10** has a chance to fail. Failure chance increases with current bonus level. The item is not destroyed on failure — the scroll is simply wasted.

---

## 12. New Item Catalog

All items mentioned in design discussions not yet in Items.txt, organized by type.

### Potions (ITEM_IDX_POTION)

| Item | Effect Line | Status |
|---|---|---|
| Potion of Minor Healing | HEAL + HP | In Items.txt |
| Potion of Cure Light Wounds | HEAL + HP | In Items.txt |
| Potion of Cure Serious Wounds | HEAL + HP | In Items.txt |
| Potion of Cure Critical Wounds | HEAL + HP (multiple) | In Items.txt |
| Potion of Healing | HEAL + HP (multiple) | In Items.txt |
| Potion of *Healing* | HEAL + HP (Massive) | In Items.txt |
| Potion of Resist Fire | Timed Resist Fire | In Items.txt |
| Potion of Resist Cold | Timed Resist Cold | In Items.txt |
| Potion of Resist Acid | Timed Resist Acid | In Items.txt |
| Potion of Resist Electricity | Timed Resist Electricity | In Items.txt |
| Potion of Speed | Timed Speed | In Items.txt |
| Potion of Infravision | Timed Infravision | In Items.txt |
| Potion of Invisibility | Timed Invisibility | In Items.txt |
| Potion of Levitation | Timed Levitation | In Items.txt |
| Potion of Courage | Cure Fear + Resist Fear | In Items.txt |
| Potion of Cure Poison | Cure Poison | In Items.txt |
| Potion of Slow Poison | Resist Poison | In Items.txt |
| Potion of Neutralize Poison | Cure Poison + Destroy Poison | In Items.txt |
| Potion of Blindness | Cause Blindness | In Items.txt |
| Potion of Confusion | Inflict Confusion | In Items.txt |
| Potion of Poison | Inflict Poison | In Items.txt |
| Potion of Sleep | Inflict Sleep | In Items.txt |
| Potion of Flames | Inflict Fire | In Items.txt |
| Potion of Apple Juice | (flavor, always pre-identified) | In Items.txt |
| Potion of Gain Strength | GAIN + STAT | Blocked: Stats #197 |
| Potion of Restore Strength | RESTORE + STAT | Blocked: Stats #197 |
| Potion of Weakness | LOSE + STAT | Blocked: Stats #197 |
| Potion of Heroism | TIMED + STAT, TIMED + HP | Blocked: Stats #197 |
| Potion of See Invisible | Timed See Invisible | Blocked: MON_FLAG_INVISIBLE |
| Potion of Gain CON/DEX/INT/WIS/CHA | GAIN + STAT (per stat) | Blocked: Stats #197 |
| Potion of Restore CON/DEX/INT/WIS/CHA | RESTORE + STAT (per stat) | Blocked: Stats #197 |

### Scrolls (ITEM_IDX_SCROLL)

| Item | Effect Line | Status |
|---|---|---|
| Scroll of Light | Light Area | In Items.txt |
| Scroll of Telepathy | Timed ESP | In Items.txt |
| Scroll of Remove Curse | Remove Curse | In Items.txt |
| Scroll of Identify | Identify | In Items.txt |
| Scroll of Phase Door | Phase Door | In Items.txt |
| Scroll of Teleportation | Teleport Self | In Items.txt |
| Scroll of Magic Mapping | Partial Mapping | In Items.txt |
| Scroll of *Magic Mapping* | Full Mapping | In Items.txt |
| Scroll of Word of Recall | Recall | In Items.txt |
| Scroll of Door/Stair Location | Detect Doors | In Items.txt |
| Scroll of Detect Traps | Detect Traps | In Items.txt |
| Scroll of Detect Monsters | Detect Monsters | In Items.txt |
| Scroll of Treasure Detection | Detect Treasure | In Items.txt |
| Scroll of Summon Monsters | Summon Monsters | In Items.txt |
| Scroll of Scare Monster | Cause Fear | In Items.txt |
| Scroll of Mass Sleep | Mass Sleep | In Items.txt |
| Scroll of Blessing | Timed Blessing | In Items.txt |
| Scroll of Curse Object | Curse Object | In Items.txt |
| Scroll of Recharging | Recharge | In Items.txt |
| Scroll of *Recharging* | Star-Recharge | In Items.txt |
| Scroll of Enchant Weapon to Hit | Enchant to Hit | In Items.txt |
| Scroll of Enchant Weapon Damage | Enchant to Damage | In Items.txt |
| Scroll of Enchant Armor | Enchant Armor | In Items.txt |
| Scroll of *Enchant Weapon* | Star-Enchant Weapon | In Items.txt |
| Scroll of *Enchant Armor* | Star-Enchant Armor | In Items.txt |
| Scroll of Trap Creation | CREATE + TRAP | Blocked: Trap system |
| Scroll of *Identify* | Full lore reveal | Blocked: Ego/unique system |
| Scroll of Restoration | (stat restore) | Blocked: Stats #197 |
| Scroll of Darkness | (Vampire PC town access) | Blocked: Day/night |

### Wands (ITEM_IDX_WAND)

| Item | Effect Line | Status |
|---|---|---|
| Wand of Light | Light Ray + Light Area | In Items.txt |
| Wand of Firebolts | Firebolt | In Items.txt |
| Wand of Fireballs | Fireball | In Items.txt |
| Wand of Frost | Frost Bolt | In Items.txt |
| Wand of Frost Balls | Frost Ball | In Items.txt |
| Wand of Lightning | Lightning Bolt | In Items.txt |
| Wand of Lightning Balls | Lightning Ball | In Items.txt |
| Wand of Acid | Acid Bolt | In Items.txt |
| Wand of Acid Balls | Acid Ball | In Items.txt |
| Wand of Sleep | Cause Sleep | In Items.txt |
| Wand of Confusion | Cause Confusion | In Items.txt |
| Wand of Paralyze | Cause Paralysis | In Items.txt |
| Wand of Stone to Mud | Stone to Mud | In Items.txt |
| Wand of Summoning | Summon Monsters | In Items.txt |
| Wand of Teleport Away | Teleport Away | In Items.txt |
| Wand of Heal Monster | Heal Monster | In Items.txt |
| Wand of Probing | Probe | In Items.txt |
| Wand of Fear | Cause Fear | In Items.txt |

### Staves (ITEM_IDX_STAFF)

| Item | Effect Line | Status |
|---|---|---|
| Staff of Light | Light Area | In Items.txt |
| Staff of Starlight | Light Ray + Light Area | In Items.txt |
| Staff of Healing | Minor Healing + Cure Poison | In Items.txt |
| Staff of Cure Light Wounds | CLW + cures | In Items.txt |
| Staff of Cure Serious Wounds | CSW + cures | In Items.txt |
| Staff of Greater Healing | Greater Healing + cures | In Items.txt |
| Staff of Fire Resistance | Timed Resist Fire | In Items.txt |
| Staff of Cold Resistance | Timed Resist Cold | In Items.txt |
| Staff of Electricity Resistance | Timed Resist Electricity | In Items.txt |
| Staff of Acid Resistance | Timed Resist Acid | In Items.txt |
| Staff of Resistance | All 4 timed resistances | In Items.txt |
| Staff of *Resistance* | All 4 timed immunities | In Items.txt |
| Staff of Perception | Identify | In Items.txt |
| Staff of Teleportation | Teleport Self | In Items.txt |
| Staff of Word of Recall | Recall | In Items.txt |
| Staff of Mapping | Partial Mapping | In Items.txt |
| Staff of Telepathy | Timed ESP | In Items.txt |
| Staff of Treasure Detection | Detect Treasure | In Items.txt |
| Staff of Summoning | Summon Monsters | In Items.txt |
| Staff of Sleep | Cause Sleep | In Items.txt |
| Staff of Mass Sleep | Mass Sleep | In Items.txt |
| Staff of Fear | Mass Fear | In Items.txt |
| Staff of Paralysis | Mass Paralyze | In Items.txt |
| Staff of Protection | Timed Blessing | In Items.txt |

### Rings (ITEM_IDX_RING)

| Item | Effect Line | Status |
|---|---|---|
| Ring of Invisibility | Invisibility | In Items.txt |
| Ring of Levitation | Levitation | In Items.txt |
| Ring of Telepathy | ESP | In Items.txt |
| Ring of Fire Resistance | Resist Fire | In Items.txt |
| Ring of Cold Resistance | Resist Cold | In Items.txt |
| Ring of Acid Resistance | Resist Acid | In Items.txt |
| Ring of Electricity Resistance | Resist Electricity | In Items.txt |
| Ring of Protection | (AC bonus) | In Items.txt |
| Ring of Free Action | Free Action | In Items.txt |
| Ring of Sensing | Treasure Sense | In Items.txt |
| Ring of Speed | Speed | In Items.txt |
| Ring of Searching | Searching | In Items.txt |
| Ring of Greed | Treasure Sense | In Items.txt |
| Ring of Tunneling | Tunneling | In Items.txt |
| Ring of Fate | (unknown great item) | Blocked: Design TBD |
| Sustain [Stat] Ring | INTRINSIC + STAT + sustain | Blocked: Stats #197 |

### Equipment

| Item | Type | Effect | Status |
|---|---|---|---|
| Boots of Speed | BOOTS | Speed | In Items.txt |
| Cloak of Protection | CLOAK | AC bonus (+10) | In Items.txt |
| Helmet of Lordly Protection | HELMET | Immune Fire | In Items.txt |
| Helm of Infravision | HELMET | Infravision | In Items.txt |
| Helm of Telepathy | HELMET | ESP | In Items.txt |
| Gloves of Dexterity | GLOVES | DEX bonus, search bonus | Blocked: Stats #197 |
| Holy Symbol | (class equip) | Priest item | Blocked: Classes #239 |
| Nature Focus | (class equip) | Druid item | Blocked: Classes #239 |
| Ki Focus | (class equip) | Monk item | Blocked: Classes #239 |
| Sustain [Stat] Armor | ARMOR | INTRINSIC + STAT + sustain | Blocked: Stats #197 |

### Empty Types — Content TBD

| Type | Design Notes |
|---|---|
| ITEM_IDX_FOOD | Eat (E) command. Rations, Elvish Waybread, etc. |
| ITEM_IDX_BOOK | 4 books per caster class. Books 1-2 in town, 3-4 dungeon-only. Deferred until enough spell effects exist as scrolls/wands/staves. |
| ITEM_IDX_ARROW | Ammunition for BOW. Stackable. |
| ITEM_IDX_BOLT | Ammunition for XBOW. Stackable. |
| ITEM_IDX_MONEY | Gold pieces, gems. |
| ITEM_IDX_CHEST | Container (ITEM_FLAG_HOLDING). |
| ITEM_IDX_BELT | Waist slot. Hill Giant Strength ego. |
| ITEM_IDX_AMULET | Neck slot. Amulet of the Magi (legendary), Amulet of Sensing. |

---

## 13. Encumbrance

Weight-based encumbrance system. Triggered when STR stat system (#197) is implemented. STR determines carry capacity; exceeding it reduces speed / prevents actions.

---

## 14. Economy & Shops

Brief reference — detailed design in town system (#243).

| Shop | Item Distribution | Special |
|---|---|---|
| General Store | 90% mundane, 10% common magic | — |
| Weaponsmith / Armorer | 95% mundane, 3% magic, 2% rare magic | — |
| Temple | Healing, restoration, Remove Curse scrolls | Blessed items |
| Magic Shop | 60% common magic, 30% uncommon, 10% rare | Ring of Fate |
| Bazaar | 2× General Store variety | — |
| Black Market | Ego+ items, dungeon level 20+ | Premium prices |
| Player House | Player-deposited items persist | Storage, no buying/selling |

### Items Never In Shops
- Spell books 3-4
- Unique items
- Some quest rewards

### Always Pre-Identified
- Flask of Oil
- Potion of Apple Juice

### Shopkeeper Pricing
Base price × markup × shopkeeper modifier × race modifier. CHA stat affects prices.

### Thief Item Theft
Thieves can steal lightweight items only: potions, scrolls. NOT books, staves, or equipment.

---

## 15. Dependency Map

| System | Blocks These Items/Features | Issue |
|---|---|---|
| **Stats (#197)** | Gain/Restore/Weakness potions, Heroism, Sustain rings, Gloves of Dexterity, class feelings, CHA shop prices, Encumbrance | #197 |
| **Classes (#239)** | Spell books, class feelings, class equipment (Holy Symbol, etc.), Mage ID spell | #239 |
| **Town (#243)** | Shop system, shopkeeper ID, Player House, Word of Recall (full loop) | #243 |
| **Trap system** | Scroll of Trap Detection/Creation | #117 |
| **Ranged (PR #235)** | Item destruction from elemental attacks | #271 |

Effect system dependencies (EFFECT_TYPE_SEE, CEffect deep copy, elemental DoHitEffects, IMMUNE/RESIST combat math) are in [Effects-Design.md](Effects-Design.md) §12.

---

## 16. Code Logic Tracking

Enhancements needed in C++ code, tracked separately from item data. Effect handler tracking is in [Effects-Design.md](Effects-Design.md) §10.

| Enhancement | Description |
|---|---|
| Sustain stat mechanics | Restore stat to max achieved + prevent stat damage. "The feeling passes." |
| Recharge risk curve | Explosion risk = f(current charges, lifetime limit, dungeon depth). Lower risk at depth. |
| Item destruction scan | Inventory scan on elemental monster hit. Material vulnerability table. |
| Enchantment system | ?Enchant scrolls modify bonuses. Failure above +10. |
| Blessed three-state | Cursed / uncursed / blessed with per-state bonuses. |
| Stacking identity check | Potions/scrolls always stack by appearance. Wands/staves/rings/amulets: flavor-learn on first ID, stack when same flavor + same instance properties. Books/food/ammo always stack. Torches by duration. Mundane-ID'd weapons/armor. Magical weapons/armor never stack. |
| Partial stack split | "How many? (1-n)" on drop/sell. Fire ammo one at a time. |
| Ground stacking | Same stacking rules on floor tiles. Loot explosion to nearby tiles. |
| Feeling tiers | Passive per-turn chance: {magical}, {excellent}, {special}. |
| Blind-use identification | Noticeable effect → auto-ID; non-noticeable → {tried}. |
| Item spawn quality chain | Ego → Legendary → Unique spawn tiers (Ego ≈1%, Legendary <0.01% depth 20+, Unique mob-carried). |

---

## 17. Constants.h Tracking

New defines and string table entries needed. Effect constants (EFFECT_TYPE, EFFECT_FLAG, EFFECT_FLAG2, EFFECT_MOD) are in [Effects-Design.md](Effects-Design.md) §11.

| Define | Type | Purpose |
|---|---|---|
| MON_FLAG_INVISIBLE (0x100000) | MON_FLAG | Invisible monster property |
| ~~ITEM_FLAG_EQUIPMENT~~ | — | Not needed; EquipTypes[] already maps ITEM_IDX→slot |
| ITEM_FLAG_BLESSED (0x4000) | ITEM_FLAG | Three-state blessed system |

### String Table

Every new EFFECT_TYPE, EFFECT_FLAG, EFFECT_MOD, ITEM_FLAG, or MON_FLAG must be added to the `Init()` string table in Constants.cpp and the corresponding `NUM_*` count must be incremented.

### Data Structures TBD

| Structure | Purpose |
|---|---|
| Ego definition format | Ego types, type restrictions, intrinsic lists |
| Uniques.txt data file | Per-unique: base type, intrinsics, carrier, lore |
| Material property | Per-item material for destruction vulnerability (or inferred from ITEM_IDX) |

---

## 18. Class/Item Synergies (#239)

Reference only — this drives book content and class equipment. See Classes system (#239) for full implementation.

| Class | Armor | Weapons | Special |
|---|---|---|---|
| Warrior | Heavy | Melee + Shield | — |
| Rogue | Light | Dual-wield | Stealth items |
| Ranger | Light | Ranged | — |
| Mage | Light | Wands/Staves | Spell Books, ID spell (lv37) |
| Priest | Medium | Maces | Spell Books, Holy Symbol |
| Druid | Medium | — | Spell Books, Nature Focus |
| Shaman | Medium | — | Spell Books, Elemental items |
| Monk | None | Unarmed | Ki Focus |

---

## 19. Intrinsics Grid (#244)

Equipment × intrinsic matrix display showing per-item resistance status:

| Variant | Source | Duration |
|---|---|---|
| Permanent buff | Equipment (Ring of Resist Fire) | While equipped |
| Temporary buff | Potion / spell | Timed (fDuration) |
| Permanent debuff | Cursed equipment | While equipped |
| Temporary debuff | Monster/trap attack | Timed |

Resistance/immunity combat math and stacking rules are in [Effects-Design.md](Effects-Design.md) §8.
