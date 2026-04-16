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
- [ ] **`m_dwKnownIntrinsics` on CItem** — per-instance bitmask. As properties are discovered (cursed, magic, bonuses), the relevant FLAG is OR-ed in. This is separate from `m_bIdentified` (which is type-level "I know what this potion is"). `m_dwKnownIntrinsics` is instance-level "I know this specific dagger is cursed".
- [ ] **`ITEM_FLAG_MAGIC`** — new flag to mark items as magical
- [ ] **Display known intrinsics in item name** — `if(known(cursed)) display(item.cursed?"cursed":"uncursed")`, `if(known(magic)) display("(+x, +y)")`. Examples: "a Dagger", "a Dagger {cursed}", "a Dagger (+3, +7)", "a Dagger (+3, +7) (cursed)"
- [ ] **Scroll of ID sets `m_dwKnownIntrinsics`** to all displayable flags (MAGIC|CURSED|bonuses). Currently `Identify()` only sets `m_bIdentified` on CItemDef.
- [ ] **?ID reveals `m_dwCharges`** for wands and staves

#### Unidentified Names / Flavors
- [ ] **Use `m_szUnidentifiedName`/`m_szUnidentifiedPlural`** — fields exist on CItemDef but aren't used. When unidentified, potions show as "a Blue Potion", scrolls as "a Scroll titled 'ABRA CADABRA'", etc. Type-level identification reveals the real name.
- [ ] **`{tried}` flag** — if you use an item but the effect wasn't noticeable enough to auto-ID, mark it "2 Blue Potions {tried}" so you know you've tested it

#### Discovery Mechanics
- [ ] **Cursed discovery via failed remove** — trying to remove a cursed item reveals `ITEM_FLAG_CURSED` in `m_dwKnownIntrinsics`. Display as "a Dagger {cursed}"
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
- [ ] **FOW rendering differentiation** — `DUNG_FLAG_VISIBLE` is set but not consumed by the renderer. Unseen tiles = background color, seen-but-not-currently-visible = dimmed/gray, currently visible = normal bright. The comment on #72 shows the three-state rendering target.

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
- [x] DoCreateEffects: light, teleport (phase door + full), magic mapping

### Remaining — Item Effect Types

#### New Effect Types
- [ ] **`EFFECT_TYPE_TIMED`** — distinct from `EFFECT_TYPE_INTRINSIC` + `EFFECT_MOD_TIMED`. The spreadsheet uses TIMED as a primary type for temporary potions: Potion of Fire Resistance, Potion of Invisibility, Potion of Blindness, Potion of Heroism, temp HP, Scroll of Blessing
- [ ] **`EFFECT_TYPE_CAUSE`** — used for Mapping, Door/Stair Location, Trap Detection, Trap Creation. Currently Magic Mapping uses `EFFECT_TYPE_CREATE` — should it be CAUSE?

#### New Effect Modifiers
- [ ] **`EFFECT_MOD_IMMUNE`** — Helmet of Lordly Protection: fire immune vs resist
- [ ] **`EFFECT_MOD_WEAK`** — weakness flag. Potion of Flames has Cold weakness (potions can shatter if user takes cold damage)
- [ ] **`EFFECT_MOD_SEE`** — See Invisible, Door/Stair Location, Trap Detection

#### New Effect Flags
- [ ] **`EFFECT_FLAG_STAT`** — stat gain/restore/lose/timed. Requires stats system (STR/DEX/CON/INT/WIS/CHA). Potion of Gain Strength, Potion of Restore Strength, Potion of Weakness, Potion of Heroism.
- [ ] **`EFFECT_FLAG_AC`** — timed AC bonus (Scroll of Blessing)
- [ ] **`EFFECT_FLAG_SUMMON`** — Scroll of Summon Monsters: summons high-level monsters surrounding the character
- [ ] **`EFFECT_FLAG_DOOR`** + `EFFECT_MOD_SEE` — Scroll of Door/Stair Location: reveals doors and stairs
- [ ] **`EFFECT_FLAG_TRAP`** — detect (with `EFFECT_MOD_SEE`) and create traps. Requires trap system.

#### Multi-Effect Items
- [ ] **Multi-effect item definitions in Items.txt** — Potion of Minor Healing (HP gain + cure blind + cure confuse), Potion of Heroism (stat boost + temp HP), Potion of Flames (fire hit + cold weakness). The effect loop processes multiple CEffects per item already, but these items need to be defined.

#### Item Data
- [ ] **Define items in Items.txt** — Ring of Levitation, Ring of Fire Resistance, Ring of Cold Resistance, Potion of Fire Resistance, Potion of Flames, Helmet of Lordly Protection, Potion of Invisibility, Ring of Invisibility, Potion of See Invisible, Potion of Blindness, Potion of Minor Healing, Potion of Gain STR, Potion of Restore STR, Potion of Weakness, Potion of Heroism, Scroll of Blessing, Scroll of Summon Monsters, Scroll of Door/Stair Location, Scroll of Trap Detection, Scroll of Trap Creation

### Remaining — Monster Effects

Monsters need richer attack types beyond simple HP damage. Currently monsters have `Attack` lines in Monsters.txt parsed as effects. The following extends the attack/effect system:

#### Monster Attack Types (new MON_FLAGs)
- [ ] **`MON_FLAG_CRAWL`** — worm mass crawl attack (HP damage)
- [ ] **`MON_FLAG_TOUCH`** — elemental/status touch attacks
- [ ] **`MON_FLAG_CLAW`** — standard melee (dragons, demons)
- [ ] **`MON_FLAG_BITE`** — bite attack
- [ ] **`MON_FLAG_BREATHE`** — breath weapon (elemental, large AoE)
- [ ] **`MON_FLAG_TRAMPLE`** — ancient dragon trample

#### Monster Attack Definitions

| Monster | Attacks | Notes |
|---------|---------|-------|
| White Worm Mass | Crawl (HP), Touch (poison, timed) | Poison touch applies timed poison |
| Red Worm Mass | Crawl (HP), Touch (fire hit) | Fire touch can burn scrolls, potions, leather |
| Blue Worm Mass | Crawl (HP), Touch (cold hit) | Cold touch can shatter potions |
| Green Worm Mass | Crawl (HP), Touch (acid hit + CHA lose) | Acid destroys scrolls, potions, leather, metal. "You feel ugly" |
| Yellow Worm Mass | Crawl (HP), Touch (stat lose DEX) | "You feel clumsy" |
| Baby Red Dragon | Claw (HP), Bite (HP), Breathe (fire, less often, less damage) | |
| Red Dragon | Claw ×2 (HP), Bite (HP), Breathe (fire) | |
| Ancient Red Dragon | Claw ×2 (HP), Bite (HP), Trample (HP), Breathe (fire) | |
| Green Dragon | Claw ×2 (HP), Bite (HP), Breathe (poison) | |
| Ghost | Touch (paralyze, timed) | |
| Greater Demon | Claw ×2 (HP), Bite (HP), Summon (lesser demons) | |

#### Item Destruction from Monster Attacks
- [ ] **Fire damage destroys scrolls, potions, leather items** — equipment damage system
- [ ] **Cold damage shatters potions** — inventory damage on cold hit
- [ ] **Acid damage destroys scrolls, potions, leather, AND metal items** — most destructive element

---

## Top Priorities

These are ordered by "unblocks the most other work" and "most visible gameplay impact":

### Tier 1 — Core Knowledge System
1. **`m_dwKnownIntrinsics` on CItem + display in item name** (#114) — This is the core acceptance criterion for identification. Everything else in #114 builds on top of this. Required before feelings, cursed discovery, or ?*ID* make sense.
2. **Unidentified names / flavors** (#114) — "Blue Potion" vs "Potion of Healing". Without this, auto-ID and Scroll of ID have no visible payoff. The fields already exist on CItemDef.
3. **FOW rendering (three-state)** (#72) — `DUNG_FLAG_VISIBLE` is being set every frame but nothing renders differently. This is the visible payoff for all the visibility work.

### Tier 2 — Effect System Completeness
4. **`EFFECT_TYPE_TIMED` as primary type** (#77) — Needed for temporary potions (resistance, invisibility, heroism, blindness). Many items in the spreadsheet depend on this.
5. **Multi-effect items in Items.txt** (#77) — Potion of Minor Healing (HP + cure blind + cure confuse) is the canonical use case. The loop already works; just need item data.
6. **`{tried}` flag** (#114) — Simple but important UX: if you quaff something and it doesn't auto-ID, mark it tried.

### Tier 3 — Monster Combat Depth
7. **Monster attack type flags** (#77) — MON_FLAG_CLAW/BITE/BREATHE/TOUCH/CRAWL/TRAMPLE. Gives monsters distinct attack flavors beyond "hits".
8. **Elemental monster attacks** (#77) — Fire/cold/acid/poison touch and breath. Requires resist/immune checks.
9. **Item destruction from attacks** (#77) — Fire burns scrolls, acid destroys metal. High-impact gameplay consequence.

### Tier 4 — Advanced Systems (Require Stats, Classes, Spells)
10. **`EFFECT_FLAG_STAT`** (#77) — Requires stat system (STR/DEX/CON/INT/WIS/CHA) to exist first
11. **Class-specific feelings** (#114) — Requires class system with stat priorities
12. **Spell of Light Area / Mage ID spell** (#72, #114) — Requires spell/magic system
13. **`?*Identify*`** (#114) — Requires ego/unique item system
