# Phase 3c Polish — Roadmap

## Repo: Rushwind13/JMoria
## build: make clean ascii test
## all tests: cd test; ./runtests.sh
## feature tests: cd test; ./runtests.sh ranged
* Use MCP to interact with Github
* Only use these exact commands for builds/testing. 

Open issues labeled `cleanup` on branch `feat/phase3c_polish`, prioritized by impact and dependency order.

---

## P1 — Bug Fixes (gameplay-breaking or visually wrong) COMPLETE 2026-05-16

- **#301** COMPLETE [all weapons add speed](https://github.com/Rushwind13/JMoria/issues/301)
  All melee weapons incorrectly add differing amounts of speed to the player (spear +70, etc.). Bullet-time gameplay is unintended.

- **#294** COMPLETE ["strikes with the EFFECT_FLAG_FIRE"](https://github.com/Rushwind13/JMoria/issues/294)
  `szElement` is printed raw instead of being run through `Element()` lookup. Produces garbage combat messages.

- **#215** COMPLETE [Items visible through walls and closed doors](https://github.com/Rushwind13/JMoria/issues/215)
  Items render without LOS checks, leaking map information to the player and the bot.

---

## P2 — Quick Cleanup (small scope, clear fix) COMPLETE 2026-05-02

- **#290** COMPLETE [High score list prints backwards](https://github.com/Rushwind13/JMoria/issues/290)
  Completed: fixed `Add()` argument order so score sort now uses descending index (`Add(ps, score, -1, false)`).

- **#303** COMPLETE [player stat reset during x)change](https://github.com/Rushwind13/JMoria/issues/303)
  Completed: extracted shared `RecalcCombatStats()` and switched `XchangeWeapons()`, `Wield()`, `RemoveEquipment()`, and player construction to use it (AC base remains `1.0`).

---

## P3 — Content / Feature Completeness COMPLETE 2026-05-15

- **#292** COMPLETE [monsters are immune to their elemental effect](https://github.com/Rushwind13/JMoria/issues/292)
  Red dragons should shrug off firebolts (and ideally be weak to frostbolts). Elemental immunity/weakness table needed.

- **#288** COMPLETE [ITEM_IDX_STAFF implementation](https://github.com/Rushwind13/JMoria/issues/288)
  Staves (`Z` keybind) are not yet implemented. Wands may share the same gap.

- **#291** COMPLETE [Rings/amulets need flavors like potions/scrolls/etc.](https://github.com/Rushwind13/JMoria/issues/291)
  Rings, staves, and amulets lack randomized flavor names (gems/metals/woods). 14 ring flavors needed; same for amulets and staves.

---

## P4 — Larger Features (with dependencies)

- **#296** COMPLETE [Monsters should have a %chance to be asleep](https://github.com/Rushwind13/JMoria/issues/296)
  Foundational sleep system: spawn chance, stealth interaction, Sleep spell, waking on attack, 4× damage bonus.
  *Prerequisite for #269 and #297.*

- **#297** COMPLETE [Monsters can open doors](https://github.com/Rushwind13/JMoria/issues/297)
  Monsters with hands open closed doors; large/heavy monsters bash through locked/spiked doors. New items: iron spikes. New spell: Wizard Lock.
  *Depends on sleep system for full behavior (#296).*

- **#269** COMPLETE [Shrieker Mushroom Patch — monster sleep and aggravate mechanic](https://github.com/Rushwind13/JMoria/issues/269)
  Stationary `MON_AI_DONTMOVE` monster that wakes and aggravates nearby monsters when the player is adjacent.
  *Blocked by monster sleep state (#296).*
