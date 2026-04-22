# WORKLIST — Monster Effects Migration

Goal: All `Attack` fields in Monsters.txt use named `Effect` entries from Effects.txt instead of inline configuration.

Sub-goals:
- Prefer to flexibly use an existing effect to replace an inline Monster config, rather than creating a new Effect
- Want to be able to use similar attacks with flavor-only changes (delivery type: BITE, CLAW, BREATHE, TOUCH, SPORE, CRAWL, TRAMPLE)
- Effects.txt entries are LEGO blocks — used for Monster attacks, Item effects, and Player magic spell effects

Assumptions:
- New Attack line format: `Attack  <DELIVERY>, <EffectName>[, <dice>[, <range>]]`
  - Delivery is always the first token (required for flavor text generation)
  - Effect name replaces all inline type/flag/element tokens
  - Dice override is optional (omit = use Effect's default Amount)
  - Range override is optional (omit = use Effect's default Range)
- Items.txt already has an equivalent override function; the item and monster parsers share the same Effect lookup
- Dragon breath: younger dragons cast line effects (e.g. Firebolt), older/ancient dragons cast ball effects (e.g. Fireball); breath damage = monster current HP (scaling in code)
- Poison attacks: dice represent both initial damage amount AND poison duration (higher level poisoning is more dangerous and lasts longer)
- XP drain dice: physical damage component; XP drain amount scaling left for later code work

---

## PHASE 1 — Effects.txt additions *(data-only, no code blocker, do now)*

| # | Task | Notes |
|---|---|---|
| 1 | ~~Add **"Physical Hit"** effect~~ ✅ DONE 2026-04-21 | `HIT`, no element, Amount=1d6, Range=1. Default for all pure physical attacks |
| 2 | ~~Add **"Pick Pocket"** effect~~ ✅ DONE 2026-04-21 | `HIT`, TREASURE, Range=1. Not assigned to any monster — placeholder for Issue #275 |
| 3 | ~~Add **"Poison Bolt"** effect~~ ✅ DONE 2026-04-21 | `HIT`, POISON, line MOD, Amount=2d8, Range=8 — for young/mid green/poison dragons breath |
| 4 | ~~Add **"Poison Ball"** effect~~ ✅ DONE 2026-04-21 | `HIT`, POISON, ball MOD, Amount=6d8, Radius=3, Range=5 — for ancient green/poison dragons breath |

---

## PHASE 2 — Code: parser prerequisite *(blocks all Monsters.txt migration)*

| # | Task | File | Notes |
|---|---|---|---|
| 5 | ~~Add `m_ed` pointer to `CAttack`~~ ✅ DONE 2026-04-21 | `src/Monster.h` | **Superseded same session** — `CAttack` reshaped: dropped `m_dwEffect`/`m_dwEffectFlags`/`m_ed`; added owned `CEffect *m_pEffect`; delivery stays on `m_dwType` |
| 6 | ~~Extend Attack parser~~ ✅ DONE 2026-04-21 | `src/FileParse.cpp` | New format: delivery→`m_dwType`, `EffectFromName()`→`m_pEffect`; old inline format now also allocates a `CEffect` into `m_pEffect`; `EffectFromName()` extracted as private `CDataFile` helper (same call site as item `Effect` named-ref path) |

---

## PHASE 3 — Monsters.txt migration *(blocked on Phase 2)*

| # | Task | Count | Notes |
|---|---|---|---|
| 7 | Pure physical attacks | ~430 | `<DELIVERY>, <Physical Hit>, <dice>` |
| 8 | Elemental melee (bite/claw/touch/crawl/spore + element) | ~55 | `<DELIVERY>, <Element Touch>, <dice>` |
| 9 | Status INTRINSIC attacks (paralyze/confuse/sleep/afraid/blind) | ~73 | `<MON_FLAG_TOUCH>, <Status Touch>[, dice]` — dice = duration for poison; status infliction deferred to Phase 5 code |
| 10 | INTRINSIC+POISON attacks | ~21 | `<MON_FLAG_TOUCH>, <Poison Touch>, <dice>` — dice = damage+duration |
| 11 | XP drain bites | 6 | `<MON_FLAG_BITE>, <XP Drain Touch>, <dice>` — drain scaling deferred |
| 12 | Breathe attacks — young (bolts) | ~20 | `<MON_FLAG_BREATHE>, <Firebolt/Frost Bolt/etc>, <dice>, <range>` |
| 13 | Breathe attacks — old/ancient (balls) | ~25 | `<MON_FLAG_BREATHE>, <Fireball/Frost Ball/etc>, <dice>, <range>` |
| 14 | Creeping Coins TREASURE → Poison Touch | 3 | `<MON_FLAG_CRAWL>, <Poison Touch>` |
| 15 | Worm mass stat drain | ~8 | **Deferred to stats-system story** — leave inline with `EFFECT_FLAG_STAT` |
| 16 | Fix 12 malformed Attack lines (bad indent/prefix) | 12 | Straighten prefix whitespace to match standard indent |
| 17 | Tourist buggy attacks | 2 | Leave as-is — design not formed |
| 18 | Stray Dog 0d0 bite | 1 | Leave as-is — Town placeholder |

### Worm Mass stat drain assignments (for future reference when stats-system story is done)

| Worm Mass | Color | Elemental attack | Stat drained |
|---|---|---|---|
| White Worm Mass | White | — | Poison only (no stat drain) |
| Clear Worm Mass | Clear | — | Poison only (no stat drain) |
| Red Worm Mass | Red | Fire crawl | — (no stat drain) |
| Blue Worm Mass | Blue | Cold crawl | — (no stat drain) |
| Yellow Worm Mass | Yellow | — | WIS ("you feel naive") |
| Green Worm Mass | Green | — | DEX ("you feel clumsy") |
| Black Worm Mass | Black | Acid crawl | CHA ("you feel ugly") |

Note: The monster effect table in WORKLIST_items_design.md (dated 2026-04-16) contains a discrepancy — it lists Yellow=-DEX and Green=acid+CHA, which contradicts the correct assignments above. This will be resolved in the stats-system story.

---

## PHASE 4 — WORKLIST updates *(data-only, do alongside Phase 2/3)*

| # | Task |
|---|---|
| 19 | Add Phase 2 code items (parser + CAttack.m_ed) to WORKLIST in proper position |
| 20 | Add Phase 5 dispatch items (below) to WORKLIST |
| 21 | Flag worm mass stat table discrepancy in WORKLIST for stats-system story |

---

## PHASE 5 — Deferred code (post-migration, separate stories)

| # | Task |
|---|---|
| 22 | AIMgr: read resolved `m_ed` effect at runtime (replaces inline flag reads) |
| 23 | Player.cpp: `EFFECT_TYPE_INTRINSIC` dispatch — actually inflict Paralyze/Confuse/Sleep/Fear/Blind/Poison |
| 24 | XP drain scaling in code |
| 25 | Steal / Pick Pocket mechanic in code (see Issue #275) |
| 26 | Dragon breath damage = monster current HP (scaling) |
| 27 | Stat drain dispatch — tied to stats-system story, needs per-stat constants |

---

## Sequencing note

Phase 1 (Effects.txt additions) and Phase 4 (WORKLIST updates) can be done immediately.
Phase 3 (the full Monsters.txt migration) is gated entirely on Phase 2 (CAttack.m_ed + parser extension). Do not begin Phase 3 until Phase 2 is complete and the game builds and parses correctly.
