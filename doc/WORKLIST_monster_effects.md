# Monster Effects Migration — COMPLETE ✅

**Status**: Phases 1–4 complete as of 2026-04-22. All 630 monster attack lines migrated to named effects.

**Migration scope**: 
- Physical (430), elemental melee (55), status intrinsics (73), poison (21), XP drain (13), breathe bolts (21), breathe balls (24), creeping coins (3)
- Script preserved: `util/migrate_attacks.py`
- Parser, CAttack.m_pEffect, AIMgr integration complete

---

## Reference: Worm Mass Stat Drain (for stats-system story)

| Worm Mass | Stat | Notes |
|---|---|---|
| Yellow | WIS | "you feel naive" |
| Green | DEX | "you feel clumsy" |
| Black | CHA | "you feel ugly" (+ Acid damage) |

See [WORKLIST_items_design.md](WORKLIST_items_design.md#blocked-by-stats-197) for deferred Phase 5 dispatch work.
