# JMoria Release 0.7.0 - Effects!

**Release Date**: April 26, 2026

## Overview

Release 0.7.0 represents a **watershed moment** for JMoria's game design. The effects system unifies all game mechanics under a single data-driven vocabulary, enabling complex interactions previously impossible. Combined with foundational systems (item identification, light/fog-of-war, speed/action economy), this release ships a fully feature-complete roguelike engine ready for expansion.

**17 issues across 4 phases**, **94 files changed**, **+19,000 lines of code**.

## Phase 1: Quick Wins & Accessibility (#260)

### Gameplay Fairness
- **#110 — Monsters Don't Move During Use Commands**: Prevents unfair AI attacks while player manages inventory (STATE_USE, STATE_RANGED blocked)
- **#111 — Filtered Inventory by Use Command Type**: Potion/quaff, scroll/read, wand/zap filters with original item letters preserved
- **#180 — Death Screen Full Dungeon Map**: 100×100 map dumped to endgame display; TEXT_MAXCHARS bumped to 12,288

## Phase 2: Gameplay Expansion & Emergent Systems (#261)

### Exploration & Resource Management
- **#117 — Search Command** (`s` key): Active search (25% success), passive per-turn (5%), reveals adjacent secret doors
- **#121 — Lantern Fuel System**: Fill lanterns with Flask of Oil (Shift+F); 15k max fuel; torches unrefu­elable
- **#45 — MON_AI_SEEK & Arbitrary Target Positions**: Monsters pursue other creatures via `m_vTargetPos`; foundation for emergent AI (tigers hunt rabbits)

## Phase 3: Deep Systems — Foundational Architecture (#262)

### Item Identification & Knowledge Tracking
- **#114 — Item Identification System** (Closed April 24):
  - `m_dwKnownProps` bitmask tracks discovered properties
  - Scroll of Identify reveals all; cursed status discovered via failed removal
  - **Partial stack split**: "How many? (1-N, * for all)" prompts on drop/sell
  - One-at-a-time arrow firing; overflow to adjacent tiles
  - Foundation for stat-based feeling tiers (deferred)

### Light & Fog-of-War Gameplay
- **#72 — Fog of War & Sight Distance Expansion** (Closed April 25):
  - `DUNG_FLAG_VISIBLE` + `UpdateVisibility()` with full Bresenham LOS
  - Depth-based lit chance (100% surface → 0% deep levels)
  - **Torches**: 3000-turn fuel, radius 3
  - **Lanterns**: Refuelable, radius 5, max 15k turns
  - **Scroll of Light**: Radius 10 room illumination
  - **Wand of Light**: Line + area effects; damages light-weak creatures (orcs, vampires)
  - **Staves of Light/Starlight**: Combined effects
  - `MON_FLAG_HURT_BY_LIGHT` working (blue light damages specific types)

### Complete Effects System — Data-Driven Game Mechanics
- **#77 — Item Effects System** (Closed April 24):
  - **Complete effect vocabulary** enabling all game mechanics:
    - **Effect Flags**: fire, cold, acid, lightning, poison, identify, AC, levitate, invisible, telepathy, speed, haste, slow, blind, poison, disease, paralysis, confusion, fear, terror, weakness, strength, regeneration, and more
    - **Effect Modifiers**: weak (resist), resistant (50%), immune (no damage), gain (bonus), lose (penalty), restore (heal), see (vision), timed (expiring)
    - **Effect Types**: intrinsic (permanent), timed (duration-tracked), hit (on-attack contact), heal (restoration), cause (inflict), gain (teach), restore (recover)
  - **150 items** in Items.txt with named effect references (zero inline effect definitions)
  - **98 named effects** in Effects.txt
  - **All 45+ monster attacks** migrated to named effects (completed 2026-04-21)
  - **Enables downstream**:
    - Stat effects (STR/DEX/CON modifiers)
    - Status effects (haste, slow, paralysis, blind, confusion, fear)
    - Spellcasting system
    - Trap effects
    - Item destruction mechanic
    - Environmental hazards

## Phase 3a: Flotsam — Polish & Major System Tuning (#276)

### UI Improvements
- **#273 — Messages Scrollback**: 5-row msgs window (configurable); empty-result messages suppressed
- **#172 — Tombstone UI**: Two-line killer name display (wraps names > 17 chars)
- **#234 — ASCII DisplayText Fixes**: `FLAG_TEXT_TRIM_TAIL` (clip bottom), dark color boost with A_BOLD, bounding box interior fill

### Infrastructure & Testing
- **#177 — RENDER_MODE Linker Flags**: Per-renderer `TEST_LD_FLAGS`; `make ascii-test` convenience target; macOS/Linux support

### Visible Monster Detection
- **#272 — Visible Monsters UI Pane**: 10-row sidebar (`v` toggle) listing detected monsters by name + distance; duplicate grouping as `Name (N)`; 5 BDD scenarios

### Action Economy & Difficulty Tuning
- **#242 — Speed System & Action Economy** (Closed April 24):
  - `m_fSpeed` on CPlayer (base 1.0f = "speed 10"); AIMgr scaled by 1/player_speed
  - **Equipment speed bonuses**:
    - **Ring of Speed**: Random +0.1 to +1.0 per spawn (probabilistic)
    - **Boots of Speed**: Fixed +1.0
    - **Gloves of Elvenkind**: Fixed +1.0 (new item)
  - **Timed speed**: Potion of Speed grants +1.0 for 80 turns
  - **Monster calibration** (enables boss encounters):
    - Ancient Dragons: 1.0 → 2.0
    - Greater Demon: 1.0 → 2.0
    - Demon Lord: 1.2 → 2.5
    - Greater Demon Lord: 1.3 → 3.0
    - Master Vampire: 1.4 → 3.0
    - Lordly Vampire: 1.5 → 3.5
    - **Balrog**: 1.2 → **4.5** (deadly at base speed; beatable with speed gear ~+30)
  - Stats pane displays `Fast(+N)` / `Slow(-N)` when modified
  - Enables #42 (Balrog boss encounter)

### Design Enhancement Systems
- **#265 — Breath Weapon HP-Scaling**: Breath damage = current HP × multiplier (monsters weaken as hurt)
- **#264 — EFFECT_MOD_MAX**: `RollMax()` utility; `MON_FLAG_MAXHP` spawns high-tier monsters at full health

## Architecture Highlights

### Data-Driven Approach
- **Zero inline effect definitions**: All 150 items reference 98 named effects from Effects.txt
- **Parser infrastructure**: FileParse utility handles angle-bracket-delimited strings, NdM dice notation, block structure
- **Resource versioning**: Monster/item defs auto-sync via named effect references (no id fragility)

### Game Engine Foundation
- **State machine**: 15+ CStateBase subclasses (CmdState, UseState, RangedState, TargetState, RestState, LookState, etc.)
- **AI system**: CAIMgr with MON_AI_* behaviors (patrol, seek player, seek positions); emergent creature pursuit
- **Dungeon generation**: 100×100 grid with room/hallway carving, light placement, monster spawning
- **Rendering**: Dual-mode SDL2/OpenGL + ASCII (256-color + 16-color fallback)

## Testing & Quality

- **147+ BDD scenarios** (Cucumber-CPP + GoogleTest)
- **Speed system**: 5 scenarios validating equipment bonuses, timed effects, ring randomization
- **Visible monsters**: 5 scenarios for detection, grouping, distance
- **Full cross-platform support**: macOS, Linux, Raspberry Pi OS
- **Code style**: clang-format enforced pre-commit

## Build & Run

```bash
make                 # Both ASCII and OpenGL
make ascii           # ASCII-only (no SDL2/OpenGL required at runtime)
make opengl          # OpenGL-only
make test            # Run BDD test suite
make ascii-test      # ASCII-only tests
```

## What's Deferred (Dependencies on #197 — Player Stats)

- DEX → base speed modifier
- STR → encumbrance penalty modifier
- Encumbrance speed penalty
- Speed breakdown display annotation (Fast +1 from Boots, +0.3 from Ring, etc.)
- Haste/Slow modifiers display separately from equipment
- Wizard-mode monster speed coloring

## What's Next (Phase 4 — Town & NPCs)

- **#243 — Town System**: Shop infrastructure, NPCs, safe hub
- **#244 — Intrinsics & Status Effects**: 70+ effect framework
- **#245 — Monster Color Effects**: Threat recognition by color
- **#246 — Item Durability**: Elemental destruction
- **#247 — XP & Level Progression**: Character progression
- **#248 — Guard Timer UI & Crime System**: Consequences
- **#249 — Loot Tables & Monster Item Drops**: Dynamic economy

## Building & Dependencies

### macOS
```bash
brew install sdl2 sdl2_image
make
```

### Linux/Debian
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libgl-dev build-essential
make
```

### Testing
See [Developer-Setup-Guide.md](Developer-Setup-Guide.md) for googletest + cucumber-cpp setup.

## Known Limitations

- Stat-based item feelings deferred to #239 (blocked by #197 Player Stats)
- `*`-key targeting from Visible Monsters pane deferred (#272 future enhancement)
- Monster AI chase behavior limited to pathfinding (no advanced tactics)
- Spell system architecture defined but not implemented (#239)

## Contributors

Massive thanks to the JMoria team for pushing this engine to feature-completeness. This release represents a watershed in game design infrastructure — everything downstream (combat variety, boss encounters, status effects, spellcasting) now has a solid foundation.

## Version Info

- **Version**: 0.7.0
- **Codename**: Effects!
- **Release Cycle**: Phases 1–3a (17 issues, 4 phases)
- **Language**: C++17
- **Platform Support**: macOS, Linux, Raspberry Pi OS
- **Engine**: SDL2/OpenGL with ASCII fallback (256-color terminal support)
- **Testing**: Cucumber-CPP + GoogleTest, 147+ scenarios
- **Build System**: Makefile with per-renderer targets
