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

## Key Finding: Two Unrelated Index Spaces

The codebase has two completely separate "index" concepts that were being conflated:

1. **CLink::m_dwIndex** — auto-assigned list position (0, 1, 2...) by `JLinkList::Add()`
2. **CMonsterDef::m_dwIndex / CItemDef::m_dwIndex** — TYPE constant from Constants.h (`MON_IDX_DRAGON=3`, `ITEM_IDX_DAGGER=26`)

These are independent. Multiple items share the same type index (6 swords all have `ITEM_IDX_SWORD=0`). No runtime code path depends on list position — spawning uses random selection + level filtering; rendering uses the type constant for glyph lookup; inventory/equipment uses type constants for slot keying.

**Resource file order does not matter for gameplay.** Entries can be freely reordered in Monsters.txt / Items.txt without breaking anything.

## Remaining Work

- [ ] **Monster spawn population changed** — Flipping list order changed which random indices map to which monsters. Level filtering still works correctly, but the population mix on early floors shifted (e.g. Icky Things → Worm Masses). The real fix is filter-first selection: collect all eligible monsters, then pick randomly from that pool. See [issue #226](https://github.com/Rushwind13/JMoria/issues/226).

- [ ] **GetLink bForceValid masks real bugs** — `GetLink(N, bForceValid=true)` silently returns the last item when the requested index doesn't exist. This can mask incorrect lookups. Consider making bForceValid default to `false` and auditing callers.

- [ ] **Vestigial `:NN` params in feature files** — Steps like `I spawn a Dagger:37` still carry a numeric suffix. The step regex extracts it but it's never used. These could be removed, but it's a cosmetic cleanup with regex changes across many steps. Low priority.

- [ ] **`#else` dead code in SpawnMonsters** — `m_llMonsterDefs->length() - 1` in the `#else` branch was a debug hack. `RANDOM_MONSTER` is always defined, making it dead code. Remove it.
