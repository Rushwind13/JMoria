# Phase 3 — COMPLETE ✅

**Repository:** `Rushwind13/JMoria`  
**Status as of April 25, 2026:** All Phase 3 issues closed and scoped work completed or transferred to blocker issues.

Phase 3 covered three foundational system design issues: #72 (Fog of War), #77 (Item Effects Framework), and #114 (Item Identification).

---

## Issue #72 — Fog of War ✅ COMPLETE (Closed April 25, 2026)

**Status:** All actionable work completed or properly transferred to blocking issues.

### Completed Work ✅
- ✅ Fog of war foundation (DUNG_FLAG_VISIBLE, UpdateVisibility(), FOW rendering)
- ✅ **Torches** — light radius 3, duration 3000 turns
- ✅ **Lanterns** — light radius 5, refueling with Flask of Oil (+5000 per can, limit 15000)
- ✅ **Wand of Light** — fully implemented with light ray + light area effects
- ✅ **Staff of Light** — Light Area effect (shares with Scroll of Light, radius 10)
- ✅ **Staff of Starlight** — Light Ray + Light Area effects  
- ✅ **Blue light damage** — complete (MON_FLAG_HURT_BY_LIGHT damages orcs, vampires, worm masses)

### Work Transferred to Blocking Issues
- 🔀 **Infravision** (race intrinsic, radius 8, sees warm creatures) → [#112 (Add Races)](https://github.com/Rushwind13/JMoria/issues/112)
- 🔀 **ESP** (race intrinsic, radius 8, detects brains) → [#112 (Add Races)](https://github.com/Rushwind13/JMoria/issues/112)
- 🔀 **Spell of Light Area** (mage spell) → [#239 (Spells system)](https://github.com/Rushwind13/JMoria/issues/239)

### Related Tracking Issues
- [#166](https://github.com/Rushwind13/JMoria/issues/166): Wand of Light light area rendering bug
- [#128](https://github.com/Rushwind13/JMoria/issues/128): Staff of Starlight detailed light ray mechanics

---

## Issue #77 — Item Effects Framework ✅ COMPLETE (Closed April 24, 2026)

**Status:** Framework and data model complete. Implementation work reorganized to blocker issues.

### Framework Complete ✅
- ✅ **Effect system design** — EFFECT_FLAG_x (fire, cold, identify, AC, etc.), EFFECT_MOD_x (resist, immune, weak, see, gain, restore, lose, timed, etc.), EFFECT_TYPE_x (intrinsic, timed, hit, heal, cause, gain, restore, lose)
- ✅ **150 items fully defined** — All items in Items.txt with named effect references (zero inline effects)
- ✅ **98 named effects** — All effects in Effects.txt
- ✅ **Monster attacks migrated** — All monster attack definitions use named effect references (complete 2026-04-21)

### Work Transferred to Blocking Issues
Work originally in #77 scope has been reorganized per latest cleanup (April 24, 2026):

**→ [#197 (Player Stats System)](https://github.com/Rushwind13/JMoria/issues/197):**
- Potion of Gain/Restore/Weakness (stat effects)
- Potion of Heroism (stat + HP)
- Yellow/Green Worm Mass stat attacks (DEX drain, CHA drain)

**→ [#272 (Visible Monsters UI)](https://github.com/Rushwind13/JMoria/issues/272):**
- Scroll of Detect Monsters integration

**→ [#274 (Trap System)](https://github.com/Rushwind13/JMoria/issues/274):**
- Scroll of Trap Creation

**→ [#128 (Magic Items Framework)](https://github.com/Rushwind13/JMoria/issues/128):**
- Scroll of Blessing (timed AC bonus)

**→ [#271 (Item Destruction)](https://github.com/Rushwind13/JMoria/issues/271):**
- Potion of Flames (fire damage + shatter)
- Red/Blue/Green Worm Mass item destruction effects

**→ [#244 (Status Effects System)](https://github.com/Rushwind13/JMoria/issues/244):**
- Potion of See Invisible (needs MON_FLAG_INVISIBLE)
- Potion of Blindness
- White Worm Mass poison touch
- Green Dragon poison breath
- Ghost paralyze touch

### Framework Status
Framework and design is complete as of April 24, 2026. Individual effect implementations are tracked by:
- Code PRs implementing effect handlers
- Separate blocker issues if effect categories need dedicated work

---

## Issue #114 — Item Identification ✅ COMPLETE (Closed April 24, 2026)

**Status:** Core feature (partial stack split) fully implemented. Remaining work transferred to blocker issues.

### Completed Work ✅
- ✅ **Partial stack split** — "How many? (1-N, * for all):" prompt on drop/sell
- ✅ **One-at-a-time arrow firing** — Arrows consumed one per shot, partial stacks remain
- ✅ **Ground overflow handling** — Items overflow to adjacent tiles when current tile occupied, fallback to ground stacking
- ✅ **CItem::Copy(quantity)** method for stack cloning
- ✅ **UseState quantity prompt** — Numeric input (0-9) and Shift+8 (*) support
- ✅ Implementation complete, tested, and merged (commit 19ae21e)

### Work Transferred to Blocker Issues

**→ [#128 (Magic Items Framework)](https://github.com/Rushwind13/JMoria/issues/128):**
- Feeling tiers (passive discovery `{magical}`, `{excellent}`, `{special}`)
- Blessed item status
- Scroll of *Identify* (star-identify for full lore)
- Ego/Legendary/Unique tier system

**→ [#239 (Classes & Spells)](https://github.com/Rushwind13/JMoria/issues/239):**
- Mage ID spells (ID level 15, *ID* level 37)
- Class-specific feelings (Warriors/Mages/Priests)

---

## Item Data Status ✅

All 150 items in Items.txt using named effect references. 98 named effects in Effects.txt. Monster attacks migrated to named effects (complete 2026-04-21).

**Supporting Documentation:**
- [#270 - Item Design & Content](https://github.com/Rushwind13/JMoria/issues/270)
- [WORKLIST_items_design.md](WORKLIST_items_design.md)
- [WORKLIST_monster_effects.md](WORKLIST_monster_effects.md)

---

## Phase 3 → Phase 4 Roadmap

All Phase 3 design and foundational work is complete. Implementation continues through dependent blocker issues organized by system:

**Immediate Blockers (for other features):**
- [#128 - Magic Items Framework](https://github.com/Rushwind13/JMoria/issues/128) — Enchantment system, ego/legendary/unique tiers, feeling tiers
- [#197 - Player Stats System](https://github.com/Rushwind13/JMoria/issues/197) — Stat gain/lose/restore, encumbrance, CHA prices
- [#239 - Classes & Spells](https://github.com/Rushwind13/JMoria/issues/239) — Spell system, class abilities, mage ID spells
- [#243 - Town System](https://github.com/Rushwind13/JMoria/issues/243) — Shop integration, Word of Recall

**Supporting Systems:**
- [#244 - Status Effects System](https://github.com/Rushwind13/JMoria/issues/244) — Timed effects (blind, paralyze, poison, fear)
- [#271 - Item Destruction](https://github.com/Rushwind13/JMoria/issues/271) — Elemental damage to inventory
- [#272 - Visible Monsters UI](https://github.com/Rushwind13/JMoria/issues/272) — Detection magic
- [#274 - Trap System](https://github.com/Rushwind13/JMoria/issues/274) — Trap creation and detection


