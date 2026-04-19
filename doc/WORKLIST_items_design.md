# WORKLIST — Item Design & Content

**Design references:** [Item-Design.md](Item-Design.md) | [Effects-Design.md](Effects-Design.md)

Section keys — Item-Design.md: §1 Data Model, §2 Wands vs Staves, §3 Quality Tiers,
§4 Identification, §5 Stacking, §6 Charges & Recharge, §7 Item Destruction,
§8 Ego Items, §9 Legendary Items, §10 Unique Items, §11 Enchant Scrolls,
§12 New Item Catalog, §13 Encumbrance, §14 Economy & Shops, §15 Dependency Map,
§16 Code Logic, §17 Constants.h, §18 Class/Item Synergies, §19 Intrinsics Grid

Section keys — Effects-Design.md: §1 Data Model, §2 Effects.txt Format, §3 Grammar,
§4 Verbs, §5 Nouns Word 1, §6 Nouns Word 2, §7 Adverbs, §8 Combat Math,
§9 Named Effect Catalog, §10 Handler Status, §11 Constants, §12 Dependencies,
§13 Monster Attack Architecture

---

## Open Design Decisions

Need explicit answers before these can be implemented.

### Effect Coverage Gaps — Items Still Needed

All flags below have no player-usable item that delivers them as a primary positive effect.
Confirmed designs are documented in Item-Design.md §12.

| Flag | Gap | Action |
|---|---|---|
| PARALYZE | ~~Nothing uses PARALYZE as a positive player-usable item~~ | Staff of Paralysis (Mass Paralyze AoE) |
| SLEEP | ~~Nothing uses SLEEP as a positive player-usable item~~ | Staff of Mass Sleep + Scroll of Mass Sleep |
| CONFUSE | Only a negative-effect potion exists | Skipped — Wand of Confusion sufficient |
| STONE_TO_MUD | ~~Wand only; no staff or ring~~ | Ring of Tunneling (permanent intrinsic) |
| AFRAID | ~~Potion of Courage confirmed (Item §12); no wand/staff yet~~ | Staff of Fear (Mass Fear AoE) |
| AC | ~~Scroll of Blessing confirmed (Item §12); no staff or ring~~ | Staff of Protection (Timed Blessing) |
| XP | One potion only; no staff | Skipped — niche, Staff of Enlightenment name reserved |
| FREE_ACTION | Ring only; no scroll or staff | Skipped — Ascension Kit intrinsic, ring-only by design |

- [ ] Resistance stacking formula: 50% + 25% = 62.5%? — deferred to intrinsics deep-dive (Effects §8)

### Open Design Questions

- [ ] Ring of Fate — design and intrinsics unknown (Item §12)
- [ ] Ego data file format: type restrictions, intrinsic list, rarity, min depth (Item §8)
- [ ] Legendary constellations: Westernesse, Holy Avenger, Defender, Amulet of the Magi, Morgul Blade — intrinsic sets TBD (Item §9)
- [ ] Unique item details: Menthir intrinsics and carrier; Excalibur intrinsics (Item §10)
- [ ] Uniques.txt data file format (Item §10)
- [ ] Food content: item list for ITEM_IDX_FOOD (Item §12)
- [ ] Book content: spell lists per caster class — deferred until enough spell effects exist as scrolls/wands/staves (Item §12)

---

## Ready to Implement

No blockers. Start any of these.

### Items.txt Entries

*All items from the initial catalog are now in Items.txt. See Item-Design.md §12 for full status.*

### Effects.txt Entries

*All 98 named effects are now in Effects.txt. See Effects-Design.md §9 for full catalog.*

### Code Logic

- [ ] Recharge risk curve: explosion risk = f(charges, lifetime, depth); lower risk at greater depth (Item §6, §16)
- [ ] Enchantment system: +1/+1d3 per scroll, failure chance above +10, scroll wasted on failure (Item §11, §16)
- [ ] Blessed three-state system: cursed / uncursed / blessed behavior differences (Item §3, §16)
- [ ] Stacking identity check — ITEM_FLAG_STACKS exists; complete per-category rules (Item §5, §16)
- [ ] Partial stack split: "How many? (1-n)" prompt on drop/sell; arrows fire one at a time (Item §5, §16)
- [ ] Ground stacking + loot explosion to adjacent open tiles on overflow (Item §5, §16)
- [ ] Feeling tiers: passive per-turn chance of {magical}, {excellent}, {special} discovery (Item §4, §16)
- [ ] Blind-use identification: noticeable effect → auto-ID; no noticeable effect → {tried} marking (Item §4, §16)
- [ ] Item spawn quality chain: Ego → Legendary → Unique tiers (builds on existing Normal/Cursed/Magic) (Item §3, §16)

### Data Model

- [ ] Ego/legendary/unique identity field on CItem (Item §1, §8–§10)
- [ ] Remove dead field: CItemDef::m_dwBaseHP — initialized to 0, never populated by parser (Item §1)
- [ ] EFFECT_MOD_SUSTAIN define + string table entry in Constants.cpp (Effects §11)

---

## Blocked

Do not start until the blocker is resolved.

### Blocked by Stats #197
- [ ] All STAT-flag items: Gain/Restore/Weakness potions, Heroism, Sustain rings, Sustain armor, Gloves of Dexterity (Item §12)
- [ ] Sustain stat mechanics in code: restore stat to max achieved + prevent stat damage (Item §16)
- [ ] Encumbrance system (Item §13)
- [ ] CHA modifier on shopkeeper prices (Item §14)

### Blocked by Classes #239
- [ ] Spell books: 4 per caster class (ITEM_IDX_BOOK) (Item §12)
- [ ] Class equipment: Holy Symbol (Priest), Nature Focus (Druid), Ki Focus (Monk) (Item §12)
- [ ] Class-specific item feelings: Warriors sense weapons, Mages sense magic, Priests sense curses (Item §4)
- [ ] Mage ID spell, level 37 (Item §4)

### Blocked by Town #243
- [ ] Staff of Word of Recall (full round-trip loop requires town) (Item §12)
- [ ] Scroll of Restoration, Scroll of Darkness (Item §12)
- [ ] Shopkeeper pricing as identification method (Item §4)

### Blocked by PR #235 (Ranged Attacks)
- [ ] Item destruction from elemental attacks: inventory scan, material vulnerability (Item §7, §16)

### Blocked by Trap system #117
- [ ] Scroll of Trap Creation (CREATE + TRAP) (Item §12)

### Blocked by MON_FLAG_INVISIBLE
- [ ] Potion of See Invisible (TIMED + INVISIBLE + SEE), Ring of See Invisible (Item §12)

### Blocked by Enchant system
- [ ] Scroll of Enchant Weapon to Hit, Scroll of Enchant Weapon Damage, Scroll of Enchant Armor (Item §12)
- [ ] Scroll of *Enchant Weapon*, Scroll of *Enchant Armor* (Item §12)

### Blocked by Ego/Unique system
- [ ] Scroll of *Identify* (full lore reveal for ego/unique items) (Item §12)

---

# Appendix: Raw Source Material

Everything below is the verbatim text from each source document, preserved for reference.

### ITEMS section
```
ITEMS:
- Magic items "x of Protection", "y of Resist Fire", but scramble the varieties each game of course.
- ego items [FB] [DB] [WB] [DF] [HA] ...
- unique items I mean Vlad had to take on a Jenoine, I'm just saying.
- Maybe uniques can't be found until the character holding them dies.
- pickaxe / shovel
- monsters can carry treasure
```

### PLAYER section (item-related)
```
- Player can only hold one of each wieldable type (for now; two rings is an enhancement)
- equipped items enhance player instrinsics; player wears ring, set the intrinsic "isInvisible" on the player.
- equipped items are used to make attacks and to defend against attacks (raise AC, raise to-hit and damage stats, &c)
- equippable items restricted to certain subset of all items ITEM_FLAG_EQUIPMENT, instead of hardcoded in CPlayer::Wield.
- implement paginated inventory
- how to remove cursed? (complete ?Remove Curse exists)
- how to shoot bows (issue #39)
- Magic spells etc? (issue #244, #239, #128)
```

### DATA / GAMEPLAY section (item-related)
```
- implement DAT file reading (XML format? INI format? passwd format?) for: dungeon, races, classes, spells, savefile
- need to add status effects (e.g. poison)
- add town level with shops and a house to store things in (some shopkeepers may raise prices depending on your character class)
```

### DOORS / TRAPS / SCENERY section (item-related)
```
- rubble has chance of treasure
```

---

## Source: _JMoria Developer's Guide.md

### Adding a new Item
```
1) If you are creating a new flavor of an existing item (adding a new type of wand), you just need to:
a) edit *Items.txt*
2) If you are creating a new type of item (no Katana? a travesty!), you need to:
a) editm _Items.txt_, then
b) add the new *ITEM_IDX_* type to _Constants.h_, and finally,
c) add the new item's emoji to the *ItemIds* list in _Item.cpp_
```

---

## Source: Designer-Reference.md

### Item Definition Format
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

### Item Types (30 ITEM_IDX constants)

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
| ITEM_IDX_POTION | Quaff (q) | Random color | — |
| ITEM_IDX_SCROLL | Read (r) | Random label | — |
| ITEM_IDX_WAND | Zap (z) | Random wood | — |
| ITEM_IDX_STAFF | Zap (z) | Random wood | — |
| ITEM_IDX_FOOD | Eat (E) | — | — |
| ITEM_IDX_BOOK | — | — | — |
| ITEM_IDX_SHOVEL | — | — | Main hand |
| ITEM_IDX_CHEST | — | — | — |
| ITEM_IDX_MONEY | — | — | — |

Potions, scrolls, wands, and staves get randomized unidentified names at load time. Other item types show their real name even before identification.

### Item Flags (12 ITEM_FLAG constants)

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
| ITEM_FLAG_BLESSED | Item is blessed |
| ITEM_COLOR_MULTI | Multi-color cycling |

### Effect Types (EFFECT_TYPE — verbs)

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

### Effect Flags — Word 1 (EFFECT_FLAG — 32 nouns)

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

### Effect Flags — Word 2 (EFFECT_FLAG2 — 4/32 bits used)

| Flag | Category | Example use |
|---|---|---|
| EFFECT_FLAG_DOOR | Detection | Detect doors/stairs |
| EFFECT_FLAG_TRAP | Detection | Detect/create traps |
| EFFECT_FLAG_MONSTERS | Detection | Detect monsters |
| EFFECT_FLAG_NO_COLLIDE | Projectile | Pass-through on line effects |

### Effect Modifiers (EFFECT_MOD — adverbs)

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

### Designer-Reference Item Examples

**Simple potion (one effect, timed)**:
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

**Permanent ring (no duration = permanent while equipped)**:
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

**Scroll (creates something in the world)**:
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

**Multi-effect item (proposed)**:
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

**Equipment with AC bonus (random)**:
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

## Source: Issue #77 — Item Effects

### Issue Body
```
(references Google Sheets spreadsheet with item/monster effect use cases)
EFFECT_FLAG/MOD/TYPE system description
```

### Comment (2024-11-06): PR #78 — Flavored Attacks
```
started with monsters, they now have flavored attacks instead of "hits" all the time
```

### Comment (2026-04-16): Item Effect Table

| Item | effect flag | effect modifier | effect type | Notes |
|---|---|---|---|---|
| Ring of Levitation | EFFECT_FLAG_LEVITATE | | EFFECT_TYPE_INTRINSIC | Provides intrinsic levitation |
| Ring of Fire Resistance | EFFECT_FLAG_FIRE | EFFECT_MOD_RESIST | EFFECT_TYPE_INTRINSIC | Provides intrinsic resist fire |
| Ring of Cold Resistance | EFFECT_FLAG_COLD | EFFECT_MOD_RESIST | EFFECT_TYPE_INTRINSIC | Provides intrinsic resist cold |
| Potion of Fire Resistance | EFFECT_FLAG_FIRE | EFFECT_MOD_RESIST | EFFECT_TYPE_TIMED | Provides temporary resist fire |
| Potion of Flames | EFFECT_FLAG_FIRE | | EFFECT_TYPE_HIT | Does NdM fire damage when quaffed or thrown |
| (second CEffect on !Flames) | EFFECT_FLAG_COLD | EFFECT_MOD_WEAK | | Potions can shatter if user takes cold damage |
| Scroll of Magic Mapping | EFFECT_FLAG_MAPPING | | EFFECT_TYPE_CAUSE | Oh, this scroll has a map on it |
| Helmet of Lordly Protection (F) | EFFECT_FLAG_FIRE | EFFECT_MOD_IMMUNE | EFFECT_TYPE_INTRINSIC | Lordly protection means "immune" |
| Potion of Invisibility | EFFECT_FLAG_INVISIBLE | | EFFECT_TYPE_TIMED | Provides temporary Invisibility |
| Ring of Invisibility | EFFECT_FLAG_INVISIBLE | | EFFECT_TYPE_INTRINSIC | Provides intrinsic invisibility |
| Potion of See Invisible | EFFECT_FLAG_INVISIBLE | EFFECT_MOD_SEE | EFFECT_TYPE_TIMED | Provides temporary ability to see invisible things |
| Potion of Blindness | EFFECT_FLAG_BLIND | | EFFECT_TYPE_TIMED | Temporary Blindness |
| Potion of Minor Healing | EFFECT_FLAG_HP | | EFFECT_TYPE_GAIN | Potion of healing grants HP, cures blind and confuse |
| (2nd effect) | EFFECT_FLAG_BLIND | | EFFECT_TYPE_HEAL | |
| (3rd effect) | EFFECT_FLAG_CONFUSE | | EFFECT_TYPE_HEAL | |
| Potion of Gain Strength | EFFECT_FLAG_STAT | | EFFECT_TYPE_GAIN | Potion grants permanent stat gain to strength stat |
| Potion of Restore Strength | EFFECT_FLAG_STAT | | EFFECT_TYPE_RESTORE | Potion restores lost strength (due to combat) |
| Potion of Weakness | EFFECT_FLAG_STAT | | EFFECT_TYPE_LOSE | Potion grants permanent stat loss to strength stat |
| Potion of Heroism | EFFECT_FLAG_STAT | | EFFECT_TYPE_TIMED | Potion grants temporary stat bonus to strength stat |
| (2nd effect) | EFFECT_FLAG_HP | | EFFECT_TYPE_TIMED | ... and some temporary HP |
| Scroll of Blessing | EFFECT_FLAG_AC | | EFFECT_TYPE_TIMED | Scroll grants temporary AC bonus |
| Scroll of Summon Monsters | EFFECT_FLAG_SUMMON | | | summons high level monsters surrounding the character |
| Scroll of Door/Stair Location | EFFECT_FLAG_DOOR | EFFECT_MOD_SEE | EFFECT_TYPE_CAUSE | shows doors and stairs within large radius of user |
| Scroll of Trap Detection | EFFECT_FLAG_TRAP | EFFECT_MOD_SEE | EFFECT_TYPE_CAUSE | shows traps within large radius of user |
| Scroll of Trap Creation | EFFECT_FLAG_TRAP | | EFFECT_TYPE_CAUSE | creates (invisible) traps surrounding the character |

### Comment (2026-04-16): Monster Effect Table

| Monster | monster flag | effect flag | effect modifier | effect type | Notes |
|---|---|---|---|---|---|
| White Worm Mass | MON_FLAG_CRAWL | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (crawls on you) |
| | MON_FLAG_TOUCH | EFFECT_FLAG_POISON | | EFFECT_TYPE_TIMED | poison touch |
| Red Worm Mass | MON_FLAG_CRAWL | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (crawls on you) |
| | MON_FLAG_TOUCH | EFFECT_FLAG_FIRE | | EFFECT_TYPE_HIT | fire touch (possible to burn scrolls and potions and leather items) |
| Blue Worm Mass | MON_FLAG_CRAWL | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (crawls on you) |
| | MON_FLAG_TOUCH | EFFECT_FLAG_COLD | | EFFECT_TYPE_HIT | cold touch (possible to shatter potions) |
| Green Worm Mass | MON_FLAG_CRAWL | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (crawls on you) |
| | MON_FLAG_TOUCH | EFFECT_FLAG_ACID | | EFFECT_TYPE_HIT | acid touch (possible to destroy scrolls, potions, leather, and metal items) |
| | | | | EFFECT_TYPE_LOSE | ... acid touch can harm CHA also (you feel ugly) |
| Yellow Worm Mass | MON_FLAG_CRAWL | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (crawls on you) |
| | MON_FLAG_TOUCH | EFFECT_FLAG_STAT | | EFFECT_TYPE_LOSE | -dex touch (you feel clumsy) |
| Red Dragon | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you) |
| | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you twice) |
| | MON_FLAG_BITE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (bites you) |
| | MON_FLAG_BREATHE | EFFECT_FLAG_FIRE | | EFFECT_TYPE_HIT | breathes fire |
| Baby Red Dragon | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you) |
| | MON_FLAG_BITE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (bites you) |
| | MON_FLAG_BREATHE | EFFECT_FLAG_FIRE | | EFFECT_TYPE_HIT | breathes fire (much less often, much less damage) |
| Ancient Red Dragon | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you) |
| | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you twice) |
| | MON_FLAG_BITE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (bites you) |
| | MON_FLAG_TRAMPLE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | ancient dragons also trample |
| | MON_FLAG_BREATHE | EFFECT_FLAG_FIRE | | EFFECT_TYPE_HIT | breathes fire |
| Green Dragon | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you) |
| | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you twice) |
| | MON_FLAG_BITE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (bites you) |
| | MON_FLAG_BREATHE | EFFECT_FLAG_POISON | | EFFECT_TYPE_HIT | breathes poison gas |
| Ghost | MON_FLAG_TOUCH | EFFECT_FLAG_PARALYZE | | EFFECT_TYPE_TIMED | ghost touch paralyzes |
| Greater Demon | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you) |
| | MON_FLAG_CLAW | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (claws you twice) |
| | MON_FLAG_BITE | EFFECT_FLAG_HP | | EFFECT_TYPE_HIT | normal attack (bites you) |
| | | EFFECT_FLAG_SUMMON | | | summons lesser demons |

---

## Source: Issue #114 — Identify Spell

### Issue Body
```
Given I have a magic thing
And I have a scroll of identify
When I choose the magic thing
Then the magic thing is identified

---

Given I have a previously unknown type of magic thing
And I Identify it using scroll of identify
When I look at another thing of the same type
Then I know it's the same type as my identified thing
---

Given I have an item
And I wield the item
When I try to unwield the item
And the item is cursed
Then the item becomes "known cursed"
And I cannot unwield the item

---

AC
* item known/unknown values are stored in a bitmask m_dwKnownIntrinsics
  * if any KI flag is set, that flag is known vs unknown
  * item can be set to ITEM_FLAG_CURSED
  * m_dwKnownIntrinsics |= ITEM_FLAG_CURSED  means the cursed status of the item is known
  * ITEM_FLAG_CURSED is set on the item means it IS cursed (two separate values, different semantics)
* there are tiers of feeling about items
  * in the item's description, "{magical}" | "{excellent}" | "{special}"
  * magical = item has bonus(es) (+4 to hit, +3 to dam, or similar bonuses)
  * excellent = item is an ego item (Longsword of Slay Beast)
  * special = item is a Unique Item (Longsword of \<hero_name\> has special abilities)
  * warriors will eventually "feel" weapons and know about them. other items will take longer.
  * mages will eventually "feel" magical auras and know about them, may detect over distance, &c. warriors not so much.
  * priests will eventually "feel" blessings and curses, know about items by handling them
* scroll of identify (#114): read a scroll, choose an item. the item's properties are revealed.
* the scroll's type (scroll of identify) becomes known
* item's state should be one of cursed, uncursed, blessed (three state, not two)
* a blessed item has bonuses between versions of the same item, a cursed item has penalties
```

### Comment (2024-11-21): Known Cursed Truth Table
```
* item cursed but I don't know yet
item flags: ITEM_FLAG_CURSED
known: 0

* I know the item is cursed
item flags: ITEM_FLAG_CURSED
known: ITEM_FLAG_CURSED

* I know the item is uncursed ("I know the item's cursed status")
item flags: 0
known: ITEM_FLAG_CURSED

* I don't know if it's cursed, and it is uncursed (it's not cursed, but I don't know it)
item flags: 0
known: 0

So the display happens when known is set, and what is displayed is the underlying item flags/values
if( known(cursed) ) display(item.cursed?"cursed":"uncursed") else ""
if( known(magic)) display("(+x, +y)") else ""
```

### Comment (2024-11-22): Magic Known Works Same Way
```
Yes, exactly, this (uncursed, magic) item would be displayed as "a Dagger (+3, +7) (uncursed)"

Before reading scroll of ID it would look like "a Dagger".

The same truth table would occur for magic. The item could be non-magic or magic, and you either know or don't know.

You maintain the "known" stuff in the one flag, and the "this is what the item is" on the other.
```

### Comment (2024-12-06): ID Reveals Charges
```
?ID should also reveal m_dwCharges if it's set
```

### Comment (2026-04-14): Class-Specific Identification Feelings
```
Class identity should extend to how quickly each class intuits item properties, enabled by stat priorities:

- **Warriors** (high STR/CON): Quickly sense weapon curses and damage bonuses; slow on magic effects
- **Rogues** (high DEX): Quickly sense traps and AC effects; moderate on curses
- **Rangers** (high DEX): Quickly sense ranged weapon properties and ammunition quality
- **Mages** (high INT): Extremely quick to sense magic/enchantment; average on curses
- **Priests** (high WIS): Quickly sense blessings and holy properties; fast on curses
- **Druids** (high WIS): Quickly sense nature-aligned properties and poisons
- **Shamans** (high WIS): Quickly sense elemental properties and spirit ties
- **Monks** (high WIS): Quickly sense cursed/blessed status and spiritual properties

This creates **passive knowledge discovery** beyond just "try to remove" or "use it". Longer you hold an item, faster your class learns its secrets.
```

### Comment (2026-04-16): Full Identification Design (Jimbo)
```
A: Identification is critical to success in the game. There is "I know nothing about this item" and you get the generic unidentified name "a Dagger", "2 Blue Potions". Potions, Scrolls, Wands, Staves, etc. are known to be magical; armor and weapons are sometimes mundane and sometimes "better". Any idiot carrying around a dagger that smells bad and lets off a black glow will eventually realize "You feel that the Dagger in your pack is cursed" and it will now display as "a Dagger {cursed}". Or, in a different case, if the Dagger is magical, then eventually a Mage would get "You feel that the Dagger in your pack is magical" or "excellent" or "special" (for Dagger(+1, +5), Dagger of Slay Beast (+1, +5), or The Dagger "Sting" (+1, +5), respectively). so with this "feeling", the Dagger would eventually (over time, low low chance like passive searching) be in inventory as Dagger {excellent}. Then, if a ?Identify is read and the Dagger is chosen, it would now appear as Dagger of Slay Beast (+1, +5) and ex)amine the weapon would tell some info about how it does x2 against beasts or something. But for a unique like Sting, it wouldn't tell you everything, there might be some lore that would only open with a ?*Identify* (with the `*`). Mages would eventually get both spells also, *ID* level 37, ID level 15. But, the question here is about blind-reading or blind-quaffing to ID things. In short... it's "a" method. But unless the Potion fixed something that you had broken (Restore DEX when your DEX is maxed will "Nothing happens", but Restore DEX when your DEX is damaged will "You feel nimble again"), it's not a *surefire* way to ID something. You can also sell an item to a shopkeeper, but they wil give you a horrible price for it (a great price if it's poison or cursed, a terrible price if it's anything worthwhile), but then you'll know what it is. If the potion doesn't do enough for you to ID it the first time, it will say "2 Blue Potions {tried}". Same thing for wands (when you zap a monster with Heal Monster wand, "The Kobold screams in agony"), same thing for Scrolls (read a Scroll of Create Traps, it does not ID itself, and you are now surrounded by 9 invisible traps). If there is some noticeable, positive effect, then you can ID by using. But if you put on a Cloak of Protection (1, +10), it will give you 11 AC, so that's noticeable. "Oh there is a map on this scroll" noticeable "You feel resistant to fire" noticeable. Ask more questions if you need to
```

---

## Source: Issue #128 — Magic Weapons

### Issue Body
```
normal item: just a thing. (Dagger). Dagger has +0/+0 to-hit/to-dam bonus.
cursed item: item has negative bonuses. Dagger(-3, -5). Player cannot remove a cursed item without ?Remove Curse
magical item: item has positive bonuses. Dagger(+3, +5). Very common, player should find at least 1-2 per level.
ego item: this item has one or more intrinsics, giving it either a name or a suffix. Flametongue(+3,+3) [FT] has intrinsic: does 2x damage to creatures weak against fire. These are semi-rare, player should find 1 to 3 over the course of a 40-level run. Some are cursed: Morgul Blade(+4, +4) [MB] gives weakness to holy damage and drains life each level, cannot be removed without Remove Curse, and the ?Remove Curse must be blessed/enhanced.
unique item: this is a named weapon (or other item) that has many different intrinsics and also some lore text. only one of each unique per game. "Sting" is a Dagger with intrinsics: gives +2 speed, gives See Invisible, warns when orcs are nearby, glows blue when orcs are nearby, and does 2x damage to orcs.

ego items: Flametongue [FT], Frost Brand [FB], Demon Bane [DB], Slay Dragon [SD], Resist Lightning [RL]
legendary items: Westernesse (weapon), Amulet of the Magi (equipment), Holy Avenger [HA] (mace), Defender [DF] (halberd), Morgul Blade [MB] (weapon)

?Enchant Weapon gives either +1 to-hit or +1 to-dam (enchant weapon to hit, enchant weapon damage)
?Enchant Armor gives either +1 AC or a small intrinsic (enchant armor)
?*Enchant Weapon* (star-enchant) gives +1d3 to both, all at once
?*Enchant Armor* gives +1d3 AC plus a random intrinsic

Enchanting a weapon past +10 to hit or +10 to dam has a chance to fail
Enchanting an armor past +10 AC has a chance to fail

?ID reveals most (all?) of the intrinsic properties
?*ID* reveals all (always), including lore text for uniques

---

Item Quality Tiers:
1. Normal (+0, +0) — mundane
2. Cursed (-N, -N) — negative bonuses, can't remove
3. Magic (+N, +N) — positive bonuses, found/enchanted
4. Ego (named suffix) — intrinsics, semi-rare
5. Legendary (constellation) — fixed combo of intrinsics
6. Unique (named) — one per game, lore text
```

### Comment (2024-11-26): Item Spawn Process
```
There is going to have to be some kind of process for spawning a weapon that looks like:
1) spawn a thing
2) is it a weapon?
3) check to see if it's magical (% chance will change based on dungeon level, item level, maybe secret character "luck" stat) note that magic just means bonuses to hit and damage, which are part of the ItemDef already -- this can be 100% random generated
4) if it's magical, see if it's cursed -- this can be 100% random generated
5) if not cursed, see if it's ego (much smaller chance, 1% or so) note that Ego (all the FT/FB/DB, and all the "of X") are randomly selected from a fixed list of ego possibilities. And e.g. "of Levitation" only applies to BOOT types (given a weapon or armor type, only certain ego statuses are available to it)
6) if ego, see if it's legendary (way small chance, .01% and also won't show up unless very deep in dungeon no chance until level 20 or so) (again this is by item type, only some legendary types can be applied to each type of item)
7) if legendary, see if it's a unique (sometimes uniques are carried by particular mobs, like you gotta kill King Arthur to get Excalibur -- there should be a Uniques.txt that lists them all) (uniques are always a specific item -- Excalibur is always a Bastard Sword, never a Mace)
```

### Comment (2026-04-14): Unique Item Persistence Design
```
Confirmed from roadmap:
- Each unique item is a singleton per character save (only one Lady Teldra can exist in a playthrough)
- Standard Mode: Unique items that aren't found before leaving a level don't respawn (gone forever)
- Hardcore Mode (Optional Setting): Unique items CAN respawn if not collected before exiting level (increases challenge for skilled players)

This design:
- Incentivizes thorough exploration and risk-taking
- Prevents "soft-lock" where you permanently miss unique items
- Hardcore mode provides scalable difficulty without permadeath-only settings
- Ties into #43 (unique item personality) — Lady Teldra's "I refuse to leave" attitude reflects the stakes
```

---

## Source: Issue #121 — Fuel/Charges

### Issue Body
```
Scroll of Recharging (charges depend on kind/user/depth, risk of explosion)
F)ill lantern with Flask of Oil (+5000 turns, max 15000)
```

### Comment (2024-11-23): Pre-Identified Potions
```
note that potion colors now cause flasks of oil to be random names
this might be a good opportunity to have some potions always be identified (Light Brown Potion of Apple Juice ... and Flask of Oil)
```

---

## Source: Issue #43 — Lady Teldra (Unique Weapon)

### Issue Body
```
Lady Teldra is a unique, soul-bonded blade. She has:
- Personality (refuses to attack peaceful creatures)
- Spell absorption
- Configurable flavor text
- One per game spawn rules
- Hard mode: leave level = lost forever
```

### Comment (2026-04-14): Unique Item Persistence
```
Related: See #128 for unique item persistence design (singleton per character save, optional hardcore respawn mode).
```

---

## Source: Issue #72 — Fog of War (Light Source Items)

### Issue Body
```
Torches light radius 3 (one use, 3000 turns)
Lanterns light radius 5 (refuel with oil, +5000 per can, limit 15000)
Some races have infravision and can see in the dark radius 8
Only works on warm things (worm masses and icky things no)
Some races have ESP and can detect brains radius 8 (undead no, worm masses no)

Scroll of Light
Spell of Light Area
Either will light current room radius 10

Staff of light, same, radius 15

Staff of starlight shoots rays of blue light in all directions and casts light area

Wand of light casts a line of blue light

Blue light damages some creatures (orcs, vampires, worm masses)
```

---

## Source: Issue #117 — Search Command (Item Mentions)

### Issue Body
```
* Ring of searching adds base %
* ?Detect Doors/Stairs shows all secret doors within radius 30
* ?Detect Traps shows traps within radius 30
* Draw green line where trap detection ends
* Show detect traps on player stats
* Gloves of Dexterity also add % search
* Search % is based on dex (bro do you even have stats)
```

---

## Source: Issue #197 — Player Stats (Item Mentions)

### Issue Body
```
start with STR, random 3d6, maybe force it to be 9-18 to start (until you have STR+1 items) -- give bonuses to hit with melee weapons, and to damage for melee and ranged weapons (bows and spears but not crossbows -- you throw harder, you pull a stronger bow), low STR will give negatives to these

INT/WIS are for when we add spellcaster classes; also help with ?-_ work better

CHA is for when we add shops, $/$$ with shopkeepers
```

### Comment (2026-04-14): XP Progression & Stat Items
```
Stat Ranges: All stats 3-18/100

Stat Buff Scaling:
- Low stats (3-8): Potion grants +1 to +3
- High stats (15-17): Potion grants +1
- 18+: Potion grants +10% (e.g., 18/00 → 18/10)
- 18/86+: Potion grants +1-2% (ultra-rare, late-game grinding)

Stat Damage & Recovery:
- Monsters with stat-damaging attacks (Yellow Worm Mass -WIS, Vampire -CON/life levels)
- Restore [Stat] Potion: Undo damage (Restore Wisdom, Restore Constitution, etc.)
- Gain [Stat] Potion: Permanent +1-3 at low values, +1 at high values (+% after 18)
- Sustain [Stat] Ring/Armor: Prevent stat damage from specific attacks
- Hardcore players grind to max all stats (54 potions minimum for 3-18/100 six times)

Thief Behavior:
- Items stolen: Only lightweight inventory (potions, scrolls), NOT books/staves/equipment
```

---

## Source: Issue #239 — Add Classes (Item Mentions)

### Issue Body
```
Spell Book System (for Spellcasters):
- Each spellcaster has access to 4 progressively rare books
- Books do not degrade over time
- Book 1-2: Can be purchased in town
- Book 3-4: Must be found in the dungeon (no purchases)

Spell Distribution (ZAngband-style):
- Book 1: Mostly low-level spells, scattered high-level utility spells
- Book 2: Mostly low-mid level, adds low-level utility spells, some high-level spells
- Book 3: Mostly mid-level, expands high-level options
- Book 4: Mostly high-level spells, includes one low-mid utility spell (fills gaps)

Character Progression Synergies:
- Warrior + High STR/CON + Heavy Armor + Melee Weapons
- Rogue + High DEX + Light Armor + Dual-Wield
- Ranger + High DEX + Ranged Weapons + Tracking
- Mage + High INT + Light Armor + Spell Books + Wands/Staves
- Priest + High WIS + Medium Armor + Spell Books + Holy Symbol
- Druid + Balanced INT/WIS + Medium Armor + Spell Books + Nature Focus
- Shaman + High WIS + Medium Armor + Spell Books + Elemental Items
- Monk + High DEX/WIS + No Armor + Unarmed Combat + Ki Focus

Different equipment priorities per class (Mage uses staves, Warrior uses shields, etc.)
Class-specific item interactions (Priest armor bonuses, Mage spell book discs, etc.)
```

### Comment (2026-04-14): Magic System Philosophy
```
No Multi-Classing — The magic system follows a Moria model (fixed, disciplined) rather than ZAngband (flexible school-selection).
- Each spellcasting class has a fixed, defined spell list
- Books are structured, not pick-and-choose
```

---

## Source: Issue #243 — Town System (Shop/Item Mentions)

### Issue Body
```
Shop Types (Always Present):
1. General Store — misc items (rope, torches, potions, scrolls)
2. Weaponsmith/Armorer — weapons, armor, shields
3. Temple — healing potions, scrolls of Restoration, blessed items
4. Magic Shop — magic items, wands, staves, some scrolls (but not spell books 3-4)
5. Player Home — storage/rest area (free healing, safe save point)

Some items NEVER appear in shops: Spell books (3-4), unique items, some quest rewards

Scroll of Word of Recall:
- Consumable scroll: "scroll of Word of Recall"
- Reading in dungeon teleports player to town (safe)
- Reading in town teleports to last dungeon position
- Single-use, but purchasable in shops (moderately rare)

Staff of Word of Recall (Treasure Item):
- Reusable version of scroll
- No charges (unlimited uses)
- High-level goal for returning to town safely
- Late-game comfort item
```

### Comment (2026-04-14): Detailed Town System Economics
```
Shop Item Distribution:
- General Store: 90% mundane, 10% magic items (at high prices)
- Weaponsmith: 95% mundane weapons, 3% magic weapons, 2% Ego weapons (e.g., Longsword of Slay Demon)
  - Ego weapon appearance rate: ~2-3 Ego items per 40-level character run
- Magic Shop: Always stocked, never "bad" items (no Heal Monster scrolls)
  - 60% good items (Cure Wounds, Identify, Magic Missile)
  - 30% meh items (Darkness, Amulet of Sensing)
  - 10% great items (Ring of Fate, Teleport Level, *Identify*)
- Temple: Healing potions, Restoration scrolls, blessed items
- Bazaar: Rare high-value items from other shops at 2x prices (appears randomly)
- Black Market: Ego and Artifact-tier weapons only (locked until character level 20+)

Item Rarity (~250k gp economy per 40-level run):
- 80-90% of useful loot comes from dungeon (killing monsters or floor drops)
- 10-20% comes from shops (utility, early-game essentials)

Player House (Town Storage):
- Items dropped in Player House persist between visits
- Strategic storage for: extra Scrolls of Word of Recall, Staves of Identify, Potions of Remove Curse, etc.

NPC Thief Behavior:
- Thieves steal coins primarily; occasionally lightweight items (potions, scrolls, but NOT books/staves/equipment)
```

---

## Source: Issue #244 — Intrinsics & Status Effects (Item Mentions)

### Issue Body (items as intrinsic sources)
```
4 Variants per Effect (Example: Haste):
1. Perm Buff — Equipment item grants Haste +10 permanently (e.g., Boots of Speed)
2. Temp Buff — Potion of Speed grants Haste +10 for 100 game turns
3. Perm Debuff — Cursed item applies Haste -10 permanently (misleading: slows character)
4. Temp Debuff — Monster/trap applies Haste -10 for 50 turns (slow effect, wears off)

Elemental Resistances:
- RF (Fire Resist): 50% damage reduction from fire attacks
- RC (Cold Resist): 50% damage reduction from cold attacks
- RL (Lightning Resist): 50% damage reduction from electricity
- RA (Acid Resist): 50% damage reduction from acid attacks
- RP (Poison Resist): Eliminates poison damage, immunity to poison stat effects

Immunities:
- Hold Life — Prevents life level drain
- Free Action — Prevents paralysis/sleep
- Sustain [Stat] — Restores stat to max achieved value + prevents any damage to that stat

Movement & Navigation:
- Levitation — Float above traps/water
- Telepathy — Detect other sentient creatures
- See Invisible — Reveal invisible creatures
- Infravision — See warm-blooded creatures in darkness (radius 8)

Special Abilities:
- Regen — Character regenerates 1 HP per turn
- Magic Sensing — Detect magical auras

Intrinsics Grid (new UI screen):
         | Head (Helm) | Hands (Gloves) | Body (Armor) | Legs (Boots) | Character
---------|-------------|----------------|--------------|--------------|----------
RF       | Immune      | Resist         | No-effect    | Weak         | Immune
Hold Life| Immune      | —              | —            | —            | Immune
Free Act | —           | Immune         | —            | —            | Immune
Speed    | —           | —              | —            | +10          | Base +5
Sustain  | —           | —              | —            | —            | Sustain WIS
```

### Comment (2026-04-14): Q&A on Intrinsics
```
Sustain WIS does 2 things: 1) restore WIS to max achieved value (like taking a !RestoreWIS), and 2) prevent any WIS damage from occurring "Yellow Worm Mass crawls on you -more- You feel naive -more- The feeling passes"

No hard cap on intrinsics. If the character can gain every positive intrinsic available, they should try to do so.

Traps can cause intrinsic effects: fire trap, teleport trap, summon monster trap, trapdoor
```

---

## Source: Issue #270 — Item Design & Content (Phase 3)

### Issue Body (full)
```
Master reference (similar to Monster-Design.md): Create a comprehensive Item-Design.md document with all item families, type progression, and data.

Current Inventory (Items.txt statistics):
- Total: ~76 items
- 19 new items added in Phase 3

Existing Item Categories:
Potions (11 variant + core):
- Core: Minor Healing, Blindness, Confusion, Poison
- Phase 3: Resist Cold, Resist Acid, Resist Electricity, Invisibility, Speed, Levitation, Infravision
- Blocked: Gain STR/Restore STR (need stats #197), Heroism (stats #197), Flames (elemental hit system), See Invisible (MON_FLAG_INVISIBLE)

Scrolls (core + Phase 3):
- Core: Identify
- Phase 3: Recall, Summon Monsters, Light, Telepathy
- Blocked: Detect Doors (EFFECT_TYPE_SEE), Trap Detection (trap system), Blessing (timed AC deep copy)

Rings (core + Phase 3):
- Core: Protection
- Phase 3: Cold Resistance, Acid Resistance, Electricity Resistance, Free Action, Speed, Telepathy

Helmets (Phase 3):
- Helm of Infravision, Helm of Telepathy
- Helmet of Lordly Protection (blocked: IMMUNE vs RESIST distinction)

Tools (core):
- Torch, Brass Lantern (light sources)
- Missing: Flask of Oil, Wand of Light, Staff of Light, Staff of Starlight

Weapons & Armor:
- Dagger, Short Sword, Long Sword, Spear, Axe, Club, Bow (base equipment)
- Leather Armor, Padded Armor, Combat Armor (base armor)
- Shield (block chance)

Remaining Work:
Tier 1 — Light Source Items:
- Wand of Light, Staff of Light, Staff of Starlight, Flask of Oil

Tier 2 — Detection Items (Blocked by EFFECT_TYPE_SEE):
- Scroll of Detect Doors, Scroll of Detect Traps, Scroll of Detect Monsters, Ring of Searching

Tier 3 — Stat-Based Items (Blocked by Stats #197):
- Potion of Gain STR/CON/DEX/INT/WIS/CHA, Potion of Restore STR/CON/DEX/INT/WIS/CHA, Potion of Heroism

Tier 4 — Elemental & Status Items:
- Potion of Flames, Potion of See Invisible, All Effect/Resistance combinations

Tier 5 — Advanced Properties (Post-Phase 3):
- Magical/Cursed/Ego/Unique item tiers, Shopkeeper pricing as identification
```

---

## Source: Issue #271 — Item Destruction from Elemental Attacks

### Issue Body (full)
```
Fire Damage:
- Destroys: scrolls, potions, leather items (armor, helmets)
- Reduces: weapon durability
- Fire resistance potions / items can reduce/negate this

Cold Damage:
- Shatters: potions exclusively
- Frozen potions break automatically
- Reduces: movement speed (separate mechanic)

Acid Damage:
- Destroys: scrolls, potions, leather items, metal items (worst)
- Acid resistance potions / items can reduce/negate this
- Visual: visible item destruction on hit ("Your scrolls are destroyed!")

Monster Attacks that Trigger:
- Red Worm Mass: Touch (fire hit)
- Blue Worm Mass: Touch (cold hit)
- Green Worm Mass: Touch (acid hit + CHA stat loss)
- Red Dragon: Breathe (fire, large AoE)
- Green Dragon: Breathe (poison — may or may not trigger, TBD)

Display:
- "Your scrolls catch fire!"
- "Ice shatters your potions!"
- "Acid melts your leather armor!"

Depends on: PR #235 (Ranged Attacks)
Postponed until after ranged attack implementation
```

---

## Source: Items.txt — Complete Current Inventory (76 items)

### Armor (6)
1. Chain Mail — ITEM_IDX_ARMOR, AC 22, Level 26
2. Leather Armor — ITEM_IDX_ARMOR, AC 8, Level 2
3. Padded Armor — ITEM_IDX_ARMOR, AC 15, Level 7
4. Plate Mail Armor — ITEM_IDX_ARMOR, AC 38, Level 48
5. Robe — ITEM_IDX_ARMOR, AC 1, Level 1
6. Splint Mail — ITEM_IDX_ARMOR, AC 25, Level 24

### Boots (2)
7. Pair of Leather Boots — ITEM_IDX_BOOTS, AC 2, Level 1
8. Pair of Steel Boots — ITEM_IDX_BOOTS, AC 5, Level 1

### Cloaks (1)
9. Cloak — ITEM_IDX_CLOAK, AC 1, Level 1

### Gloves (2)
10. Pair of Leather Gloves — ITEM_IDX_GLOVES, AC 2, Level 1
11. Set of Gauntlets — ITEM_IDX_GLOVES, AC 4, Level 1

### Helmets (4)
12. Steel Cap — ITEM_IDX_HELMET, AC 3, Level 7
13. Steel Helm — ITEM_IDX_HELMET, AC 5, Level 10
14. Helm of Infravision — ITEM_IDX_HELMET, AC 4, Level 15, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_INFRA, ITEM_FLAG_MAGIC
15. Helm of Telepathy — ITEM_IDX_HELMET, AC 4, Level 25, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_ESP, ITEM_FLAG_MAGIC

### Light Sources (3)
16. Torch — ITEM_IDX_TORCH, Radius 3, Duration 3000, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_LIGHT
17. Brass Lantern — ITEM_IDX_TORCH, Radius 5, Duration 7500, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_LIGHT, ITEM_FLAG_NEEDSAMMO
18. Flask of Oil — ITEM_IDX_FUEL, Duration 5000, EFFECT_TYPE_GAIN+EFFECT_FLAG_FUEL, ITEM_FLAG_STACKS

### Rings (10)
19. Ring of Invisibility — ITEM_IDX_RING, Level 20, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_INVISIBLE
20. Ring of Levitation — ITEM_IDX_RING, Level 20, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_LEVITATE
21. Ring of Telepathy — ITEM_IDX_RING, Level 20, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_ESP
22. Ring of Fire Resistance — ITEM_IDX_RING, Level 10, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_FIRE+EFFECT_MOD_RESIST
23. Ring of Protection — ITEM_IDX_RING, Level 20, ACBonus 1d20
24. Ring of Cold Resistance — ITEM_IDX_RING, Level 10, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_COLD+EFFECT_MOD_RESIST
25. Ring of Acid Resistance — ITEM_IDX_RING, Level 12, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_ACID+EFFECT_MOD_RESIST
26. Ring of Electricity Resistance — ITEM_IDX_RING, Level 12, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_ELECTRICITY+EFFECT_MOD_RESIST
27. Ring of Free Action — ITEM_IDX_RING, Level 15, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_FREE_ACTION
28. Ring of Speed — ITEM_IDX_RING, Level 30, EFFECT_TYPE_INTRINSIC+EFFECT_FLAG_SPEED

### Potions (12)
29. Potion of Minor Healing — ITEM_IDX_POTION, Level 2, HEAL+HP / HEAL+POISON / HEAL+BLIND, Damage 1d20
30. Potion of Resist Fire — ITEM_IDX_POTION, Level 4, INTRINSIC+FIRE+RESIST, Duration 150
31. Potion of Infravision — ITEM_IDX_POTION, Level 4, INTRINSIC+INFRA, Duration 100
32. Potion of Resist Cold — ITEM_IDX_POTION, Level 4, INTRINSIC+COLD+RESIST, Duration 150
33. Potion of Resist Acid — ITEM_IDX_POTION, Level 6, INTRINSIC+ACID+RESIST, Duration 150
34. Potion of Resist Electricity — ITEM_IDX_POTION, Level 6, INTRINSIC+ELECTRICITY+RESIST, Duration 150
35. Potion of Invisibility — ITEM_IDX_POTION, Level 8, INTRINSIC+INVISIBLE, Duration 100
36. Potion of Speed — ITEM_IDX_POTION, Level 10, INTRINSIC+SPEED, Duration 80
37. Potion of Levitation — ITEM_IDX_POTION, Level 6, INTRINSIC+LEVITATE, Duration 100
38. Potion of Blindness — ITEM_IDX_POTION, Level 1, INTRINSIC+BLIND, Duration 50
39. Potion of Confusion — ITEM_IDX_POTION, Level 1, INTRINSIC+CONFUSE, Duration 30
40. Potion of Poison — ITEM_IDX_POTION, Level 3, INTRINSIC+POISON, Duration 80
41. Potion of Cure Poison — ITEM_IDX_POTION, Level 3, HEAL+POISON

### Scrolls (10)
42. Scroll of Light — ITEM_IDX_SCROLL, Level 1, CREATE+LIGHT
43. Scroll of Telepathy — ITEM_IDX_SCROLL, Level 4, INTRINSIC+ESP, Duration 100
44. Scroll of Remove Curse — ITEM_IDX_SCROLL, Level 1, DESTROY+ITEM_FLAG_CURSED
45. Scroll of Identify — ITEM_IDX_SCROLL, Level 1, RESTORE+IDENTIFY
46. Scroll of Phase Door — ITEM_IDX_SCROLL, Level 2, CREATE+TELEPORT+AREA
47. Scroll of Teleportation — ITEM_IDX_SCROLL, Level 10, CREATE+TELEPORT
48. Scroll of Magic Mapping — ITEM_IDX_SCROLL, Level 8, CREATE+MAPPING+AREA
49. Scroll of *Magic Mapping* — ITEM_IDX_SCROLL, Level 25, CREATE+MAPPING
50. Scroll of Word of Recall — ITEM_IDX_SCROLL, Level 8, CREATE+RECALL
51. Scroll of Summon Monsters — ITEM_IDX_SCROLL, Level 3, CREATE+SUMMON

### Shields (4)
52. Large Steel Shield — ITEM_IDX_SHIELD, AC 6, Level 2, ITEM_FLAG_OFFHAND
53. Small Steel Shield — ITEM_IDX_SHIELD, AC 4, Level 2, ITEM_FLAG_OFFHAND
54. Small Wooden Shield — ITEM_IDX_SHIELD, AC 2, Level 1, ITEM_FLAG_OFFHAND
55. Steel Shield — ITEM_IDX_SHIELD, AC 5, Level 2, ITEM_FLAG_OFFHAND

### Tools (2)
56. Shovel — ITEM_IDX_SHOVEL, Level 1
57. Pickaxe — ITEM_IDX_SHOVEL, Level 1

### Wands (1)
58. Wand of Light — ITEM_IDX_WAND, Level 1, Effect <Light Ray> / Effect <Light Area>, Charges <3d8>, ITEM_FLAG_STACKS

### Weapons (18)
59. Bastard Sword — ITEM_IDX_SWORD, Level 5, Damage 2d8
60. Battle Axe — ITEM_IDX_AXE, Level 10, Damage 2d4, ITEM_FLAG_2HANDED
61. Battle Mace — ITEM_IDX_MACE, Level 110(!), Damage 2d3, ITEM_FLAG_2HANDED
62. Club — ITEM_IDX_MACE, Level 10, Damage 2d3
63. Dagger — ITEM_IDX_DAGGER, Level 1, Damage 1d4
64. Glaive — ITEM_IDX_POLEARM, Level 10, Damage 2d5, ITEM_FLAG_2HANDED
65. Halberd — ITEM_IDX_POLEARM, Level 10, Damage 2d5, ITEM_FLAG_2HANDED
66. Hand Axe — ITEM_IDX_AXE, Level 7, Damage 2d4
67. Lance — ITEM_IDX_SPEAR, Level 10, Damage 3d6
68. Long Bow — ITEM_IDX_BOW, Level 5, Damage 1d8, ITEM_FLAG_NEEDSAMMO
69. Long Sword — ITEM_IDX_SWORD, Level 1, Damage 1d10
70. Mace — ITEM_IDX_MACE, Level 10, Damage 1d8
71. Morning Star — ITEM_IDX_MACE, Level 10, Damage 2d4
72. Quarter Staff — ITEM_IDX_POLEARM, Level 10, Damage 1d6, ITEM_FLAG_2HANDED
73. Scimitar — ITEM_IDX_SWORD, Level 10, Damage 1d8
74. Spear — ITEM_IDX_SPEAR, Level 10, Damage 2d4
75. Trident — ITEM_IDX_POLEARM, Level 10(?), Damage 3d4, ITEM_FLAG_2HANDED
76. War Hammer — ITEM_IDX_MACE, Level 10, Damage 1d5, ITEM_FLAG_2HANDED

---

## All Named Items Mentioned (Not Yet In Items.txt)

### Specific Items Referenced Across All Sources

**Potions (not yet in Items.txt)**:
- Potion of Gain Strength (EFFECT_FLAG_STAT + EFFECT_TYPE_GAIN) — #77 comment, #197 comment
- Potion of Restore Strength (EFFECT_FLAG_STAT + EFFECT_TYPE_RESTORE) — #77 comment, #197 comment
- Potion of Weakness (EFFECT_FLAG_STAT + EFFECT_TYPE_LOSE) — #77 comment
- Potion of Heroism (EFFECT_FLAG_STAT + EFFECT_TYPE_TIMED, EFFECT_FLAG_HP + EFFECT_TYPE_TIMED) — #77 comment
- Potion of See Invisible (EFFECT_FLAG_INVISIBLE + EFFECT_MOD_SEE + EFFECT_TYPE_TIMED) — #77 comment
- Potion of Flames (EFFECT_FLAG_FIRE + EFFECT_TYPE_HIT, EFFECT_FLAG_COLD + EFFECT_MOD_WEAK) — #77 comment
- Potion of Gain CON/DEX/INT/WIS/CHA — #197 comment (all 6 stats)
- Potion of Restore CON/DEX/INT/WIS/CHA — #197 comment (all 6 stats)
- Potion of Apple Juice — #121 comment (always identified, flavor)
- Potion of Remove Curse — #243 comment (Town: "Potions of Remove Curse" in Player House storage)

**Scrolls (not yet in Items.txt)**:
- Scroll of Blessing (EFFECT_FLAG_AC + EFFECT_TYPE_TIMED) — #77 comment
- Scroll of Door/Stair Location (EFFECT_FLAG_DOOR + EFFECT_MOD_SEE + EFFECT_TYPE_CAUSE) — #77 comment, #117
- Scroll of Trap Detection (EFFECT_FLAG_TRAP + EFFECT_MOD_SEE + EFFECT_TYPE_CAUSE) — #77 comment, #117
- Scroll of Trap Creation (EFFECT_FLAG_TRAP + EFFECT_TYPE_CAUSE) — #77 comment
- Scroll of Detect Monsters (EFFECT_FLAG_MONSTERS) — #270
- Scroll of Enchant Weapon to Hit — #128
- Scroll of Enchant Weapon Damage — #128
- Scroll of Enchant Armor — #128
- Scroll of *Enchant Weapon* (star-enchant, +1d3 both) — #128
- Scroll of *Enchant Armor* (star-enchant, +1d3 AC + random intrinsic) — #128
- Scroll of *Identify* (star-identify, full lore reveal) — #114, #128
- Scroll of Curse Object — #128
- Scroll of Recharging — #121
- Scroll of Restoration — #243 (Temple shop sells these)
- Scroll of Darkness — #243 comment (Vampire PCs use to enter town)
- Scroll of Create Traps — #114 comment ("you are now surrounded by 9 invisible traps")

**Rings (not yet in Items.txt)**:
- Ring of Searching (adds % to detect) — #117
- Ring of Fate — #243 comment (Magic Shop "great item")
- Sustain [Stat] Ring — #197 comment, #244

**Equipment (not yet in Items.txt)**:
- Boots of Speed (Haste +10 permanently) — #244
- Cloak of Protection (AC bonus) — #114 comment ("Cloak of Protection (1, +10) gives 11 AC")
- Gloves of Dexterity (adds % search) — #117
- Helmet of Lordly Protection (EFFECT_FLAG_FIRE + EFFECT_MOD_IMMUNE) — #77 comment, #270
- Holy Symbol — #239 (Priest class equipment)
- Nature Focus — #239 (Druid class equipment)
- Ki Focus — #239 (Monk class equipment)
- Sustain [Stat] Armor — #244
- Amulet of Sensing — #243 comment (Magic Shop "meh item")
- Amulet of the Magi — #128 (Legendary constellation)

**Staves (not yet in Items.txt)**:
- Staff of Light (lights room, radius 15) — #72, #270
- Staff of Starlight (shoots rays + area light) — #72, #270
- Staff of Word of Recall (reusable, no charges) — #243
- Staff of Identify — #243 comment (Player House storage)

**Wands (not yet in Items.txt)**:
- Wand of Heal Monster — #114 comment ("The Kobold screams in agony" on zap)

**Weapons — Ego Items (not yet in Items.txt)**:
- Flametongue [FT] — 2x damage vs fire-weak creatures — #128
- Frost Brand [FB] — #128, WORKLIST.txt
- Demon Bane [DB] — #128, WORKLIST.txt
- Slay Dragon [SD] — #128
- Slay Beast — #114 comment ("Dagger of Slay Beast (+1, +5)")
- Resist Lightning [RL] — #128 (armor ego)
- Westernesse [WB] — #128, WORKLIST.txt (weapon legendary constellation)
- Holy Avenger [HA] — #128, WORKLIST.txt (mace legendary)
- Defender [DF] — #128, WORKLIST.txt (halberd legendary)
- Morgul Blade [MB] — #128 (cursed ego: weakness to holy, drains life, ?Remove Curse must be blessed)
- Longsword of Slay Demon — #243 comment (Weaponsmith Ego weapon example)

**Weapons — Unique Items (not yet in Items.txt)**:
- Sting — unique Dagger (+2 speed, See Invisible, orc warning, orc glow, 2x orc damage) — #128, #114 comment
- Menthir — unique Helm — #128
- Excalibur — unique Bastard Sword (carried by King Arthur) — #128
- Lady Teldra — unique soul-bonded blade (personality, spell absorption, peaceful creature refusal) — #43

**Food (not yet in Items.txt)**:
- (No food items currently exist. ITEM_IDX_FOOD type defined but empty.)

**Books (not yet in Items.txt)**:
- 4 progressively rare books per spellcaster class — #239

**Ammunition (not yet in Items.txt)**:
- Arrows (ITEM_IDX_ARROW defined but no items)
- Bolts (ITEM_IDX_BOLT defined but no items)

**Money (not yet in Items.txt)**:
- (ITEM_IDX_MONEY type defined but no items)

**Chests (not yet in Items.txt)**:
- (ITEM_IDX_CHEST type defined but no items)

**Belts (not yet in Items.txt)**:
- Hill Giant Strength belt — #128 (ego item)
- (ITEM_IDX_BELT type defined but no items)

**Amulets (not yet in Items.txt)**:
- (ITEM_IDX_AMULET type defined but no items beyond Amulet of the Magi and Amulet of Sensing references)

---