# Phase 3 Remaining Work — Deep Systems

Phase 3 covers Issues #72 (Fog of War), #77 (Item Effects), and #114 (Item Identification).
Foundation commit landed — this tracks what's left before Phase 3 is done.

---

## Issue #114 — Item Identification

### Remaining

#### Core (Acceptance Criteria)
- [ ] **Feeling tiers** — passive discovery over time: "magical" (has bonuses), "excellent" (ego item like Slay Beast), "special" (unique like "Sting"). Displayed as "a Dagger {excellent}" before full ID
- [ ] **Class-specific feelings** — Warriors sense weapon curses fast, Mages sense magic fast, Priests sense blessings/curses, Rogues sense traps/AC. Low chance per turn like passive searching. Requires class/stat system.

#### Advanced
- [ ] **`?*Identify*` (star-identify)** — full lore reveal for unique/ego items. Scroll of *Identify* at level 30+. Mage spell at level 37.
- [ ] **Shopkeeper pricing as identification** — selling to a shop reveals what the item is (bad price for good stuff, good price for cursed stuff)

---

## Issue #72 — Fog of War

#### Light Sources
- [ ] **Lanterns** — light radius 5, refuel with oil, +5000 per oil can, limit 15000. Lantern fuel item partially scaffolded in Phase 2 (`USE_FUEL`).
- [ Remaining
#### Spells
- [ ] **Spell of Light Area** — mage spell (not scroll). Requires magic/spell system.

---

## Issue #77 — Item Effects

### Remaining — Item Effect Types
- [x] **`EFFECT_MOD_TIMED`** (0x10) — temporary effect duration
- [ ] **Effect dispatch for `EFFECT_MOD_TIMED` / `EFFECT_MOD_TIMED`** — already resolved in design (use INTRINSIC + MOD_TIMED); needs code implementation routing, and round-trip

#### Flags Blocked by Other Systems
- [ ] **`EFFECT_FLAG_STAT`** — exists in word 1, but stat gain/restore/lose requires stats system (#197)
- [ ] **`EFFECT_FLAG_AC`** — exists in word 1, but timed AC bonus requires implementation in DoIntrinsicEffects

#### Item Data ✅ (150 items, 98 named effects)
See [#270 - Item Design & Content](https://github.com/Rushwind13/JMoria/issues/270) for comprehensive item inventory and design document.

All items are in Items.txt using named effect references (zero inline effects). 150 items total, 98 named effects in Effects.txt.

Recent additions (74 new items across two passes):
Items in Items.txt but effect code blocked by other systems:
- [ ] Scroll of Blessing / Staff of Protection — needs timed AC in DoIntrinsicEffects
- [ ] **Potion of Heroism** — stat boost + temp HP. Blocked by stats system (#197).ess — needs stats system (#197)
- [ ] Potion of Heroism — needs stats system (#197)

### Remaining — Monster Effects

- [ ] **Potion of Flames** — fire hit + cold weakness. Needs DoHitEffects handler for fire.
- [ ] Light source items — Staff of Light, Staff of Starlight need integrationtxt](Resources/Monsters.txt):


#### Monster Effects Migration ✅ DONE 2026-04-21

All monster attacks migrated from inline effect format to named effects. See [WORKLIST_monster_effects.md](WORKLIST_monster_effects.md) for full tracking.

**Phase 5 (Dispatch code) — Deferred**
- [ ] Player.cpp (item 23): dispatch INTRINSIC status effects (Paralyze/Confuse/Sleep/Fear/Blind/Poison)
- [ ] XP drain scaling (item 24)
- [ ] Steal/Pick Pocket mechanic (item 25)
- [ ] Dragon breath damage scaling (item 26)
- [ ] Stat drain dispatch (item 27) — requires stats-system story

#### Item Destruction from Monster Attacks

**See [#271 - Item Destruction from Elemental Attacks](https://github.com/Rushwind13/JMoria/issues/271)** — Moved to Phase 3+ (postponed until after Ranged Attacks PR #235)

---

## Top Priorities

These are ordered by "unblocks the most other work" and "most visible gameplay impact":




### Tier 3 — Monster Combat Depth ✅ (content complete, code TBD)
11. **Item destruction from attacks** — See [#271](https://github.com/Rushwind13/JMoria/issues/271). Postponed until after Ranged Attacks (PR #235).

### Tier 4 — Light Economy Integration
12. **Light radius extends visibility** (#72, #121) — UpdateVisibility() uses torch Radius to extend sight range
13. **Light source items** (#270) — Staff of Light, Staff of Starlight in Items.txt; need light source code integration. Flask of Oil not yet added.

### Tier 5 — Advanced Systems (Require Stats, Classes, Spells)
15. **`EFFECT_FLAG_STAT`** (#77) — Requires stat system (STR/DEX/CON/INT/WIS/CHA) to exist first
16. **Class-specific feelings** (#114) — Requires class system with stat priorities
17. **Spell of Light Area / Mage ID spell** (#72, #114) — Requires spell/magic system
18. **`?*Identify*`** (#114) — Requires ego/unique item system
