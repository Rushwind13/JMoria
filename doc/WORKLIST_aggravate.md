# Issue #269 — Shrieker Mushroom Patch / Aggravate Mechanic
## Branch: feat/phase3c_polish
## Build: make clean ascii test
## Tests: cd test; ./runtests.sh

---

## Background

All SLEEP/wake functionality is complete (#296). This work adds:
- `EFFECT_FLAG_AGGRAVATE` (area and single-target variants)
- A minimal **action-choice structure** in AIMgr so monsters can choose: move, attack, or do nothing
- Shrieker Mushroom Patch monster that uses an AoE aggravate attack when the player is in LOS
- Scroll/Staff (area) and Wand (single-target) items that cast the aggravate effect

> **Scope note:** A full ranged combat overhaul (attack groups, etc.) is a separate larger enhancement. The AIMgr work here is the minimal scaffolding needed: adding `BRAINSTATE_ATTACK` alongside the existing states (REST, SEEK, GOTODEST, IDLE), so that attacking is a proper state transition rather than an inline side-effect of movement.

---

## Phase 1 — Effect Flag & Data

### T1 — Add `EFFECT_FLAG_AGGRAVATE` to `Constants.h`
- Add a new bit in the effect flags block (currently 32 flags defined in word 1, room available in word 2)
- Name: `EFFECT_FLAG_AGGRAVATE`
- Add the corresponding string mapping in `Constants.cpp` so it prints cleanly in logs/messages

### T2 — Add `<Aggravate Monsters>` (area) to `Effects.txt`
- Area-of-effect version: `Radius=25`
- Uses `EFFECT_FLAG_AGGRAVATE`
- This is what the Shrieker, Scroll, and Staff cast

### T3 — Add `<Aggravate Monster>` (single-target) to `Effects.txt`
- No radius (single target)
- Uses `EFFECT_FLAG_AGGRAVATE`
- This is what the Wand casts

---

## Phase 2 — Aggravate Logic

### T4 — Implement aggravate effect handler in `Effect.cpp`
When `EFFECT_FLAG_AGGRAVATE` fires:

**Area variant (Radius > 0):**
- Iterate all monsters within radius tiles
- Wake each monster (100%): clear `EFFECT_FLAG_SLEEP`, clear `BRAINSTATE_REST`
- Set each monster to seek the **source position** of the effect (not necessarily the player — see T5)
- Print a message: `"The Shrieker Mushroom Patch emits a horrible wail!"` (once per trigger)
- Print a message: `"The Kobold wakes up."` (per aggravated monster, within sight)
- Print a message: `"You hear a stirring in the distance!"` (once per trigger, if any aggravated monsters are out of sight)

**Single-target variant:**
- Wake the one targeted monster (100%)
- Set it to seek the source position of the effect

### T5 — Add "seek arbitrary position" to `CAIBrain` (`AIMgr.cpp/.h`)
Currently `BRAINSTATE_SEEK` with `MON_AI_SEEKPLAYER` always overwrites `m_vTargetPos` with the player position. Monsters woken by aggravate need to seek the effect source (which may be the mushroom's tile, or the player's tile if a player item cast it).

**Design:**
- In `UpdateSeek()`: if intrinsic EFFECT_FLAG_AGGRAVATE is set, set `m_vTargetPos` to aggravate position as target instead of the player, regardless of brain type
- Clear intrinsic EFFECT_FLAG_AGGRAVATE when the monster reaches the aggravate tile (within 1 tile) — at that point normal AI takes over 
- Clear intrinsic EFFECT_FLAG_AGGRAVATE when the monster can see the player — at that point normal AI takes over 
- The aggravate effect (T4) sets `m_vTargetPos = source position` and intrinsic EFFECT_FLAG_AGGRAVATE is set on each woken monster, then calls `SetState(BRAINSTATE_SEEK)`

---

## Phase 3 — AIMgr Action-Choice Structure

### T6 — Add `BRAINSTATE_ATTACK` and action-choice to `AIMgr.cpp`

**Current state:** The state machine has REST, SEEK, GOTODEST, and IDLE. `UpdateSeek()` exclusively sets a movement target and transitions to `BRAINSTATE_GOTODEST`. `MON_AI_DONTMOVE` calls `SetRandomDest()` (sets a random direction; if the player is in that tile, `CollideWithPlayer()` fires normally via `UpdateGoToDest`) — this is correct and unchanged.

**Add `BRAINSTATE_ATTACK` to the state machine:**

In `AIMgr.h` — add to the brain state enum:
```cpp
BRAINSTATE_ATTACK,
```

In `Update()` — add the case:
```cpp
case BRAINSTATE_ATTACK:
    return UpdateAttack( fCurTime );
```

**Modify `UpdateSeek()` — action-choice step before the brain-type switch:**

```
UpdateSeek():
    1. (existing) fear/flee check → m_vTargetPos = flee dest, SetState(BRAINSTATE_GOTODEST), return
    2. NEW: BuildEligibleAttacks() — filter attack list by range vs. player distance
       - player adjacent (≤1 tile):     all attacks eligible
       - player not adjacent, in LOS:   only attacks where effect.Range > 0 eligible  
       - no eligible attacks:           ATTACK not available this tick
    3. NEW: ChooseAction() — weighted random over available actions:
       - sets m_vTargetPos = player pos, SetState(BRAINSTATE_ATTACK), return
       - OR SetState(BRAINSTATE_IDLE), return   ← "do nothing"
       - OR fall through to brain-type switch   ← "move"
    4. (existing) brain-type switch → SetRandomDest / WalkSeek → SetState(BRAINSTATE_GOTODEST)
```

All state transitions are explicit `SetState()` calls — no magic return values or boolean flags.

**Per-brain-type action weights:**

| Brain type | MOVE % | ATTACK % | NOTHING % |
|---|---|---|---|
| `MON_AI_SEEKPLAYER` | 24 | 75 | 1 |
| `MON_AI_75RANDOMMOVE` | 40 | 55 | 5 |
| `MON_AI_100RANDOMMOVE` | 60 | 30 | 10 |
| `MON_AI_DONTMOVE` | 1 | 75 | 19 |

*(Starting values — tunable. When an action is filtered out, redistribute weight proportionally among remaining choices.)*

**`UpdateAttack()` — new method:**
```
UpdateAttack():
    - Select randomly from the eligible attack list built in UpdateSeek()
    - Melee (effect.Range == 0 or no effect):
        CollideWithPlayer()   // player is adjacent; same path as movement collision
    - Ranged (effect.Range > 0):
        pAttack->m_pEffect->ApplyXxx( m_vPos )   // see T6a; origin = monster's position
    - Print flavor text via AttackFlavorText() as today
    - SetState(BRAINSTATE_SEEK)
```

**`BRAINSTATE_IDLE` — "do nothing":**
`UpdateIdle()` already exists. When `ChooseAction()` selects NOTHING, transition to `BRAINSTATE_IDLE`. No new code required for the "nothing" path.

---

### T6a — Move all outward effect-resolution methods off `CPlayer` and onto `CEffect`

These methods apply effects *outward* (to monsters or the dungeon). They are currently `CPlayer` methods only because the player was the only thing that could cast them. They belong on `CEffect`, taking an origin position, so any caster (player, monster, future spell/item) calls the same code with no special cases.

Methods to move from `CPlayer` → `CEffect`:

| Current `CPlayer` method | New `CEffect` method |
|---|---|
| `DoAreaHit(CEffect*)` | `ApplyArea(JVector vOrigin)` |
| `DoBallHit(CEffect*)` | `ApplyBall(JVector vOrigin)` |
| `DoLineHit(CEffect*)` | `ApplyLine(JVector vOrigin)` |
| `DoLightRay(CEffect*)` | `ApplyLightRay(JVector vOrigin)` |
| `DoElementalHit(CEffect*)` | `ApplyElemental(JVector vOrigin)` |
| `DoPhysicalHit(CEffect*)` | `ApplyPhysical(JVector vOrigin)` |
| `DoStatusHit(CEffect*, uint32)` | `ApplyStatus(JVector vOrigin, uint32 dwFlag)` |
| `DoStoneToMud(CEffect*)` | `ApplyStoneToMud(JVector vOrigin)` |
| `DoTeleportAway(CEffect*)` | `ApplyTeleportAway(JVector vOrigin)` |

Each `CPlayer` stub becomes a one-liner: `pEffect->ApplyXxx( m_vPos )`.

Methods that stay on `CPlayer` (self-effects applied to the player character):
- `DoEffects` — stat/status changes to the player
- `DoHealEffects` — player healing
- `DoHealHP` — player HP restore
- `DoHitEffects` — effects applied when the player strikes a monster

**What remains unchanged:**
- `CollideWithPlayer()` — still fires for any melee collision during `UpdateGoToDest`
- All sleep/paralysis/fear gating — handled before `UpdateSeek` is reached; these are bitmask checks on `m_dwActiveEffects` already in place
- Confused, slowed, and poisoned monsters still act — those conditions affect speed or target selection, not the action-choice step

---

## Phase 4 — Monster & Item Data

### T7 — Add Shrieker Mushroom Patch to `Monsters.txt`
- **Tile**: `,` (camouflages as food)
- **Color**: bright/glowing red (alarm light aesthetic)
- **AI**: `MON_AI_DONTMOVE`
- **Speed**: `0.0`
- **Flags**: `MON_FLAG_EMPTY_MIND | MON_FLAG_HURT_BY_LIGHT | MON_FLAG_NEVER_SLEEP`
  - `EMPTY_MIND` prevents telepathy detection and ESP-style waking
  - `HURT_BY_LIGHT` for flavor / light bolt vulnerability
  - `NEVER_SLEEP` — it's always "on"
- **Attack**: `MON_FLAG_SPORE` with `<Aggravate Monsters>` effect
  - Effect fires as AoE centered on the Shrieker's tile (caster = source)
  - Woken monsters seek the Shrieker's tile; if they encounter the player en route, normal AI takes over
- **HD/AC/Level**: Set to low-power (e.g., Level 3, 1d4 HP, AC 10) — it can't move or deal damage, the threat is the alarm
- **Level appearance range**: Low dungeon levels (2–5) — found early as an environmental hazard

### T8 — Add Scroll of Aggravate Monsters to `Items.txt`
- Uses `<Aggravate Monsters>` area effect (radius 25)
- Flavor: randomized scroll name (existing scroll flavor system)
- Level: low (common, intentionally dangerous to use carelessly)

### T9 — Add Staff of Aggravate Monsters to `Items.txt`
- Uses `<Aggravate Monsters>` area effect (radius 25)
- Charges: ~5–10
- Level: low-medium

### T10 — Add Wand of Aggravate Monster to `Items.txt`
- Uses `<Aggravate Monster>` single-target effect
- Charges: ~8–12
- Level: low-medium
- Use case: wake one specific monster intentionally (tactical, or for testing sleep interactions)

---

## Implementation Order

```
T1 → T2 → T3     (flag + data, no code changes)
T6a               (refactor CEffect.Apply*; no behavior change, unblocks everything)
T4 + T5           (aggravate effect logic + EFFECT_FLAG_AGGRAVATE seek-state; depends on T1, T6a)
T6                (BRAINSTATE_ATTACK + action-choice; depends on T6a)
T7                (monster data; depends on T6)
T8 + T9 + T10    (item data; depends on T4)
```

---

## Open Questions

- **`ChooseAction()` starting weights**: The MOVE/ATTACK/NOTHING percentages in T6 are initial guesses. Worth playtesting before locking in, especially the `MON_AI_DONTMOVE` split.
