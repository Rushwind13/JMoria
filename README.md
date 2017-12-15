# JMoria
My from-scratch implementation of a Roguelike game that will be an homage to IMoria

Requires SDL2, SDL2_image, OpenGL

To compile it, run _"make"_

To run it, run _"jmoria"_

Keyboard commands recognized:
* *Ctrl-C* - Exit
* *Arrow keys (or numberpad)* - movement
* *o* - open a door
* *c* - close a door
* *T* - tunnel through rubble
* *<* - go up a staircase
* *>* - go down a staircase
* *i* - show character inventory (removed this in favor of just having it onscreen all the time)
* *w* - wield an item
* *t* - remove (take off) a piece of equipment
* *d* - drop
* *.* - rest one turn
* *R* - rest indefinitely (not fully implemented)
* *q* - quaff a potion

Monster definitions are in _Resources/Monsters.txt_
Item definitions are in _Resources/Items.txt_

Graphics tileset is _Resources/Courier.png_
