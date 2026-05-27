# Issue #316 — Strings Table / Messages Improvements

**Status**: In Progress  
**Last Updated**: May 25, 2026  
**Parent Roadmap**: WORKLIST_roadmap_phase3d_shiny.md

---

## Summary

Three deliverables:
1. **Message display improvements** — MsgsDT message flow, overflow handling, routing cleanup. ✅ **Done.**
2. **String table** — `const char *g_Strings[STR_MAX]` array + `Strings.h` enum + `Resources/Strings.txt` data file. `g_Strings[STR_KEY]` returns player-visible text. 🔲 **Up next.**
3. **Effect-flag string standardization** — `CEffectDef` gains `m_dwStrIds[EFFECT_STR_SLOT_MAX]`, indexed by a local slot enum (`EFFECT_NAME`, `EFFECT_HIT_PLAYER`, `EFFECT_HIT_MONSTER`, `EFFECT_EMIT_PLAYER`, …). Each slot stores a `STR_*` index into `g_Strings`. 🔲 **Follows string table.**

---

## Architecture Decisions

### Message Display (implemented)

The original `-more-` page-break-on-`\n` design was replaced with a simpler, better-feeling approach:

- **`CDisplayText::EnableMore()`** — sets `m_bMoreOnNewline` flag on MsgsDT only. When set:
  - `\n` in `Printf()` is converted to a space (messages concatenate on one line).
  - Before appending, overflow is detected: if the new text won't fit on the current line, `DrawFormattedStr("\n")` is called first to start a fresh line.
  - Result: messages flow naturally left-to-right; long messages wrap to the next row without a pause prompt.
- **`CMorePromptState`** exists (`STATE_MORE = 13`) and is wired into `SetState()` / `Game.cpp`, but is not actively invoked by normal message flow. It is available for future use (e.g., full-screen recall pages).
- **`DrawStr` bounding box fix** — overflow check moved to the top of the draw loop so the last row's characters are not drawn past the panel boundary.
- **Monster recall panel** — `PrintRecall()` now routes to `GetMonsterRecall()` (`m_pMonRecallDT`) everywhere (TargetState, LookState). The V panel uses real `\n` (no `EnableMore` flag) since it is formatted multi-line output. Pressing `*` (target mode) auto-opens the V panel if not already visible.
- **Message routing cleanup** — error/prompt messages that belong in the Use panel now go to `GetUse()->Printf()` instead of `GetMsgs()`. Redundant "Wield/Drop/etc. which item?" prompts removed from MsgsDT (already shown as `meta.header` in the Use panel). Wand "emits X" message gated on `IsIdentified()`.
- **Level change** — `GetMsgs()->Clear()` called before the arrival message so stale dungeon messages don't bleed across levels.
- **Inventory filter display** — `cListId` now increments for every inventory slot (not just filtered-visible ones), so the letter shown next to an item matches the slot letter the player must type. Out-of-alphabet guard retained (`cListId > meta.limit` → print footer and break).
- **Dungeon tile visibility** — `UpdateVisibility()` now sets `DUNG_FLAG_SEEN` alongside `DUNG_FLAG_VISIBLE` when a tile passes the Bresenham LOS test. Previously only `DUNG_FLAG_VISIBLE` was set, so stairs and floor features were only rendered within radius-1 (matching `UpdateSeen()`) while items were visible at full 5-tile LOS range via `DrawItems()`/`PlayerCanSee()`. Both now reveal at the same distance.

### String Table (planned)

#### Three separate lookup concerns

| Table | Key type | Value type | Purpose |
|---|---|---|---|
| `g_Constants.m_StringTable` | `char*` name | `int` | Parses data files: `"STR_FIRE_HIT"` → `STR_FIRE_HIT` (int). **Add STR_* entries here.** |
| `g_Strings[STR_MAX]` | `int` (enum, O(1) array) | `const char*` | Maps int → player-visible text: `STR_FIRE_HIT` → `"You are engulfed in flames!"` |
| `CEffectDef::m_dwStrIds[slot]` | `eEffectStrSlot` (local enum) | `int` (STR_* value) | Per-effect indirection: `EFFECT_HIT_PLAYER` → `STR_FIRE_HIT` |

#### New / changed files

- **`src/Strings.h`** — plain `STR_*` enum (sequential ints, not bitmasks) + `extern const char *g_Strings[STR_MAX]`. `STR_INVALID = -1`, values start at 0.
- **`src/Strings.cpp`** — `g_Strings` definition + `LoadStrings(szBasedir)` that opens `Resources/Strings.txt`, resolves each key via `g_Constants.LookupString()`, and stores the display string at that index.
- **`Resources/Strings.txt`** — block format, key = `STR_*` name, value = display text (same angle-brackets as Monsters.txt / Items.txt).
- **`src/Effect.h`** — add `eEffectStrSlot` enum + `m_dwStrIds[EFFECT_STR_SLOT_MAX]` on `CEffectDef`:
  ```cpp
  enum eEffectStrSlot {
      EFFECT_STR_NAME = 0,       // effect name for recall/display
      EFFECT_STR_HIT_PLAYER,     // "You are engulfed in flames!"
      EFFECT_STR_HIT_MONSTER,    // "The %s is engulfed in flames!"
      EFFECT_STR_EMIT_PLAYER,    // "You emit a jet of fire."
      EFFECT_STR_EMIT_MONSTER,    // "The %s emits a jet of fire."
      EFFECT_STR_STATUS_PLAYER,  // "are paralyzed"  → Printf(g_Strings[STR_STATUS_CHANGE], this)
      EFFECT_STR_STATUS_MONSTER, // "is paralyzed"   → Printf(g_Strings[STR_STATUS_CHANGE_MON], monName, this)
      EFFECT_STR_STAT_LABEL,     // "Paralyzed"      → stats panel label
      EFFECT_STR_SLOT_MAX
  };
  ```
  `STR_STATUS_CHANGE` (`"You %s."`) and `STR_STATUS_CHANGE_MON` (`"The %s %s."`) are shared STR entries, not per-effect.
- **`src/Constants.h`** — register all `STR_*` values in `m_StringTable` (same as `MON_IDX_*`, `EFFECT_FLAG_*`, etc.) so `Effects.txt` parser can resolve them.
- **`src/FileParse.cpp`** — parse new string-slot fields from `Effects.txt` using `g_Constants.LookupString()` → store into `m_dwStrIds[]`.
- **`Resources/Effects.txt`** — add fields per effect: `NameStr`, `HitPlayerStr`, `HitMonsterStr`, `EmitPlayerStr`, `StatusPlayerStr`, `StatusMonsterStr`, `StatLabelStr`.

#### Key-not-found behaviour

`g_Strings[n]` where `n` is out of range or the slot was never set returns `NULL`. Callers guard with `if (str) Printf("%s", str)`. Missing entries logged as `LOG_LEVEL_WARN` at load time.

#### STR_* naming convention

| Prefix | Scope |
|---|---|
| `STR_STAT_*` | Stats panel labels: `STR_STAT_PARALYZED`, `STR_STAT_BLIND`, `STR_STAT_INFRAVISION`, … |
| `STR_WIZ_*` | Wizard/debug mode messages: `STR_WIZ_ON`, `STR_WIZ_ADDED_ITEMS`, `STR_WIZ_IDENTIFIED` |
| `STR_STATUS_CHANGE` | Shared composition template: `"You %s."` |
| `STR_STATUS_CHANGE_MON` | Shared composition template: `"The %s %s."` |
| Per-effect `STR_*` | Named by effect + slot, e.g. `STR_STATUS_PARALYZE` = `"are paralyzed"`, `STR_HIT_FIRE` = `"You are engulfed in flames!"` |
| Everything else | Semantic grouping: `STR_DOOR_*`, `STR_COMBAT_*`, `STR_LIGHT_*`, `STR_RECALL_*`, `STR_LEVEL_*`, etc. |

Strings that directly affect the PLAYER can have `_PLAYER` suffix if needed to prevent a naming clash, but can otherwise be "the default" STR_STATUS_PARALYZE = paralyze player STR_STATUS_PARALYZE_MON = paralyze monster
---

## Task Checklist

### Phase 1 — Message Display Infrastructure ✅

- [x] `Constants.h`: Add `STATE_MORE = 13`, bump `STATE_MAX = 14`
- [x] `MorePromptState.h` / `MorePromptState.cpp`: Minimal state class (wired, not yet invoked)
- [x] `Game.h` / `Game.cpp`: Wire `m_pMorePromptState` and `SetState(STATE_MORE)`
- [x] `CDisplayText::EnableMore()` + `m_bMoreOnNewline`: `\n`→space, overflow line-break detection
- [x] `DrawStr`: bounding box overflow check at top of draw loop
- [x] `Dungeon.cpp` `OnChangeLevel`: `GetMsgs()->Clear()` before arrival message
- [x] `UseState.cpp`: Remove redundant "which item?" `Printf` calls from MsgsDT
- [x] `RangedState.cpp`: Error messages → `GetUse()->Printf()`; "emits X" gated on `IsIdentified()`
- [x] `TargetState.cpp` / `LookState.cpp`: Route `PrintRecall()` to `GetMonsterRecall()` panel; `*` auto-opens V panel
- [x] `Player.cpp` `DisplayInventory`: Fix filter misnumbering — `cListId` increments for all slots
- [x] `Dungeon.cpp` `UpdateVisibility`: Set `DUNG_FLAG_SEEN` alongside `DUNG_FLAG_VISIBLE` in Bresenham pass
- [x] Build clean (`make ascii`)

### Phase 2 — String Table 🔲

- [ ] Agree `STR_*` naming convention from audit below
- [ ] `src/Strings.h`: `STR_*` enum + `extern const char *g_Strings[STR_MAX]`
- [ ] `src/Strings.cpp`: `g_Strings` definition + `LoadStrings(szBasedir)`
- [ ] `src/Constants.h`: Register all `STR_*` values in `m_StringTable` + bump `NUM_STRINGS`
- [ ] `src/FileParse.h` / `FileParse.cpp`: `ReadStringEntry()` reading `Resources/Strings.txt` block format
- [ ] `Resources/Strings.txt`: New data file with all static player-visible strings
- [ ] `src/Effect.h`: Add `eEffectStrSlot` enum + `m_dwStrIds[EFFECT_STR_SLOT_MAX]` to `CEffectDef`
- [ ] `src/FileParse.cpp`: Parse `NameStr` / `HitPlayerStr` / `HitMonsterStr` / `EmitPlayerStr` fields in `ReadEffect()`
- [ ] `Resources/Effects.txt`: Add string fields to each effect
- [ ] `src/Effect.cpp` / `src/Player.cpp`: Replace static `Printf` strings with `g_Strings[...]` lookups
- [ ] All other static `Printf` sites migrated (see audit)
- [ ] Build clean, BDD tests pass
- [ ] Update WORKLIST_roadmap_phase3d_shiny.md, close GitHub issue #316

---

---

## Files Touched

### Phase 1 (done)

| File | Change |
|------|--------|
| `src/Constants.h` | `STATE_MORE = 13`, `STATE_MAX = 14` |
| `src/MorePromptState.h` | New minimal state header |
| `src/MorePromptState.cpp` | New minimal state implementation |
| `src/Game.h` / `Game.cpp` | Wire `m_pMorePromptState`, `SetState(STATE_MORE)` |
| `src/DisplayText.h` / `DisplayText.cpp` | `EnableMore()`, `m_bMoreOnNewline`, `\n`→space, overflow detection, bounding box fix |
| `src/Dungeon.cpp` | `OnChangeLevel`: clear MsgsDT; `UpdateVisibility`: set SEEN with VISIBLE |
| `src/UseState.cpp` | Remove redundant MsgsDT prompts |
| `src/RangedState.cpp` | Error messages → `GetUse()`; `IsIdentified()` gate on emits message |
| `src/TargetState.cpp` | `PrintRecall` → `GetMonsterRecall()`; auto-open V panel on `*` |
| `src/LookState.cpp` | `PrintRecall` → `GetMonsterRecall()` |
| `src/Player.cpp` | Fix `DisplayInventory` filter misnumbering |

### Phase 2 (planned)

| File | Change |
|------|--------|
| `src/Strings.h` | New — `STR_*` enum, `extern g_Strings[]` |
| `src/Strings.cpp` | New — array definition, `LoadStrings()` |
| `src/Constants.h` | Register `STR_*` in `m_StringTable` |
| `src/FileParse.h` / `FileParse.cpp` | `ReadStringEntry()` |
| `Resources/Strings.txt` | New data file |
| `src/Effect.h` | `eEffectStrSlot` enum, `m_dwStrIds[]` on `CEffectDef` |
| `Resources/Effects.txt` | Add string-slot fields |
| `src/Effect.cpp` | Replace static strings with `g_Strings[]` |
| `src/Player.cpp` | Replace static strings with `g_Strings[]` |
| `src/AIMgr.cpp`, `src/ModState.cpp`, `src/CmdState.cpp`, `src/Dungeon.cpp`, `src/UseState.cpp`, `src/RangedState.cpp`, `src/TargetState.cpp`, `src/LookState.cpp` | Replace static strings with `g_Strings[]` |

---

## String Audit — All `DT->Printf()` Call Sites

Legend:
- **[STR]** — migrates to `g_Strings`; `Printf(g_Strings[STR_KEY], arg…)` works for both static and format strings
- **[STR via slot]** — status verb fragments stored via `eEffectStrSlot` on the effect; composed at call site with `STR_STATUS_CHANGE` / `STR_STATUS_CHANGE_MON`
- **[STR_STAT_*]** — stats panel label → `g_Strings[STR_STAT_*]`
- **[STR_WIZ_*]** — wizard/debug message → `g_Strings[STR_WIZ_*]`
- **[STAYS]** — intentionally left in code (appearance name tables, RGBA data, `EffectFlagToString`)

### AIMgr.cpp

| Line | String | Tag |
|------|--------|-----|
| 248 | `"The %s wakes up!\n"` | STR |
| 306 | `"You hear a door smash open.\n"` | STR |
| 316 | `"You hear a door creak open.\n"` | STR |
| 371 | `"The %s %s you.\n"` (verb from `AttackFlavorText()`) | STR |
| 377 | `"(It was an excellent hit! (x2 damage)\n"` | STR |
| 586 | `"The %s %s you.\n"` | STR |
| 597 | `"The %s %s you.\n"` (verb = `AttackFlavorText()` or `"misses"`) | STR |
| — | `"misses"` (inline miss verb) | STR via slot |

### CmdState.cpp

| Line | String | Tag |
|------|--------|-----|
| 640 | `"I do not see any stairs here.\n"` | STR |
| 647 | `"You enter a maze of up staircases.\n"` | STR |
| 655 | `"You enter a long maze of up staircases.\n"` | STR |
| 664 | `"You enter a maze of down staircases.\n"` | STR |
| 672 | `"You enter a long maze of down staircases.\n"` | STR |
| 680 | `"You can't do that here.\n"` | STR |

### Dungeon.cpp

| Line | String | Tag |
|------|--------|-----|
| 262 | `"You are in town.\n"` | STR |
| 269 | `"You pass through a one-way door, to arrive on level %d.\n"` | STR |
| 1637 | `"You have found a secret door!\n"` | STR |
| 1794 | `"The %s disappears.\n"` | STR |
| 1835 | `"The room is flooded with light!\n"` | STR |
| 1838 | `"Nothing happens.\n"` | STR |

### Effect.cpp

| Line | String | Tag |
|------|--------|-----|
| 91 | `"You miss the %s.\n"` | STR |
| 99 | `"(Critical hit!)\n"` | STR |
| 119 | `"The %s dies.\n"` | STR |
| 121 | `"The %s is hit.\n"` | STR |
| 147 | `"The %s shrivels away in the bright light!\n"` | STR |
| 149 | `"The %s screams in agony.\n"` | STR |
| 153 | `"The %s is unaffected.\n"` | STR |
| 177 | `"The %s strikes the %s with %s.\n"` | STR |
| 198 | `"The %s shrugs off the attack!\n"` | STR |
| 203 | `"The %s is especially vulnerable!\n"` | STR |
| 206 | `"The %s is destroyed!\n"` | STR |
| 208 | `"The %s is hit.\n"` | STR |
| 248 | `"The %s shrugs off the attack!\n"` | STR |
| 254 | `"The %s is especially vulnerable!\n"` | STR |
| 256 | `"The %s is destroyed!\n"` | STR |
| 258 | `"The %s is hit.\n"` | STR |
| 283 | `"The %s affects %d creature%s.\n"` | STR |
| 288 | `"Nothing happens.\n"` | STR |
| 328 | `"The %s shrugs off the attack!\n"` | STR |
| 334 | `"The %s is especially vulnerable!\n"` | STR |
| 336 | `"The %s is destroyed!\n"` | STR |
| 338 | `"The %s is hit.\n"` | STR |
| 343 | `"The ball explodes harmlessly.\n"` | STR |
| 371 | `"The %s strikes the %s with %s.\n"` | STR |
| 380 | `"The %s shrugs off the attack!\n"` | STR |
| 385 | `"The %s is especially vulnerable!\n"` | STR |
| 388 | `"The %s is destroyed!\n"` | STR |
| 390 | `"The %s is hit.\n"` | STR |
| 408–416 | `"fall asleep"` / `"are paralyzed"` / `"flee in terror"` / `"feel confused"` | STR via slot |
| 417 | `"You %s.\n"` → `STR_STATUS_CHANGE` | STR |
| 427 | `"The %s is unaffected.\n"` | STR |
| 439–447 | `"falls asleep"` / `"is paralyzed"` / `"flees in terror"` / `"looks confused"` | STR via slot |
| 449 | `"The %s %s.\n"` → `STR_STATUS_CHANGE_MON` | STR |
| 468 | `"The %s crumbles to dust!\n"` | STR |
| 470 | `"The %s cracks!\n"` | STR |
| 479 | `"The wall turns to mud and collapses!\n"` | STR |
| 484 | `"The door dissolves!\n"` | STR |
| 489 | `"The secret door dissolves!\n"` | STR |
| 495 | `"Nothing happens.\n"` | STR |
| 524 | `"The %s vanishes!\n"` | STR |
| 530 | `"Nothing happens.\n"` | STR |
| 548 | `"The %s: HP %d/%d  AC %d  Lvl %d  Spd %.1f\n"` | STR |
| 572 | `"The %s looks healthier.\n"` | STR |
| 695 | `"The %s wakes up!\n"` | STR |

### LookState.cpp

| Line | String | Tag |
|------|--------|-----|
| 45 | `"Direction(1 2 3 4 6 7 8 9):\n"` | STR |
| 64 | `"You can't see that from here.\n"` | STR |
| 131 | `"You see here a %s.\nTarget selected.\n"` | STR |
| 145 | `"You see here a %s\n"` | STR |
| 185 | `"You see %s.\n"` | STR |

### ModState.cpp

| Line | String | Tag |
|------|--------|-----|
| 49, 103, 148, 325, 398 | `"Direction(1 2 3 4 6 7 8 9):\n"` | STR |
| 60 | `"You have picked the lock.\n"` | STR |
| 64 | `"You failed to pick the lock.\n"` | STR |
| 73 | `"The door is held fast.\n"` | STR |
| 77 | `"I do not see anything to open there.\n"` | STR |
| 114 | `"You have closed the door.\n"` | STR |
| 118 | `"You failed to close the door.\n"` | STR |
| 123 | `"I do not see anything to close there.\n"` | STR |
| 159 | `"You have removed the rubble.\n"` | STR |
| 163 | `"You dig in the rubble...\n"` | STR |
| 168 | `"Tunnel through what? Empty air?.\n"` | STR |
| 332 | `"I do not see a closed door there.\n"` | STR |
| 339 | `"The spike breaks.\n"` | STR |
| 349 | `"You spike the door shut.\n"` | STR |
| 354 | `"You have no iron spikes.\n"` | STR |
| 404 | `"I do not see a door there.\n"` | STR |
| 408 | `"You bash the door open!\n"` | STR |
| 414 | `"You slam against the door but it holds.\n"` | STR |

### Player.cpp — gameplay messages

| Line | String | Tag |
|------|--------|-----|
| 168, 200 | `"You have found a secret door!\n"` | STR |
| 176 | `"You found nothing.\n"` | STR |
| 625 | `"You have %d %s.\n"` | STR |
| 636 | `"You have a %s.\n"` | STR |
| 693, 712 | `"You were wielding the %s..."` | STR |
| 777 | `"You can't remove the %s... it seems to be cursed.\n"` | STR |
| 862 | `"You switch to your %s.\n"` | STR |
| 866 | `"You switch to your bare hands.\n"` | STR |
| 919 | `"Your light has gone out!\n"` | STR |
| 923 | `"Your light is growing very faint.\n"` | STR |
| 927 | `"Your light is growing faint.\n"` | STR |
| 978 | `"You %s the %s.\n"` (hit/miss + monster name) | STR |
| 984 | `"(It was an excellent hit! (x2 damage)\n"` | STR |
| 993 | `"You %s the %s.\n"` ("have slain") | STR |
| 1021 | `"Ouch! You bumped into %s!\n"` | STR |
| 1107 | `"The %s wakes up!\n"` | STR |
| 1150 | `"Welcome to level %d.\n"` | STR |
| 1218 | `"You are immune!\n"` | STR |
| 1220 | `"You resist!\n"` | STR |
| 1402 | `"You organize your pack.\n"` | STR |
| 1426 | `"Your %s cannot be refueled.\n"` | STR |
| 1464 | `"You are no longer afraid.\n"` | STR |
| 1472 | `"You can see again.\n"` | STR |
| 1480 | `"You can think clearly again.\n"` | STR |
| 1488 | `"You are no longer poisoned.\n"` | STR |
| 1496 | `"You can move again.\n"` | STR |
| 1504 | `"You wake up.\n"` | STR |
| 1525 | `"You feel amazing!\n"` | STR |
| 1529 | `"You feel a lot better.\n"` | STR |
| 1533 | `"You feel better.\n"` | STR |
| 1537 | `"You feel a bit better.\n"` | STR |
| 1564 | `"You feel more protected. (+%d AC)\n"` | STR |
| 1607 | `"Your %s %s!\n"` (burns/melts/etc.) | STR |
| 1647 | `"Your %s is pitted by %s! (to-hit reduced)\n"` | STR |
| 1653 | `"Your %s is pitted by %s! (to-damage reduced)\n"` | STR |
| 1664 | `"Your %s is damaged by %s!\n"` | STR |
| 1691 | `"The area around you is revealed.\n"` | STR |
| 1696 | `"The dungeon is revealed to you.\n"` | STR |
| 1727 | `"You feel a brief shimmer.\n"` | STR |
| 1732 | `"Nothing happens.\n"` | STR |
| 1738 | `"You feel a wrenching sensation.\n"` | STR |
| 1750 | `"You already feel the pull of recall.\n"` | STR |
| 1762 | `"You feel yourself starting to drift...\n"` | STR |
| 1776 | `"Recall depth reset (was: %dft)\n"` | STR |
| 1781 | `"The world spins and you find yourself in town.\n"` | STR |
| 1785 | `"The world spins and you are back at %d ft.\n"` | STR |
| 1807 | `"It is now cursed.\n"` | STR |
| 1829 | `"It is no longer cursed.\n"` | STR |
| 1855 | `"You feel resistant to fear.\n"` | STR |
| 1857 | `"You are afraid!\n"` | STR |
| 1860 | `"You are blind.\n"` | STR |
| 1863 | `"You are confused.\n"` | STR |
| 1867 | `"You feel resistant to poison.\n"` | STR |
| 1869 | `"You are poisoned.\n"` | STR |
| 1872 | `"You can't move!\n"` | STR |
| 1875 | `"You fall asleep.\n"` | STR |
| 1878 | `"Your eyes feel tingly.\n"` | STR |
| 1881 | `"You sense stray thoughts around you.\n"` | STR |
| 1885 | `"You feel resistant to fire.\n"` | STR |
| 1889 | `"You feel resistant to cold.\n"` | STR |
| 1893 | `"You feel resistant to electricity.\n"` | STR |
| 1897 | `"You feel resistant to acid.\n"` | STR |
| 1900 | `"You fade from view.\n"` | STR |
| 1903 | `"You feel light on your feet.\n"` | STR |
| 1906 | `"You feel free to move.\n"` | STR |
| 1909 | `"You feel yourself moving faster.\n"` | STR |
| 1935 | `"You are no longer resistant to fear.\n"` | STR |
| 1944 | `"You are no longer resistant to poison.\n"` | STR |
| 1957 | `"Your eyes stop tinging.\n"` | STR |
| 1960 | `"You no longer sense stray thoughts.\n"` | STR |
| 1963 | `"You no longer feel resistant to fire.\n"` | STR |
| 1966 | `"You no longer feel resistant to cold.\n"` | STR |
| 1969 | `"You no longer feel resistant to electricity.\n"` | STR |
| 1972 | `"You no longer feel resistant to acid.\n"` | STR |
| 1975 | `"You reappear.\n"` | STR |
| 1978 | `"You float gently to the ground.\n"` | STR |
| 1981 | `"You feel sluggish.\n"` | STR |
| 1984 | `"You feel yourself slowing down.\n"` | STR |
| 1992 | `"You feel less protected.\n"` | STR |
| 2010 | `"You feel completely healthy.\n"` | STR |
| 2013 | `"Nothing happens.\n"` | STR |
| 2031 | `"You feel more experienced.\n"` | STR |
| 2050 | `"You feel less experienced.\n"` | STR |
| 2061 | `"You feel weakened.\n"` | STR |
| 2117 | `"You sense the presence of doors!\n"` | STR |
| 2119 | `"You sense the presence of stairs!\n"` | STR |
| 2140 | `"You sense traps.\n"` | STR |
| 2169 | `"You sense the presence of monsters!\n"` | STR |
| 2195 | `"You sense the presence of treasure!\n"` | STR |
| 2197 | `"You sense no treasure nearby.\n"` | STR |
| 2250 | `"*** Wizard Mode: On *** ...\n"` | STR_WIZ_* |
| 2331 | `"Wizard: added %d wands/staves...\n"` | STR_WIZ_* |
| 2354 | `"Wizard: identified %d items.\n"` | STR_WIZ_* |
| 2373 | `"It is %s.\n"` (identify) | STR |
| 2383 | `"The %s explodes in a shower of sparks!\n"` | STR |
| 2389 | `"The %s glows with magical energy. (%d charges)\n"` | STR |
| 2396, 2401 | `"It glows with power.\n"` | STR |
| 2408 | `"It glows with a soft light.\n"` | STR |
| 2417 | `"It is now cursed.\n"` | STR |
| 2422 | `"It is no longer cursed.\n"` | STR |

### Player.cpp — inventory/equip headers (`sprintf` into `meta.header`)

| Line | String | Tag |
|------|--------|-----|
| 356 | `"Quaff which potion?\n"` | STR |
| 359 | `"Read which scroll?\n"` | STR |
| 362 | `"Wield which item?\n"` | STR |
| 365 | `"Zap which wand?\n"` | STR |
| 368 | `"Fire which ammo?\n"` | STR |
| 371 | `"Use which staff?\n"` | STR |
| 374 | `"You are Carrying:\n"` | STR |
| 337 | `"Inventory past first page not shown.\n"` | STR |
| 441 | `"Equipment is limited to 10 items...\n"` | STR |
| 446 | `"Fire which weapon?\n"` | STR |
| 449 | `"You are wearing:\n"` | STR |

### RangedState.cpp

| Line | String | Tag |
|------|--------|-----|
| 82 | `"You have nothing to fire with.\n"` | STR |
| 102 | `"You have nothing to fire.\n"` | STR |
| 116 | `"Zap which wand? [a-z]\n"` | STR |
| 191 | `"Choose ammo from inventory (a to z):\n"` | STR |
| 229, 304, 569 | `"Nothing happens.\n"` | STR |
| 237 | `"Choose target with * or Directional (1 2 3 4 6 7 8 9)\n"` | STR |
| 245 | `"You can't fire a %s!\n"` | STR |
| 270 | `"Choose an item from inventory(a to z):\n"` | STR |
| 313 | `"Choose target with * or Directional (1 2 3 4 6 7 8 9)\n"` | STR |
| 321 | `"You can't zap a %s!\n"` | STR |
| 343 | `"Choose target: * or Direction (1 2 3 4 6 7 8 9)\n"` | STR |
| 469 | `"Nothing valid to %s.\n"` | STR |
| 587 | `"The %s emits a %s.\n"` | STR |
| 591 | `"The %s glows.\n"` | STR |
| 898 | `"The arrow breaks.\n"` | STR |

### TargetState.cpp

| Line | String | Tag |
|------|--------|-----|
| 49 | `"(* for target, . to choose, ESC to exit):\n"` | STR |
| 205 | `"Target selected.\n"` | STR |
| 209 | `"You can no longer see that target.\n"` | STR |

### UseState.cpp

| Line | String | Tag |
|------|--------|-----|
| 86, 438, 545, 590 | `"Cancelled.\n"` | STR |
| 130, 233, 292, 348 | `"Choose an item from inventory(a to z):\n"` | STR |
| 140 | `"You are now wielding the %s.\n"` | STR |
| 146 | `"You are now wearing the %s.\n"` (inferred) | STR |
| 154 | `"You can't wield a %s!\n"` | STR |
| 180 | `"Choose an item from equipment(a to z):\n"` | STR |
| 190 | `"You take off the %s.\n"` | STR |
| 195 | `"You can't remove that!\n"` | STR |
| 201 | `"The %s is welded to your body!\n"` | STR |
| 248 | `"How many? (1-%d, * for all): "` | STR |
| 255 | `"You dropped the %s.\n"` | STR |
| 259 | `"You dropped %d %s.\n"` (inferred) | STR |
| 266 | `"You can't drop a %s here!\n"` | STR |
| 298 | `"You can't read a %s!\n"` | STR |
| 309, 320 | `"You read the %s.\n"` | STR |
| 324 | `"The %s slips from your fingers and returns to your pack!\n"` | STR |
| 358 | `"You drank the %s.\n"` | STR |
| 362 | `"You drank %d %s.\n"` (inferred) | STR |
| 369 | `"You can't drink a %s!\n"` | STR |
| 462 | `"Please select a valid item.\n"` | STR |
| 507 | `"No such item. %s\n"` | STR |
| 564 | `"Invalid quantity.\n"` | STR |
| 574 | `"You dropped %d.\n"` | STR |
| 578 | `"Could not drop items.\n"` | STR |
| 737 | `"Choose an item from inventory (a to z):\n"` | STR |
| 745 | `"You can't use a %s as a staff!\n"` | STR |
| 755, 784–788 (staff) | `"Nothing happens.\n"` / `"The %s emits a %s.\n"` / `"The %s glows.\n"` | STR |
| 815 | `"Choose an item from inventory(a to z):\n"` | STR |
| 824 | `"You fill your lantern with the %s.\n"` | STR |
| 829 | `"You have no lantern to fill.\n"` | STR |
| 834 | `"You can't use a %s as fuel!\n"` | STR |

### Inline string tables — final decisions

| Location | Content | Decision |
|----------|---------|----------|
| `Effect.cpp` 408–416 | Status verbs (player): `"fall asleep"`, `"are paralyzed"`, `"flee in terror"`, `"feel confused"` | **STR via slot** — stored as `EFFECT_STR_STATUS_PLAYER` on each effect |
| `Effect.cpp` 439–447 | Status verbs (monster): `"falls asleep"`, `"is paralyzed"`, `"flees in terror"`, `"looks confused"` | **STR via slot** — stored as `EFFECT_STR_STATUS_MONSTER` on each effect |
| `AIMgr.cpp` `"misses"` | Attack miss verb | **STR via slot** — `EFFECT_STR_STATUS_MONSTER` reused, or dedicated slot TBD |
| `Constants.h` `PotionColor[]` / `ScrollName[]` / `Lumber[]` / `Metal[]` | Un-ID'd appearance names (32 entries each, shuffled) | **STAYS** — `Shuffle()` makes string table impractical; stays as inline array |
| `Constants.h` `PotionRGBA[]` / `LumberRGBAs[]` / `MetalRGBAs[]` | RGBA color data `"r,g,b,a"` | **STAYS** — not player-visible text, render data |
| `Constants.h` `EffectFlagToString()` | Maps flag → internal flag name (e.g. `"EFFECT_FLAG_FIRE"`) | **STAYS** — system/debug utility, not a gameplay message |

### Stats panel (`Player.cpp` 262–329, re-rendered every frame)

| String | Tag |
|--------|-----|
| `"Name: %s"`, `"Race: %s"`, `"Class: %s"` | STR_STAT_* |
| `"AC: %d"`, `"HP: %d / %d"`, `"Damage: %s"`, `"+to Hit: %d"`, `"+to Dam: %d"` | STR_STAT_* |
| `"Speed: Fast(+%d)"`, `"Speed: Slow(%d)"` | STR_STAT_* |
| `"Level: %d"`, `"Depth: %d'"`, `"Exp: %d"`, `"Light: %d turns"` | STR_STAT_* |
| `"Infravision"`, `"Telepathy"`, `"Res: Poison"`, `"Res: Fear"` | STR_STAT_* (also `EFFECT_STR_STAT_LABEL` on effect) |
| `"Paralyzed"`, `"Blind"`, `"Asleep"`, `"Confused"` | STR_STAT_* (also `EFFECT_STR_STAT_LABEL` on effect) |
| `"Target: %s"`, `"Pos: <%.0f %.0f>"`, `"Target Pos: <%.0f %.0f>"`, `"Player Pos: <%.0f %.0f>"` | STR_STAT_* |
| `"** WIZARD MODE **"` | STR_WIZ_* |
