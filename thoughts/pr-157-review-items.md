# PR #157 Review Items

Tracking file for review comments from the AI Observability PR.

Status key: `[ ]` pending, `[x]` done, `[~]` partial, `[-]` wontfix

---

## AIMgr.cpp

- [x] **Line 203**: Pre-calculate `m_szState` (change when state changes, not every update)
- [x] **Line 221**: Use existing JLog code instead of duplicate logging structure (`LOG_LEVEL_AI`)

## Dungeon.cpp

- [ ] **Line 16**: Code smell - extern MonIDs/ItemIDs violates separation of dungeon/item/monster draw
- [x] **Line 829**: RLE encoding belongs in Draw/Render, not Dungeon
- [ ] **Line 876**: RLE is a render concern; AI should ignore unlit tiles; use class structures instead of string traversal
- [x] **Line 891**: Use `JLog(LOG_LEVEL_AI, ...)` instead of AILog
- [x] **Line 904**: Use `JVector` instead of separate px, py variables
- [x] **Line 908**: Use `JRect` for view bounds
- [x] **Line 914**: Use `JVector.IsInWorld()` for bounds checking
- [ ] **Line 923**: Performance concern - lots of stack allocation every frame
- [ ] **Line 937**: Abstract tile char lookup - already exists somewhere
- [x] **Line 963**: Use existing "is pos in rect" function (Util.h or JRect::Contains)
- [x] **Line 984**: Use pos in rect (JRect::Contains)
- [ ] **Line 1004**: Bit-banging output is correct JMoria behavior, but JSON compatibility is awkward
- [ ] **Line 1017**: Wall type should use DUNG_FLAG_LIT or distinguish tunnelable walls

## Game.cpp / Game.h

- [x] **Line 52**: Error - `m_dwTurnCount` duplicates `m_fGameTime`
- [x] **Line 646**: Delete duplicate, move `m_fGameTime++` here, use JLog
- [x] **Game.h:57**: Delete `m_dwTurnCount` declaration
- [x] **Game.h:100**: Delete `GetTurnCount()` accessor

## Player.cpp / Player.h

- [ ] **Line 589**: Use existing pretty vector printing function
- [ ] **Player.h:178**: Question about `m_szKilledBy` visibility (should it be outside Player?)

## main.cpp

- [x] **Line 41**: AI log directory creation belongs in JLog code
- [x] **Line 149**: `AILog_Init`/`AILog_Term` pattern should be owned by `g_pGame`

## Documentation

- [-] **thoughts/ai-logging.md:14**: JSON may hurt at some point (noted, acceptable for now)
- [x] **thoughts/ai-logging.md:369**: AI logging should be off by default (env var: JMORIA_AI_LOG=1)
- [ ] **thoughts/ai-logging.md:371**: Future: AI should be able to play (send input, watch logs)
- [x] **thoughts/architecture.md:1**: Wrong filename - moved to `doc/rendering_architecture.md`

---

## Summary

| Area | Done | Pending |
|------|------|---------|
| AIMgr.cpp | 2 | 0 |
| Dungeon.cpp | 6 | 6 |
| Game.cpp/h | 4 | 0 |
| Player.cpp/h | 0 | 2 |
| main.cpp | 2 | 0 |
| Documentation | 2 | 2 |
| **Total** | **16** | **10** |

---

## Notes

Items marked done in this PR:
- m_szState pre-calculation (CAIBrain)
- JLog integration (LOG_LEVEL_AI)
- JVector/JRect usage in DumpToAILog
- JRect::Contains for bounds checking
- Removed m_dwTurnCount, use m_fGameTime via GetITime()
- Moved AILog init/term to CGame (owned by g_pGame)
- AI logging off by default (env var JMORIA_AI_LOG=1 to enable)
- Moved architecture.md to doc/rendering_architecture.md

Deferred items (separate PRs or future work):
- Move RLE encoding to Render
- Remove extern MonIDs/ItemIDs
- AI logging off by default
- AI playing capability
