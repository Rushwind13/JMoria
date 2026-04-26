# Phase 3b: Jetsam — Cleanup, Bugs, and Overflow Features

**Repo**: `Rushwind13/JMoria`
**Branch**: `phase3b_jetsam`  
**Date Started**: April 26, 2026  
**Description**: This phase tackles uncovered issues from the previous analysis: minor bugs, code cleanup tasks, dungeon generation improvements, and post-phase-3 gameplay systems that don't fit neatly into the main Priority 1-5 roadmap.

---

## 🐛 Phase 3b Issues

### Critical Bug Fixes (2 Issues)

#### #165 - Telepathy Lasts Forever
**Status**: Not started  
**Type**: Bug  
**Description**: Telepathy potion effect does not expire; lasts indefinitely instead of timing out like other temporary effects.
- Current: Potion of Telepathy applies EFFECT_FLAG_ESP permanently
- Expected: Temporary duration (100-200 turns typical for potions)
- **Root Cause**: Likely missing EFFECT_MOD_TIMED on the effect, or duration tracking not implemented for ESP

**Impact**: Gameplay imbalance (permanent detection is overpowered), inconsistent potion behavior  
**Dependencies**: None blocking  
**Effort**: Low (likely one-line fix in Effects.txt or effect application logic)  
**Files**: `Resources/Effects.txt`, `src/Player.cpp` (ApplyEffect logic)

---

#### #166 - Wand of Light Does Not Light Tiles
**Status**: Not started  
**Type**: Bug  
**Description**: Wand of Light damages light-weak creatures but fails to actually light tiles/rooms.
- **Expected**: Wand projectile creates light as it passes (line of light), and area effect lights current room
- **Actual**: Damages monsters but doesn't set DUNG_FLAG_VISIBLE on affected tiles
- **Related**: Staff of Light (works), Scroll of Light (works), so the pattern exists

**Impact**: Inconsistent light item behavior, reduced utility of wand  
**Dependencies**: #72 (Fog of War / Lighting system) already complete  
**Effort**: Low-Medium (integrate existing lighting logic into wand hit handler)  
**Files**: `src/Player.cpp` (Wand zap handler), `src/Dungeon.cpp` (LightArea call)

---

### Code Cleanup (2 Issues)

#### #176 - Move srand/time Seed from main.cpp into Game::Init()
**Status**: Not started  
**Type**: Enhancement (Code cleanup)  
**Description**: Remove `srand(time(NULL))` from `main.cpp` and move to `Game::Init()`.
- **Current**: main.cpp includes `<ctime>` and calls `srand(time(NULL))`
- **Goal**: Reduce includes in main.cpp, centralize initialization in Game class
- **Benefit**: Cleaner separation of concerns; main.cpp only handles CLI parsing

**Impact**: Code organization, reduced coupling  
**Dependencies**: None blocking  
**Effort**: Low (3-line move)  
**Files**: `src/main.cpp`, `src/Game.cpp`

---

#### #175 - Replace strcmp with jstrcmp in main.cpp CLI Parsing
**Status**: Not started  
**Type**: Enhancement (Code cleanup)  
**Description**: `main.cpp` uses `strcmp()` for `--renderer=` argument parsing; replace with existing `jstrcmp()`.
- **Current**: `#include <cstring>` for strcmp
- **Goal**: Drop last C standard library include from main.cpp
- **Benefit**: Consistent string handling, cleaner header includes

**Impact**: Code consistency, reduced includes  
**Dependencies**: None blocking  
**Effort**: Low (1-line replace)  
**Files**: `src/main.cpp`

---

### Gameplay Systems (2 Issues)

#### #271 - Item Destruction from Elemental Attacks
**Status**: Not started  
**Type**: Gameplay  
**Description**: When monsters deal elemental damage (fire, cold, acid), items in player's inventory should take damage/be destroyed.
- **Fire Damage**: Destroys scrolls (50-100%), potions (50-100%), leather items (25-50%)
- **Cold Damage**: Shatters potions (100%), reduces movement speed
- **Acid Damage**: Destroys scrolls (100%), potions (100%), metal items heavily damaged (75-100%)
- **Interactions**: Fire resistance potions / items can reduce/negate damage; acid resistance same

**Impact**: Adds inventory risk/reward, creates item economy depth, forces strategic loot choices per level  
**Dependencies**: #77 (Item Effects) — elemental damage pipeline; #246 (Item Durability) — durability system framework; PR #235 (Ranged Attacks) — coordinate with combat refactor  
**Effort**: High  
**Blocking**: Pairs with item durability system; enables dungeon threats to matter  
**Files**: `src/Player.cpp` (monster hit handler), `src/Item.cpp` (destruction logic), `Resources/Items.txt` (item vulnerability metadata)

---

#### #236 - Monster Recall — Knowledge of the Ancestors
**Status**: Not started  
**Type**: Gameplay (Knowledge/UI System)  
**Description**: Build persistent monster knowledge glossary as player encounters creatures. On encountering a monster type again, display accumulated knowledge.
- **Tracked Data**: Encounter count, physical description, typical depth, XP value per level, attack types, resistances/weaknesses, special abilities
- **Storage**: Local file `~/.jmoria/monster_recall.txt` (persists across games)
- **Display**: Popup or dedicated UI pane showing knowledge when targeting/encountering a known monster
- **Example Message**: "You have encountered 7 of these creatures. Taller than an average orc, white handprint across its face. Normally lives at 450' and is worth 250 XP for a 13th level character. Harmed by bright light and showers. Resistant to fear and fire. Can pick up and wield weapons."

**Impact**: Enables emergent learning system, rewards repeated play, adds flavor/lore  
**Dependencies**: None blocking (future UI framework); pairs with #45 (Monster behavior depth)  
**Effort**: Very High (requires UI pane, file I/O, knowledge parsing + formatting)  
**Blocking**: Lays groundwork for Item Recall (#114 extension) and Bestiary systems  
**Files**: New `src/MonsterRecall.cpp`, `src/DisplayText.cpp` (new pane), `Player.cpp` (encounter tracking)

---

### Dungeon Generation (1 Issue)

#### #226 - Monster Spawn Control: Filter-First Selection, Themed Levels, Room-Specific Spawns
**Status**: Not started  
**Type**: Enhancement (Dungeon Generation)  
**Description**: Improve monster spawn selection from current retry-loop model to filter-first approach, enabling themed levels and room-specific spawns.
- **Current Problem**: `ChooseMonsterForDepth()` picks random index, checks level, retries up to 10 times. Fragile to list ordering.
- **Phase 1 (filter-first)**: Collect all eligible monsters first, then pick random from pool. Eliminates retry logic.
- **Phase 2 (themed levels)**: Assign themes (Sewer, Icebox, Zoo, Crypt, Inferno) to dungeon levels; restrict spawn pool to thematic creatures
- **Phase 3 (room-specific)**: Individual rooms carry spawn override metadata; e.g., lava room spawns fire creatures only

**Impact**: Better dungeon cohesion, improved spawn distribution independence from file order, enables specialized encounters  
**Dependencies**: Dungeon generation system (already functional)  
**Effort**: Medium-High (phase 1 low, phase 2-3 require generation system extension)  
**Blocking**: Enables monster encounter balance testing  
**Files**: `src/Dungeon.cpp` (ChooseMonsterForDepth), `src/DungeonMap.cpp` (spawn metadata)

---

## Implementation Roadmap

**Suggested Order** (by impact and dependency):

1. **#176, #175** (code cleanup, 30 min) — Quick wins, unblock main.cpp
2. **#165, #166** (bug fixes, 1-2 hours) — High-value low-effort fixes
3. **#226 Phase 1** (spawn filter, 1-2 hours) — Foundation for dungeon quality
4. **#271** (item destruction, 2-3 days) — Requires coordination with combat refactor
5. **#236** (monster recall, 2-3 days) — Ambitious but high flavor/engagement value
6. **#226 Phase 2-3** (themed levels, 1-2 days after Phase 1) — Polish dungeon generation

---

## Testing Strategy

- **#176, #175**: Compile-test only (no behavioral change)
- **#165, #166**: BDD scenarios for potion duration, wand lighting, combat logging
- **#226**: Test spawn distribution across 100 dungeons; verify filter-first matches old random selection (deterministic comparison with same seed)
- **#271**: Unit tests for item vulnerability types; integration tests with monster elemental attacks
- **#236**: Manual testing with repeated monster encounters; verify knowledge.txt persistence

---

## Notes

- These 8 issues represent "overflow" from the main Priority 1-5 roadmap
- #271 and #236 are significant gameplay additions; rest are bugs/polish
- #226 Phase 1 is low-risk refactor; Phases 2-3 add thematic depth
- All issues are independent except #271 (depends on ongoing combat refactor coordination)

---

## References

- [Core Roadmap](WORKLIST_jmoria_core_roadmap.md) — main Priority 1-5 systems
- [Bot Roadmap](WORKLIST_bot_roadmap.md) — AI testing infrastructure
- [GitHub Issues #165, #166, #226, #176, #175, #271, #236](https://github.com/Rushwind13/JMoria/issues) — detailed specs
