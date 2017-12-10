#  JMoria Developer's Guide

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

Create a Pull request on Github when you're satisfied.
