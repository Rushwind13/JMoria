# WORKLIST: Resource Indexing

## Completed

- [x] **JLinkList::Add() reversed insertion order** — `Add()` with no index used to prepend (head-insert), making the in-memory list the reverse of the resource file order. Changed to append (tail-insert) so list order matches file order. Added `m_lpTail` for O(1) append. All 96 BDD tests pass.
- [x] **JLinkList::m_lpTail** — Added tail pointer, maintained in Add/Remove/Terminate. Eliminates O(n) traversal for every append during resource loading.
- [x] **list_resource.sh compensating reversal** — `sort -nr` was used to reverse grep output to match the old reversed runtime order. Removed; now uses file order directly.
- [x] **find_resource.sh misleading comment** — Referenced "reverse lines" but didn't actually reverse. Cleaned up pipeline.
- [x] **tilesets.feature @skip for ASCII builds** — 4 tileset scenarios require OpenGL; tagged `@skip` for ASCII-only builds.
- [x] **Off-by-one in GetMonsterDef / GetItemDef boundary check** — Both used `>= length() - 1` which excluded the last valid entry. Fixed to `>= length()`.
- [x] **GetItemDef(int) inconsistency** — Used `GetNthLink()` (position-based) while `GetMonsterDef(int)` used `GetLink()` (index-keyed). Standardized both to `GetLink()`.
- [x] **Feature file equip/remove steps used wrong integer indices** — `the player equips the item 27` called `GetItemDef(27)` which returned a random item def unrelated to the spawned item. Tests passed only because `GetLink(bForceValid=true)` returns any available item on a single-item inventory. Fixed: steps now use name-based lookup (`the player equips the Dagger`). Monster spawn steps in GameSteps.cpp also converted from integer to name-based lookup.
- [x] **Vestigial `:NN` params in feature files** — Removed numeric suffixes from all feature files and step definition regexes. Steps like `I spawn a Dagger:37` are now just `I spawn a Dagger`.
- [x] **CRenderNull for headless tests** — Tests using `RenderMode::None` now get a no-op renderer instead of NULL, preventing ncurses terminal corruption and NULL pointer crashes.
- [x] **Log level overhaul** — Systematic audit of DungeonMap, Dungeon, Player, Monster, and test step definitions. Normal retries downgraded from ERROR/WARN to DEBUG/NOISIER. Gameplay messages (cursed items, equip conflicts) WARN→INFO. Diagnostic test prints ERROR→DEBUG or removed. DumpMap() WARN→DEBUG. Test log level set to WARN for clean output.
- [x] **DirName() and StairName() helpers** — Direction integers and stair type integers now print as human-readable names in log output instead of raw numbers.
- [x] **JFAILED result code** — Added `#define JFAILED 4` for dungeon generation functions that fail normally (e.g. room placement retries). Distinct from JERROR() which logs at ERROR level.
- [x] **Feature file ordering** — Renamed `dungeonmap.feature` → `_dungeonmap.feature` and `vectors.feature` → `_vectors.feature` so visual/tall tests run first and don't eat scrollback.

## Key Finding: Two Unrelated Index Spaces

The codebase has two completely separate "index" concepts that were being conflated:

1. **CLink::m_dwIndex** — auto-assigned list position (0, 1, 2...) by `JLinkList::Add()`
2. **CMonsterDef::m_dwIndex / CItemDef::m_dwIndex** — TYPE constant from Constants.h (`MON_IDX_DRAGON=3`, `ITEM_IDX_DAGGER=26`)

These are independent. Multiple items share the same type index (6 swords all have `ITEM_IDX_SWORD=0`). No runtime code path depends on list position — spawning uses random selection + level filtering; rendering uses the type constant for glyph lookup; inventory/equipment uses type constants for slot keying.

**Resource file order does not matter for gameplay.** Entries can be freely reordered in Monsters.txt / Items.txt without breaking anything.

## Remaining Work

- [ ] **Monster spawn population changed** — Flipping list order changed which random indices map to which monsters. Level filtering still works correctly, but the population mix on early floors shifted (e.g. Icky Things → Worm Masses). The real fix is filter-first selection: collect all eligible monsters, then pick randomly from that pool. See [issue #226](https://github.com/Rushwind13/JMoria/issues/226).

- [ ] **GetLink bForceValid masks real bugs** — `GetLink(N, bForceValid=true)` silently returns the last item when the requested index doesn't exist. This can mask incorrect lookups. Consider making bForceValid default to `false` and auditing callers.

- [ ] **`#else` dead code in SpawnMonsters** — `m_llMonsterDefs->length() - 1` in the `#else` branch was a debug hack. `RANDOM_MONSTER` is always defined, making it dead code. Remove it.

- [ ] **Log level audit** — Most ERROR/WARN misuse has been fixed, but a full audit of remaining JLog calls (especially in game-time code paths) would catch any remaining cases where normal gameplay emits WARN/ERROR.
