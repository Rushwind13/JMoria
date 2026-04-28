# Phase 3a: Flotsam — Prioritized Work List

Repo: Rushwind13/JMoria

These are targeted improvements and one major system that were floating without a dedicated implementation plan. Issues #265, #264, #172, #234, #177, #272, #273, and #242. Smallest wins first; biggest system last.

---

## Prioritized Work

### ✅ P1 — #273: Messages Scrollback (COMPLETE — commit b22288d)

1. ✅ **Suppress empty-result messages**: Silent when detect/search finds nothing (doors, traps, monsters).
2. ✅ **Expand Msgs window to 5 rows**: Both renderers derive from `MSGS_ROWS 5` in `Constants.h`; OpenGL rect was previously hardcoded to 40px.
3. ✅ **Make Msgs window height configurable**: `#define MSGS_ROWS 5` in `Constants.h` is the single source of truth.
4. ✅ **No blank lines when full**: Fixed `Paginate()` off-by-one — `dwAddLinesMax = usedLines + 1 + m_dwFreeLines`.

---

### ✅ P2 — #265: Breath Weapon Damage Scales with Monster HP (COMPLETE)

1. ✅ **Implement HP-proportional breath damage for dragons**: In `CMonster::Damage()`, when `m_pCurrentAttack->m_dwType & MON_FLAG_BREATHE`, damage is `m_fCurHP * fDamageMult` instead of a dice roll. Applies to all breath-weapon monsters automatically.
2. ✅ **Apply the same scaling rule to all breath-weapon monsters**: The `MON_FLAG_BREATHE` check in `Damage()` covers all 45+ breath-weapon monsters in `Monsters.txt` with no per-monster changes needed.
3. ✅ **Validate weakening behavior**: Two BDD scenarios added to `monsters.feature`: one verifies breath damage equals current HP at full health; the second deals 10 damage and re-verifies the proportional reduction. All 140 tests pass.

---

### ✅ P2 — #264: EFFECT_MOD_MAX (COMPLETE)

1. ✅ **Define `EFFECT_MOD_MAX` in `Constants.h`**: Added `#define EFFECT_MOD_MAX 0x000000400`; incremented `NUM_EFFECT_MODIFIERS` to 11; registered in `StringTable` alongside other `EFFECT_MOD_*` entries.
2. ✅ **Implement `RollMax(const char *szFormat)` in `Util.cpp`/`Util.h`**: Parses NdM string and returns `dice * sides` (maximum possible roll) without randomness. Mirrors signature of existing `Roll(const char *)` overload.
3. ✅ **Implement `MON_FLAG_MAXHP` in `CMonster::Init()`**: When `pmd->m_dwFlags & MON_FLAG_MAXHP`, calls `RollMax(pmd->m_szHD)` instead of `Roll(pmd->m_szHD)` to give the monster max HP at spawn.
4. ✅ **Apply `EFFECT_MOD_MAX` to damage rolls in `CMonster::Damage()`**: When the current attack's effect modifier includes `EFFECT_MOD_MAX`, calls `RollMax(szDamage)` instead of `Roll(szDamage)`.
5. ✅ **Tag applicable monsters in `Monsters.txt`**: Balrog and all six Ancient Dragons tagged with `MON_FLAG_WARM, MON_FLAG_MAXHP`.
6. ✅ **Add BDD scenario**: Verifies that a monster spawned with `MON_FLAG_MAXHP` always has `curHP == maxHP`. Also fixed `FileParse.cpp` flags-loop to trim leading whitespace from comma-separated flag tokens.

---

### ✅ P2 — #172: Tombstone UI fix (COMPLETE — commit a1d2c4c)

1. ✅ **Define tombstone field width constant**: Added `#define TOMBSTONE_FIELD_WIDTH 17` in `EndGameState.cpp`.
2. ✅ **Expand the tombstone format string for a two-line killer slot**: Added second `|%*s%*s|` row immediately after the existing killer-name row.
3. ✅ **Wrap long names in `CEndGameState::DoTomb()`**: Splits `m_pScore->m_szKilledBy` across two `char[18]` buffers (`szKiller1`, `szKiller2`) when > 17 chars; second row renders blank when name fits on one line.
4. ✅ **Clamp padding to zero**: All padding values guarded with `MAX(0, ...)`.
5. ✅ **Add BDD scenario**: `game.feature` — tombstone with killer `"Yellow Mushroom Patch"` asserts no characters past closing `|`. `EndGameState.h`, `Game.h`, `DisplayText.h` all use `#ifdef UNIT_TEST` pattern to expose test-only members.

---

### ✅ P2 — #234: ASCII DisplayText bugs (COMPLETE — commit 21a0eb7)

**Bug 1 — Equipment window truncates from the wrong end**
1. ✅ **Add `FLAG_TEXT_TRIM_TAIL` flag**: Added `#define FLAG_TEXT_TRIM_TAIL 0x10` to `DisplayText.h`; `Paginate()` early-returns with `m_szDrawPtr = m_szText` when set, so `DrawStr`'s bounds-check clips at the bottom showing first N lines.
2. ✅ **Apply tail-trim flag to `m_pEquipDT`**: `Game.cpp` now sets `FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX | FLAG_TEXT_TRIM_TAIL` on the equipment window.

**Bug 2 — Colors too dark on black ASCII background**
3. ✅ **Boost dark colours in `RenderASCII.cpp`**: `GetColorPair()` now applies `A_BOLD` when `r+g+b < 300`, mapping dark foreground colours to their terminal bright variant instead of being invisible on black.

**Bug 3 — Dungeon geometry bleeds through DisplayText stat box**
4. ✅ **Already handled**: `DrawTextBoundingBox()` fills its interior with spaces before any text is written; `DisplayText::Draw()` is called after the dungeon draw pass. No code change required.

---

### ✅ P2 — #177: RENDER_MODE in linker flags for `make test` (COMPLETE — commit 84530b6)

1. ✅ **Conditionalize macOS `TEST_LD_FLAGS` by `RENDER_MODE`**: Full `ifeq`/`else ifeq`/`else` ladder added — ascii omits SDL2/OpenGL; opengl omits ncurses; both includes all three.
2. ✅ **Conditionalize Linux `TEST_LD_FLAGS`**: Verified unchanged and correct — renderer libs come via `LD_FLAGS` on Linux, no change needed.
3. ✅ **Add `make ascii-test` convenience target**: `ascii-test: $(MAKE) RENDER_MODE=ascii test` added as phony target.
4. ✅ **Smoke-tested**: `make ascii-test` links `AllSteps` with only `-lncurses` (no SDL/OpenGL); 142 scenarios pass.

---

### ✅ P3 — #272: Visible Monsters UI Pane (COMPLETE)

1. **Add a new `DisplayText` region for "Visible Monsters"**: Model it on the existing Inv/Equip sidebar regions.
2. **Populate pane during normal play**: Show all monsters currently in the player's line-of-sight, listed by name and distance, updated each turn.
3. **Populate pane after Detect Monsters fires**: After `EFFECT_FLAG_SEE` / `EFFECT_MOD_MONSTERS` resolves, display all detected monsters (using the existing `m_bDetected` flag and `m_llVisibleMonsters` list) by name and distance.
4. **Auto-expire detected entries**: Pane reverts to line-of-sight–only monsters on the turn after detection expires. (this should happen automatically with UpdateVisibleMonsters())
5. **Wire Scroll of Detect Monsters to the pane**: The item is already in `Items.txt` (`EFFECT_FLAG_SEE` / `EFFECT_MOD_MONSTERS`); ensure its effect triggers the pane display.
6. **Future / deferred — `*` targeting from pane**: Player can cycle through the Visible Monsters list with `*` to set targeting cursor. Defer to a follow-on issue; do not block the pane display work.
7. ✅ **Toggleable pane**: `v` key toggles the pane. Available lowercase keys were presented to user; `v` ("visible") selected.

**Implementation notes:**
- `m_pMonstersDT` / `m_bShowMonsters` added to `CGame`; `GetMonsters()`, `ToggleMonsters()`, `IsShowingMonsters()` public accessors
- `ASCIILayout::monsters` region: 10 rows tall at the bottom of the left sidebar; `l.stats` bottom clamped to `monstersTop` so borders don't overlap
- `CPlayer::DisplayVisibleMonsters()`: two-pass walk of `m_llVisibleMonsters` (already distance-sorted); first pass counts by `m_md->m_dwIndex`; second pass prints in distance order, skipping already-seen types. Count shown as `Name (N)` when N > 1
- `DrawStr` bounding-box fix: `drawBottom` reduced by an extra `insetY` when `FLAG_TEXT_BOUNDING_BOX` is set, preventing text from rendering on top of the bottom border character
- `IRenderBackend::ShouldAutoShowMonsters()` returns `false` (hidden by default, toggleable by user)
- 5 BDD scenarios added to `test/features/visible_monsters.feature`; 147 scenarios pass

✅ **COMPLETE** — commits on `feat/phase3a_flotsam`

---

### ✅ P4 — #242: Speed System & Action Economy (COMPLETE — commit 33dccfb)

#### Foundation: Action Economy Engine
1. ✅ **Add `m_fSpeed` to CPlayer**: Base 1.0f (= "speed 10"). `GetSpeed()` accessor added. AIMgr scaled by `1.0f / player_speed` on each player action — fast player means monsters accumulate time more slowly.
2. ✅ **Base player speed set to 1.0f**: Confirmed compatible with existing turn loop; no loop refactor needed.
3. ✅ **Calibrate monster speeds in `Monsters.txt`**: Flaming Bats 2.0 (unchanged), Ancient Dragons 2.0, Greater Demon 2.0, Demon Lord 2.5, Greater Demon Lord 3.0, Master Vampire 3.0, Lordly Vampire 3.5, Balrog 4.5.

#### Equipment Speed Bonuses
4. ✅ **Ring of Speed**: Random +0.1 to +1.0 per-spawn (= speed +1 to +10) via `EFFECT_FLAG_SPEED` check in `CItem::Init()`.
5. ✅ **Boots of Speed**: Fixed `Speed 1.0` in `Items.txt`; `m_fSpeedBonus` applied via `Wield()`/`RemoveEquipment()`.
6. ✅ **Gloves of Elvenkind**: New item added to `Items.txt` with `Speed 1.0`.
7. ✅ **Potion of Speed temporary effect**: `+1.0f` via `DoIntrinsicEffects(EFFECT_FLAG_SPEED)`; expires via `EFFECT_MOD_TIMED` added to `Effects.txt`.

#### Stat Interactions (deferred)
8. ⬜ **DEX → base speed modifier**: Deferred to #197 (Stats).
9. ⬜ **STR → encumbrance penalty**: Deferred to #197 (Stats).
10. ⬜ **Encumbrance speed penalty**: Deferred to #197 (Stats).

#### Display
11. ✅ **Show player speed on Stats pane**: `Fast(+N)` / `Slow(-N)` shown; hidden at base speed (no UI noise for new players).
12–15. ⬜ **Breakdown annotation, Haste/Slow display, wizard-mode display**: Deferred.

#### Testing
✅ **5 BDD scenarios** in `test/features/speed.feature` + `SpeedSteps.cpp`: base speed, equip boots, remove boots, drink potion, equip ring.

---

## Appendix A — Verbatim Actionable Items from GitHub Issues

---

### Issue #265 — "breath weapon damage does same damage as monster HP"

> monsters breath get weaker as they get hurt.

*(No additional comments on this issue.)*

**Extracted actionable items (verbatim from issue body):**
- Breath weapon damage does same damage as monster HP always
- Dragons do this always

---

### Issue #272 — "Add 'Visible Monsters' UI pane for Detect Monsters targeting"

**Issue body:**

> When the player reads a Scroll of Detect Monsters (EFFECT_TYPE_SEE + EFFECT_FLAG2_MONSTERS), all monsters within detection range appear on screen for one turn. Currently the player can target them with `*`, but there is no dedicated UI pane listing what was detected.
>
> **Desired behavior:**
> - After Detect Monsters fires, show a "Visible Monsters" pane (similar to Inv/Equip sidebars) listing the detected monsters by name and distance
> - The pane should also display during normal play for monsters in line-of-sight
> - Player should be able to target from this list (future: `*` integration)
> - Pane clears when detection expires (next turn) and reverts to showing only normally-visible monsters
>
> **Context:**
> - `m_llVisibleMonsters` already tracks which monsters the player can see (used by targeting)
> - `m_bDetected` flag on CMonster enables one-turn detection rendering
> - DisplayText regions already exist for Msgs, Stats, Inv, Equip, Use — this would be an additional region. Ask user about location and size.
>
> **Related code:**
> - `CPlayer::UpdateVisibleMonsters()` in Player.cpp
> - `CDungeon::DrawMonsters()` in Dungeon.cpp
> - `CDisplayText` region system in DisplayText.cpp

**Comment by Rushwind13:**

> ## Detect Monsters Item (from #77)
>
> | Item | Effect Flag | Effect Modifier | Effect Type | Notes |
> |------|-------------|-----------------|-------------|-------|
> | Scroll of Detect Monsters | EFFECT_FLAG_SEE | EFFECT_MOD_MONSTERS | — | Reveals all monsters in radius 30 |
>
> This item is already in Items.txt and should be integrated with the Visible Monsters UI pane.

---

### Issue #273 — "Scrollback space in Messages DisplayText window is limited"

**Issue body:**

> * save scrollback space by not outputting anything if nothing is detected.
> * make the window larger always? 5 lines?
> * make the window configurable in size? (how about other flyouts)
> * do not leave bottom line blank (bug with bounding box offsets)

*(No comments on this issue.)*

---

### Issue #242 — "Speed System & Difficulty Tuning (Action Economy Mechanic)"

**Issue body:**

> ## Summary
>
> Implement a speed/action economy system that creates difficulty scaling and high-level character differentiation. Speed is the **primary difficulty lever** in a turn-based roguelike and directly affects whether players can fight the Balrog.
>
> ## Mechanics
>
> ### Character Speed Base
> - **Base**: 10 (represents normal speed, move once per AIMgr update)
> - **Speed Rating**: 10 ÷ base = action multiplier
>   - Speed 10 = 10/10 = 1 action per turn (baseline)
>   - Speed 20 = 20/10 =  2 actions per turn
>   - Speed 50 = 50/10 =  5 actions per turn
>
> ### Speed Modifiers (Stacking)
>
> **Encumbrance (Negative)**:
> - Carrying heavy items or armor reduces speed
> - E.g., "Slow (-2)" means speed rating goes from 10 → 8/10 = 0.8x normal speed
> - Heavy armor should penalize; light armor minimal
>
> **Potions & Abilities (Temporary +10)**:
> - Potion of Speed: +10 speed (speed 20 = 2x normal rate), wears off
> - Other utility potions for different effects
>
> **Equipment (Permanent Bonuses)**:
> - Ring of Speed (+1 to +20): "Ring +9" gives +9 speed (10 → 19)
> - Boots of Speed, Gloves of Elvenkind, etc.: +10 or variable speed bonuses
> - These are treasure goals for late-game character enhancement
>
> **Base Stats**:
> - DEX affects initial speed (high DEX = +1 speed, low DEX = -1 speed)
> - STR affects encumbrance speed penalty (high STR = less penalty)
>
> ### Monster Speed Ratings (Calibration)
>
> **Current Monsters** (reference points):
> - Flaming Bats: Speed 2
> - Small monsters: Speed 1-2
>
> **High-Level Monsters** (design goals):
> - Vampire Lords: Speed 3-4 (extremely dangerous)
> - Major Demons: Speed 2-3 (brutal)
> - Ancient Dragons: Speed 2 (lethal)
> - **Balrog (Boss)**: Speed 4-5 (appears as blur to low-speed characters)
>
> **Difficulty Scaling**:
> - Level 1 character (speed ~10) vs Balrog (speed 45-50): Balrog acts 4-5 times per player action
>   - Player gets one attack; Balrog gets 4-5 attacks + breathe + special ability
>   - Completely unfair, player dies immediately
> - Level 40 character (speed 10 base + 15 from ring +5 from boots): speed 30
>   - vs Balrog speed 45: Balrog still acts 1.5x per player action (challenging but winnable)
>   - vs average monsters (speed 2): Player now acts much faster (1.5x their speed)
>
> ## Implementation Notes
>
> ### Turn Timing
> - Track "accumulated speed" per entity (already done: m_fSpeed)
> - When accumulated_speed >= action_threshold, entity acts and resets (should already be integral part of the Update() mechanism)
> - This avoids "discrete turns" and allows fluid action economy (game is intended to be turn-based: everything only moves (a grid space amount of movement) when it has passed an integer amount of "update time")
>
> ### Display
> - Show player speed prominently on stats screen (no display for regular speed. only display + or - offsets to normal e.g. `Slow(-2)` or `Fast(+8)` (same character after !Speed))
> - Show encumbrance indicator (is it hurting speed? AC? both?) hurts speed, encumbrance and AC are not linked, the "indicator" is as above.
> - Optionally show monster speeds in wizard mode (for testing/learning) this is interesting, maybe in wizard mode, the monsters could be colored by speed instead of their usual color (targeting does something similar to show current target)
>
> ### Balancing Levers
> 1. **Monster speed values**: Tune per creature (Balrog may start at 4, adjust if too easy/hard)
> 2. **Available speed items**: Ring of Speed (+9), Boots (+10), other accessories
> 3. **Encumbrance penalties**: How much does armor slow down? up to Padded, 0. Leather -2, studded -4, chain -6, plate mail -8, plate armor -10 (but magical armor is lighter and has reduced/minimal/zero? encumbrance penalty)
> 4. **Player base speed**: Adjust DEX formula if needed
> 5. **Potion of Speed duration**: How long does +10 last? (standard potion duration, ndm turns, about the same amount of time it would take to rest and regain 1hp)
>
> ## Dependencies
>
> - Requires #197 (Player Stats) for DEX/STR/encumbrance mechanics
> - Affects #42 (Balrog difficulty) — Balrog must have tuned speed to be defeatable
> - Pairs with equipment system (rings, boots, items with speed bonuses)
> - Pairs with monster definitions (speed intrinsic)
>
> ## Future Considerations
>
> - **Multiplayer Scaling** (see #240): Should difficulty increase with party size? Or monster speed + count?
> - **Monster Speed Ranges** per depth: Could spawn deeper/faster monsters earlier for multiplayer challenge
> - **Haste/Slow Spells** (future magic system): Temporarily modify speed via magic
> - **Speed Diminishing Returns**: Cap at some max speed (so not all endgame is just speed-stacking)?
>
> ## Success Criteria
>
> - Balrog feels appropriately dangerous when under-geared (speed ~15-20)
> - Balrog feels defeatable when properly equipped (speed ~30)
> - Speed rings/boots become desirable treasure
> - Character build variety emerges around speed optimization
> - Low-speed character vs high-speed monster creates tension/gameplay variation

**Comment 1 by Rushwind13:**

> **Major System Issue** — Speed/Action Economy is the **primary difficulty tuning lever** in JMoria's turn-based system.
>
> **Directly blocks/enables**:
> - #42 (Balrog) — Balrog speed 4-5 requires high-speed player to fight on fair terms; this creates the difficulty curve
> - #197 (Stats) — DEX affects base speed; STR affects encumbrance penalties
> - Equipment system — Rings of Speed, Boots of Speed become desirable treasure
>
> **Related to**:
> - #128 (Magic Items) — Speed rings/boots are magical equipment rewards
> - #72 (Fog of War) — Light sources don't affect speed, but both affect survival
>
> See WORKLIST_jmoria_core_roadmap.md for design notes on monster speed calibration (Flaming Bats speed 2, Balrog speed 4-5, etc.)

**Comment 2 by Rushwind13:**

> ## Equipment Slot Limits & Speed Stacking
>
> **Confirmed Design:**
> - **2 Rings** (each can grant +0 to +10 speed): Max +20 from rings
> - **1 Boots** (grants +0 to +10 speed or other effects)
> - **1 Gloves** (grants +0 to +10 speed or other intrinsics)
> - **Other equipment** (weapon, armor, helm, amulet) can also have speed bonuses
> - **Potion of Speed**: +10 temporary, stacks with equipment (e.g., speed 30 from gear + 10 from potion = 40 temporary)
>
> **Maximum Realistic Speed**: ~40-50 (multiple +10 items + potion buff)
> - No hard cap, but equipment availability limits stacking naturally
>
> **UI Display**:
> - Hide speed value until items/buffs change it (e.g., equip first speed ring = "Speed displayed now")
> - Show on stats screen as "Speed: 15 (+3 from boots, +5 from ring)" (there is going to be a character data screen that includes all of this type of modifier in a huge table with all the intrinsics)
> - Show Haste/Slow modifiers separately -- no, they cancel each other out
>
> **Power Fantasy**: Speed is primary but not only power fantasy
> - **Intrinsic Management** is bigger win driver — Free Action + Levitation + Resistances (both are needed)
> - **Stat Optimization** — max stats enable different builds
> - **Equipment Synergy** — combining items (e.g., Holy Avenger + AC gear + Regeneration ring)
