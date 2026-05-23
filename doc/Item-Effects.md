# Wand & Staff Effect Status Reference

All 42 wands and staves (18 wands + 24 staves) and their implementation status.

**Keybinds**: `z` = zap wand (aimed), `Z` = use staff (area/self)

---

## Wands (18) — Use `z`

Wands always target a specific tile or monster. Trajectory is built through RangedState.

| # | Item Name | Effect Name | EFFECT_TYPE | Implementation | Handler |
|---|-----------|-------------|-------------|----------------|---------|
| 1 | Wand of Heal Monster | Heal Monster | HIT + EFFECT_FLAG_HP | ✅ Done | `DoHealMonster()` |
| 2 | Wand of Light | Light Ray | HIT + EFFECT_FLAG_LIGHT | ✅ Done | built-in |
| 3 | Wand of Stone to Mud | Stone to Mud | HIT + EFFECT_FLAG_STONE_TO_MUD | ✅ Done | `DoStoneToMud()` |
| 4 | Wand of Sleep | Cause Sleep | HIT + EFFECT_FLAG_SLEEP | ✅ Done | `DoStatusHit()` |
| 5 | Wand of Confusion | Cause Confusion | HIT + EFFECT_FLAG_CONFUSE | ✅ Done | `DoStatusHit()` |
| 6 | Wand of Firebolts | Firebolt | HIT + EFFECT_FLAG_FIRE + MOD_LINE | ✅ Done | `DoLineHit()` |
| 7 | Wand of Fear | Cause Fear | HIT + EFFECT_FLAG_AFRAID | ✅ Done | `DoStatusHit()` |
| 8 | Wand of Frost | Frost Bolt | HIT + EFFECT_FLAG_COLD + MOD_LINE | ✅ Done | `DoLineHit()` |
| 9 | Wand of Lightning | Lightning Bolt | HIT + EFFECT_FLAG_ELECTRICITY + MOD_LINE | ✅ Done | `DoLineHit()` |
| 10 | Wand of Summoning | Summon Monsters | CREATE + EFFECT_FLAG_SUMMON | ✅ Done | built-in |
| 11 | Wand of Teleport Away | Teleport Away | HIT + EFFECT_FLAG_TELEPORT | ✅ Done | `DoTeleportAway()` |
| 12 | Wand of Acid | Acid Bolt | HIT + EFFECT_FLAG_ACID + MOD_LINE | ✅ Done | `DoLineHit()` |
| 13 | Wand of Fireballs | Fireball | HIT + EFFECT_FLAG_FIRE + MOD_BALL | ✅ Done | `DoBallHit()` |
| 14 | Wand of Frost Balls | Frost Ball | HIT + EFFECT_FLAG_COLD + MOD_BALL | ✅ Done | `DoBallHit()` |
| 15 | Wand of Lightning Balls | Lightning Ball | HIT + EFFECT_FLAG_ELECTRICITY + MOD_BALL | ✅ Done | `DoBallHit()` |
| 16 | Wand of Paralyze | Cause Paralysis | HIT + EFFECT_FLAG_PARALYZE | ✅ Done | `DoStatusHit()` |
| 17 | Wand of Probing | Probe | HIT + EFFECT_FLAG_IDENTIFY | ✅ Done | `DoProbeHit()` |
| 18 | Wand of Acid Balls | Acid Ball | HIT + EFFECT_FLAG_ACID + MOD_BALL | ✅ Done | `DoBallHit()` |

---

## Staves (24) — Use `Z`

Staves never require targeting. Effect fires at player position (area or self). Handled entirely by UseState → `OnHandleStaff()`.

| # | Item Name | Effect(s) | EFFECT_TYPE | Implementation | Handler |
|---|-----------|-----------|-------------|----------------|---------|
| 1 | Staff of Cure Light Wounds | CLW + Cure Poison + Cure Blindness | HEAL | ✅ Done | built-in |
| 2 | Staff of Light | Light Area | CREATE + EFFECT_FLAG_LIGHT | ✅ Done | built-in |
| 3 | Staff of Healing | Minor Healing + Cure Poison | HEAL | ✅ Done | built-in |
| 4 | Staff of Mapping | Partial Mapping | CREATE + EFFECT_FLAG_MAPPING + MOD_AREA | ✅ Done | built-in |
| 5 | Staff of Treasure Detection | Detect Treasure | CREATE + EFFECT_FLAG_TREASURE | ✅ Done | `DoDetectTreasure()` |
| 6 | Staff of Sleep | Cause Sleep (area) | HIT + EFFECT_FLAG_SLEEP + MOD_AREA | ✅ Done | `DoAreaHit()` |
| 7 | Staff of Acid Resistance | Timed Resist Acid | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 8 | Staff of Cold Resistance | Timed Resist Cold | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 9 | Staff of Electricity Resistance | Timed Resist Electricity | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 10 | Staff of Fear | Mass Fear | HIT + EFFECT_FLAG_AFRAID + MOD_AREA | ✅ Done | `DoAreaHit()` |
| 11 | Staff of Fire Resistance | Timed Resist Fire | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 12 | Staff of Protection | Timed Blessing (AC buff) | HIT + EFFECT_FLAG_AC + MOD_TIMED | ✅ Done | `DoACBuff()` |
| 13 | Staff of Starlight | Light Ray + Light Area | HIT/CREATE + EFFECT_FLAG_LIGHT | ✅ Done | built-in |
| 14 | Staff of Teleportation | Teleport Self | CREATE + EFFECT_FLAG_TELEPORT | ✅ Done | built-in |
| 15 | Staff of Paralysis | Mass Paralyze | HIT + EFFECT_FLAG_PARALYZE + MOD_AREA | ✅ Done | `DoAreaHit()` |
| 16 | Staff of Perception | Identify item | RESTORE + EFFECT_FLAG_IDENTIFY | ✅ Done | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` |
| 17 | Staff of Summoning | Summon Monsters | CREATE + EFFECT_FLAG_SUMMON | ✅ Done | built-in |
| 18 | Staff of Telepathy | Timed ESP | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 19 | Staff of Mass Sleep | Mass Sleep (large area) | HIT + EFFECT_FLAG_SLEEP + MOD_AREA | ✅ Done | `DoAreaHit()` |
| 20 | Staff of Resistance | All 4 resistances | INTRINSIC + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 21 | Staff of Word of Recall | Recall | CREATE + EFFECT_FLAG_RECALL | ✅ Done | built-in |
| 22 | Staff of *Resistance* | All 4 immunities | INTRINSIC + EFFECT_MOD_IMMUNE + EFFECT_MOD_TIMED | ✅ Done | built-in |
| 23 | Staff of Cure Serious Wounds | CSW + Cure Poison + Blindness + Confusion | HEAL | ✅ Done | built-in |
| 24 | Staff of Greater Healing | Greater Healing + 4 cures | HEAL | ✅ Done | built-in |

---

## Items Needing Item-Choice Prompt (`JNEED_CHOOSE_ITEM`)

These are routed as `EFFECT_TARGET_ITEM` and prompt for a selected item (`a-z` inventory, `A-J` equipment) before completion.

| Item | Effect | Target Validation (`IsValidTarget`) | Prompt | Completion Handler | Verified? |
|------|--------|--------------------------------------|--------|--------------------|-----------|
| Staff of Perception | Identify | Item must be unidentified | `Identify which item? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (`Identify()`) | ✅ |
| Scroll of Identify | Identify | Item must be unidentified | `Identify which item? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (`Identify()`) | ✅ |
| Scroll of Recharge | Recharge | `NeedsFuel()` (wands, staves, lanterns) | `Recharge which wand/staff? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (adds charges, can overload/explode) | ✅ |
| Scroll of Enchant Weapon | Enchant to-hit / to-dam | Item must be weapon or ranged weapon | `Enchant which weapon? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (`+1` to-hit and/or to-dam, recalc stats) | ✅ |
| Scroll of Enchant Armor | Enchant AC | Item must be armor | `Enchant which weapon? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (`+1` AC bonus, recalc stats) | ✅ |
| Scroll of Remove Curse | Remove curse | Any item accepted | `Remove curse from which item? [a-z inv, A-J equip]` | `TargetEffect()` → `OnHandleChooseItem()` → `ApplyChosenItem()` (clears/sets curse based on source effect) | ✅ |

---

## In-Game Test Checklist

Use wizard mode `^i` (or `^g` stock-all command once added) to obtain items.

### Wands (z)
- [ ] Wand of Heal Monster — heals targeted monster HP
- [ ] Wand of Light — lights up corridor/room in bolt path
- [ ] Wand of Stone to Mud — turns wall tile to floor on hit
- [ ] Wand of Sleep — target monster falls asleep (skips turns)
- [ ] Wand of Confusion — target monster moves erratically
- [ ] Wand of Firebolts — fire bolt damages monster
- [ ] Wand of Fear — target monster flees
- [ ] Wand of Frost — cold bolt damages monster
- [ ] Wand of Lightning — lightning bolt damages monster, may arc
- [ ] Wand of Summoning — spawns monsters near player
- [ ] Wand of Teleport Away — teleports targeted monster elsewhere
- [ ] Wand of Acid — acid bolt damages monster
- [ ] Wand of Fireballs — firebolt explodes on hit, AoE damage
- [ ] Wand of Frost Balls — frost bolt explodes on hit, AoE damage
- [ ] Wand of Lightning Balls — lightning bolt explodes on hit, AoE damage
- [ ] Wand of Paralyze — target monster cannot move
- [ ] Wand of Probing — reveals monster stats in message area
- [ ] Wand of Acid Balls — acid bolt explodes on hit, AoE damage

### Staves (Z)
- [ ] Staff of Cure Light Wounds — heals player, cures poison/blindness
- [ ] Staff of Light — lights up surrounding area
- [ ] Staff of Healing — heals player, cures poison
- [ ] Staff of Mapping — reveals partial map around player
- [ ] Staff of Treasure Detection — highlights nearby treasure
- [ ] Staff of Sleep — nearby monsters fall asleep
- [ ] Staff of Acid Resistance — player gains timed acid resist
- [ ] Staff of Cold Resistance — player gains timed cold resist
- [ ] Staff of Electricity Resistance — player gains timed elec resist
- [ ] Staff of Fear — nearby monsters flee
- [ ] Staff of Fire Resistance — player gains timed fire resist
- [ ] Staff of Protection — player gains timed AC bonus
- [ ] Staff of Starlight — lights ray + area
- [ ] Staff of Teleportation — player teleports to random location
- [ ] Staff of Paralysis — nearby monsters cannot move
- [ ] Staff of Perception — prompts item selection, then identifies it
- [ ] Staff of Summoning — spawns monsters near player
- [ ] Staff of Telepathy — player gains timed ESP (see all monsters)
- [ ] Staff of Mass Sleep — large radius monsters fall asleep
- [ ] Staff of Resistance — all 4 resistances for a time
- [ ] Staff of Word of Recall — player will recall to level 1 after delay
- [ ] Staff of *Resistance* — all 4 immunities for a time
- [ ] Staff of Cure Serious Wounds — larger heal + 4 cures
- [ ] Staff of Greater Healing — greatest heal + 4 cures

### Charge / Misc
- [ ] Verify Z with 0-charge staff shows "nothing happens" message
- [ ] Verify z with 0-charge wand shows "nothing happens" message
- [x] Verify Recharge scroll adds charges to a wand/staff
- [ ] Verify Enchant Weapon scrolls increase to-hit and to-dam
- [ ] Verify Enchant Armor scroll increases AC bonus
- [ ] Verify Remove Curse scroll clears cursed flag on equipment
