# JMoria Monster Design Document

This is the master reference for all monsters in JMoria. Monsters.txt is built from this document.

## How to Read This Document

- **EXISTS** = currently in Monsters.txt and working
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
| 10 | HUMANOID | `h` | Small humanoids | dark elves, gnolls, mind flayers, bugbears |
| 11 | ICKY | `i` | Icky Things | |
| 12 | OOZE | `j` | Oozes/Slimes | Mobile counterpart to stationary JELLY `J` |
| 13 | KOBOLD | `k` | Kobolds | |
| 14 | LOUSE | `l` | Lice | |
| 15 | LEECH | `l` | Leeches | Shares tile with LOUSE |
| 16 | MOLD | `m` | Molds | |
| 17 | NAGA | `n` | Nagas | |
| 18 | ORC | `o` | Orcs | includes goblins (Skaven, Goblin, Cave/Hill/Black Orc, Uruk-hai) |
| 19 | PERSON | `p` | Persons (human-sized) | warriors, mages, priests, rangers, rogues |
| 20 | RAT | `r` | Rats | Brust: Teckla (rat), Norska (rabbit) |
| 21 | SKELETON | `s` | Skeletons | |
| 22 | MINOR_DEMON | `u` | Minor demons | |
| 23 | WORM | `w` | Worm masses | includes Purple Worm at deep levels |
| 24 | SPIDER | `x` | Spiders | Brust: Creotha |
| 25 | YEEK | `y` | Yeeks | |
| 26 | ZOMBIE | `z` | Zombies & Mummies | themed variants (Kobold Zombie, Mummified Orc, etc.) |

### Uppercase (major creatures)

| IDX | MON_IDX | Tile | Family | Notes |
|-----|---------|------|--------|-------|
| 27 | FROG | `A` | Amphibians | A = Amphibian |
| 28 | BALROG | `B` | Balrog | Boss creature |
| 29 | DOG | `C` | Canines | C = Canid; Brust: Lyorn (dog) |
| 30 | ANCIENT_DRAGON | `D` | Ancient dragons | Only Ancient age |
| 31 | DINOSAUR | `D` | Dinosaurs | Shares tile with ANCIENT_DRAGON |
| 32 | ELEMENTAL | `E` | Elementals | Fire, Ice, Lightning, Earth, Air + Lords |
| 33 | BIRD | `F` | Birds | Fowl — hawks, eagles, ravens; Brust: Athyra |
| 34 | GHOUL | `G` | Ghouls/ghasts | |
| 35 | GHOST | `G` | Ghosts/phantoms | Shares tile with GHOUL |
| 36 | HARPY | `H` | Harpies | |
| 37 | INSECT | `I` | Insects | bees, scorpions, wasps |
| 38 | JELLY | `J` | Jellies (stationary) | mobile oozes/slimes → `j` |
| 39 | BEETLE | `K` | Beetles | K = Killer beetle |
| 40 | LICH | `L` | Liches | |
| 41 | MAMMAL | `M` | Mammals | bears, boars, hippos; Brust: Tsalmoth |
| 42 | CAT | `M` | Felines | Shares tile with MAMMAL; Brust: Dzur |
| 43 | OGRE | `O` | Ogres | |
| 44 | GIANT | `P` | Giants/ettins | |
| 45 | L_PERSON | `P` | Large persons | Dragaeran persons; shares tile with GIANT |
| 46 | REPTILE | `R` | Reptiles | |
| 47 | SNAKE | `S` | Yendi | Brust: snakes renamed to Yendi |
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
| 55 | MIMIC | `&` | Mimics | |
| 56 | LURKER | `.` | Lurkers | Disguised as floor |
| 57 | SHROOM | `,` | Mushroom patches | Camouflage as food |
| 58 | COIN | `$` | Creeping coins | |
| 59 | TOWNSFOLK | `t` | Town NPCs | Tourist, etc. |
| 60 | ANIMATED_WEAPON | `\|` | Animated weapons | Camouflage as dropped weapons |

### Resolved Types (reference)

These creature concepts are subfamilies within existing types — no new MON_IDX needed:

- **Goblin** → ORC `o` (Skaven, Goblin, Cave/Hill/Black Orc, Uruk-hai)
- **Pudding** → JELLY `J` (stationary) or OOZE `j` (mobile)
- **Mummy** → ZOMBIE `z` (themed variants: Mummified Orc, etc.)
- **Mind Flayer** → HUMANOID `h`
- **Wolf** → DOG `C` (Stray Dog → Jackal → Wild Dog → Lyorn → Wolf → Dire Wolf → Warg → Shadow Mastiff)
- **Hippo** → MAMMAL `M`
- **Purple Worm** → WORM `w` (surprise factor at deep levels, high HP)

---

## Brust / Dragaeran Naming Convention

The game world uses Brust-flavored naming wherever possible. Animals and monsters from the Dragaeran cycle replace their generic counterparts.

- **No "(Dragaeran)" suffix** — Dragaeran names replace the generic name directly
- Animals use lowercase naming (teckla, lyorn, tiassa) — creatures on `r`, `C`, `f` tiles
- Dragaeran persons use uppercase naming (Teckla, Lyorn, Tiassa) — persons on `P` tile
- Duplicate names across types (e.g., Teckla rat + Teckla person) are a **known issue** with `GetMonsterDef(const char*)` first-match lookup — see [#267](https://github.com/Rushwind13/JMoria/issues/267). Normal gameplay unaffected (uses index-based lookup).

---

## Monster Families — Full Design

Attack format: `EFFECT_TYPE, MON_FLAG, [EFFECT_FLAG], NdM[, cooldown]` (legacy format — see [Effects-Design.md](Effects-Design.md) §13 for the new AttackGroup architecture)

### `a` — Ants (MON_IDX_ANT)

| Monster | Level | Status |
|---------|-------|--------|
| Giant Ant | 3 | EXISTS |

### `b` — Bats (MON_IDX_BAT)

| Monster | Level | Status |
|---------|-------|--------|
| Bat | 1 | EXISTS |
| Fruit Bat | 2 | EXISTS |
| Large Bat | 5 | EXISTS |
| Vampire Bat | 8 | EXISTS |
| Flaming Bat | 8 | EXISTS |

### `c` — Centipedes (MON_IDX_CENTIPEDE)

| Monster | Level | Status |
|---------|-------|--------|
| White Centipede | 1 | EXISTS |
| Yellow Centipede | 2 | EXISTS |
| Giant Centipede | 3 | EXISTS |
| Red Centipede | 4 | EXISTS |
| Blue Centipede | 5 | EXISTS |
| Green Centipede | 6 | EXISTS |
| Cyan Centipede | 8 | EXISTS |
| Magenta Centipede | 10 | EXISTS |

### `d` — Dragons (MON_IDX_DRAGON) and Hydras (MON_IDX_HYDRA)

Age progression: Baby → Young → Adult → Mature (untitled) → (Ancient = `D`)
Color progression by depth: White → Black → Green → Blue → Red
5 colors × 4 ages + 5 mature = 25 dragons on `d`

**Drakes** (elemental breath specialists):

| Monster | Level | Status |
|---------|-------|--------|
| Fire Drake | 7 | EXISTS |
| Frost Drake | 7 | EXISTS |
| Storm Drake | 7 | EXISTS |
| Acid Drake | 7 | EXISTS |

**Dragons** (color × age):

| Monster | Level | Status |
|---------|-------|--------|
| Baby White Dragon | 5 | EXISTS |
| Young White Dragon | 10 | EXISTS |
| Adult White Dragon | 18 | EXISTS |
| White Dragon | 25 | EXISTS |
| Baby Black Dragon | 8 | EXISTS |
| Young Black Dragon | 14 | EXISTS |
| Adult Black Dragon | 22 | EXISTS |
| Black Dragon | 30 | EXISTS |
| Baby Green Dragon | 10 | EXISTS |
| Young Green Dragon | 18 | EXISTS |
| Adult Green Dragon | 26 | EXISTS |
| Green Dragon | 35 | EXISTS |
| Baby Blue Dragon | 12 | EXISTS |
| Young Blue Dragon | 22 | EXISTS |
| Adult Blue Dragon | 30 | EXISTS |
| Blue Dragon | 40 | EXISTS |
| Baby Red Dragon | 15 | EXISTS |
| Young Red Dragon | 25 | EXISTS |
| Adult Red Dragon | 35 | EXISTS |
| Red Dragon | 45 | EXISTS |

**Hydras** (MON_IDX_HYDRA — multi-headed mechanics, share `d` tile):

| Monster | Level | Status |
|---------|-------|--------|
| 5-Headed Hydra | 15 | EXISTS |
| 7-Headed Hydra | 25 | EXISTS |
| 11-Headed Hydra | 40 | EXISTS |

### `D` — Ancient Dragons (MON_IDX_ANCIENT_DRAGON) and Dinosaurs (MON_IDX_DINOSAUR)

| Monster | Level | Status |
|---------|-------|--------|
| Brontosaurus | 15 | EXISTS (DINOSAUR) |
| Tyrannosaurus Rex | 15 | EXISTS (DINOSAUR) |
| Ancient White Dragon | 50 | EXISTS |
| Ancient Black Dragon | 55 | EXISTS |
| Ancient Green Dragon | 60 | EXISTS |
| Ancient Blue Dragon | 65 | EXISTS |
| Ancient Red Dragon | 70 | EXISTS |
| Ancient Multi-hued Dragon | 75 | EXISTS |

### `e` — Eyes (MON_IDX_EYE)

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Floating Eye | 1 | EXISTS |

### `f` — Flies / Dragon Flies / Faerie Dragons (MON_IDX_FLY, DRAGON_FLY, FAERIE_DRAGON)

**Flies** (MON_IDX_FLY):

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Fruit Fly | 1 | EXISTS | |
| Giant Fly | 3 | EXISTS | |
| Fire Fly | 5 | EXISTS | |
| Tiassa | 18 | EXISTS | Brust: confuse attack, fast 2.0 speed |

**Dragon Flies** (MON_IDX_DRAGON_FLY):

| Monster | Level | Status |
|---------|-------|--------|
| White Dragon Fly | 3 | EXISTS |
| Black Dragon Fly | 5 | EXISTS |
| Green Dragon Fly | 7 | EXISTS |
| Blue Dragon Fly | 9 | EXISTS |
| Red Dragon Fly | 11 | EXISTS |

**Faerie Dragons** (MON_IDX_FAERIE_DRAGON):

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Pink Baby Faerie Dragon | 5 | EXISTS | |
| Cyan Young Faerie Dragon | 10 | EXISTS | |
| Jhereg | 10 | EXISTS | Brust: venomous flying lizard, fast 1.8 speed |
| Violet Immature Faerie Dragon | 15 | EXISTS | |
| Indigo Faerie Dragon | 20 | EXISTS | |
| Greater Jhereg | 25 | EXISTS | Brust |
| Purple Lordly Faerie Dragon | 30 | EXISTS | |
| Platinum Duke Faerie Dragon | 40 | EXISTS | |
| Prismatic Emperor Faerie Dragon | 50 | EXISTS | |

### `g` — Golems (MON_IDX_GOLEM)

| Monster | Level | Status |
|---------|-------|--------|
| Clay Golem | 20 | EXISTS |
| Stone Golem | 30 | EXISTS |
| Iron Golem | 40 | EXISTS |

### `h` — Small Humanoids (MON_IDX_HUMANOID)

| Monster | Level | Status |
|---------|-------|--------|
| Hobgoblin | 3 | EXISTS |
| Gnoll | 5 | EXISTS |
| Bugbear | 7 | EXISTS |
| Dark Elf | 10 | EXISTS |
| Dark Elven Warrior | 15 | EXISTS |
| Dark Elven Mage | 18 | EXISTS |
| Dark Elven Priest | 20 | EXISTS |
| Dark Elven Ranger | 22 | EXISTS |
| Dark Elven Lord | 28 | EXISTS |
| Mind Flayer | 30 | EXISTS |

### `i` — Icky Things (MON_IDX_ICKY)

| Monster | Level | Status |
|---------|-------|--------|
| White Icky Thing | 1 | EXISTS |
| Red Icky Thing | 1 | EXISTS |
| Blue Icky Thing | 1 | EXISTS |
| Yellow Icky Thing | 1 | EXISTS |
| Green Icky Thing | 1 | EXISTS |
| Opal Icky Thing | 1 | EXISTS |
| Clear Icky Thing | 3 | EXISTS |

### `j` — Oozes / Slimes (MON_IDX_OOZE)

Mobile counterpart to stationary JELLY `J`. Oozes and slimes move; jellies don't.

| Monster | Level | Status |
|---------|-------|--------|
| Grey Ooze | 5 | EXISTS |
| Green Slime | 8 | EXISTS |
| Ochre Ooze | 12 | EXISTS |
| Black Pudding | 18 | EXISTS |

### `k` — Kobolds (MON_IDX_KOBOLD)

| Monster | Level | Status |
|---------|-------|--------|
| Small Kobold | 1 | EXISTS |
| Kobold | 2 | EXISTS |
| Kobold Shaman | 4 | EXISTS |
| Kobold Warrior | 5 | EXISTS |
| Kobold Chieftain | 7 | EXISTS |

### `l` — Lice / Leeches (MON_IDX_LOUSE, MON_IDX_LEECH)

| Monster | Level | Status |
|---------|-------|--------|
| Louse | 1 | EXISTS (LOUSE) |
| Giant Leech | 5 | EXISTS (LEECH) |

### `m` — Molds (MON_IDX_MOLD)

| Monster | Level | Status |
|---------|-------|--------|
| White Mold | 1 | EXISTS |
| Brown Mold | 3 | EXISTS |
| Green Mold | 5 | EXISTS |
| Black Mold | 8 | EXISTS |

### `n` — Nagas (MON_IDX_NAGA)

| Monster | Level | Status |
|---------|-------|--------|
| Naga | 15 | EXISTS |
| Water Naga | 20 | EXISTS |
| Spirit Naga | 28 | EXISTS |
| Guardian Naga | 35 | EXISTS |

### `o` — Orcs (MON_IDX_ORC)

Includes goblins as subfamily (Skaven, Goblin progression).

| Monster | Level | Status |
|---------|-------|--------|
| Skaven | 1 | EXISTS |
| Goblin | 2 | EXISTS |
| Orc | 5 | EXISTS |
| Cave Orc | 7 | EXISTS |
| Orc Shaman | 8 | EXISTS |
| Hill Orc | 10 | EXISTS |
| Black Orc | 12 | EXISTS |
| Orc Captain | 14 | EXISTS |
| Orc Chieftain | 18 | EXISTS |
| Uruk-hai | 20 | EXISTS |

### `p` — Persons (MON_IDX_PERSON)

| Monster | Level | Status |
|---------|-------|--------|
| Acolyte | 3 | EXISTS |
| Rogue | 5 | EXISTS |
| Apprentice Mage | 6 | EXISTS |
| Warrior | 8 | EXISTS |
| Mage | 10 | EXISTS |
| Priest | 12 | EXISTS |
| Ranger | 15 | EXISTS |
| Veteran Warrior | 20 | EXISTS |
| Master Ranger | 25 | EXISTS |
| Champion | 30 | EXISTS |
| High Priest | 35 | EXISTS |
| Archmage | 40 | EXISTS |

### `r` — Rats (MON_IDX_RAT)

Brust: Teckla (rat), Norska (rabbit).

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Teckla | 1 | EXISTS | Brust name for rat |
| Large Teckla | 3 | EXISTS | |
| Norska | 0 | EXISTS | Brust: rabbit, fast 1.5 speed |
| Large Norska | 5 | EXISTS | |
| Giant Teckla | 5 | EXISTS | |

### `s` — Skeletons (MON_IDX_SKELETON)

| Monster | Level | Status |
|---------|-------|--------|
| Kobold Skeleton | 3 | EXISTS |
| Cursed Skeleton | 7 | EXISTS |
| Orc Skeleton | 8 | EXISTS |
| Yendi Skeleton | 12 | EXISTS |
| Human Skeleton | 15 | EXISTS |
| Elf Skeleton | 18 | EXISTS |
| Troll Skeleton | 22 | EXISTS |
| Dragon Skeleton | 30 | EXISTS |

### `u` — Minor Demons (MON_IDX_MINOR_DEMON)

| Monster | Level | Status |
|---------|-------|--------|
| Imp | 5 | EXISTS |
| Lesser Demon | 15 | EXISTS |

### `w` — Worm Masses (MON_IDX_WORM)

| Monster | Level | Status |
|---------|-------|--------|
| White Worm Mass | 1 | EXISTS |
| Red Worm Mass | 1 | EXISTS |
| Blue Worm Mass | 1 | EXISTS |
| Yellow Worm Mass | 1 | EXISTS |
| Green Worm Mass | 1 | EXISTS |
| Clear Worm Mass | 3 | EXISTS |
| Black Worm Mass | 5 | EXISTS |
| Purple Worm | 30 | EXISTS |

### `x` — Spiders (MON_IDX_SPIDER)

Brust: Creotha (spider). Shelob stays as-is (unique literary reference).

| Monster | Level | Status |
|---------|-------|--------|
| Huge Creotha | 5 | EXISTS |
| Venomous Creotha | 7 | EXISTS |
| Cave Creotha | 12 | EXISTS |
| Giant Creotha | 18 | EXISTS |
| Shelob | 45 | EXISTS |

### `y` — Yeeks (MON_IDX_YEEK)

| Monster | Level | Status |
|---------|-------|--------|
| Yeek | 1 | EXISTS |
| Blue Yeek | 3 | EXISTS |
| Master Yeek | 6 | EXISTS |

### `z` — Zombies & Mummies (MON_IDX_ZOMBIE)

Themed variants — zombie/mummy versions of base creature types.

| Monster | Level | Status |
|---------|-------|--------|
| Kobold Zombie | 3 | EXISTS |
| Orc Zombie | 6 | EXISTS |
| Mummified Orc | 8 | EXISTS |
| Human Zombie | 10 | EXISTS |
| Troll Zombie | 15 | EXISTS |
| Zombie Dragon | 25 | EXISTS |

### `A` — Amphibians (MON_IDX_FROG)

| Monster | Level | Status |
|---------|-------|--------|
| Giant Frog | 5 | EXISTS |
| Death Slaad | 10 | EXISTS |

### `B` — Balrog (MON_IDX_BALROG)

| Monster | Level | Status |
|---------|-------|--------|
| Balrog | 98 | EXISTS |

### `C` — Canines (MON_IDX_DOG)

Progression: Stray Dog → Jackal → Wild Dog → Lyorn → Wolf → Dire Wolf → Warg → Shadow Mastiff.
Brust: Lyorn (dog).

| Monster | Level | Status |
|---------|-------|--------|
| Stray Dog | 1 | EXISTS |
| Jackal | 2 | EXISTS |
| Wild Dog | 4 | EXISTS |
| Lyorn | 6 | EXISTS (Brust name) |
| Wolf | 8 | EXISTS |
| Dire Wolf | 12 | EXISTS |
| Warg | 15 | EXISTS |
| Shadow Mastiff | 20 | EXISTS |

### `D` — Ancient Dragons / Dinosaurs

*(See combined table above under `D`)*

### `E` — Elementals (MON_IDX_ELEMENTAL)

| Monster | Level | Status |
|---------|-------|--------|
| Fire Elemental | 25 | EXISTS |
| Ice Elemental | 28 | EXISTS |
| Lightning Elemental | 30 | EXISTS |
| Earth Elemental | 32 | EXISTS |
| Air Elemental | 35 | EXISTS |
| Elemental Lord | 50 | EXISTS |

### `F` — Birds (MON_IDX_BIRD)

Brust: Athyra (owl-like bird).

| Monster | Level | Status |
|---------|-------|--------|
| Raven | 5 | EXISTS |
| Hawk | 8 | EXISTS |
| Athyra | 20 | EXISTS (Brust: confuse attack, fast 2.0 speed) |
| Giant Eagle | 25 | EXISTS |

### `G` — Ghouls / Ghosts (MON_IDX_GHOUL, MON_IDX_GHOST)

| Monster | Level | Status |
|---------|-------|--------|
| Ghoul | 8 | EXISTS (GHOUL) |
| Ghast | 12 | EXISTS (GHOUL) |
| Ghost | 15 | EXISTS (GHOST) |
| Phantom | 20 | EXISTS (GHOST) |

### `H` — Harpies (MON_IDX_HARPY)

| Monster | Level | Status |
|---------|-------|--------|
| White Harpy | 20 | EXISTS |
| Green Harpy | 25 | EXISTS |
| Black Harpy | 30 | EXISTS |

### `I` — Insects (MON_IDX_INSECT)

| Monster | Level | Status |
|---------|-------|--------|
| Giant Bee | 5 | EXISTS |
| Giant Scorpion | 10 | EXISTS |
| Giant Wasp | 12 | EXISTS |

### `J` — Jellies (MON_IDX_JELLY) — stationary only

Stationary jellies stay here. Mobile oozes/slimes on `j` (MON_IDX_OOZE).

| Monster | Level | Status |
|---------|-------|--------|
| Red Jelly | 1 | EXISTS |
| Green Jelly | 2 | EXISTS |
| White Jelly | 3 | EXISTS |
| Yellow Jelly | 4 | EXISTS |
| Blue Jelly | 5 | EXISTS |
| Ochre Jelly | 8 | EXISTS |
| Black Jelly | 10 | EXISTS |
| Grape Jelly | 12 | EXISTS |
| Gelatinous Cube | 15 | EXISTS |

### `K` — Beetles (MON_IDX_BEETLE)

| Monster | Level | Status |
|---------|-------|--------|
| Giant Beetle | 5 | EXISTS |
| Death Watch Beetle | 12 | EXISTS |

### `L` — Liches (MON_IDX_LICH)

| Monster | Level | Status |
|---------|-------|--------|
| Lich | 30 | EXISTS |
| Greater Lich | 40 | EXISTS |
| Master Lich | 50 | EXISTS |
| Emperor Lich | 65 | EXISTS |

### `M` — Mammals / Felines (MON_IDX_MAMMAL, MON_IDX_CAT)

Brust: Tsalmoth (bear-like), Dzur/Young Dzur/Elder Dzur (great cat).

**Mammals** (MON_IDX_MAMMAL):

| Monster | Level | Status |
|---------|-------|--------|
| Wild Boar | 3 | EXISTS |
| Black Bear | 5 | EXISTS |
| Wolverine | 8 | EXISTS |
| Tsalmoth | 12 | EXISTS (Brust name) |
| Cave Bear | 15 | EXISTS |
| Lion | 18 | EXISTS |
| Hippo | 20 | EXISTS |

**Felines** (MON_IDX_CAT — shares `M` tile):

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Wild Cat | 3 | EXISTS | |
| Young Dzur | 8 | EXISTS | Brust: young great cat |
| Tiger | 15 | EXISTS | |
| Dzur | 35 | EXISTS | Brust: triple attack claw/claw/bite |
| Elder Dzur | 55 | EXISTS | Brust |

### `O` — Ogres (MON_IDX_OGRE)

| Monster | Level | Status |
|---------|-------|--------|
| Ogre | 10 | EXISTS |
| Ogre Mage | 18 | EXISTS |
| Ogre Chieftain | 22 | EXISTS |

### `P` — Giants (MON_IDX_GIANT) and Dragaeran Persons (MON_IDX_L_PERSON)

**Giants** (MON_IDX_GIANT):

| Monster | Level | Status |
|---------|-------|--------|
| Hill Giant | 18 | EXISTS |
| Stone Giant | 22 | EXISTS |
| Frost Giant | 28 | EXISTS |
| Fire Giant | 32 | EXISTS |
| Ettin | 35 | EXISTS |
| Cloud Giant | 38 | EXISTS |
| Storm Giant | 42 | EXISTS |
| Titan | 50 | EXISTS |

**Dragaeran Persons** (MON_IDX_L_PERSON — shares `P` tile):

Level spread covers 4–75, filling gaps alongside Giants (18–50).
Dragaeran persons are named after Brust's Great Houses. See [#267](https://github.com/Rushwind13/JMoria/issues/267) for duplicate-name issue with animal counterparts.

| Monster | Level | Status | Notes |
|---------|-------|--------|-------|
| Teckla | 4 | EXISTS | Commoner. 100RANDOM AI. Duplicate name with `r` Teckla rat |
| Lyorn | 15 | EXISTS | Warrior-scholar. Duplicate name with `C` Lyorn dog |
| Tiassa | 18 | EXISTS | Clever, quick. Confuse attack. Duplicate name with `f` Tiassa fly |
| Hawklord | 22 | EXISTS | Scholar with minor magic. Confuse attack |
| Yendi | 25 | EXISTS | Sneaky, venomous. Poison attack |
| Dzurlord | 35 | EXISTS | Fierce warrior. Triple attack mirrors Dzur animal |
| Cat-centaur | 40 | EXISTS | Triple attack claw/claw/bite |
| Dragonlord | 40 | EXISTS | Warrior-noble with fire sorcery |
| Phoenix Guard | 50 | EXISTS | Elite soldier |
| Guard Captain | 60 | EXISTS | Officer. Fire secondary attack |
| Lavode | 75 | EXISTS | Magic blade + staff. Cold damage (Iceflame) |

### `R` — Reptiles (MON_IDX_REPTILE)

| Monster | Level | Status |
|---------|-------|--------|
| Newt | 1 | EXISTS |
| Lizardman | 5 | EXISTS |
| Salamander | 8 | EXISTS |
| Fire Salamander | 12 | EXISTS |
| Crocodile | 15 | EXISTS |
| Basilisk | 20 | EXISTS |
| Emperor Crocodile | 25 | EXISTS |

### `S` — Yendi (MON_IDX_SNAKE)

Brust: Snakes renamed to Yendi.

| Monster | Level | Status |
|---------|-------|---------|
| Giant Yendi | 2 | EXISTS |
| Poison Yendi | 15 | EXISTS |
| King Yendi | 37 | EXISTS |

### `T` — Trolls (MON_IDX_TROLL)

| Monster | Level | Status |
|---------|-------|--------|
| Troll | 10 | EXISTS |
| Hill Troll | 15 | EXISTS |
| Ice Troll | 18 | EXISTS |
| Fire Troll | 20 | EXISTS |
| Stone Troll | 22 | EXISTS |
| Troll Berserker | 25 | EXISTS |
| Troll Shaman | 28 | EXISTS |
| War Troll | 30 | EXISTS |
| Olog-hai | 35 | EXISTS |

### `U` — Major Demons (MON_IDX_MAJOR_DEMON)

| Monster | Level | Status |
|---------|-------|--------|
| Greater Demon | 30 | EXISTS |
| Demon Lord | 45 | EXISTS |
| Greater Demon Lord | 60 | EXISTS |

### `V` — Vampires (MON_IDX_VAMPIRE)

| Monster | Level | Status |
|---------|-------|--------|
| Vampire | 25 | EXISTS |
| Greater Vampire | 32 | EXISTS |
| Master Vampire | 40 | EXISTS |
| Lordly Vampire | 50 | EXISTS |
| Emperor Vampire | 60 | EXISTS |

### `W` — Wights / Wraiths (MON_IDX_WIGHT, MON_IDX_WRAITH)

| Monster | Level | Status |
|---------|-------|--------|
| Wight | 10 | EXISTS (WIGHT) |
| Wraith | 18 | EXISTS (WRAITH) |
| Spectre | 25 | EXISTS (WRAITH) |
| Emperor Wraith | 40 | EXISTS (WRAITH) |

### `X` — Xorns (MON_IDX_XORN)

| Monster | Level | Status |
|---------|-------|--------|
| Xorn | 20 | EXISTS |

### `Y` — Yetis (MON_IDX_YETI)

| Monster | Level | Status |
|---------|-------|--------|
| Yeti | 12 | EXISTS |

### `,` — Mushroom Patches (MON_IDX_SHROOM)

**CAMOUFLAGE**: Mushroom patches display as `,` — same tile as food items on the ground. A starving player sees "food", approaches, and gets ambushed.

| Monster | Level | Status |
|---------|-------|--------|
| Yellow Mushroom Patch | 1 | EXISTS |
| Red Mushroom Patch | 2 | EXISTS |
| White Mushroom Patch | 3 | EXISTS |
| Blue Mushroom Patch | 4 | EXISTS |
| Clear Mushroom Patch | 5 | EXISTS |
| Black Mushroom Patch | 8 | EXISTS |

### `&` — Mimics (MON_IDX_MIMIC)

| Monster | Level | Status |
|---------|-------|--------|
| Mimic | 15 | EXISTS |
| Chest Mimic | 22 | EXISTS |

### `.` — Lurkers (MON_IDX_LURKER)

| Monster | Level | Status |
|---------|-------|--------|
| Lurker | 10 | EXISTS |
| Trapper | 18 | EXISTS |

### `$` — Creeping Coins (MON_IDX_COIN)

| Monster | Level | Status |
|---------|-------|--------|
| Creeping Copper Coins | 5 | EXISTS |
| Creeping Silver Coins | 10 | EXISTS |
| Creeping Gold Coins | 18 | EXISTS |

### `t` — Townsfolk (NPC_IDX_TOURIST)

| Monster | Level | Status |
|---------|-------|--------|
| Tourist | 1 | EXISTS (NPC) |

### `|` — Animated Weapons (MON_IDX_ANIMATED_WEAPON)

**CAMOUFLAGE**: Uses `|` tile — same as dropped weapons on the ground. Must use same color as the real weapon to complete the disguise.

| Monster | Level | Status |
|---------|-------|--------|
| Animated Sword | 15 | EXISTS |
| Animated Spear | 12 | EXISTS |
| Animated Axe | 18 | EXISTS |
| Animated Club | 8 | EXISTS |
