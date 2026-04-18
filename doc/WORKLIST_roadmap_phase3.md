# Phase 3 Remaining Work — Deep Systems

Phase 3 covers Issues #72 (Fog of War), #77 (Item Effects), and #114 (Item Identification).
Foundation commit landed — this tracks what's left before Phase 3 is done.

---

## Issue #114 — Item Identification

### Done
- [x] `CItemDef::m_bIdentified`, `IsIdentified()`/`Identify()`
- [x] `ITEM_FLAG_IDENTIFIED` per-instance flag
- [x] `GetName()`/`GetPlural()` use `IsIdentified()`
- [x] Auto-identify on use when effect is noticeable (`m_bLastEffectNoticed`)
- [x] Scroll of Identify: single-item picker via `USE_IDENTIFY` in UseState
- [x] `m_bPendingIdentify` flow between DoRestoreEffects and UseState

### Remaining

#### Core (Acceptance Criteria)
- [x] **`m_dwKnownProps` on CItem** — per-instance bitmask (KNOWN_CURSED, KNOWN_BONUSES, KNOWN_CHARGES, KNOWN_TRIED). KnowsProperty()/RevealProperty()/RevealAllProperties(). CItemDef::FormatProperties() builds type-aware display strings (weapons: +hit/+dam, armor: [+AC], wands: charges, cursed: {cursed}).
- [x] **`ITEM_FLAG_MAGIC`** — new flag in Constants.h, added to string table
- [x] **Display known properties in item name** — GetName()/GetPlural() call FormatProperties(). Examples: "a Dagger", "a Dagger {cursed}", "a Dagger (+3, +7)", "a Wand of Light (5 charges)"
- [x] **Scroll of ID sets all known properties** — Identify() calls RevealAllProperties()
- [x] **?ID reveals `m_dwCharges`** for wands and staves via KNOWN_CHARGES

#### Unidentified Names / Flavors
- [x] **Use `m_szUnidentifiedName`/`m_szUnidentifiedPlural`** — FileParse generates randomized names at load. GetName()/GetPlural() use them when unidentified.
- [x] **`{tried}` flag** — KNOWN_TRIED on CItemDef (m_bTried). Use without auto-ID marks items "{tried}"

#### Discovery Mechanics
- [x] **Cursed discovery via failed remove** — RevealProperty(KNOWN_CURSED) on failed equipment remove. Display as "a Dagger {cursed}"
- [ ] **Feeling tiers** — passive discovery over time: "magical" (has bonuses), "excellent" (ego item like Slay Beast), "special" (unique like "Sting"). Displayed as "a Dagger {excellent}" before full ID
- [ ] **Class-specific feelings** — Warriors sense weapon curses fast, Mages sense magic fast, Priests sense blessings/curses, Rogues sense traps/AC. Low chance per turn like passive searching. Requires class/stat system.
- [ ] **Cursed/uncursed/blessed states** — three-state system with bonuses/penalties between versions of the same item

#### Advanced
- [ ] **`?*Identify*` (star-identify)** — full lore reveal for unique/ego items. Scroll of *Identify* at level 30+. Mage spell at level 37.
- [ ] **Shopkeeper pricing as identification** — selling to a shop reveals what the item is (bad price for good stuff, good price for cursed stuff)

---

## Issue #72 — Fog of War

### Done
- [x] `DUNG_FLAG_VISIBLE` flag
- [x] `UpdateVisibility()` — clears/sets VISIBLE based on LOS from player
- [x] Called from `CDungeon::Update()` after `UpdateSeen()`

### Remaining

#### Rendering
- [x] **FOW rendering differentiation** — Seen-but-not-visible tiles render as dim grey (60,60,80). Currently visible tiles render normally. Unseen tiles not drawn.

#### Light Sources
- [ ] **Lanterns** — light radius 5, refuel with oil, +5000 per oil can, limit 15000. Lantern fuel item partially scaffolded in Phase 2 (`USE_FUEL`).
- [ ] **Staff of Light** — lights current room, radius 15
- [ ] **Staff of Starlight** — shoots rays of blue light in all directions AND casts light area
- [ ] **Wand of Light** — casts a line of blue light (uses existing wand/ray system)

#### Racial Sight
- [ ] **Infravision** — racial intrinsic, radius 8. Only works on warm-blooded creatures (not worm masses, icky things). `EFFECT_FLAG_INFRA` and `SIGHT_DISTANCE_INFRA` exist but no race assignment or creature temperature filter.
- [ ] **ESP** — racial intrinsic, radius 8. Only detects creatures with brains (not undead, not worm masses). `EFFECT_FLAG_ESP` and `SIGHT_DISTANCE_ESP` exist but no race assignment or creature filter.

#### Light Damage
- [ ] **Blue light damages creatures** — orcs, vampires, worm masses vulnerable. Needs `MON_FLAG_LIGHT_VULNERABLE` or similar.

#### Spells
- [ ] **Spell of Light Area** — mage spell (not scroll). Requires magic/spell system.

---

## Issue #77 — Item Effects

### Done
- [x] `CEffect::m_dwFlags` changed from `int` to `uint32`
- [x] DoIntrinsicEffects: afraid, blind, confuse, poison, paralyze, sleep, infra, ESP, fire/cold/elec/acid resist, invisible, levitate, free action, speed, light
- [x] UndoIntrinsicEffects: matching undo messages for all
- [x] DoGainEffects: fuel, XP
- [x] DoLoseEffects: XP, HP (max reduction)
- [x] DoRestoreEffects: identify, HP (conditional)
- [x] DoCreateEffects: light, teleport (phase door + full), magic mapping, recall, summon monsters

### Remaining — Item Effect Types

#### Effect Type Decisions (resolved)
- [x] **`EFFECT_TYPE_TIMED` NOT needed** — use `EFFECT_TYPE_INTRINSIC` + `EFFECT_MOD_TIMED` instead. Was a spreadsheet typo.
- [x] **`EFFECT_TYPE_CAUSE` NOT needed** — Mapping stays `EFFECT_TYPE_CREATE`. Detection effects will use proposed `EFFECT_TYPE_SEE` (reserved at 0x100 in Constants.h).
- [x] **`EFFECT_TYPE_SEE`** — reserved as comment. "SEE changes what you know, CREATE changes the world." Will handle: mapping, identify, detect doors/traps/monsters.

#### Effect Modifiers (all exist in Constants.h)
- [x] **`EFFECT_MOD_IMMUNE`** (0x04) — Helmet of Lordly Protection: fire immune vs resist
- [x] **`EFFECT_MOD_WEAK`** (0x08) — weakness flag. Potion of Flames has Cold weakness
- [x] **`EFFECT_MOD_SEE`** (0x02) — See Invisible, Door/Stair Location, Trap Detection
- [x] **`EFFECT_MOD_TIMED`** (0x10) — temporary effect duration
- [x] **`EFFECT_MOD_AREA`** (0x20), **`EFFECT_MOD_LINE`** (0x40), **`EFFECT_MOD_BALL`** (0x80) — shape modifiers
- [x] **`EFFECT_MOD_ENCHANT`** (0x100) — enchantment modifier

#### Effect Flags — Word 1 (EFFECT_FLAG, 32/32 bits allocated)
All 32 flags exist and are in the string table. Includes: FIRE, COLD, ELEC, ACID, POISON, AFRAID, BLIND, CONFUSE, PARALYZE, SLEEP, INFRA, ESP, INVISIBLE, LEVITATE, FREE_ACTION, SPEED, LIGHT, RECALL, TELEPORT, STONE_TO_MUD, IDENTIFY, MAPPING, SUMMON, AC, STAT, HP, FUEL, XP, FOOD, SEE_INVIS, INTRINSIC, RESIST.

#### Effect Flags — Word 2 (EFFECT_FLAG2, 3/32 bits used) ✅
- [x] **`EFFECT_FLAG_DOOR`** (0x01) + `EFFECT_MOD_SEE` — Scroll of Door/Stair Location
- [x] **`EFFECT_FLAG_TRAP`** (0x02) — detect (with `EFFECT_MOD_SEE`) and create traps
- [x] **`EFFECT_FLAG_MONSTERS`** (0x04) — detect monsters
- [x] **`m_dwFlags2` on CEffect** — second 32-bit word, parsed transparently by `LookupEffectFlag()`
- [x] **`LookupEffectFlag()` / `EffectFlagToString()`** — convenience methods route between words invisibly
- [x] **7 test scenarios** in effects.feature covering lookup, routing, and round-trip

#### Flags Blocked by Other Systems
- [ ] **`EFFECT_FLAG_STAT`** — exists in word 1, but stat gain/restore/lose requires stats system (#197)
- [ ] **`EFFECT_FLAG_AC`** — exists in word 1, but timed AC bonus requires implementation in DoIntrinsicEffects

#### Multi-Effect Items
- [x] **Potion of Minor Healing** — already existed with HP + cure poison + cure blind (3 effects)
- [ ] **Potion of Heroism** — stat boost + temp HP. Blocked by stats system (#197).
- [ ] **Potion of Flames** — fire hit + cold weakness. Needs DoHitEffects handler for fire.

#### Item Data
See [#270 - Item Design & Content](https://github.com/Rushwind13/JMoria/issues/270) for comprehensive item inventory and design document.

Items already added (19 new, 76 total):
- [x] **Potions** (11 new): Resist Cold, Resist Acid, Resist Electricity, Invisibility, Speed, Levitation, Blindness, Confusion, Poison, Cure Poison, Infravision
- [x] **Rings** (5 new): Cold Resistance, Acid Resistance, Electricity Resistance, Free Action, Speed, Telepathy
- [x] **Helmets** (2 new): Helm of Infravision, Helm of Telepathy
- [x] **Scrolls** (1 new): Scroll of Recall, Scroll of Summon Monsters, Scroll of Light, Scroll of Telepathy
- [x] **Light sources**: Torch (3), Brass Lantern (5)

Items NOT yet added (blocked by missing systems):
- [ ] Wand of Light, Staff of Light, Staff of Starlight — light source integration
- [ ] Flask of Oil — refuel consumable
- [ ] Potion of See Invisible — needs invisible monster system (no MON_FLAG_INVISIBLE yet)
- [ ] Potion of Gain STR / Restore STR / Weakness — needs stats system (#197)
- [ ] Potion of Heroism — needs stats system (#197)
- [ ] Potion of Flames — needs elemental hit handler in DoHitEffects
- [ ] Helmet of Lordly Protection — needs IMMUNE vs RESIST distinction in combat
- [ ] Scroll of Blessing — needs timed AC (CEffect copy doesn't deep-copy m_szAmount for rolled value storage)
- [ ] Scroll of Detect Doors/Stairs/Traps — needs EFFECT_TYPE_SEE implementation

### Remaining — Monster Effects

Monsters need richer attack types beyond simple HP damage. Currently monsters have `Attack` lines in Monsters.txt parsed as effects. The following extends the attack/effect system:

#### Monster Attack Types (6 MON_FLAGs — already in Constants.h ✅)
These are all defined, in the string table, and already used in Monsters.txt Attack lines.
- [x] **`MON_FLAG_CRAWL`** (0x80) — worm mass crawl attack (HP damage)
- [x] **`MON_FLAG_TOUCH`** (0x02) — elemental/status touch attacks
- [x] **`MON_FLAG_CLAW`** (0x10) — standard melee (dragons, demons)
- [x] **`MON_FLAG_BITE`** (0x04) — bite attack
- [x] **`MON_FLAG_BREATHE`** (0x40) — breath weapon (elemental, large AoE)
- [x] **`MON_FLAG_TRAMPLE`** (0x20) — ancient dragon trample

#### Monster Attack Definitions ✅

All monster definitions complete and in [Resources/Monsters.txt](Resources/Monsters.txt):

| Monster | Attacks | Notes |
|---------|---------|-------|
| White Worm Mass | Crawl (HP), Touch (poison, timed) | ✅ EXISTS |
| Red Worm Mass | Crawl (HP), Touch (fire hit) | ✅ EXISTS |
| Blue Worm Mass | Crawl (HP), Touch (cold hit) | ✅ EXISTS |
| Green Worm Mass | Crawl (HP), Touch (acid hit + CHA lose) | ✅ EXISTS |
| Yellow Worm Mass | Crawl (HP), Touch (stat lose DEX) | ✅ EXISTS |
| Baby Red Dragon | Claw (HP), Bite (HP), Breathe (fire) | ✅ EXISTS (Level 15) |
| Red Dragon | Claw ×2 (HP), Bite (HP), Breathe (fire) | ✅ EXISTS (Level 45) |
| Ancient Red Dragon | Claw ×2 (HP), Bite (HP), Trample (HP), Breathe (fire) | ✅ EXISTS (Level 70) |
| Green Dragon | Claw ×2 (HP), Bite (HP), Breathe (poison) | ✅ EXISTS (Level 35) |
| Ghost | Touch (paralyze, timed) | ✅ EXISTS (Level 15) |
| Greater Demon | Claw ×2 (HP), Bite (HP), Summon (lesser demons) | ✅ EXISTS (Level 30) |

#### Item Destruction from Monster Attacks

**See [#271 - Item Destruction from Elemental Attacks](https://github.com/Rushwind13/JMoria/issues/271)** — Moved to Phase 3+ (postponed until after Ranged Attacks PR #235)

---

## Top Priorities

These are ordered by "unblocks the most other work" and "most visible gameplay impact":

### Tier 1 — Core Knowledge System ✅
1. ~~**`m_dwKnownProps` on CItem + display in item name**~~ (#114) — Done. FormatProperties() with type-aware templates.
2. ~~**Unidentified names / flavors**~~ (#114) — Done. FileParse randomized names + {tried} marking.
3. ~~**FOW rendering (three-state)**~~ (#72) — Done. Dim grey for seen-not-visible.

**Also fixed**: LOS Bresenham was passing sight_distance instead of actual target distance, causing line to extend past target into walls. Fixed in CanSeeEachOther() and UpdateVisibility(). All 12 targeting/ranged tests now pass.

### Tier 2 — Effect Vocabulary + Item Content ✅ (vocabulary done)
The effect vocabulary (types, modifiers, flags, second bitmask) is complete. Remaining work is item content:
4. ~~**`EFFECT_TYPE_TIMED`**~~ — Resolved: use `EFFECT_TYPE_INTRINSIC` + `EFFECT_MOD_TIMED`. Not a separate type.
5. ~~**EFFECT_FLAG2 second bitmask**~~ — Done. DOOR, TRAP, MONSTERS + LookupEffectFlag/EffectFlagToString.
6. ~~**`{tried}` flag**~~ (#114) — Done (moved to Tier 1).
7. **Multi-effect items in Items.txt** (#77) — Potion of Minor Healing (HP + cure blind + cure confuse) is the canonical use case. The loop already works; just need item data.
8. **Define ~20 new items in Items.txt** (#77) — Potions, scrolls, rings using existing vocabulary. No code changes needed for most.

### Tier 3 — Monster Combat Depth ✅ (content complete, code TBD)
~~**Prerequisite**: Add 6 new `MON_FLAG_*` constants~~ — Already done (CRAWL, TOUCH, CLAW, BITE, BREATHE, TRAMPLE all in Constants.h and Monsters.txt).
9. ~~**New monster definitions in Monsters.txt**~~ (#77) — DONE. All dragons, worm masses, ghosts, demons fully defined with attack lines. See [Resources/Monsters.txt](Resources/Monsters.txt) and [doc/Monster-Design.md](Monster-Design.md).
10. **Elemental resistances in combat** (#77) — Fire/cold/acid/poison resist/immune checks. Potion of Fire Resistance exists; extend to all elemental types.
11. **Item destruction from attacks** — See [#271](https://github.com/Rushwind13/JMoria/issues/271). Postponed until after Ranged Attacks (PR #235).

### Tier 4 — Light Economy Integration
12. **Light radius extends visibility** (#72, #121) — UpdateVisibility() uses torch Radius to extend sight range
13. **Light source items** (#270) — Wand/Staff of Light, Flask of Oil for refueling
14. **Infravision/ESP integration** (#72) — UpdateVisibility() checks for EFFECT_FLAG_INFRA/ESP on player

### Tier 5 — Advanced Systems (Require Stats, Classes, Spells)
15. **`EFFECT_FLAG_STAT`** (#77) — Requires stat system (STR/DEX/CON/INT/WIS/CHA) to exist first
16. **Class-specific feelings** (#114) — Requires class system with stat priorities
17. **Spell of Light Area / Mage ID spell** (#72, #114) — Requires spell/magic system
18. **`?*Identify*`** (#114) — Requires ego/unique item system
