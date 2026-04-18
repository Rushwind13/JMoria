# Effect Design — JMoria

Authoritative design document for the JMoria effect system. Effects are reusable LEGO pieces — a single named effect can be referenced by items, monsters, and player spells.

---

## 1. Effect Data Model

### CEffectDef (template — parsed from Effects.txt)

The shared definition. One entry per named effect. Stored on CDungeon, looked up by name.

| Field | Type | Description |
|---|---|---|
| m_szName | string | Human-readable name ("Firebolt", "Resist Acid") |
| m_dwEffect | int | EFFECT_TYPE_* verb |
| m_dwFlags | uint32 | EFFECT_FLAG_* noun (word 1) |
| m_dwFlags2 | uint32 | EFFECT_FLAG2_* noun (word 2) |
| m_dwModifier | int | EFFECT_MOD_* adverb |
| m_szAmount | NdM string | Damage/healing dice rolled per use |
| m_fDuration | float | Duration in game turns (timed effects) |
| m_fRange | float | Max range in tiles (0 = self only) |
| m_fRadius | float | AoE radius (0 = single target) |

### CEffect (instance — lives on an item or monster)

An effect instance on a specific CItemDef or CMonsterDef. Points back to its shared definition via `m_ed`.

| Field | Type | Description |
|---|---|---|
| m_ed | CEffectDef* | Pointer to shared definition (NULL for legacy inline effects) |
| m_dwEffect | int | EFFECT_TYPE_* verb (copied from def or inline) |
| m_dwFlags | uint32 | EFFECT_FLAG_* noun word 1 |
| m_dwFlags2 | uint32 | EFFECT_FLAG2_* noun word 2 |
| m_dwModifier | int | EFFECT_MOD_* adverb |
| m_szAmount | NdM string | Dice string (copied from def or inline) |
| m_fDuration | float | Duration |

A CEffect that was resolved from a named reference has `m_ed` pointing to the catalog entry. Inline effects parsed from comma-separated `<>,<>` syntax have `m_ed = NULL`.

### Relationship Diagram

```
Effects.txt → CEffectDef (shared catalog on CDungeon)
                  ↑
                  m_ed
                  |
Items.txt  → CItemDef → CEffect (instance, one per Effect line)
Monsters.txt → CMonsterDef → CAttack (monster version — to be unified)
```

The goal: in the fullness of time, most effects will be defined in Effects.txt and referenced by name. Inline `<>,<>` syntax remains for backward compatibility and one-off effects.

---

## 2. Effects.txt Format

Each named effect is a block in [Resources/Effects.txt](../Resources/Effects.txt):

```
Effect <effect-name>
{
    Type        <EFFECT_TYPE_*>
    Flag        <EFFECT_FLAG_*>
    Flag2       <EFFECT_FLAG2_*>
    Modifier    <EFFECT_MOD_*>
    Amount      <NdM>
    Duration    float
    Range       float
    Radius      float
}
```

Only `Type` and `Flag` are required. All other fields are optional. Parsed by `CDataFile::ReadEffect()` using the same FileParse bracket syntax as Items.txt and Monsters.txt.

### Load Order

Effects.txt loads first (before Monsters.txt and Items.txt) because both items and monsters may reference effects by name. The dependency order in CDungeon::Init() is:

1. **Effects.txt** → `m_llEffectDefs`
2. **Monsters.txt** → `m_llMonsterDefs` (can reference effects)
3. **Items.txt** → `m_llItemDefs` (can reference effects)

### Referencing from Items.txt

An item references a named effect with a single-token Effect line:

```
Effect <Firebolt>
Effect <Light Area>
```

A single token (no commas after the first `<...>`) triggers a catalog lookup by name. The CEffect instance gets `m_ed` pointing to the shared CEffectDef, and copies all fields from it.

Inline (legacy) format still works:

```
Effect <EFFECT_TYPE_HIT>,<EFFECT_FLAG_FIRE>,<EFFECT_MOD_LINE>,<2d8>
```

---

## 3. Effect Grammar

An effect is a sentence: **verb** + **noun** + **adverb**.

- **Verb** (EFFECT_TYPE): What the effect does
- **Noun** (EFFECT_FLAG / EFFECT_FLAG2): What it affects
- **Adverb** (EFFECT_MOD): How it does it

### Reading an Effect

| Effect Name | Verb | Noun | Adverb | In English |
|---|---|---|---|---|
| Minor Healing | HEAL | HP | — | Restore HP |
| Firebolt | HIT | FIRE | LINE | Deal fire damage along a line |
| Fireball | HIT | FIRE | BALL | Deal fire damage in a radius |
| Light Ray | HIT | LIGHT | LINE | Light-damage along a line |
| Light Area | CREATE | LIGHT | AREA | Illuminate the surrounding area |
| Teleport Away | HIT | TELEPORT | — | Teleport target away |
| Resist Acid | INTRINSIC | ACID | RESIST | Grant permanent acid resistance |
| Sustain CHA | INTRINSIC | STAT | SUSTAIN | Prevent CHA loss |
| Enchant Weapon | GAIN | TOHIT | ENCHANT | Permanently increase weapon bonus |
| Detect Doors | SEE | DOOR | — | Reveal nearby doors |

### The Verb Question: What Makes Something a HIT?

All of these reduce HP on a target:
- Swing sword, stab spear, bite, claw, crawl-on, fire bolt, shoot arrow

Are these all the same verb? **Nearly.** The verb is the *game mechanical action* — what the engine does — not the flavor text. A sword swing and a fire bolt both resolve as "reduce HP on target," but they differ in:

| Property | Melee | Ranged/Spell | Monster Attack |
|---|---|---|---|
| Verb | HIT | HIT | HIT |
| Noun | (physical — no EFFECT_FLAG) | FIRE, COLD, etc. | FIRE, POISON, etc. |
| Delivery | Adjacent tile | LINE / BALL / self | Adjacent or ranged |
| Source | Player weapon | Item / spell | Monster CAttack |

The verb HIT means "apply damage/effect to target." The noun determines *what kind* of damage. The adverb determines *how it reaches* the target. Physical melee is HIT with no elemental flag — the damage comes from the weapon's Damage dice, not the effect's Amount.

**Open question:** Should physical melee attacks be modeled as effects at all, or do they stay as the weapon's base Damage field? Currently, weapon damage uses CItemDef::m_szBaseDamage + to-hit/to-dam bonuses (separate from the effect system). Effects are for *additional* properties: elemental damage, status infliction, etc. A Flametongue sword has base weapon damage AND an effect that adds fire damage.

**Open question:** Monster attacks (CAttack) use a parallel structure to CEffect but are not yet unified. Should CAttack become CEffect with `m_ed` pointing to an Effects.txt entry? This would let a monster's "bite + poison" reference the same `<Poison>` effect that a poisoned dagger uses.

---

## 4. Verbs (EFFECT_TYPE)

| Define | Hex | Meaning | Handler | Example |
|---|---|---|---|---|
| HEAL | 0x01 | Restore HP/MP/stat, cure condition | DoHealEffects | Potion of Healing |
| HIT | 0x02 | Deal damage or directed effect at target | DoHitEffects | Wand of Firebolts |
| CREATE | 0x04 | Bring something into the world | DoCreateEffects | Staff of Light |
| DESTROY | 0x08 | Remove something from the world | DoDestroyEffects | Scroll of Destroy Traps |
| INTRINSIC | 0x10 | Grant permanent property | DoIntrinsicEffects | Ring of Resist Fire |
| RESTORE | 0x20 | Restore a depleted resource | DoRestoreEffects | Potion of Restore Strength |
| GAIN | 0x40 | Permanently increase a value | DoGainEffects | Potion of Gain Strength |
| LOSE | 0x80 | Permanently decrease a value | DoLoseEffects | Potion of Weakness |
| SEE | 0x100 | Reveal/detect without changing the world | (not yet coded) | Scroll of Detect Monsters |

### Verb Semantics

- **HEAL vs RESTORE**: HEAL cures a condition (poison, blindness, fear). RESTORE returns a stat/resource to its maximum.
- **HIT vs CREATE**: HIT is directed at a target. CREATE affects the user or the world. A wand *hits* a target with teleport; a staff *creates* teleportation on the user.
- **SEE vs CREATE**: "SEE changes what you know, CREATE changes the world." Detect Doors reveals doors on the map (SEE). Light Area actually lights up tiles (CREATE).
- **GAIN vs RESTORE**: GAIN permanently increases the maximum. RESTORE returns to the current maximum.
- **INTRINSIC**: Grants a permanent (while-equipped) or timed property. Duration > 0 means timed; duration = 0 means permanent.

---

## 5. Nouns — Word 1 (EFFECT_FLAG, 32 flags)

| Group | Flags | Notes |
|---|---|---|
| Elements | FIRE, COLD, ELECTRICITY, ACID | Damage types. Also used with RESIST/IMMUNE/WEAK |
| Conditions | POISON, LIGHT, PARALYZE, TREASURE | Status effects and light |
| Mental | AFRAID, BLIND, SLEEP, CONFUSE | Mental conditions |
| Utility | STONE_TO_MUD, FUEL, INFRA, ESP | World interaction |
| Info | IDENTIFY, RECALL, MAPPING, SUMMON | Knowledge and travel |
| Stats | STAT, TOHIT, TODAM, AC | Character attributes |
| Resources | XP, HP, MP, TELEPORT | Resource pools and travel |
| Intrinsics | FREE_ACTION, INVISIBLE, LEVITATE, SPEED | Permanent player properties |

---

## 6. Nouns — Word 2 (EFFECT_FLAG2, 4/32 bits used)

Overflow flags for nouns that don't fit in word 1. Parsed transparently — same syntax.

| Flag | Category | Example |
|---|---|---|
| DOOR | Detection | Detect doors/stairs |
| TRAP | Detection | Detect/create traps |
| MONSTERS | Detection | Detect monsters |
| NO_COLLIDE | Projectile | Line effects pass through targets |

NO_COLLIDE is a property of the *effect*, not the item. A light ray passes through everything; a fireball stops at first target. The item that fires the effect doesn't need to know — it's the effect's delivery mechanism that determines pass-through.

---

## 7. Adverbs (EFFECT_MOD, 9 modifiers)

| Define | Meaning | Example |
|---|---|---|
| RESIST | 50% damage reduction | Ring of Resist Fire |
| SEE | Reveal / perceive | Potion of See Invisible |
| IMMUNE | 0 damage (full immunity) | Helmet of Lordly Protection |
| WEAK | Increased damage taken (×2) | Potion of Flames (cold weakness side effect) |
| TIMED | Temporary effect with duration | Potion of Resist Fire |
| AREA | Affects surrounding area | Staff of Light |
| LINE | Affects a line from source | Wand of Light |
| BALL | Affects a radius from impact point | Fireball |
| ENCHANT | Modifies item bonuses | Scroll of Enchant Weapon |

### Modifier Combinations

Some effects combine multiple adverbs (e.g., TIMED + SEE for temporary See Invisible). The modifier field is a single int, so combinations that need multiple modifiers require multiple Effect lines or future expansion.

---

## 8. Effect Combat Math

### Resistance Model

| Modifier | Damage Multiplier | How Granted |
|---|---|---|
| Normal | ×1 (full damage) | Default |
| RESIST | ×0.5 (half damage) | Equipment intrinsic, timed potion |
| IMMUNE | ×0 (no damage) | Rare equipment, race innate |
| WEAK | ×2 (double damage) | Cursed equipment, monster debuff |

### Stacking (TBD)

**Open question:** If a player has Resist Fire from a ring and Resist Fire from a potion, do they stack? Options:
- 50% + 25% = 62.5% reduction (diminishing returns)
- 50% + 50% = 75% (multiplicative)
- Cap at one source (no stacking)

Deferred to intrinsics deep-dive.

### Intrinsics Grid Interaction

Four variants per element exist in gameplay:

| Variant | Source | Duration |
|---|---|---|
| Permanent buff | Equipment (Ring of Resist Fire) | While equipped |
| Temporary buff | Potion / spell | Timed (m_fDuration turns) |
| Permanent debuff | Cursed equipment | While equipped |
| Temporary debuff | Monster/trap attack | Timed |

The intrinsics grid on the player's character sheet shows which resistances are active and from which source. Display design is in [Item-Design.md](Item-Design.md) §19.

---

## 9. Named Effect Catalog

### Currently Defined (in Effects.txt)

| Name | Verb | Noun | Adverb | Amount | Range | Radius | Used By |
|---|---|---|---|---|---|---|---|
| Light Ray | HIT | LIGHT | LINE | 1d5 | 15 | — | Wand of Light |
| Light Area | CREATE | LIGHT | AREA | — | — | 15 | Wand of Light, Scroll of Light |
| Firebolt | HIT | FIRE | LINE | 2d8 | 15 | — | Wand of Firebolts |
| Fireball | HIT | FIRE | BALL | 6d8 | 20 | 3 | Wand of Fireballs |
| Frost Bolt | HIT | COLD | LINE | 2d8 | 15 | — | Wand of Frost Bolts |
| Lightning Bolt | HIT | ELECTRICITY | LINE | 3d6 | 20 | — | Wand of Lightning |
| Teleport Away | HIT | TELEPORT | — | — | 15 | — | Wand of Teleport Away |
| Minor Healing | HEAL | HP | — | 4d4 | — | — | Potion of Minor Healing |

### Planned Effects (not yet in Effects.txt)

These are the reusable LEGO pieces needed to build planned items, monsters, and spells. Each entry here will become an Effects.txt block.

#### Damage Effects

| Name | Verb | Noun | Adverb | Amount | Notes |
|---|---|---|---|---|---|
| Acid Bolt | HIT | ACID | LINE | 2d8 | Wand of Acid Bolts |
| Poison Sting | HIT | POISON | — | 1d4 | Spider/snake/scorpion attacks |
| Stone to Mud | HIT | STONE_TO_MUD | — | — | Wand of Stone to Mud |

#### Healing / Restoration Effects

| Name | Verb | Noun | Adverb | Amount | Notes |
|---|---|---|---|---|---|
| Major Healing | HEAL | HP | — | 8d8 | Potion of Major Healing |
| Cure Poison | HEAL | POISON | — | — | Potion of Cure Poison |
| Cure Blindness | HEAL | BLIND | — | — | Potion of Cure Blindness |
| Cure Fear | HEAL | AFRAID | — | — | Potion of Heroism (component) |
| Restore Strength | RESTORE | STAT | — | — | Potion of Restore Strength (needs stat ID) |

#### Intrinsic Effects (permanent while source active)

| Name | Verb | Noun | Adverb | Notes |
|---|---|---|---|---|
| Resist Fire | INTRINSIC | FIRE | RESIST | Ring of Resist Fire, Armor of Resist Fire |
| Resist Cold | INTRINSIC | COLD | RESIST | Ring of Resist Cold |
| Resist Acid | INTRINSIC | ACID | RESIST | Helm of Lordly Protection (component) |
| Resist Electricity | INTRINSIC | ELECTRICITY | RESIST | Armor ego |
| Immune Fire | INTRINSIC | FIRE | IMMUNE | Very rare equipment |
| Free Action | INTRINSIC | FREE_ACTION | — | Ring of Free Action |
| See Invisible | INTRINSIC | INVISIBLE | SEE | Ring of See Invisible, Sting |
| Levitation | INTRINSIC | LEVITATE | — | Ring of Levitation, Boots of Levitation |
| Speed | INTRINSIC | SPEED | — | Boots of Speed, Sting (+2 speed) |
| Sustain CHA | INTRINSIC | STAT | SUSTAIN | Ring/armor; needs SUSTAIN modifier (TBD) |
| Infravision | INTRINSIC | INFRA | — | Racial intrinsic, Ring of Infravision |
| ESP | INTRINSIC | ESP | — | Helmet of Telepathy |

#### Detection Effects

| Name | Verb | Noun | Adverb | Notes |
|---|---|---|---|---|
| Detect Doors | SEE | DOOR | — | Scroll of Door/Stair Location |
| Detect Traps | SEE | TRAP | — | Scroll of Trap Detection |
| Detect Monsters | SEE | MONSTERS | — | Scroll of Detect Monsters |
| Detect Treasure | SEE | TREASURE | — | (future) |
| Magic Mapping | SEE | MAPPING | — | Scroll of Magic Mapping (reveals dungeon layout) |

#### Creation Effects

| Name | Verb | Noun | Adverb | Notes |
|---|---|---|---|---|
| Create Trap | CREATE | TRAP | — | Scroll of Trap Creation |
| Create Light (self) | CREATE | LIGHT | — | Different from Light Area (self-only radius) |
| Recall | CREATE | RECALL | — | Staff of Word of Recall |
| Teleport Self | CREATE | TELEPORT | — | Staff of Teleportation, Scroll of Teleportation |
| Summon Monsters | CREATE | SUMMON | — | Monster ability, cursed scroll |

#### Timed Effects

| Name | Verb | Noun | Adverb | Duration | Notes |
|---|---|---|---|---|---|
| Timed Resist Fire | INTRINSIC | FIRE | RESIST + TIMED | varies | Potion of Resist Fire |
| Timed See Invisible | HIT | INVISIBLE | TIMED + SEE | varies | Potion of See Invisible |
| Timed Heroism | INTRINSIC | HP + STAT | TIMED | varies | Potion of Heroism |
| Timed Blessing | HIT | AC | TIMED | varies | Scroll of Blessing |
| Timed Speed | INTRINSIC | SPEED | TIMED | varies | Potion of Speed |

#### Enchantment / Modification Effects

| Name | Verb | Noun | Adverb | Amount | Notes |
|---|---|---|---|---|---|
| Enchant to Hit | GAIN | TOHIT | ENCHANT | +1 | Scroll of Enchant Weapon to Hit |
| Enchant to Damage | GAIN | TODAM | ENCHANT | +1 | Scroll of Enchant Weapon Damage |
| Enchant Armor | GAIN | AC | ENCHANT | +1 | Scroll of Enchant Armor |
| Star Enchant Weapon | GAIN | TOHIT + TODAM | ENCHANT | +1d3 | Scroll of *Enchant Weapon* |
| Star Enchant Armor | GAIN | AC | ENCHANT | +1d3 | Scroll of *Enchant Armor* (+ random intrinsic) |

### Example: Composing an Item from Effects

A Helm of Lordly Protection from Acid would reference:

```
Item <Helm of Lordly Protection from Acid>
{
    Plural      <Helms of Lordly Protection from Acid>
    Type        <ITEM_IDX_HELMET>
    Effect      <Resist Acid>
    Effect      <Sustain CHA>
    Effect      <Protection>
    AC          5.0
    ACBonus     <1d10>
    ...
}
```

Each of those effects is independently defined in Effects.txt. `<Resist Acid>` is the same one that appears on a Ring of Resist Acid. `<Sustain CHA>` is the same one on a Ring of Sustain Charisma. The item is a *composition* of reusable effect LEGO pieces.

---

## 10. Effect Handler Status

Code handlers in CPlayer that dispatch effect verbs. Effect-specific tracking — item system tracking is in [Item-Design.md](Item-Design.md).

| Handler | Verb | Status | Notes |
|---|---|---|---|
| DoHealEffects | HEAL | Working | HP healing, poison/blind/fear/confuse/sleep cure |
| DoHitEffects | HIT | Partial | LIGHT works. Elemental damage (FIRE, COLD, etc.) not yet dispatched to player-side damage calc |
| DoCreateEffects | CREATE | Working | LIGHT area, RECALL, TELEPORT |
| DoDestroyEffects | DESTROY | Partial | Uses ITEM_FLAG_CURSED instead of EFFECT_FLAG — design smell |
| DoIntrinsicEffects | INTRINSIC | Working | Sets intrinsic flag on player. Duration=0 → permanent. |
| DoRestoreEffects | RESTORE | Working | Restore HP/stat to max |
| DoGainEffects | GAIN | Working | Permanent increase |
| DoLoseEffects | LOSE | Working | Permanent decrease |
| (EFFECT_TYPE_SEE) | SEE | Not started | "SEE changes what you know." Detection/reveal for doors, traps, monsters |

### Known Handler Issues

- **Elemental DoHitEffects**: Fire, Cold, Electricity, Acid damage from items is not yet dispatched on the player side. Effects.txt defines the amounts (e.g., Firebolt 2d8) but DoHitEffects doesn't roll `m_szAmount` for elemental nouns — only LIGHT is handled.
- **CEffect deep copy**: When a timed effect is applied, the CEffect is shallow-copied. `m_szAmount` (a `char*`) is not deep-copied, so the rolled value may be lost. Blocks Scroll of Blessing (timed AC).
- **DESTROY handler**: Uses `ITEM_FLAG_CURSED` check instead of `EFFECT_FLAG`. Intent unclear — should the effect noun specify what to destroy?
- **IMMUNE vs RESIST**: The combat math (§8 above) is designed but not implemented. Damage calculation doesn't check `EFFECT_MOD_IMMUNE` vs `EFFECT_MOD_RESIST` yet.

---

## 11. Constants.h — Effect Defines

### EFFECT_TYPE (9 verbs)

All defined, string table entries added, NUM_EFFECT_TYPES=9.

### EFFECT_FLAG (32/32 bits used)

All 32 bits allocated. See §5 for the full list.

### EFFECT_FLAG2 (4/32 bits used)

DOOR, TRAP, MONSTERS, NO_COLLIDE. 28 bits free.

### EFFECT_MOD (9 modifiers)

RESIST, SEE, IMMUNE, WEAK, TIMED, AREA, LINE, BALL, ENCHANT. Plus STAR (reserved for star-enchantment delivery).

### Pending Defines

| Define | Type | Purpose | Blocks |
|---|---|---|---|
| EFFECT_MOD_SUSTAIN | EFFECT_MOD | Prevent stat loss | Sustain rings/armor |

---

## 12. Dependencies

Effect system dependencies — what blocks effect work.

| System | Blocks These Effects | Issue |
|---|---|---|
| **Stats (#197)** | GAIN/RESTORE/LOSE STAT effects, Sustain, Heroism | #197 |
| **Elemental DoHitEffects** | All elemental damage effects (Firebolt, Fireball, etc.) | Phase 3 |
| **CEffect deep copy** | All timed effects (Blessing, timed Resist, timed Speed) | Phase 3 |
| **EFFECT_TYPE_SEE handler** | Detection effects (doors, traps, monsters) | Phase 3 |
| **MON_FLAG_INVISIBLE** | See Invisible effect | #72 |
| **Trap system** | Detect Traps, Create Traps | #117 |
| **CAttack → CEffect unification** | Monster attacks referencing shared effects | Future |
