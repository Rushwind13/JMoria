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
* *Ctrl-C* - Exit
* *Arrow keys (or numberpad)* - movement
* *hjklyubn* - movement
* *HJKLYUBN* - run until disturbed
* *o* - open a door
* *c* - close a door
* *T* - tunnel through rubble
* *<* - go up a staircase
* *>* - go down a staircase
* *i* - show character inventory (removed this in favor of just having it onscreen all the time)
* *w* - wield an item
* *t* - remove (take off) a piece of equipment
* *d* - drop
* *** - choose target monster (choose target with *.*)
* *:* - look (choose target with *.*)
* *.* - rest one turn
* *R* - rest until at full health, or until disturbed
* *q* - quaff a potion
* *r* - read a scroll
* *z* - zap a wand
* *N* - set character name
* *Ctrl-T* - ^t - teleport (Note: will turn on Wizard Mode)
* *Ctrl-F* - ^f - set player intrinsic flag (Note: needs Wizard Mode)
* *Ctrl-I* - ^i - create item (Note: needs Wizard Mode)
* *Ctrl-S* - ^s - summon monster (Note: needs Wizard Mode)


Monster definitions are in _Resources/Monsters.txt_
Item definitions are in _Resources/Items.txt_

Graphics tileset is _Resources/Courier.png_ (OpenGL mode only)
