# JMoria
My from-scratch implementation of a Roguelike game that will be an homage to IMoria

## Build Modes

JMoria supports three build configurations:

| Command | Renderer | Dependencies |
|---|---|---|
| `make` | Both (runtime selection) | SDL2, SDL2_image, OpenGL, ncurses |
| `make ascii` | ASCII only | ncurses |
| `make opengl` | OpenGL only | SDL2, SDL2_image, OpenGL |

## Running

* **ASCII-only build:** `./jmoria` (renders in the current terminal)
* **OpenGL-only build:** `./jmoria` (opens an OpenGL window)
* **Both build:** `./jmoria --renderer=ascii` or `./jmoria --renderer=opengl` (required)

## Keyboard commands

### Panel toggles (display-only, no turn consumed)
* *i* - show character inventory
* *e* - show equipment
* *@* - show character stats
* *v* - show visible monsters sidebar
* *V* - show monster recall panel
* *(* - show item recall panel
* *)* - show map overview panel

### Movement
* *Arrow keys (or numberpad)* - movement
* *hjklyubn* - movement
* *HJKLYUBN* - run until disturbed

### Actions
* *o* - open a door
* *c* - close a door
* *T* - tunnel through rubble
* *<* - go up a staircase
* *>* - go down a staircase
* *g* - get/pick up item
* *d* - drop item
* *w* - wield an item
* *t* - remove (take off) a piece of equipment
* *q* - quaff a potion
* *r* - read a scroll
* *z* - zap a wand
* *F* - fill/fuel a light source
* *S* - spike a door shut (requires iron spike in inventory)
* *s* - search for secret doors and traps
* *.* - rest one turn
* *R* - rest until at full health, or until disturbed

### Interaction
* *** - choose target monster (choose target with *.*)
* *:* - look (choose target with movement keys)
* *Ctrl-N* - set character name
* *p* - purchase something in a store

### Wizard Mode (disables score saving)
* *Ctrl-T* - teleport (enables Wizard Mode)
* *Ctrl-F* - set player intrinsic flag (needs Wizard Mode)
* *Ctrl-I* - create item (needs Wizard Mode)
* *Ctrl-S* - summon monster (needs Wizard Mode)
* *Ctrl-D* - identify all inventory (needs Wizard Mode)
* *Ctrl-E* - give all wands and staves (needs Wizard Mode)
* *Ctrl-W* - exit wizard mode

### System
* *Ctrl-C* - Exit


Monster definitions are in _Resources/Monsters.txt_
Item definitions are in _Resources/Items.txt_

Graphics tileset is _Resources/Courier.png_ (OpenGL mode only)
