# JMoria Designer Reference

Quick reference for adding items and monsters to JMoria.
All definitions live in `Resources/Items.txt` and `Resources/Monsters.txt`.

---

## Item Definition Format

```
Item <Name>
{
    Plural      <Name Plural>
    Type        <ITEM_IDX_type>
    Effect      <EFFECT_TYPE_x>,<EFFECT_FLAG_x>
    Effect      <EFFECT_TYPE_x>,<EFFECT_FLAG_x>,<EFFECT_MOD_x>
    Value       200.0
    Level       4
    Weight      0.2
    Duration    150.0
    Radius      5.0
    AC          10.0
    ACBonus     <1d5>
    Damage      <2d6>
    To-HitBonus 3.0
    To-DamBonus 3.0
    Speed       1.0
    Color       <255,0,0,255>
    Flags       <ITEM_FLAG_STACKS>
    Flags       <ITEM_FLAG_CURSED,ITEM_FLAG_MAGIC>
}
```

Only `Type` is required. Everything else is optional.
Multiple `Effect` lines = multi-effect item (processed in order).
Multiple `Flags` on one line are comma-separated.

---

## Item Types (ITEM_IDX)

| Constant | Use Command | Flavor System | Equipment Slot |
|---|---|---|---|
| ITEM_IDX_SWORD | — | — | Main hand |
| ITEM_IDX_DAGGER | — | — | Main hand |
| ITEM_IDX_MACE | — | — | Main hand |
| ITEM_IDX_AXE | — | — | Main hand |
| ITEM_IDX_SPEAR | — | — | Main hand |
| ITEM_IDX_POLEARM | — | — | Main hand |
| ITEM_IDX_2H_SWORD | — | — | Both hands |
| ITEM_IDX_BOW | — | — | Main hand (needs ammo) |
| ITEM_IDX_XBOW | — | — | Main hand (needs ammo) |
| ITEM_IDX_ARROW | — | — | Ammo |
| ITEM_IDX_BOLT | — | — | Ammo |
| ITEM_IDX_SHIELD | — | — | Off hand |
| ITEM_IDX_ARMOR | — | — | Body |
| ITEM_IDX_HELMET | — | — | Head |
| ITEM_IDX_CLOAK | — | — | Back |
| ITEM_IDX_GLOVES | — | — | Hands |
| ITEM_IDX_BOOTS | — | — | Feet |
| ITEM_IDX_BELT | — | — | Waist |
| ITEM_IDX_RING | — | — | Finger |
| ITEM_IDX_AMULET | — | — | Neck |
| ITEM_IDX_TORCH | — | — | Light source |
| ITEM_IDX_FUEL | — | — | — (consumable) |
| ITEM_IDX_POTION | Quaff (q) | Random color | — |
| ITEM_IDX_SCROLL | Read (r) | Random label | — |
| ITEM_IDX_WAND | Zap (z) | Random wood | — |
| ITEM_IDX_STAFF | Zap (z) | Random wood | — |
| ITEM_IDX_FOOD | Eat (E) | — | — |
| ITEM_IDX_BOOK | — | — | — |
| ITEM_IDX_SHOVEL | — | — | Main hand |
| ITEM_IDX_CHEST | — | — | — |
| ITEM_IDX_MONEY | — | — | — |

Potions, scrolls, wands, and staves get randomized unidentified names at load time.
Other item types show their real name even before identification.

---

## Item Flags (ITEM_FLAG)

| Flag | Meaning |
|---|---|
| ITEM_FLAG_CURSED | Item is cursed (can't remove) |
| ITEM_FLAG_STACKS | Item stacks in inventory |
| ITEM_FLAG_IDENTIFIED | Pre-identified (no mystery) |
| ITEM_FLAG_MAGIC | Item is magical (detectable) |
| ITEM_FLAG_2HANDED | Requires both hands |
| ITEM_FLAG_OFFHAND | Equips to off hand |
| ITEM_FLAG_MAINHAND | Equips to main hand |
| ITEM_FLAG_NEEDSAMMO | Ranged weapon needs ammo |
| ITEM_FLAG_HOLDING | Container item |

Comma-separate multiple flags: `<ITEM_FLAG_STACKS,ITEM_FLAG_CURSED>`

---

## Effect System

Effects can be defined **inline** on an item or as a **named reference** to `Effects.txt`.

### Inline Effects

Each Effect line has 2, 3, or 4 parts:

```
Effect  <TYPE>,<FLAG>
Effect  <TYPE>,<FLAG>,<MOD>
Effect  <TYPE>,<FLAG>,<MOD>,<NdM>
```

**TYPE** = what the effect does (verb).
**FLAG** = what it targets (noun).
**MOD** = how it does it (adverb). Optional.
**NdM** = damage/healing dice. Optional.

### Named Effect References

Items can reference shared effects from `Effects.txt` by name:

```
Effect  <Light Ray>
Effect  <Firebolt>
```

A single token (no commas) = catalog lookup. The CEffect gets a pointer (`m_ed`) to the shared CEffectDef.

### Effects.txt Format

```
Effect <effect-name>
{
    Type        <EFFECT_TYPE_*>
    Flag        <EFFECT_FLAG_*>
    Flag2       <EFFECT_FLAG_*>
    Modifier    <EFFECT_MOD_*>
    Amount      <NdM>
    Duration    float
    Range       float
    Radius      float
}
```

All fields except `Type` and `Flag` are optional.

### Effect Types (EFFECT_TYPE)

| Type | Purpose | Handler |
|---|---|---|
| EFFECT_TYPE_HEAL | Cure/remove a condition | DoRestoreEffects |
| EFFECT_TYPE_HIT | Deal damage (monster attacks) | DoHitEffects |
| EFFECT_TYPE_CREATE | Create something in the world | DoCreateEffects |
| EFFECT_TYPE_DESTROY | Destroy something | — (reserved) |
| EFFECT_TYPE_INTRINSIC | Grant/inflict a status | DoIntrinsicEffects |
| EFFECT_TYPE_RESTORE | Restore a value | DoRestoreEffects |
| EFFECT_TYPE_GAIN | Increase a resource | DoGainEffects |
| EFFECT_TYPE_LOSE | Decrease a resource | DoLoseEffects |
| EFFECT_TYPE_SEE | Reveal/detect (reserved) | — (proposed) |

### Effect Flags — Word 1 (EFFECT_FLAG)

32 flags. These are the nouns of the effect system.

| Flag | Category | Example use |
|---|---|---|
| EFFECT_FLAG_FIRE | Element | Fire resistance, fire damage |
| EFFECT_FLAG_COLD | Element | Cold resistance, cold damage |
| EFFECT_FLAG_ELECTRICITY | Element | Lightning resist/damage |
| EFFECT_FLAG_ACID | Element | Acid resist/damage |
| EFFECT_FLAG_POISON | Status | Cure poison, poison attack |
| EFFECT_FLAG_LIGHT | World | Create light, light radius |
| EFFECT_FLAG_PARALYZE | Status | Paralyze attack |
| EFFECT_FLAG_TREASURE | Detection | Detect treasure |
| EFFECT_FLAG_AFRAID | Status | Cause/cure fear |
| EFFECT_FLAG_BLIND | Status | Cause/cure blindness |
| EFFECT_FLAG_SLEEP | Status | Put to sleep |
| EFFECT_FLAG_CONFUSE | Status | Cause/cure confusion |
| EFFECT_FLAG_STONE_TO_MUD | World | Dig through walls |
| EFFECT_FLAG_FUEL | Resource | Add lantern fuel |
| EFFECT_FLAG_INFRA | Sight | Grant infravision |
| EFFECT_FLAG_ESP | Sight | Grant telepathy |
| EFFECT_FLAG_IDENTIFY | Knowledge | Identify item |
| EFFECT_FLAG_RECALL | Travel | Word of recall |
| EFFECT_FLAG_MAPPING | Knowledge | Reveal map |
| EFFECT_FLAG_SUMMON | World | Summon monsters |
| EFFECT_FLAG_STAT | Character | Stat gain/lose/restore |
| EFFECT_FLAG_TOHIT | Combat | To-hit bonus |
| EFFECT_FLAG_TODAM | Combat | To-damage bonus |
| EFFECT_FLAG_AC | Combat | AC bonus |
| EFFECT_FLAG_XP | Resource | Experience gain/lose |
| EFFECT_FLAG_HP | Resource | HP gain/lose/restore |
| EFFECT_FLAG_MP | Resource | Mana gain/lose |
| EFFECT_FLAG_TELEPORT | Travel | Teleport |
| EFFECT_FLAG_FREE_ACTION | Status | Immune to paralyze |
| EFFECT_FLAG_INVISIBLE | Status | Grant invisibility |
| EFFECT_FLAG_LEVITATE | Status | Grant levitation |
| EFFECT_FLAG_SPEED | Character | Speed bonus |

### Effect Flags — Word 2 (EFFECT_FLAG2)

Overflow flags. 4/32 bits used. Parsed transparently — no special syntax.

| Flag | Category | Example use |
|---|---|---|
| EFFECT_FLAG_DOOR | Detection | Detect doors/stairs |
| EFFECT_FLAG_TRAP | Detection | Detect/create traps |
| EFFECT_FLAG_MONSTERS | Detection | Detect monsters |
| EFFECT_FLAG_NO_COLLIDE | Projectile | Line effects pass through targets |

### Effect Modifiers (EFFECT_MOD)

| Modifier | Meaning | Example |
|---|---|---|
| EFFECT_MOD_RESIST | Resistance (reduced damage) | Ring of Fire Resistance |
| EFFECT_MOD_SEE | Detection/sight | See Invisible, Detect Doors |
| EFFECT_MOD_IMMUNE | Immunity (no damage) | Helmet of Lordly Protection |
| EFFECT_MOD_WEAK | Weakness (increased damage) | Potion of Flames (cold weakness) |
| EFFECT_MOD_TIMED | Temporary duration | Potion of Resist Fire |
| EFFECT_MOD_AREA | Area of effect | Staff of Light |
| EFFECT_MOD_LINE | Line of effect | Wand of Light |
| EFFECT_MOD_BALL | Ball/sphere | — (reserved) |
| EFFECT_MOD_ENCHANT | Enchantment | Scroll of Enchant Weapon |

---

## Item Examples

### Simple potion (one effect, timed)
```
Item <Potion of Resist Fire>
{
    Plural      <Potions of Resist Fire>
    Type        <ITEM_IDX_POTION>
    Effect      <EFFECT_TYPE_INTRINSIC>,<EFFECT_FLAG_FIRE>,<EFFECT_MOD_RESIST>
    Value       200.0
    Level       4
    Weight      0.2
    Duration    150.0
    Flags       <ITEM_FLAG_STACKS>
}
```

### Permanent ring (no duration = permanent while equipped)
```
Item <Ring of Fire Resistance>
{
    Plural      <Rings of Fire Resistance>
    Type        <ITEM_IDX_RING>
    Effect      <EFFECT_TYPE_INTRINSIC>,<EFFECT_FLAG_FIRE>,<EFFECT_MOD_RESIST>
    Value       300.0
    Level       10
    Weight      0.2
    Color       <225,225,50,255>
}
```

### Scroll (creates something in the world)
```
Item <Scroll of Light>
{
    Plural      <Scrolls of Light>
    Type        <ITEM_IDX_SCROLL>
    Effect      <EFFECT_TYPE_CREATE>,<EFFECT_FLAG_LIGHT>
    Value       20.0
    Level       1
    Weight      2.0
    Color       <255,255,255,255>
    Flags       <ITEM_FLAG_STACKS>
}
```

### Multi-effect item (proposed)
```
Item <Potion of Minor Healing>
{
    Plural      <Potions of Minor Healing>
    Type        <ITEM_IDX_POTION>
    Effect      <EFFECT_TYPE_HEAL>,<EFFECT_FLAG_HP>
    Effect      <EFFECT_TYPE_HEAL>,<EFFECT_FLAG_BLIND>
    Effect      <EFFECT_TYPE_HEAL>,<EFFECT_FLAG_CONFUSE>
    Value       50.0
    Level       2
    Weight      0.2
    Flags       <ITEM_FLAG_STACKS>
}
```

### Equipment with AC bonus (random)
```
Item <Ring of Protection>
{
    Plural      <Rings of Protection>
    Type        <ITEM_IDX_RING>
    ACBonus     <1d20>
    Value       1000.0
    Level       20
    Weight      0.2
    Color       <225,50,225,255>
}
```

---

## Monster Definition Format

```
Monster <Name>
{
    Plural      <Name Plural>
    Appear      <NdM>
    Speed       1.0
    MoveType    <MON_AI_type>
    HD          <NdM>
    AC          40.0
    Level       5
    ExpValue    20.0
    Type        <MON_IDX_type>
    Flags       <MON_FLAG_x,MON_FLAG_y>
    Attack      <EFFECT_TYPE_x>,<MON_FLAG_attack>,damage
    Attack      <EFFECT_TYPE_x>,<MON_FLAG_attack>,<EFFECT_FLAG_x>,damage
    Color       <r,g,b,a>
}
```

`Appear` = how many spawn together (dice roll). Omit for single spawn.

### Attack Line Format

2 forms:

```
Attack  <TYPE>,<ATTACK_FLAG>,damage         # plain damage
Attack  <TYPE>,<ATTACK_FLAG>,<EFFECT>,damage  # damage + elemental effect
```

**TYPE**: Usually `EFFECT_TYPE_HIT` (damage) or `EFFECT_TYPE_INTRINSIC` (status).
**ATTACK_FLAG**: The attack verb (MON_FLAG).
**EFFECT**: Optional elemental/status flag.
**damage**: NdM dice notation (e.g. `1d8`, `3d10`).

---

## Monster Types (MON_IDX)

Each type maps to a tile character. Lowercase = minor, uppercase = major.

| Constant | Tile | Examples |
|---|---|---|
| MON_IDX_ANT | a | Giant Ant |
| MON_IDX_BAT | b | Flaming Bat |
| MON_IDX_CENTIPEDE | c | — |
| MON_IDX_DRAGON | d | White/Red/Blue/Green Dragon |
| MON_IDX_DINOSAUR | Z | Brontosaurus, T-Rex |
| MON_IDX_EYE | e | — |
| MON_IDX_CAT | f | — |
| MON_IDX_GOLEM | g | — |
| MON_IDX_HUMANOID | h | — |
| MON_IDX_HARPY | H | White Harpy |
| MON_IDX_ICKY | i | Blue/Clear/Green Icky Thing |
| MON_IDX_KOBOLD | k | Kobold |
| MON_IDX_LOUSE | l | — |
| MON_IDX_LEECH | — | — |
| MON_IDX_MOLD | m | — |
| MON_IDX_NAGA | n | — |
| MON_IDX_ORC | o | — |
| MON_IDX_PERSON | p | — |
| MON_IDX_RAT | r | — |
| MON_IDX_SKELETON | s | Cursed Skeleton |
| MON_IDX_MINOR_DEMON | u | — |
| MON_IDX_WORM | w | White/Red/Blue/Yellow/Green Worm Mass |
| MON_IDX_SPIDER | S | Huge Spider, Venomous Spider |
| MON_IDX_YEEK | y | — |
| MON_IDX_ZOMBIE | z | — |
| MON_IDX_FROG | F | Giant Frog |
| MON_IDX_BALROG | B | Balrog (boss) |
| MON_IDX_DOG | C | Shadow Mastiff |
| MON_IDX_ANCIENT_DRAGON | D | Ancient Multi-hued Dragon |
| MON_IDX_FLY | — | — |
| MON_IDX_DRAGON_FLY | — | — |
| MON_IDX_FAERIE_DRAGON | — | — |
| MON_IDX_GHOUL | — | — |
| MON_IDX_GHOST | G | Ghost |
| MON_IDX_GIANT | P | — |
| MON_IDX_INSECT | — | — |
| MON_IDX_JELLY | J | Red/Green Jelly |
| MON_IDX_BEETLE | — | — |
| MON_IDX_LICH | L | — |
| MON_IDX_OGRE | O | — |

New monster type? Add `MON_IDX_*` to `Constants.h`, add emoji to `MonIds` in `Monster.cpp`.

---

## Monster AI Types (MON_AI)

| Constant | Behavior |
|---|---|
| MON_AI_100RANDOMMOVE | Pure random movement |
| MON_AI_75RANDOMMOVE | 75% random, 25% seek |
| MON_AI_40RANDOMMOVE | 40% random, 60% seek |
| MON_AI_20RANDOMMOVE | 20% random, 80% seek |
| MON_AI_SEEKPLAYER | Always seek player |
| MON_AI_RUNAWAY | Flee from player |
| MON_AI_STATIONARY | Never moves |
| MON_AI_SEEKTARGET | Seek arbitrary target |

---

## Monster Attack Flags (MON_FLAG — attack verbs)

| Flag | Flavor text | Typical monsters |
|---|---|---|
| MON_FLAG_CRAWL | "crawls on you" | Worm masses |
| MON_FLAG_TOUCH | "touches you" | Worms (elemental), ghost |
| MON_FLAG_CLAW | "claws you" | Dragons, demons |
| MON_FLAG_BITE | "bites you" | Dragons, snakes, spiders |
| MON_FLAG_BREATHE | "breathes fire" | Dragons (with EFFECT_FLAG) |
| MON_FLAG_TRAMPLE | "tramples you" | Ancient dragons |
| MON_FLAG_SPORE | "releases spores" | Mushroom patches |
| MON_FLAG_DROOL | "drools on you" | Jellies |

---

## Monster Property Flags (MON_FLAG — properties)

| Flag | Meaning |
|---|---|
| MON_FLAG_WARM | Warm-blooded (visible to infravision) |
| MON_FLAG_EMPTY_MIND | No mind (immune to ESP) |
| MON_FLAG_REGENERATE | Regenerates HP |
| MON_FLAG_HURT_BY_LIGHT | Takes damage from light |
| MON_FLAG_BREED | Can multiply |

---

## Monster Examples

### Simple monster (worm mass, breeds, random movement)
```
Monster <White Worm Mass>
{
    Plural      <White Worm Masses>
    Appear      <1d3>
    Speed       0.5
    MoveType    <MON_AI_100RANDOMMOVE>
    HD          <1d8>
    AC          40.0
    Level       1
    ExpValue    5.0
    Type        <MON_IDX_WORM>
    Attack      <EFFECT_TYPE_INTRINSIC>,<EFFECT_FLAG_POISON>,1d5
    Color       <255,255,255,255>
    Flags       <MON_FLAG_BREED,MON_FLAG_EMPTY_MIND,MON_FLAG_HURT_BY_LIGHT>
}
```

### Elemental worm (two attacks, one with effect flag)
```
Monster <Red Worm Mass>
{
    Plural      <Red Worm Masses>
    Appear      <1d3>
    Speed       0.5
    MoveType    <MON_AI_100RANDOMMOVE>
    HD          <1d8>
    AC          40.0
    Level       1
    ExpValue    5.0
    Type        <MON_IDX_WORM>
    Attack      <EFFECT_TYPE_HIT>,<MON_FLAG_TOUCH>,1d2
    Attack      <EFFECT_TYPE_HIT>,<MON_FLAG_CRAWL>,<EFFECT_FLAG_FIRE>,1d5
    Color       <255,0,0,255>
    Flags       <MON_FLAG_BREED,MON_FLAG_EMPTY_MIND,MON_FLAG_HURT_BY_LIGHT>
}
```

### Dragon (multiple attacks, warm-blooded)
```
Monster <Red Dragon>
{
    Plural      <Red Dragons>
    Speed       1.0
    MoveType    <MON_AI_SEEKPLAYER>
    HD          <11d8>
    AC          120.0
    Level       35
    ExpValue    300.0
    Type        <MON_IDX_DRAGON>
    Flags       <MON_FLAG_WARM>
    Attack      <EFFECT_TYPE_HIT>,<MON_FLAG_CLAW>,1d8
    Attack      <EFFECT_TYPE_HIT>,<MON_FLAG_CLAW>,1d8
    Attack      <EFFECT_TYPE_HIT>,<MON_FLAG_BITE>,3d10
    Color       <255,0,0,255>
}
```

---

## Color Format

RGBA: `<red,green,blue,alpha>` — each 0-255.

| Color | RGBA |
|---|---|
| White | 255,255,255,255 |
| Red | 255,0,0,255 |
| Green | 0,255,0,255 |
| Blue | 0,0,255,255 |
| Yellow | 255,255,0,255 |
| Black | 0,0,0,255 |

Multi-hued (monsters only): `<<r,g,b,a>;<r,g,b,a>;<r,g,b,a>>`

---

## Adding New Types

### New flavor of existing type
Edit `Items.txt` or `Monsters.txt` only. No code changes.

### New item type
1. Add `ITEM_IDX_*` to `Constants.h` (increment count)
2. Add emoji to `ItemIds` in `Item.cpp`
3. Add string table entry in `Constants.h` Init()
4. Add item definitions to `Items.txt`

### New monster type
1. Add `MON_IDX_*` to `Constants.h` (increment count)
2. Add emoji to `MonIds` in `Monster.cpp`
3. Add string table entry in `Constants.h` Init()
4. Add monster definitions to `Monsters.txt`

---

## Utility Scripts

```
./scripts/find_monster.sh <name>     # search Monsters.txt
./scripts/find_item.sh <name>        # search Items.txt
./scripts/list_monster.sh            # list all monsters
./scripts/list_item.sh               # list all items
```
