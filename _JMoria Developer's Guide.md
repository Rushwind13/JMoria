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
