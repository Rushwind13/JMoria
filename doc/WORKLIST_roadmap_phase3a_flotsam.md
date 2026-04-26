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

### P2 - #172: Tombstone UI fix (Low, ~1-2 hrs)

1. **Define tombstone field width constant**: Add `#define TOMBSTONE_FIELD_WIDTH 17` in `EndGameState.cpp` (or `Constants.h`) to encode the 17-character inner width of the `|...|` border — this is currently an implicit magic number in `DoTomb()`.
2. **Expand the tombstone format string for a two-line killer slot**: In the `CEndGameState` constructor, add one extra `|%*s%*s|` + `|                 |` row immediately after the existing killer-name row in the `tombstone[]` char literal. The tombstone becomes one row taller only when the killer name wraps; the second row is always present in the format but will be padded with spaces when the name fits on one line.
3. **Wrap long names in `CEndGameState::DoTomb()`**: Before computing padding, check if `m_pScore->m_szKilledBy` (or `m_szName`) exceeds `TOMBSTONE_FIELD_WIDTH`. If so, split into two local `char[]` buffers: `szLine1` (first 17 chars) and `szLine2` (remainder, up to 17 chars). Pass both to the `Printf` call — line1 into the existing `%*s%*s` slot, line2 into the new second slot. When the name fits on one line, pass an empty string for line2 so the extra row renders as blank `|                 |`. Do not modify `m_pScore` fields in place.
4. **Clamp padding to zero**: Guard all padding values with `max(0, ...)` so that an unexpectedly long segment still renders within the border rather than producing a negative `%*s` width.
5. **Add BDD scenario**: Spawn a game, trigger the end-game state with a killer name longer than 17 chars (e.g., `"Yellow Mushroom Patch"`), and assert that the rendered tombstone output contains no characters past the closing `|` on either killer-name line.

---

### P2 - #234: ASCII DisplayText bugs (Low, ~1-2 hrs)

**Bug 1 — Equipment window truncates from the wrong end**
1. **Identify the trim path in `CDisplayText`**: When the equipment list exceeds the window height, the oldest/first line (`a)`) is currently scrolled off the top. Equipment slots should always show from `a)` downward; trim the bottom of the list instead. Locate the scrolling/trim logic in `DisplayText.cpp` (the `Paginate()` or line-overflow path) and add a `FLAG_TEXT_TRIM_TAIL` flag (or equivalent) that trims from the end rather than the top.
2. **Apply tail-trim flag to `m_pEquipDT`**: In `Game.cpp` where `m_pEquipDT` is constructed and `SetFlags()` is called, add the tail-trim flag alongside the existing `FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX`.

**Bug 2 — Colors too dark on black ASCII background**
3. **Audit color palette in `RenderASCII.cpp`**: Identify all `init_pair` / color definitions. Colors that were readable on a light-green background may fall below contrast threshold on black. Either increase the brightness/attribute (e.g., add `A_BOLD`) for dark foreground colors, or remap them to brighter ncurses color constants. Validate each color against `COLOR_BLACK` background.

**Bug 3 — Dungeon geometry bleeds through DisplayText stat box**
4. **Re-render DisplayText boxes after dungeon draw**: In the main render loop (`Render.cpp` or `Game.cpp`), ensure all `CDisplayText` regions (`MsgsDT`, `StatsDT`, `InvDT`, `EquipDT`) are drawn *after* the dungeon tile pass so their background overwrites any dungeon tiles that fall in the same screen region. In the ASCII renderer, explicitly clear (fill with spaces) each DisplayText rect before writing text so the terminal cells are owned by the UI layer.

---

### P2 - #177: RENDER_MODE in linker flags for `make test` (Low, ~1-2 hrs)

1. **Conditionalize macOS `TEST_LD_FLAGS` by `RENDER_MODE`**: The macOS `TEST_LD_FLAGS` (line 46 of `Makefile`) currently always appends to `LOCAL_LIB_PATHS` regardless of renderer mode. Mirror the same `ifeq ($(RENDER_MODE),ascii)` / `else ifeq` / `else` ladder used for `LD_FLAGS` so that `TEST_LD_FLAGS` omits SDL2/OpenGL entries when building ASCII-only. The test-framework libs (`-lcucumber-cpp`, boost, googletest) are renderer-agnostic and remain unconditional.
2. **Conditionalize Linux `TEST_LD_FLAGS`**: On Linux, `TEST_LD_FLAGS = $(TEST_LD_FLAGS_LINUX)` is set after the `LD_FLAGS` conditional block, so it is currently renderer-agnostic (fine, since SDL2 comes via `LD_FLAGS`). Verify this is still correct once step 1 is done; no change may be needed on Linux.
3. **Add `make ascii-test` convenience target**: Add a phony target `ascii-test: ; $(MAKE) RENDER_MODE=ascii test` so headless CI or machines without SDL can build and run tests in one step without setting the variable manually.
4. **Smoke-test on a machine (or CI container) without SDL**: After the Makefile changes, run `make RENDER_MODE=ascii test` and confirm the link succeeds with no unresolved SDL/OpenGL symbols.

---

### P3 — #272: Visible Monsters UI Pane (Medium, ~4–8 hrs)

1. **Add a new `DisplayText` region for "Visible Monsters"**: Model it on the existing Inv/Equip sidebar regions.
2. **Populate pane during normal play**: Show all monsters currently in the player's line-of-sight, listed by name and distance, updated each turn.
3. **Populate pane after Detect Monsters fires**: After `EFFECT_FLAG_SEE` / `EFFECT_MOD_MONSTERS` resolves, display all detected monsters (using the existing `m_bDetected` flag and `m_llVisibleMonsters` list) by name and distance.
4. **Auto-expire detected entries**: Pane reverts to line-of-sight–only monsters on the turn after detection expires. (this should happen automatically with UpdateVisibleMonsters())
5. **Wire Scroll of Detect Monsters to the pane**: The item is already in `Items.txt` (`EFFECT_FLAG_SEE` / `EFFECT_MOD_MONSTERS`); ensure its effect triggers the pane display.
6. **Future / deferred — `*` targeting from pane**: Player can cycle through the Visible Monsters list with `*` to set targeting cursor. Defer to a follow-on issue; do not block the pane display work.
7. **Toggleable pane** - ask user which key to use to toggle this view (let user know which lowercase and uppercase keys are available)

---

### P4 — #242: Speed System & Action Economy (Major, ~2–3 days)

#### Foundation: Action Economy Engine
1. **Add `m_nAccumulatedSpeed` to each entity** (player + monsters): Each game tick adds the entity's speed value; when the accumulator reaches the action threshold (10), the entity acts and the accumulator resets.
2. **Set base player speed to 10**: One action per AIMgr update at base. Confirm existing turn loop is compatible or refactor loop to accumulate-then-act.
3. **Set base speed on all existing monsters**: Audit `Monsters.txt` speed fields; calibrate per the design table (see Appendix A §242). Priority calibration targets: Flaming Bats 2, Vampire Lords 3–4, Major Demons 2–3, Ancient Dragons 2, Balrog 4–5.

#### Equipment Speed Bonuses
4. **Implement Ring of Speed item effect**: Ring grants +1 to +10 permanent speed bonus while worn. Two-ring equipment slot limit applies.
5. **Implement Boots of Speed item effect**: Boots grant +10 permanent speed bonus while worn.
6. **Implement Gloves of Elvenkind speed bonus**: Gloves grant +10 or variable speed bonus while worn.
7. **Implement Potion of Speed temporary effect**: +10 speed for the duration of the current dungeon level (or combat duration — confirm design choice and add `Constants.h` constant). Stacks with equipment bonuses.

#### Stat Interactions (requires #197 foundation)
8. **DEX → base speed modifier**: High DEX grants +1 speed; low DEX gives −1 speed. Wire to player stat sheet once #197 is active.
9. **STR → encumbrance penalty modifier**: High STR reduces the speed penalty from heavy armor/items. Wire once #197 is active.
10. **Encumbrance speed penalty**: Heavy armor and overloaded inventory reduce speed. Define encumbrance threshold constants in `Constants.h`.

#### Display
11. **Show player speed on the Stats pane**: Display current effective speed (e.g., `Slow(-2)` or nothing for speed 10 or `Fast(+10)`) on the character stats sidebar.
12. **Show speed modifier breakdown**: When applicable, show a breakdown annotation (e.g., `Speed: 15 (+3 boots, +5 ring)`) so the player understands what is contributing.
13. **Show Haste/Slow modifiers separately**: Temporary Haste/Slow effects displayed distinctly from permanent equipment speed.
14. **Wizard-mode monster speed display**: Optionally render monster speed values in wizard mode for tuning and testing.
15. **Hide speed display until first speed modifier is applied**: Only begin showing the speed field on the stats screen after the player equips or drinks a speed item (to reduce early-game UI noise). Confirm this UX decision; make it a `Constants.h` toggle if preferred.

#### Balancing
16. **Define speed cap constant (optional)**: Speed is soft-limited by item availability. Document this decision in `Constants.h` with a comment.
17. **Confirm Potion of Speed duration semantics**: It is a normal duration timer? Record this decision in `Constants.h` as `SPEED_POTION_DURATION`.

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
