# Phase 3 Remaining Work — Deep Systems

Phase 3 covers Issues #72 (Fog of War), #77 (Item Effects), and #114 (Item Identification).
Foundation commit landed — this tracks what's left before Phase 3 is done.

**Item Data Status** ✅: All 150 items in Items.txt using named effect references. 98 named effects in Effects.txt. Monster attacks migrated to named effects (complete 2026-04-21). References: [#270 - Item Design & Content](https://github.com/Rushwind13/JMoria/issues/270) | [WORKLIST_items_design.md](WORKLIST_items_design.md) | [WORKLIST_monster_effects.md](WORKLIST_monster_effects.md)

---

## Issue #72 — Fog of War

### Remaining — No Blockers

#### Light Source Mechanics
- [ ] **Lanterns** — light radius 5, refuel with oil, +5000 per oil can, limit 15000. Lantern fuel item partially scaffolded in Phase 2 (`USE_FUEL`). Integration with `UpdateVisibility()`.
- [ ] **Light radius extends visibility** — Extend `UpdateVisibility()` to use torch/lantern radius; integrate Staff of Light / Staff of Starlight item effects.
- [ ] **Staff of Light / Staff of Starlight** — Integrate light source effect items with visibility system.

### Blocked — Requires Spell/Magic System
- [ ] **Spell of Light Area** — mage spell (not scroll). Requires magic/spell system.

---

## Issue #77 — Item Effects

### Item Data ✅ (150 items, 98 named effects)
All items in Items.txt using named effect references (zero inline effects).

### Remaining — No Blockers

#### Core Effect Dispatch
- [ ] **Timed AC bonus** (`EFFECT_FLAG_AC` + `EFFECT_MOD_TIMED`) — Scroll of Blessing, Staff of Protection. Implement `DoIntrinsicEffects` AC path + round-down timer.
- [ ] **DoHitEffects handler for damage types** — Potion of Flames (fire hit). Implement elemental damage routing.

#### Monster Effect Dispatch (Phase 5)
- [ ] **EFFECT_TYPE_INTRINSIC status effects** — Player.cpp dispatch for Paralyze/Confuse/Sleep/Fear/Blind/Poison. Each needs a timed intrinsic bit and per-turn handler.
- [ ] **XP drain scaling** — Monster touch/attack XP loss calculation based on attacker level/depth.
- [ ] **Dragon breath damage scaling** — Breath attack damage based on dragon HD vs player stats.

### Remaining — Requires Additional Systems

#### Blocked by Enchant System (Issue #128)
- [ ] **Enchantment system** — `+1/+1d3` per scroll, failure chance above +10, scroll wasted on failure. Implements Scroll of Enchant Weapon (To Hit), Scroll of Enchant Weapon (Damage), Scroll of Enchant Armor, Scroll of *Enchant Weapon*, Scroll of *Enchant Armor*.

#### Blocked by Ego/Unique System (Issue #128)
- [ ] **Item spawn quality chain** — Ego → Legendary → Unique tiers on top of Normal/Cursed/Magic path. Requires ego/legendary/unique identity field on CItem.
- [ ] **Scroll of *Identify*** — full lore reveal for ego/unique items (Level 30+, Mage spell level 37).

#### Blocked by Stats System (#197)
- [ ] **Potion of Heroism** — stat boost + temporary HP (requires stat gain/restore/lose mechanics).
- [ ] **Stat-flag items** — Gain/Restore/Weakness potions, Sustain rings, Sustain armor, Gloves of Dexterity (all require stat system).
- [ ] **Sustain stat mechanics** — restore stat to max achieved + prevent stat damage.
- [ ] **EFFECT_FLAG_STAT** — stat gain/restore/lose dispatch (requires stats system).
- [ ] **EFFECT_MOD_SUSTAIN** — define + string table entry in Constants.cpp.

#### Blocked by Classes System (#239)
- [ ] **Spell books** — 4 per caster class (ITEM_IDX_BOOK). Requires class/spell system.
- [ ] **Class equipment** — Holy Symbol (Priest), Nature Focus (Druid), Ki Focus (Monk).
- [ ] **Mage ID spell** — level 37 (requires spell/magic system).

#### Blocked by Town System (#243)
- [ ] **Staff of Word of Recall** — full round-trip loop requires town (implemented in #243).
- [ ] **Scroll of Restoration / Scroll of Darkness** — Town-gated items.
- [ ] **Shopkeeper pricing as identification** — selling to shop reveals item identity (bad price for good, good price for cursed).

#### Blocked by Trap System (#117)
- [ ] **Scroll of Trap Creation** — CREATE + TRAP effect.

#### Blocked by Ranged Attacks (PR #235)
- [ ] **Item destruction from elemental attacks** — inventory scan for material vulnerability (Red Worm fire, Green Worm acid, etc.). Requires projectile effect callbacks.

#### Blocked by MON_FLAG_INVISIBLE Feature
- [ ] **Potion of See Invisible** — TIMED + INVISIBLE + SEE effect.
- [ ] **Ring of See Invisible** — permanent intrinsic.

### Data Model Updates
- [ ] **Ego/legendary/unique identity field** on CItem (Item-Design.md §1, §8–§10).
- [ ] **Remove dead field** — CItemDef::m_dwBaseHP (initialized to 0, never populated).

---

## Issue #114 — Item Identification

### Remaining — Core (No Blockers)

#### Feeling Tiers (Passive Discovery)
- [ ] **Feeling tiers** — passive per-turn chance of `{magical}` (has bonuses), `{excellent}` (ego item like Slay Beast), `{special}` (unique like "Sting"). Displayed as "a Dagger {excellent}" before full ID.

### Stacking & Item Management

#### Partial Stack Split
- [ ] **Stacking identity check** — ITEM_FLAG_STACKS exists; complete per-category rules.
- [ ] **Partial stack split** — "How many? (1-n)" prompt on drop/sell; arrows fire one at a time.
- [ ] **Ground stacking + loot overflow** — items overflow to adjacent open tiles when pickup limit exceeded.

### Blocked — Requires Stats/Classes/Spells

#### Class-Specific Identification
- [ ] **Class-specific feelings** — Warriors sense weapon curses fast, Mages sense magic fast, Priests sense blessings/curses, Rogues sense traps/AC. Passive per-turn low chance. Requires class system with stat priorities.

#### Advanced Identification
- [ ] **`?*Identify*` (star-identify)** — full lore reveal for unique/ego items. Scroll of *Identify* (level 30+), Mage spell at level 37. Requires ego/unique system.

---

## Supporting Item Features — Blocked

### Blocked by Stats System (#197)
- [ ] **Encumbrance system** — Item weight, inventory carrying capacity, movement speed penalty.
- [ ] **CHA modifier on shopkeeper prices** — charisma affects shop prices.
- [ ] **Worm mass stat drain** — Yellow=-DEX/WIS, Green=-DEX/acid, Black=-CHA assignments (contradictory data in WORKLIST_items_design; resolve in stats-system story).

### Blocked by Trap System (#117)
- [ ] **Rubble treasure chance** — dungeon generation hazard trap integration.

---

## Top Priorities — Execution Order

These are ordered by "unblocks the most other work" and "most visible gameplay impact":

### Tier 1 — Visible Gameplay Impact (No Blockers, Ready Now)
1. **Timed AC bonus** (Issue #77) — `EFFECT_FLAG_AC` + `EFFECT_MOD_TIMED` in DoIntrinsicEffects. Items: Scroll of Blessing, Staff of Protection.
2. **Monster effect dispatch — INTRINSIC status effects** (Issue #77, Phase 5) — Player.cpp: Paralyze, Confuse, Sleep, Fear, Blind, Poison (each needs timed intrinsic bit + per-turn handler).
3. **Item spawn quality chain** (Issue #128) — Ego → Legendary → Unique tiers. Requires ego/legendary/unique identity field on CItem.
4. **Enchantment system** (Issue #128) — `+1/+1d3` per scroll, failure risk above +10. Scaffolds ego item system.
5. **Light radius extends visibility** (Issue #72) — Extend `UpdateVisibility()` to use torch/lantern radius; Staff of Light integration.

### Tier 2 — Item Identification & Management (Mostly No Blockers)
6. **Feeling tiers** (Issue #114) — Passive discovery `{magical}` / `{excellent}` / `{special}` display.
7. **Partial stack split** (Issue #114) — "How many?" prompt on drop/sell; arrows one-at-a-time.
8. **DoHitEffects for damage types** (Issue #77) — Potion of Flames fire damage routing.

### Tier 3 — Item Destruction & Interaction
9. **Item destruction from elemental attacks** (Issue #77/PR #235) — Inventory scan, material vulnerability (Red Worm/Green Worm/etc.).

### Tier 4 — Advanced Features (Requires Other Systems)
10. **Lantern light mechanics** (Issue #72) — Integrate refuel logic, fuel consumption, light radius.
11. **Stats system items** (#197) — Heroism potion, stat gain/restore/lose, sustain mechanics, encumbrance.
12. **Class-specific feelings** (Issue #114, #239) — Warriors sense weapons, Mages sense magic, Priests sense curses/blessings.
13. **Spell-based items & identification** (Issue #114, #239) — Spell books, Mage ID spell, Spell of Light Area.
14. **`?*Identify*` (star-identify)** (Issue #114, #128) — Full lore reveal for uniques/egos.
15. **Town integration** (Issue #243) — Word of Recall, Restoration scroll, shopkeeper pricing as ID method.

---

## Open Design Questions

Require explicit answers before implementation.

- [ ] **Resistance stacking formula** — 50% + 25% = 62.5%? (deferred to intrinsics deep-dive)
- [ ] **Ego data file format** — type restrictions, intrinsic list, rarity, min depth
- [ ] **Legendary constellations** — Westernesse, Holy Avenger, Defender, Amulet of the Magi, Morgul Blade — intrinsic sets TBD
- [ ] **Unique item details** — Menthir intrinsics/carrier; Excalibur intrinsics; Lady Teldra personality/lore
- [ ] **Uniques.txt data file format** — singleton persistence, carrier monster, lore text structure
- [ ] **Food content** — item list for ITEM_IDX_FOOD
- [ ] **Book content** — spell lists per caster class (deferred until enough spell effects exist as scrolls/wands/staves)


