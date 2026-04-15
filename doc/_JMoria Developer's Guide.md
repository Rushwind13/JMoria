#  JMoria Developer's Guide

## Branching Strategy

Use the following branch naming convention:

```
<gh-username>/<branch-type>/<feature-name>
```

**Branch types:**
- `feat/` - New features
- `fix/` - Bug fixes
- `refactor/` - Code refactoring
- `docs/` - Documentation updates
- `test/` - Test additions or fixes

**Examples:**
- `mschober/feat/linux-install`
- `mschober/fix/opengl-header`
- `mschober/refactor/dungeon-generation`

Create a Pull Request to `develop` when your feature is ready for review.

## Coding Standards

For code style, naming conventions, and patterns used in this codebase, see [doc/coding-standards.md](doc/coding-standards.md).

## Adding a new Monster
1) If you are creating a new flavor of an existing monster (adding a new type of orc), you just need to:
	a) edit *Monsters.txt*
2) If you are creating a new type of monster (no Vogons? a travesty!), you need to:
	a) edit *Monsters.txt*, then
	b) add the new *MON_IDX_* type to _Constants.h_, and finally,
	c) add the new monster's emoji to the *MonIds* list in _Monster.cpp_
## Adding a new Item
1) If you are creating a new flavor of an existing item (adding a new type of wand), you just need to:
a) edit *Items.txt*
2) If you are creating a new type of item (no Katana? a travesty!), you need to:
a) editm _Items.txt_, then
b) add the new *ITEM_IDX_* type to _Constants.h_, and finally,
c) add the new item's emoji to the *ItemIds* list in _Item.cpp_
## How to see your new thingy in the game
1) In _Dungeon.cpp_, you can find *#define RANDOM_MONSTER*,
2) comment this out, and you can choose which monster from the config you want to see.

similar instruction for items

Before you check in, please `lint` your code, with `clang-format -i
Create a Pull request on Github when you're satisfied.

## Monster Types and their tile bindings
~~a - ant~~
~~b - bat~~
c - centipede
~~d - dragon / dinosaur (lesser)~~
e - eye
f - Feline (tiger, &c)
g - golem
h - small humanoid (hobbit, gnome, elf)
~~i - icky thing~~
j
~~k - kobold~~
l - lice, leech
m - mold
n - naga
~~o - orc (snaga, uruk-hai, &c)~~
p - person (novice paladin / ranger / priest / warrior / mage)
q -
r - rats and mice
~~s - skeletons (human and animal)~~
t -
u - minor demon
v -
~~w - worm mass, also purple worm~~
~~x - spider~~
y - yeek
z - zombie
~~A - amphibian~~
B - Balrog,
~~C - Canid (dog, wolf, dire wolf, &c)~~
~~D - Dragon / Dinosaur (Ancient / greater)~~
E -
~~F - fly but also dragon fly and faerie dragon~~
~~G - ghost, ghoul~~
H - large humanoid (giant, ettin) and harpy
I - Insect
~~J - Oozes and Jellies and Slimes~~
K - Killer beetle
~~L - Lich~~
M
N
O - Ogre
P - Titan and other large persons
Q
R - Reptile
~~S - snake~~
T - Troll
U - major demon
~~V - Vampire~~
~~W - Wight, wraith~~
X - Xorn
Y - yeti
Z
& - mimic
. - lurker
~~, - mushroom~~
$ - creeping coin

## DisplayText Placements
Msgs - two rows at the top of the screen (for gameplay output)
Stats - left-hand sidebar (for AC, HP, Lvl, and other stats)
Inv - upper right-hand sidebar (displays inventory)
Equip - lower right-hand sidebar (displays equipment)
Use - central popup (only shown when needed) (shows inv/equip for commands that require it)

# JMoria Dungeon Creation Architecture / Notes
(10/2024)

My oldest comments (from 2003 and 2005) in `WORKLIST.txt` talk about how dungeon generation is broken; my last big enhancement run (in 2017) was to figure out how to do the dungeon calculations stepwise, so I could watch and in theory see WTF was wrong.
So I am (in 2024) building out a set of tests so I can grok what I did and what I'm trying to do.
To give flavor for the problem space:

### DungeonMap::FillArea has a complicated job.

The dungeon is just a 100x100 grid of solid rock (`DUNG_IDX_WALL`), which I carve rooms and hallways out of. In normal gameplay, visibility is mostly driven by seen/lit state; in debug modes (CLOCKSTEP and wizard mode), tiles can be shown even when normal seen/lit rules are bypassed.

So, to carve out a *5x5 room*, you want to turn a *5x5* area from `DUNG_IDX_WALL` into `DUNG_IDX_FLOOR`, but then you want to flag a *7x7* area (the entire room, plus the wall surrounding it) as `DUNG_FLAG_LIT`.

Note that *positions in-world are zero-based (0-99)*, but *rectangles have l,t,r, and b corners defined in world coords*. Also note: generation hallways are intentionally represented as thin rects (one axis has zero span) and expanded by direction logic.

#### To create a room
So a size 5,5 room in the upper-left corner of the map would have world coords 1,1,5,5 (floor in positions 12345 on second-through-sixth rows) and impose DUNG_FLAG_LIT on world coords 0,0 to 6,6 (lit in positions 0123456 on the first 7 rows).

#### To create a hallway
A 5-unit-long east-west hallway in the upper-left corner of the world would have coords 1,1,5,1 (floor in positions 12345 on second row). Hallways are not automatically lit by `LightArea()` during generation; room lighting and visibility rules differ between gameplay and debug modes.

#### Boundary checking is key to this problem
The boundary checking has a boatload of little off-by-one errors, as you can imagine.


## CLOCKSTEP Mode: Visual Dungeon Generation Debugger

CLOCKSTEP mode enables step-by-step visualization of dungeon generation for debugging the complex room/hallway creation algorithm.

### Enabling CLOCKSTEP

Enable CLOCKSTEP by passing `-DCLOCKSTEP` through `CC_FLAGS` when building:
```makefile
make ascii CC_FLAGS="-w -DCLOCKSTEP"
tmux new-session -d -s crawler -x 125 -y 30 './jmoria 2>/tmp/jmoria_map.txt'
tmux send-keys -t crawler Space   # step generation
tmux send-keys -t crawler Escape  # spawn player + start playing
```

Rebuild with `make clean && make`.

### Controls

- **SPACE** - Advance dungeon generation by one tick (calls `CreateOneStep()`)
- **ESC** - Exit CLOCKSTEP, spawn player at a valid location, and transition to normal gameplay

### How It Works

**State Flow**: `STATE_INTRO` → `STATE_CLOCKSTEP` → `STATE_COMMAND`

When CLOCKSTEP mode is active:

1. **Initialization** (`OnHandleInit`):
   - Creates initial dungeon level via `OnChangeLevel()`
   - Sets `m_bLevelPopulated = false`
   - Does NOT spawn player yet (deferred to avoid NULL crashes)
   - Displays control instructions in Stats panel

2. **Generation Loop** (`DoTick` on SPACE press):
   - Calls `Dungeon::Tick()` which calls `DungeonMap::CreateOneStep()`
   - Updates diagnostics overlay:
     - Current tick count
     - Random seed (for reproducibility)
     - Stack depth (creation algorithm state)
     - Room and hallway counts
   - Forces dungeon redraw after each step
   - Returns `true` while still generating, `false` when complete

3. **Population** (after generation completes):
   - Calls `PopulateLevel()` exactly once to place:
     - Scenery (stairs, doors, rubble)
     - Items (weapons, potions, scrolls)
     - Monsters
   - Sets `m_bLevelPopulated = true` to prevent duplicate spawns
   - SPACE key now ignored (returns -1)

4. **Player Spawn** (on ESC):
   - Spawns player at valid location
   - Transitions to `STATE_COMMAND` (normal gameplay)

### Implementation Details

**Deferred Player Spawn**: Player is NOT spawned during generation because many rendering and game logic functions check `g_pGame->GetPlayer()->m_bHasSpawned`. Spawning early causes NULL pointer crashes in:
- `CDungeon::Draw()` - player position for camera centering
- `CDungeon::UpdateSeen()` - player vision radius
- `CDungeon::PlayerCanSee()` - line-of-sight checks
- `CDungeon::IsLit()` - light source from player

**Level Population Guard**: The `m_bLevelPopulated` flag ensures `PopulateLevel()` is called exactly once. Without this guard, pressing SPACE after generation would re-call `PopulateLevel()` every tick, creating duplicate stairs/items/monsters.

**Viewport Adjustments** (`PreDraw()` in CLOCKSTEP mode):
- Zoom set to `DUNG_WIDTH / 2` (50) to show entire 100×100 dungeon
- Origin set to (0, 0) instead of player-centered
- Creates rect `(-50, 50, 50, -50)` showing full dungeon centered on screen

**Rendering Adjustments** (`DrawDungeon()` in CLOCKSTEP mode):
- Visibility check bypassed: `g_pGame->GetGameStateIndex() != STATE_CLOCKSTEP` added to DUNG_FLAG_SEEN check
- Lighting override disabled: `IsLit()` color (yellow) skipped in CLOCKSTEP
- Tiles render with natural colors from `m_dtd->m_Color` definitions:
  - Floor: light gray (192, 192, 192)
  - Walls: dark gray (64, 64, 64)
  - Doors: purple (64, 32, 128)

### Code Locations

- State machine: [src/ClockStepState.h](src/ClockStepState.h), [src/ClockStepState.cpp](src/ClockStepState.cpp)
- Dungeon generation: [src/DungeonMap.cpp](src/DungeonMap.cpp) `CreateOneStep()`
- Rendering adjustments: [src/Dungeon.cpp](src/Dungeon.cpp) `DrawDungeon()`, `PreDraw()`
- Makefile: [Makefile](Makefile) line with `CFLAGS` definition

### Debugging Tips

- Use fixed seeds for reproducible dungeons: `CreateDungeon(depth, seed)`
- Check diagnostics overlay for stuck generation (stack depth not changing)
- Watch room/hall counts to verify placement algorithm working
- Use BDD tests in [test/features/dungeonmap.feature](test/features/dungeonmap.feature) for regression testing

---

## Dungeon Generation Diagnostics and Invariants

The most useful generation diagnostics are emitted under the `[DUNGEN]` prefix and should be treated as the primary debug surface for dungeon creation issues.

### What to Watch

- Generation summary: steps created, rooms, hallways, skipped steps, fill operations
- Failure counters: repeated failures and hallway truncation/connection behavior
- Timing: total generation time and approximate steps/second
- Origin-room logging: first room coordinates + total room count

### Core Invariants (Useful for Debugging)

- `GetRoomRect()` / `GetHallRect()` reject clamped or degenerate geometry
- Fill precondition: non-wall fills should only occur where interior was previously wall
- Retry policy: level generation retries when room count is too low
- Connectivity helpers:
   - hallway truncation to connect on collision
   - adjacent-structure connection for sidling room walls

### Repro and Inspection Workflow

1. Build deterministic/diagnostic run variant as needed.
2. Generate via fixed seed path (`CreateDungeon(depth, seed)`) when reproducing bugs.
3. Use CLOCKSTEP for stepwise flow, then check `[DUNGEN]` summary output.
4. Use full-map dump output to inspect macro layout and entity placement.

### Current Key Tuning Knobs

- Room size: `DUNG_ROOM_MINWIDTH/HEIGHT`, `DUNG_ROOM_MAXWIDTH/HEIGHT`
- Hall length: `DUNG_HALL_MINLENGTH`, `DUNG_HALL_MAXLENGTH`
- Branching: `MAX_RECURDEPTH`, `MAX_TRIES`, `MAX_STEP_FAILURES`
- Placement spacing: `DOOR_OFFSET`, `WALL_OFFSET`
- Generation quality floor: `DUNG_MIN_ROOMS_REQUIRED`, `DUNG_MAX_GENERATION_RETRIES`

### Debugging Tips

- Monitor `[DUNGEN]` log lines for generation progress and conflicts
- Count skipped steps vs. successful steps to identify "tails out" behavior
- Check room/hallway counts at generation end to verify algorithm completeness
- Use fixed seeds + DUNGEN_DEBUG to reproduce and diagnose specific failures
- Pre-fill invariant violations indicate algorithm bug (should never occur)

---

## Performance & Stability Features

### Frame Rate Throttling

JMoria implements a 30fps cap in the main game loop to prevent excessive CPU usage (previously reported as 90%+ CPU during idle gameplay).

**Implementation** ([src/main.cpp](src/main.cpp) lines 75-84):
```cpp
// Frame rate cap: limit to ~30fps to prevent 90%+ CPU usage
// Each frame should take ~33ms (1000ms / 30fps)
unsigned int frameTime = Util::GetTickCount() - curTime;
if( frameTime < 33 )
{
    SDL_Delay( 33 - frameTime );
}
```

This throttling:
- Limits CPU usage to ~3-5% during idle gameplay (down from 90%+)
- Reduces power consumption on laptops and Raspberry Pi
- Maintains smooth 30fps rendering without stuttering
- Adds SDL_Delay only when frame completes early (no impact on slow frames)

### Dungeon Generation Timing

Performance measurements track generation time for optimization and regression detection.

**Timing Infrastructure** ([src/Util.h](src/Util.h), [src/Util.cpp](src/Util.cpp)):
- `Util::GetTimeInMillis()`: High-resolution timing using `gettimeofday()`
- Returns current time in milliseconds with microsecond precision
- Cross-platform (Linux, macOS, Raspberry Pi OS)

**Generation Metrics** ([src/DungeonMap.h](src/DungeonMap.h) DungeonGenDiagnostics):
```cpp
struct DungeonGenDiagnostics {
    double start_time_ms;     // Generation start time
    double end_time_ms;       // Generation end time
    double total_time_ms;     // Total generation time
    int steps_created;        // Performance: steps per second
    ...
};
```

**Timing Output** (logged at INFO level after generation):
```
Rooms in current level: 12
Hallways in current level: 21
Generation time: 1.77 ms (0.002 seconds)
```

**DUNGEN_DEBUG Timing** (when diagnostics enabled):
```
[DUNGEN] Generation complete in 1.77 ms
[DUNGEN]   Steps: 47 created, 12 rooms, 21 halls, 8 skipped
[DUNGEN]   Performance: 26553.7 steps/second
```

**CLOCKSTEP Timing Display**: Elapsed time shown in diagnostics overlay:
```
Tick! 15
Seed: 12345
Stack: 3
Rooms: 8
Halls: 14
Time: 125.3 ms
```

### Memory Management Audit

Dungeon generation memory management verified leak-free:

**CDungeonCreationStep Cleanup**:
- Successful steps pushed to JStack, cleaned up by CLink destructor
- Failed steps deleted immediately in MakeRoomStep/MakeHallStep
- Stack termination calls Remove() on all remaining links

**Verification Points**:
- [src/DungeonMap.cpp](src/DungeonMap.cpp) line 724: `delete pStep;` on room creation failure
- [src/DungeonMap.cpp](src/DungeonMap.cpp) line 787: `delete pStep;` on hallway creation failure  
- [src/DungeonMap.cpp](src/DungeonMap.cpp) line 601: `m_stkDungeonMapCreation->Remove(pLink);` after processing
- [src/JLinkList.h](src/JLinkList.h) lines 22-30: CLink destructor deletes m_lpData
- [src/JLinkList.h](src/JLinkList.h) lines 60-64: JStack::Remove() deletes link

No memory leaks detected in 99 BDD test scenarios (1271+ dungeons generated).

### CLOCKSTEP Yield Mechanism

Optional delay prevents CPU spikes during visual step-through debugging:

**Configuration** ([src/ClockStepState.cpp](src/ClockStepState.cpp) line 195):
```cpp
// Optional: Add small delay to prevent CPU spike during stepped generation
// Yields to system and keeps UI responsive. Can be disabled for faster generation.
// SDL_Delay( 1 ); // Uncomment to add 1ms delay per step
```

Uncommenting this line adds 1ms yield per generation step:
- Reduces CPU usage from 100% to ~10-15% during CLOCKSTEP
- Keeps UI responsive when generating large dungeons
- No impact on normal (non-CLOCKSTEP) generation
- Trade-off: slower visual step-through (negligible for debugging)

### Performance Testing

Generation time tracked in BDD tests for performance regression detection:

- Typical depth-1 dungeon: 0.5-2ms (500-2000 steps/second)
- Typical depth-10 dungeon: 2-5ms (1000-5000 steps/second)
- 100-dungeon stress test: <200ms total (determinism + performance validation)

Test fixture system ([test/fixtures/](test/fixtures/)) enables CI performance monitoring:
- Baseline fixtures generated with known seeds
- CI can detect generation slowdowns by comparing to baselines
- Automated regression tests flag >20% performance degradation

### Profiling & Optimization Tips

**Identifying Hot Spots**:
1. Enable DUNGEN_DEBUG to measure steps/second
2. Profile with `valgrind --tool=callgrind` for CPU hotspots
3. Check generation time vs. depth correlation

**Common Optimizations**:
- Reduce `MAX_RECURDEPTH` to limit recursion (trades dungeon complexity for speed)
- Adjust `MAX_TRIES` per step (trades quality for performance)
- Use larger `DUNG_ROOM_MIN*/MAX*` values (fewer steps for same coverage)

**Frame Rate Tuning**: Adjust throttle in [src/main.cpp](src/main.cpp):
- 60fps: `if( frameTime < 16 ) SDL_Delay( 16 - frameTime );`  
- 30fps (default): `if( frameTime < 33 ) SDL_Delay( 33 - frameTime );`
- 15fps: `if( frameTime < 66 ) SDL_Delay( 66 - frameTime );`

---

## Developer Prompts

The open PRs in the repo represent updates to WORKLIST.txt covering needed enhancements/fixes to several functional areas. Check open PRs for the current suggestions, then surface Github issues, code, and tests that are not currently covered by WORKLIST suggestions within existing PRs. Provide a brief synopsis of outstanding functionality, and update WORKLIST.txt with ideas for future work on the project.

We are going to work on open PR#145. Confirm that the correct branch is checked out, and then surface a list of work items based upon the already-edited files in the PR.

To update the dungeon creation WORKLIST, surface code and tests that handle and validate dungeon generation, with a brief synopsis of functionality, and ideas for future work on this part of the project


To update the TargetState WORKLIST, surface code and tests that handle and validate line-of-sight, and targeting, with a brief synopsis of functionality, noting any race conditions with targeting and line-drawing, and ideas for future work on this part of the project

To update the INVENTORY WORKLIST, surface code and tests that handle and validate Inventory and Equipment, with a brief synopsis of functionality, and ideas for future work on this part of the project

To update the WORKLIST.txt for , surface code and tests that handle and validate Inventory and Equipment, with a brief synopsis of functionality, and ideas for future work on this part of the project

Surface any skipped tests, with a brief synopsis of skipped functionality. Then, unskip the tests, run all tests, and append WORKLIST.txt with ideas for future work on this part of the project. Leave tests in a working state, with as many working as possible.

To update the WORKLIST.txt for Items; surface Github issues, code, and tests that handle and validate Items, with a brief synopsis of functionality, and ideas for future work on this part of the project.

To update the WORKLIST.txt for Monsters; surface Github issues, code, and tests that handle and validate Monsters, with a brief synopsis of functionality, and ideas for future work on this part of the project.

To update the WORKLIST.txt for Players; surface Github issues, code, and tests that handle and validate Players, ignoring anything to do with Inventory or Equipment, with a brief synopsis of functionality, and ideas for future work on this part of the project.

To update the WORKLIST.txt for IDENTIFY, we are going to continue the work that was started in PR#126.
Confirm that you can read PR #126
Confirm that you can read Issue #114 and Issue #128
In addition, Use the entirety of Issue#114 and Issue#128, including followup comments (ignore images).
Make sure that you are using the feat/identify branch

There are code callouts where isIdentified() would be useful, surface any code or tests that relate to item identification, including what is displayed when an item is partially identified, and how item statuses can be identifed. Give a brief synopsis of the existing functionality, a brief synopsis of the enhancements, and update the WORKLIST with a checklist of ideas for future work on the project.

---
