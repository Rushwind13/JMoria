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

The dungeon is just a 100x100 grid of solid rock (`DUNG_IDX_WALL`), which I carve rooms and hallways out of. The things you can see (floor, doors, walls) are visible because they are lighted (`DUNG_FLAG_LIT`); all unlighted stuff is blank on-screen.

So, to carve out a *5x5 room*, you want to turn a *5x5* area from `DUNG_IDX_WALL` into `DUNG_IDX_FLOOR`, but then you want to flag a *7x7* area (the entire room, plus the wall surrounding it) as `DUNG_FLAG_LIT`.

Note that *positions in-world are zero-based (0-99)*, but *sizes of rectangles are one-based* (you would want a 1xN hallway, not a "zero-width" hallway). But *rectangles have l,t,r, and b corners defined in world coords*.

#### To create a room
So a size 5,5 room in the upper-left corner of the map would have world coords 1,1,5,5 (floor in positions 12345 on second-through-sixth rows) and impose DUNG_FLAG_LIT on world coords 0,0 to 6,6 (lit in positions 0123456 on the first 7 rows).

#### To create a hallway
A 5-unit-long east-west hallway in the upper-left corner of the world would have coords 1,1,5,1 (floor in positions 12345 on second row) and impose DUNG_FLAG_LIT on world coords 0,0 to 6,2 (lit in positions 0123456 on first 3 rows)

#### Boundary checking is key to this problem
The boundary checking has a boatload of little off-by-one errors, as you can imagine.


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