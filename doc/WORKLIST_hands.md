# WORKLIST: Monsters Can Open Doors (Issue #297)

**Status**: Not started
**Related Issue**: [#297 — Monsters can open doors](https://github.com/Rushwind13/JMoria/issues/297)
**Repo**: `Rushwind13/JMoria`
**Branch**: `feat/phase3c_polish`
**Build**: `make clean ascii test`
**Test**: `cd test; ./runtests.sh`
**Areas**: AIMgr, Dungeon tile system, Player commands, Item system, Effects/spells, Constants, Monsters.txt, Items.txt

---

## Overview

Three interlocking mechanics:
1. **Monsters with hands** — open closed doors as part of pathfinding (non-destructive).
2. **Large/heavy monsters** — bash through any door (closed, spiked, wizard-locked) → leaves a "broken door" tile; wakes/aggravates nearby monsters.
3. **Player door tools** — player can spike doors shut (`s` command with iron spikes), bash doors (`C` command), and cast Wizard Lock.

Prerequisite #296 (monster sleep system) is complete.

---

## A — Infrastructure: New Tile Type

- [ ] **`DUNG_IDX_BROKEN_DOOR = 10`** — Add to tile enum *before DUNG_IDX_PLAYER* (renumber PLAYER and MAX) in [src/Dungeon.cpp](../src/Dungeon.cpp)
  - Passable (monsters and player walk through freely)
  - Renders as a different color of the door glyph (')
  - `ModifiedTileTypes[DUNG_IDX_BROKEN_DOOR] = DUNG_IDX_INVALID` (can't modify a broken door)
  - Bump `DUNG_IDX_PLAYER` from 10 → 11; update `TileIDs` string in [src/Dungeon.cpp](../src/Dungeon.cpp)
  - Bump `DUNG_IDX_MAX` from 11 → 12; update `TileIDs` string in [src/Dungeon.cpp](../src/Dungeon.cpp)
  - Add render case in [src/Render.cpp](../src/Render.cpp) / [src/RenderASCII.cpp](../src/RenderASCII.cpp)

- [ ] **Door state flags on tile** — Tiles need to track `spiked` and `wizard-locked` states so the
  bash/open logic can distinguish them.
  - new `EFFECT_FLAG_LOCK` (word 2) to use for both the Iron Spike effect and the Wizard Lock effect. 
  - Use `m_dwCurHP` in `CDungeonTileDef` as "must do this much damage to door to bash it".

---

## B — New Monster Flags

- [ ] **`MON_FLAG_HANDS 0x00001000`** — Monster can open (non-destructively) any closed door.
  Add define to [src/Constants.h](../src/Constants.h), register in `m_StringTable`, increment `NUM_MON_FLAGS`.

- [ ] **`MON_FLAG_LARGE 0x00002000`** — Monster is large/heavy enough to bash through doors.
  Same registration steps as above.

- [ ] **Monsters.txt** — Audit existing monsters and add flags:
  - `MON_FLAG_HANDS`: orcs, trolls, humanoids, ogres, etc.
  - `MON_FLAG_LARGE`: trolls, giants, ancient dragons, balrog, etc.
  - Tip: use `./util/list_monster.sh` to enumerate candidates.

---

## C — AI Logic: Door Handling in `WalkSeek`

Changes in [src/AIMgr.cpp](../src/AIMgr.cpp) `CAIBrain::WalkSeek()`:

- [ ] **Detect closed-door tile** in the monster's intended next step.

- [ ] **`MON_FLAG_HANDS` branch** — If next tile is `DUNG_IDX_DOOR` (and not spiked/locked):
  - Call `CDungeon::ModifyTile()` to open it (`DUNG_IDX_OPEN_DOOR`).
  - Monster opening the door takes one turn, steps through on next turn.
  - Print message: *"You hear a door creak open."* (player within shrieker radius).

- [ ] **`MON_FLAG_LARGE` branch** — If next tile is `DUNG_IDX_DOOR` (any state, including spiked/locked):
  - Roll bash attempt (can only succeed for LARGE monsters; must damage the door enough to break it).
  - Set tile to `DUNG_IDX_BROKEN_DOOR`.
  - Call `CAIMgr::WakeNearby()` — aggravate monsters within radius (reuse shrieker logic from #269).
  - Print message: *"You hear a door smash open."* (player within shrieker radius).

- [ ] **Fall-through**: monsters without either flag treat closed doors as impassable walls (current behavior).

---

## D — New Item: Iron Spikes

- [ ] **`ITEM_IDX_SPIKE = 31`** — Add to [src/Constants.h](../src/Constants.h); push `ITEM_IDX_MAX` to 32.
  Register in `m_StringTable`.

- [ ] **Item.cpp** — Add glyph `;` to `ItemIDs[]` for `ITEM_IDX_SPIKE`.
  - `;` is the original Moria convention for iron spikes, unused in the current `ItemIDs` string, and avoids the gem-confusion of `*` and the floor-item/ammo convention of `,`.

- [ ] **Items.txt** — Add `Iron Spike` entry:
  - Stackable consumable (`ITEM_FLAG_STACKABLE`).
  - Weight: moderate (slows the player if carrying too many).
  - Value: low.
  - Suggest level 1 (available early).

- [ ] **Player command: `s)pike door`** — In [src/CmdState.cpp](../src/CmdState.cpp):
  - Keybind `s` (direction required). Prompt direction if not adjacent to a door.
  - Require `DUNG_IDX_DOOR` tile in chosen direction; fail with message if open or already spiked.
  - Consume one `ITEM_IDX_SPIKE` from inventory.
  - Set `EFFECT_FLAG_LOCK` flag on tile (via door-state mechanism from section A).
  - Print: *"You spike the door shut."*

- [ ] **Un-spiking** — Player can un-spike by moving into the door (costs a turn, consumes no spike).
  Print: *"You pull the spike free."*

---

## E — New Spell: Wizard Lock

- [ ] **Effects.txt** — Add `Wizard Lock` effect entry:
  - `Type`: targeted tile (door in direction).
  - Marks tile as `EFFECT_FLAG_LOCK` (via door-state mechanism from section A).
  - Only `MON_FLAG_LARGE` monsters (or dispel magic) can bypass it.

- [ ] **Scroll and Wand** — Decide delivery mechanism:
  - `Scroll of Wizard Lock` (single-use, scroll type).
  - `Wand of Wizard Lock` (single target, wand type).
  - Add entries to Items.txt referencing the effect.

- [ ] **Effect handler** in [src/Effect.cpp](../src/Effect.cpp):
  - On cast: require target tile is `DUNG_IDX_DOOR`; apply `EFFECT_FLAG_LOCK` flag.
  - Print: *"The door glows briefly and clicks shut."*
  - On monster approach: `MON_FLAG_LARGE` eventually bashes through (same as spiked); others blocked.

---

## F — Player Door Bashing

- [ ] **Player command: `C)bash door`** — In [src/CmdState.cpp](../src/CmdState.cpp):
  - Keybind `C` (Shift+c, direction required). Mnemonic: `c` = close gently, `C` = close *really* hard = bash.
  - `C` currently toggles the character stats pane → **reassign stats toggle to `@`** (Shift+2; universal roguelike convention for character sheet). Update `IsToggleCommand` in [src/CmdState.cpp](../src/CmdState.cpp).
  - Requires `IsModifierNeeded` to also match `JKEY_c` when Shift is held (or a new `IsBashCommand` predicate).
  - Prompt direction.
  - Require `DUNG_IDX_DOOR` (any state) in chosen direction.
  - Roll bash: `d20 + STR bonus` vs door DC (closed=10, spiked=15, wizard-locked=20).
    - Metal armor grants +2 to roll.
    - On success: set tile to `DUNG_IDX_BROKEN_DOOR`.
    - On failure: player stumbles; print *"You slam against the door but it holds."*; costs turn.
  - On success: call `CAIMgr::WakeNearby()` to aggravate nearby monsters.
  - Print: *"You bash the door open!"*

---

## G — Tests

- [ ] **Feature file** `test/features/doors.feature`:
  - Scenario: monster with hands opens a closed door.
  - Scenario: large monster bashes through a closed door → tile becomes broken.
  - Scenario: large monster bashes through a spiked door.
  - Scenario: player spikes a door; monster without LARGE flag is blocked.
  - Scenario: player bashes a door (success and failure rolls).
  - Scenario: Wizard Lock scroll locks a door; non-LARGE monster blocked.

---

## H — Keybind Fixes (prerequisite to F and D)

These collisions exist independently of this feature but must be resolved before wiring up `C` and `s`.

- [ ] **Stats pane: `C` → `@`** — `C` (Shift+c) currently toggles the character stats panel.
  Move to `@` (Shift+2): update `IsToggleCommand` in [src/CmdState.cpp](../src/CmdState.cpp).
  `@` is the universal roguelike convention for "character sheet" and is currently free.

- [ ] **Character name: `N` → `^n`** — `N` (Shift+n) is listed as "name your character" in
  `IsStringInputCommand` but **collides with run-SE** (Shift+n in vi-keys). Move to `^n`
  (Ctrl+n): change `JMOD_SHIFT` → `JMOD_CTRL` for `JKEY_n` in `IsStringInputCommand`.

- [ ] **Look: `:` (Shift+;) does not work in ASCII renderer; make sure that it wires properly with JKEY_ 

---

## Dependencies & Order

```
A (tile infra)  ──►  C (AI WalkSeek)
                ──►  D (iron spikes + s)pike)
                ──►  E (Wizard Lock)
                ──►  F (player bash)
B (flags)       ──►  C
H (keybinds)    ──►  D (s)pike uses `s`)
                ──►  F (bash uses `C`)
                ──►  G (tests last)
```

Recommend implementing **H → A → B → C → D → F → E → G**.
