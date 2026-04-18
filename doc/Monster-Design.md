# JMoria Monster Design Document

This is the master reference for all monsters in JMoria. Monsters.txt is built from this document.

## How to Read This Document

- **EXISTS** = currently in Monsters.txt and working
- **PLANNED** = designed here, not yet in Monsters.txt
- **REVIEW** = exists but has known issues (depth, attacks, etc.)
- Monsters are grouped by family (MON_IDX type)
- Within each family, monsters are ordered by intended dungeon level

## Monster Type Index (MON_IDX → Tile)

### Lowercase (minor creatures)

| IDX | MON_IDX | Tile | Family | Notes |
|-----|---------|------|--------|-------|
| 0 | ANT | `a` | Ants | |
| 1 | BAT | `b` | Bats | |
| 2 | CENTIPEDE | `c` | Centipedes | |
| 3 | DRAGON | `d` | Dragons (baby → mature) | |
| 4 | HYDRA | `d` | Hydras | Shares tile with DRAGON (multi-headed mechanics) |
| 5 | EYE | `e` | Eyes | |
| 6 | FLY | `f` | Flies | |
| 7 | DRAGON_FLY | `f` | Dragon flies | Shares tile with FLY |
| 8 | FAERIE_DRAGON | `f` | Faerie dragons | Shares tile with FLY |
| 9 | GOLEM | `g` | Golems | |
| 10 | HUMANOID | `h` | Small humanoids | hobbits, gnomes, elves, dwarves, mind flayers |
| 11 | ICKY | `i` | Icky Things | |
| 12 | OOZE | `j` | Oozes/Slimes | Mobile counterpart to stationary JELLY `J` |
| 13 | KOBOLD | `k` | Kobolds | |
| 14 | LOUSE | `l` | Lice | |
| 15 | LEECH | `l` | Leeches | Shares tile with LOUSE |
| 16 | MOLD | `m` | Molds | |
| 17 | NAGA | `n` | Nagas | |
| 18 | ORC | `o` | Orcs | includes goblins (Skaven, Orc, Uruk-hai) |
| 19 | PERSON | `p` | Persons (human-sized) | warriors, mages, priests, etc. |
| 20 | RAT | `r` | Rats/mice | |
| 21 | SKELETON | `s` | Skeletons | |
| 22 | MINOR_DEMON | `u` | Minor demons | |
| 23 | WORM | `w` | Worm masses | includes Purple Worm at deep levels |
| 24 | SPIDER | `x` | Spiders | |
| 25 | YEEK | `y` | Yeeks | |
| 26 | ZOMBIE | `z` | Zombies & Mummies | themed variants (Kobold Zombie, Mummified Orc, etc.) |

### Uppercase (major creatures)

| IDX | MON_IDX | Tile | Family | Notes |
|-----|---------|------|--------|-------|
| 27 | FROG | `A` | Amphibians | A = Amphibian |
| 28 | BALROG | `B` | Balrog | Boss creature |
| 29 | DOG | `C` | Canines | C = Canid |
| 30 | ANCIENT_DRAGON | `D` | Ancient dragons | Only Ancient age |
| 31 | DINOSAUR | `D` | Dinosaurs | Shares tile with ANCIENT_DRAGON |
| 32 | ELEMENTAL | `E` | Elementals | Fire, Cold, Lightning, Acid + Earth, Air |
| 33 | BIRD | `F` | Birds | Fowl — hawks, eagles, ravens |
| 34 | GHOUL | `G` | Ghouls/ghasts | |
| 35 | GHOST | `G` | Ghosts/spectres | Shares tile with GHOUL |
| 36 | HARPY | `H` | Harpies | |
| 37 | INSECT | `I` | Insects | bees, scorpions |
| 38 | JELLY | `J` | Jellies (stationary) | mobile oozes/slimes → `j` |
| 39 | BEETLE | `K` | Beetles | K = Killer beetle |
| 40 | LICH | `L` | Liches | |
| 41 | MAMMAL | `M` | Mammals | bears, hippos, big cats |
| 42 | CAT | `M` | Felines | Shares tile with MAMMAL |
| 43 | OGRE | `O` | Ogres | |
| 44 | GIANT | `P` | Giants/ettins | |
| 45 | L_PERSON | `P` | Large persons | titans; shares tile with GIANT |
| 46 | REPTILE | `R` | Reptiles | |
| 47 | SNAKE | `S` | Snakes | |
| 48 | TROLL | `T` | Trolls | |
| 49 | MAJOR_DEMON | `U` | Major demons | |
| 50 | VAMPIRE | `V` | Vampires | |
| 51 | WIGHT | `W` | Wights | |
| 52 | WRAITH | `W` | Wraiths | Shares tile with WIGHT |
| 53 | XORN | `X` | Xorns | |
| 54 | YETI | `Y` | Yetis | |

### Special

| IDX | MON_IDX | Tile | Family | Notes |
|-----|---------|------|--------|-------|
| 55 | MIMIC | `&` | Mimics | Blocked by treasure chests |
| 56 | LURKER | `.` | Lurkers | Disguised as floor |
| 57 | SHROOM | `,` | Mushroom patches | Camouflage as food |
| 58 | COIN | `$` | Creeping coins | |
| 59 | TOWNSFOLK | `t` | Town NPCs | Tourist, etc. |
| 60 | ANIMATED_WEAPON | `\|` | Animated weapons | Camouflage as dropped weapons |

### Resolved Types (no new MON_IDX needed)

| Type | Resolution |
|------|------------|
| Goblin | Subfamily of ORC `o` (Skaven, Orc, Uruk-hai progression) |
| Pudding | Part of JELLY `J` (stationary) or OOZE `j` (mobile) |
| Mummy | Part of ZOMBIE `z` (themed variants: Mummified Orc, etc.) |
| Mind Flayer | Part of HUMANOID `h` (small humanoid) |
| Wolf | Part of DOG `C` (Stray Dog → Coyote → Wolf → Dire Wolf → Warg → Shadow Mastiff) |
| Hippo | Part of MAMMAL `M` |
| Purple Worm | Part of WORM `w` (surprise factor at deep levels, high HP) |

---

## Monster Families — Full Design

Attack format: `EFFECT_TYPE, MON_FLAG, [EFFECT_FLAG], NdM[, cooldown]`

### `a` — Ants (MON_IDX_ANT)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Giant Ant | 3 | 3d8 | 80 | 1.0 | SEEKPLAYER | HIT,BITE,1d3 | WARM | 140,96,1 | EXISTS |

### `b` — Bats (MON_IDX_BAT)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Flaming Bat | 5 | 4d8 | 50 | 2.0 | 100RANDOM | HIT,BITE,FIRE,2d8 | WARM | 255,100,0 | REVIEW: no normal bat exists; flaming bat at level 5 seems high for HD 4d8 |

### `c` — Centipedes (MON_IDX_CENTIPEDE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `d` — Dragons (MON_IDX_DRAGON) and Drakes / Hydras (MON_IDX_HYDRA)

Age progression: Baby → Young → Adult → Mature (untitled) → (Ancient = `D`)
Color progression by depth: White → Black → Green → Blue → Red
5 colors × 5 ages = 25 dragons + Ancient variants on `D`

**Hydras** use separate MON_IDX_HYDRA (multi-headed mechanics). Small hydras `d`, big hydras `D`.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Fire Drake | 7 | 6d8 | 50 | 1.4 | SEEKPLAYER | HIT,BITE,2d8; HIT,BREATHE,FIRE,1d10 | WARM | 255,100,0 | EXISTS |
| Frost Drake | 7 | 6d8 | 50 | 1.4 | SEEKPLAYER | HIT,BITE,2d8; HIT,BREATHE,COLD,1d10 | WARM | 150,200,255 | EXISTS |
| Storm Drake | 7 | 6d8 | 50 | 1.4 | SEEKPLAYER | HIT,BITE,2d8; HIT,BREATHE,ELECTRICITY,1d10 | WARM | 255,255,0 | EXISTS |
| Acid Drake | 7 | 6d8 | 50 | 1.4 | SEEKPLAYER | HIT,BITE,2d8; HIT,BREATHE,ACID,1d10 | WARM | 0,200,0 | EXISTS |
| White Dragon | 15 | 7d8 | 80 | 1.0 | SEEKPLAYER | HIT,CLAW,1d4; HIT,CLAW,1d4; HIT,BITE,2d8 | WARM | 255,255,255 | REVIEW: no breath attack |
| Black Dragon | 20 | 8d8 | 80 | 1.0 | SEEKPLAYER | HIT,CLAW,1d4; HIT,CLAW,1d4; HIT,BITE,3d6 | WARM | 30,30,30 | REVIEW: no breath attack |
| Green Dragon | 25 | 9d8 | 90 | 1.0 | SEEKPLAYER | HIT,CLAW,1d6; HIT,CLAW,1d6; HIT,BITE,2d10 | WARM | 0,200,0 | REVIEW: no breath attack |
| Blue Dragon | 30 | 10d8 | 90 | 1.0 | SEEKPLAYER | HIT,CLAW,1d6; HIT,CLAW,1d6; HIT,BITE,3d8 | WARM | 0,100,255 | REVIEW: no breath attack |
| Red Dragon | 35 | 11d8 | 120 | 1.0 | SEEKPLAYER | HIT,CLAW,1d8; HIT,CLAW,1d8; HIT,BITE,3d10 | WARM | 255,0,0 | REVIEW: no breath attack |

### `D` — Dinosaurs (MON_IDX_DINOSAUR) — now on `D` tile

Dinosaurs moved from `d` to `D`. Shares tile with Ancient Dragons and large Hydras.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Brontosaurus | 15 | 25d8 | 60 | 1.0 | SEEKPLAYER | HIT,TRAMPLE,3d6 | WARM | 0,81,4 | EXISTS: tile changing to D |
| Tyrannosaurus Rex | 15 | 18d8 | 60 | 2.0 | SEEKPLAYER | HIT,CLAW,1d6; HIT,CLAW,1d6; HIT,BITE,5d8 | WARM | 206,91,24 | EXISTS: tile changing to D |

### `e` — Eyes (MON_IDX_EYE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Floating Eye | 1 | 1d4 | 20 | 0.25 | DONTMOVE | INTRINSIC,PARALYZE,1d20 | HURT_BY_LIGHT | 255,100,0 | REVIEW: leading space on Monster line in data file |

### `f` — Flies / Dragon Flies / Faerie Dragons (MON_IDX_FLY, DRAGON_FLY, FAERIE_DRAGON)

Flies, Dragon Flies, and Faerie Dragons moved here from `F`. Cats moved to `M` (Mammal).

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(needs design: Giant Fly, Fire Fly, Dragon Fly, Faerie Dragon progression)* | | | | | | | | | PLANNED |

### `g` — Golems (MON_IDX_GOLEM)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `h` — Small Humanoids (MON_IDX_HUMANOID)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: hobbit, gnome, elf, dwarf, mind flayer)* | | | | | | | | | |

### `i` — Icky Things (MON_IDX_ICKY)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| White Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 255,255,255 | EXISTS |
| Red Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 255,0,0 | EXISTS |
| Blue Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 0,0,255 | EXISTS |
| Yellow Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 255,255,0 | EXISTS |
| Green Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 0,255,0 | EXISTS |
| Clear Icky Thing | 3 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | 255,255,255,128 | EXISTS |
| Opal Icky Thing | 1 | 2d8 | 40 | 1.0 | 100RANDOM | ? | ? | multi | EXISTS |
| *(missing: Black, Brown, Grey Icky Things)* | | | | | | | | | PLANNED |

### `j` — Oozes / Slimes (MON_IDX_OOZE)

Mobile counterpart to stationary JELLY `J`. Oozes and slimes move; jellies don't.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(needs design: Green Slime, Grey Ooze, Ochre Ooze, Black Pudding, etc.)* | | | | | | | | | PLANNED |

### `k` — Kobolds (MON_IDX_KOBOLD)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Kobold | 2 | 1d4 | 40 | 1.0 | SEEKPLAYER | HIT,BITE,1d4 | WARM | 170,170,170 | REVIEW: needs Small Kobold, Shaman, Chieftain, Brute |

### `l` — Lice / Leeches (MON_IDX_LOUSE, MON_IDX_LEECH)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `m` — Molds (MON_IDX_MOLD)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: color molds like icky things)* | | | | | | | | | |

### `n` — Nagas (MON_IDX_NAGA)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `o` — Orcs (MON_IDX_ORC)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Skaven, Orc, Uruk-hai + Shaman/Captain/Chieftain/Brute variants)* | | | | | | | | | |

### `p` — Persons (MON_IDX_PERSON)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: AD&D class level titles — warriors, mages, priests, rangers, rogues)* | | | | | | | | | |

### `r` — Rats (MON_IDX_RAT)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `s` — Skeletons (MON_IDX_SKELETON)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Cursed Skeleton | 7 | 5d8 | 40 | 0.7 | SEEKPLAYER | HIT,CLAW,2d6; HIT,TOUCH,AFRAID,1d10 | EMPTY_MIND | 192,192,192 | EXISTS |

### `u` — Minor Demons (MON_IDX_MINOR_DEMON)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Lesser Demon, etc.)* | | | | | | | | | |

### `w` — Worm Masses (MON_IDX_WORM)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| White Worm Mass | 1 | 1d8 | 40 | 0.5 | 100RANDOM | INTRINSIC,POISON,1d5 | BREED,EMPTY_MIND,HURT_BY_LIGHT | 255,255,255 | REVIEW: only INTRINSIC attack, no physical HIT |
| Red Worm Mass | 1 | 1d8 | 40 | 0.5 | 100RANDOM | HIT,TOUCH,1d2; HIT,CRAWL,FIRE,1d5 | BREED,EMPTY_MIND,HURT_BY_LIGHT | 255,0,0 | EXISTS |
| Blue Worm Mass | 1 | 1d8 | 40 | 0.5 | 100RANDOM | HIT,TOUCH,1d2; HIT,CRAWL,COLD,1d5 | BREED,EMPTY_MIND,HURT_BY_LIGHT | 0,0,255 | EXISTS |
| Yellow Worm Mass | 1 | 1d8 | 40 | 0.5 | 100RANDOM | HIT,TOUCH,1d2; HIT,CRAWL,POISON,1d5 | BREED,EMPTY_MIND,HURT_BY_LIGHT | 255,255,0 | EXISTS |
| Green Worm Mass | 1 | 1d8 | 40 | 0.5 | 100RANDOM | HIT,TOUCH,1d2; HIT,CRAWL,ACID,1d5 | BREED,EMPTY_MIND,HURT_BY_LIGHT | 0,255,0 | EXISTS |
| *(missing: Black, Brown, Grey, Clear, Purple Worm Mass)* | | | | | | | | | PLANNED |

### `x` — Spiders (MON_IDX_SPIDER)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Huge Spider | 5 | 2d8 | 50 | 1.0 | 100RANDOM | HIT,BITE,1d6 | WARM | 128,128,128 | EXISTS |
| Venomous Spider | 7 | 6d8 | 50 | 0.8 | SEEKPLAYER | HIT,BITE,2d6; INTRINSIC,POISON,1d6 | WARM | 0,180,0 | EXISTS |

### `y` — Yeeks (MON_IDX_YEEK)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `z` — Zombies & Mummies (MON_IDX_ZOMBIE)

Themed variants — zombie/mummy versions of base creature types. Inherit some base creature traits.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|---------|
| Kobold Zombie | 3 | | | | SEEKPLAYER | | EMPTY_MIND | | PLANNED |
| Orc Zombie | 6 | | | | SEEKPLAYER | | EMPTY_MIND | | PLANNED |
| Human Zombie | 10 | | | | SEEKPLAYER | | EMPTY_MIND | | PLANNED |
| Troll Zombie | 15 | | | | SEEKPLAYER | | EMPTY_MIND,REGENERATE | | PLANNED: regenerates like base troll |
| Zombie Dragon | 25 | | | | SEEKPLAYER | | EMPTY_MIND | | PLANNED: no breath attack (mindless) |
| Mummified Orc | 8 | | | | SEEKPLAYER | | EMPTY_MIND | | PLANNED |
| *(more themed variants TBD)* | | | | | | | | | |

### `A` — Amphibians (MON_IDX_FROG)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Giant Frog | 5 | 3d8 | 40 | 2.0 | SEEKPLAYER | HIT,BITE,2d8 | WARM | 71,165,0 | EXISTS |
| Death Slaad | 10 | 4d8 | 50 | 2.0 | SEEKPLAYER | HIT,BITE,FIRE,2d4 | WARM | 71,165,0 | EXISTS |

### `B` — Balrog (MON_IDX_BALROG)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Balrog | 100 | 100d8 | ? | 4.0 | SEEKPLAYER | BITE; CLAW×2; BREATHE FIRE; BREATHE POISON; whip/sword | WARM,REGENERATE | ? | PLANNED (boss) |

### `C` — Canines (MON_IDX_DOG)

Progression: Stray Dog (town) → Coyote → Wolf → Dire Wolf → Warg → Shadow Mastiff

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|---------|
| Shadow Mastiff | 15 | 4d8 | 50 | 1.0 | SEEKPLAYER | HIT,BITE,2d8 | WARM,HURT_BY_LIGHT | multi | REVIEW: need full progression (Stray Dog through Warg) |

### `D` — Ancient Dragons (MON_IDX_ANCIENT_DRAGON)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Ancient Multi-hued Dragon | 50 | 15d8 | 110 | 1.0 | SEEKPLAYER | HIT,CLAW,1d4×2; HIT,BITE,2d8; BREATHE FIRE/COLD/ELEC/ACID/POISON 15d8 cd5 | WARM | multi | EXISTS |
| *(missing: Ancient White, Ancient Black, Ancient Green, Ancient Blue, Ancient Red)* | | | | | | | | | PLANNED |

### `E` — Elementals (MON_IDX_ELEMENTAL)

At least Fire, Cold, Lightning, Acid + Earth, Air. Possibly Elemental Lords at deep levels.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(needs design: Fire Elemental, Ice Elemental, Lightning Elemental, Acid Elemental, Earth Elemental, Air Elemental, Elemental Lords)* | | | | | | | | | PLANNED |

### `F` — Birds (MON_IDX_BIRD)

`F` freed by flies moving to `f`. Fowl / Birds.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(needs design: Hawk, Raven, Eagle, Giant Eagle progression)* | | | | | | | | | PLANNED |

### `G` — Ghouls / Ghosts (MON_IDX_GHOUL, MON_IDX_GHOST)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: ghoul, ghast, ghost, spectre)* | | | | | | | | | |

### `H` — Harpies (MON_IDX_HARPY)

Moved from `h` to `H`.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|---------|
| White Harpy | 15 | 5d8 | 60 | 1.0 | SEEKPLAYER | HIT,CLAW,3d6 | WARM | 255,255,255 | REVIEW: Plural is "White Harpy" (should be "White Harpies") |
| *(needs design: color variants, flying humanoid subtypes)* | | | | | | | | | PLANNED |

### `I` — Insects (MON_IDX_INSECT)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Giant Bee, Giant Scorpion)* | | | | | | | | | |

### `J` — Jellies (MON_IDX_JELLY) — stationary only

Stationary jellies stay here. Mobile oozes/slimes moved to `j` (MON_IDX_OOZE).

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Red Jelly | 1 | 2d5 | 10 | 1.0 | DONTMOVE | HIT,TOUCH,1d2; HIT,SPORE,FIRE,1d5 | EMPTY_MIND,HURT_BY_LIGHT | 255,0,0 | EXISTS |
| Green Jelly | 2 | 3d5 | 10 | 1.0 | DONTMOVE | HIT,TOUCH,1d2; HIT,SPORE,ACID,1d5 | EMPTY_MIND,HURT_BY_LIGHT | 0,200,0 | EXISTS |
| *(missing: Yellow, Blue, Black, Clear, Ochre Jelly; Gelatinous Cube; oozes; slimes; puddings)* | | | | | | | | | PLANNED |

### `K` — Beetles (MON_IDX_BEETLE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `L` — Liches (MON_IDX_LICH)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Lich, Ancient Lich, Emperor Lich)* | | | | | | | | | |

### `M` — Mammals (MON_IDX_MAMMAL)

Bears, hippos, big cats. Cats moved here from `f`.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(needs design: Cat, Panther, Tiger, Black Bear, Grizzly Bear, Cave Bear, Hippo)* | | | | | | | | | PLANNED |

### `O` — Ogres (MON_IDX_OGRE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `P` — Giants & Large Persons (MON_IDX_GIANT, MON_IDX_L_PERSON)

Giants moved from `H` to `P`. Shares tile with L_PERSON (Titans).

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|---------|
| *(needs design: Hill Giant, Stone Giant, Frost Giant, Fire Giant, Cloud Giant, Storm Giant, Ettin, Titan)* | | | | | | | | | PLANNED |

### `R` — Reptiles (MON_IDX_REPTILE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `S` — Snakes (MON_IDX_SNAKE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Giant Snake | 2 | 4d8 | 34 | 1.0 | 100RANDOM | HIT,BITE,1d3 | WARM | 198,142,0 | EXISTS |
| *(missing: Poison Snake, King Cobra, etc.)* | | | | | | | | | PLANNED |

### `T` — Trolls (MON_IDX_TROLL)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(Troll Berserker commented out in Monsters.txt: level 8, 7d8, REGENERATE+WARM, Claw 3d6 + Trample 2d10)* | | | | | | | | | PLANNED |

### `U` — Major Demons (MON_IDX_MAJOR_DEMON)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Greater Demon, Demon Lord, Greater Demon Lord)* | | | | | | | | | |

### `V` — Vampires (MON_IDX_VAMPIRE)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design: Vampire, Master Vampire, Vampire Lord)* | | | | | | | | | |

### `W` — Wights / Wraiths (MON_IDX_WIGHT, MON_IDX_WRAITH)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `X` — Xorns (MON_IDX_XORN)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `Y` — Yetis (MON_IDX_YETI)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `,` — Mushroom Patches (MON_IDX_SHROOM)

**CAMOUFLAGE**: Mushroom patches display as `,` — same tile as food items on the ground. A starving player sees "food", approaches, and gets ambushed.

**Shrieker Mechanic** (needs new flags/constants):
- Shrieker Mushroom Patch: "makes a loud wailing sound" → wakes sleeping monsters in range + aggravates (removes MONSTER_SLEEPING, adds AGGRAVATE_MONSTER effect within X tiles)
- Regular Mushroom Patch: releases spores (poison/paralysis)
- **TODO**: Need `MONSTER_SLEEPING` state flag and `AGGRAVATE_MONSTER` mechanic

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|---------|
| Yellow Mushroom Patch | 1 | 1d4 | 50 | 1.0 | DONTMOVE | HIT,SPORE,1d1; INTRINSIC,POISON,1d5 | EMPTY_MIND,HURT_BY_LIGHT | 255,255,0 | EXISTS |
| Shrieker Mushroom Patch | 3 | | | | DONTMOVE | *(shrieker mechanic — wakes/aggravates)* | EMPTY_MIND,HURT_BY_LIGHT | | PLANNED |
| *(more color variants TBD)* | | | | | | | | | PLANNED |

### `&` — Mimics (MON_IDX_MIMIC)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(blocked by treasure chests system)* | | | | | | | | | |

### `.` — Lurkers (MON_IDX_LURKER)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `$` — Creeping Coins (MON_IDX_COIN)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| *(empty — needs design)* | | | | | | | | | |

### `t` — Townsfolk (MON_IDX_TOWNSFOLK)

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Tourist | 1 | 16d20 | ? | 1.0 | NPC_100RANDOM | *(peaceful)* | ? | ? | EXISTS (NPC) |

### `|` — Animated Weapons (MON_IDX_ANIMATED_WEAPON)

**CAMOUFLAGE**: Uses `|` tile — same as dropped weapons on the ground. Must use same color as the real weapon to complete the disguise. Player sees a weapon, approaches, and it attacks.

| Monster | Level | HD | AC | Speed | AI | Attacks | Flags | Color | Status |
|---------|-------|----|----|-------|----|---------|-------|-------|--------|
| Animated Sword | | | | | SEEKPLAYER | HIT,BITE | EMPTY_MIND | *(match Long Sword color)* | PLANNED |
| Animated Spear | | | | | SEEKPLAYER | HIT,BITE | EMPTY_MIND | *(match Spear color)* | PLANNED |
| Animated Axe | | | | | SEEKPLAYER | HIT,BITE | EMPTY_MIND | *(match Battle Axe color)* | PLANNED |
| Animated Club | | | | | SEEKPLAYER | HIT,BITE | EMPTY_MIND | *(match Club color)* | PLANNED |


---

## Brust / Dragaeran Creatures — Design Notes

The game world uses Brust-flavored naming wherever possible. Animals and monsters from the Dragaeran cycle replace their generic counterparts:

### Naming Convention
- **No "(Dragaeran)" suffix** — Dragaeran names replace the generic name directly
- If both a Dragaeran and mundane version exist (e.g., a `teckla` rat and a Teckla person), use **lowercase for the animal** and **uppercase for the Dragaeran** (e.g., `teckla` vs `Teckla`)
- Open question: will duplicate names break monster lookup? Needs testing with the data file parser

### Implemented Renames (Wave 7)
- **Rat → Teckla** (all 3 tiers: Teckla, Large Teckla, Giant Teckla) — `r` RAT type
- **Spider → Creotha** (Huge, Venomous, Cave, Giant) — `S` SPIDER type. Shelob stays as-is (unique)
- **Panther → Young Dzur** — `M` CAT type
- **Grizzly Bear → Tsalmoth** — `M` MAMMAL type
- **Coyote → Lyorn** — `C` DOG type

### Implemented Brust Creatures (Wave 7)
- **Norska** (0), **Large Norska** (5) — `r` RAT type (rabbits)
- **Jhereg** (10), **Greater Jhereg** (25) — `f` FAERIE_DRAGON type (venomous flying lizards)
- **Tiassa** (18) — `f` FLY type (confuse attack)
- **Athyra** (20) — `F` BIRD type (confuse attack)
- **Dzur** (35), **Elder Dzur** (55) — `M` CAT type
- **Teckla (Dragaeran)** (4) — `P` L_PERSON type (rename needed — see below)
- **Cat-centaur** (40) — `P` L_PERSON type

### TODO: Dragaeran Persons (`P` L_PERSON)
Drop the "(Dragaeran)" suffix — just use the House name directly:
- **Teckla** — peasant/commoner, low level (~4). Same name as the animal; context (tile `P` vs `r`) distinguishes them
- **Dragonlord** — Dragaeran warrior-noble. Mid-high level (or several ranks of them)
- **Dzurlord** — fierce Dzur-house warrior, aggressive. Mid-high level
- **Hawklord** — Dragaeran scholar. Mid level
- **Tiassa** (person) — same name as the flying creature on `f`; `P` tile distinguishes. Mid level
- **Lyorn** (person) — same name as the animal on `C`; `P` tile distinguishes. Mid level
- **Yendi** (person) — sorcerous, sneaky. Mid level
- **Phoenix Guard** — elite soldier, high level (can have ranks, Guard Sergeant, Guard Captain, Captain of the Phoenix Guards (unique))
- **Lavode** — one of the Lavode troops, very high level warrior / wizard with magic blade and magic staff

### TODO: Sethra Lavode (Unique)
- Sethra Lavode should exist as a unique monster
- **Not killable in any real sense** — needs special handling (immortal / respawns / unkillable flag?)
- May need to move uniques out of Monsters.txt once we introduce loot tables and flavor text
- Design the unique monster system before implementing Sethra

### TODO: Snake → Yendi Rename
- Rename ALL snakes to Yendi: "X Snake" → "X Yendi"
- e.g., Giant Snake → Giant Yendi, King Snake → King Yendi, etc.
- Snake Skeleton -> Skeleton Yendi
- Uses `J` (or whatever tile snakes are on) — same type, just renamed
