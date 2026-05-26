# Issue #316 — Strings Table / Messages Improvements

**Status**: In Progress  
**Last Updated**: May 25, 2026  
**Parent Roadmap**: WORKLIST_roadmap_phase3d_shiny.md

---

## Summary

Three deliverables:
1. **Message display improvements** — MsgsDT message flow, overflow handling, routing cleanup. ✅ **Done.**
2. **String table** — `Resources/Strings.txt` data file + `CStringTable` parser. `CGame::GetStr("KEY")` returns looked-up strings. 🔲 **Up next.**
3. **Effect-flag string standardization** — Messages tied to `EFFECT_FLAG_*` constants moved into Strings.txt with canonical keys like `EFFECT_HIT_FIRE`, `EFFECT_RESIST_FIRE`, etc. 🔲 **Follows string table.**

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

- `Resources/Strings.txt` — new data file, same angle-bracket format as Monsters.txt / Items.txt.
- `CStringTable` class (`StringTable.h` / `StringTable.cpp`) parses the file via `CDataFile`/`FileParse`.
- `CGame::GetStr("KEY")` returns the string for a given key (falls back to the key itself if missing, so partial migration never crashes).
- `\n` inside a Strings.txt value means a real newline — the string is intended for panels that don't have `EnableMore` (e.g., monster recall, help screens) or for future paginated display.
- Effect-flag messages get canonical keys: `EFFECT_HIT_FIRE`, `EFFECT_RESIST_FIRE`, etc.

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

- [ ] `Resources/Strings.txt`: New data file
- [ ] `src/StringTable.h` / `src/StringTable.cpp`: Parser class
- [ ] `src/Game.h` / `Game.cpp`: `GetStr("KEY")` method, load/term lifecycle
- [ ] Audit all `GetMsgs()->Printf(...)` and `GetUse()->Printf(...)` call sites — identify strings that belong in the table vs. stay inline (dynamic/format strings stay inline)
- [ ] Migrate static player-visible strings to Strings.txt entries
- [ ] Effect-flag strings in Strings.txt, used in `Effect.cpp` / `Player.cpp`
- [ ] Build clean, BDD tests pass
- [ ] Update WORKLIST_roadmap_phase3d_shiny.md, close GitHub issue #316

---

## Strings.txt Format

```
# Strings.txt — Player-visible string table
# Same angle-bracket format as Monsters.txt / Items.txt

String <PLAYER_LEVEL_UP>
{
    Text    <You feel more experienced.>
}

String <EFFECT_HIT_FIRE>
{
    Text    <You are engulfed in flames!\nThe heat sears your flesh.>
}
```

`\n` in a Text value is a literal newline in the returned string. Callers that pass the string to a panel with `EnableMore()` will have it converted to a space (flowing text); callers that pass to a raw panel (recall, help) get a real line break.

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
| `Resources/Strings.txt` | New data file |
| `src/StringTable.h` | New class |
| `src/StringTable.cpp` | New class |
| `src/Game.h` / `Game.cpp` | `GetStr()` method, load/term |
| `src/Effect.cpp`, `src/Player.cpp` | Use `GetStr()` for static effect messages |
