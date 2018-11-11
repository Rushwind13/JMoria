// Constants.h
// Global constants for JMoria
//
// Created 4/11/3
//
// Note: wanted to use enums, but couldn't figure out
// how to make them work with CL /EP for monsters.dat
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__
#include <stdio.h>

#define VERSION "0.20"

// all the states which the game can run in
// this modifies the event handling
#define STATE_INVALID    -1
#define STATE_COMMAND     0
#define STATE_MODIFY      1
#define STATE_MENU        2
#define STATE_USE         3
#define STATE_STRINGINPUT 4
#define STATE_ENDGAME     5
#define STATE_CLOCKSTEP   6
#define STATE_MAX         7

// Various statuses that someone could have
#define STATUS_INVALID		-1
#define STATUS_DEAD			 0
#define STATUS_ALIVE		 1
#define STATUS_GORGED		 2
#define STATUS_FULL			 3
#define STATUS_HUNGRY		 4
#define STATUS_STARVING		 5
#define STATUS_CONFUSED		 6
#define STATUS_STUNNED		 7
#define STATUS_AFRAID		 8
#define STATUS_SLEEPING		 9
#define STATUS_MAX			10

// Types of Dungeon Tiles
#define DUNG_IDX_INVALID	    -1
#define DUNG_IDX_FLOOR		     0
#define DUNG_IDX_WALL		     1
#define DUNG_IDX_DOOR		     2
#define DUNG_IDX_OPEN_DOOR	     3
#define DUNG_IDX_UPSTAIRS	     4
#define DUNG_IDX_LONG_UPSTAIRS   5
#define DUNG_IDX_DOWNSTAIRS      6
#define DUNG_IDX_LONG_DOWNSTAIRS 7
#define DUNG_IDX_RUBBLE		     8
//JUNK,
#define DUNG_IDX_PLAYER		     9
#define DUNG_IDX_MAX		    10

// Dungeon Tile flags
#define DUNG_FLAG_LIT		0x00000001

// Dungeon Flags
#define DUNG_CFG_MONSTERS_PER_LEVEL 0.005f
#define DUNG_CFG_ITEMS_PER_LEVEL 0.01f
#define DUNG_CFG_START_LEVEL 1
#define DUNG_CFG_MAX_SPAWN_TRIES 10

// Types of Monsters
#define MON_IDX_INVALID        -1
#define MON_IDX_ANT             0
#define MON_IDX_BAT             1
#define MON_IDX_CENTIPEDE       2
#define MON_IDX_DRAGON          3
#define MON_IDX_DINOSAUR        4
#define MON_IDX_EYE             5
#define MON_IDX_CAT             6
#define MON_IDX_GOLEM           7
#define MON_IDX_HUMANOID        8
#define MON_IDX_HARPY           9
#define MON_IDX_ICKY           10
#define MON_IDX_KOBOLD         11
#define MON_IDX_LOUSE          12
#define MON_IDX_LEECH          13
#define MON_IDX_MOLD           14
#define MON_IDX_NAGA           15
#define MON_IDX_ORC            16
#define MON_IDX_PERSON         17
#define MON_IDX_RAT            18
#define MON_IDX_SKELETON       19
#define MON_IDX_MINOR_DEMON    20
#define MON_IDX_WORM           21
#define MON_IDX_SPIDER         22
#define MON_IDX_YEEK           23
#define MON_IDX_ZOMBIE         24
#define MON_IDX_FROG           25
#define MON_IDX_BALROG         26
#define MON_IDX_DOG            27
#define MON_IDX_ANCIENT_DRAGON 28
#define MON_IDX_FLY            29
#define MON_IDX_DRAGON_FLY     30
#define MON_IDX_FAERIE_DRAGON  31
#define MON_IDX_GHOUL          32
#define MON_IDX_GHOST          33
#define MON_IDX_GIANT          34
#define MON_IDX_INSECT         35
#define MON_IDX_JELLY          36
#define MON_IDX_BEETLE         37
#define MON_IDX_LICH           38
#define MON_IDX_OGRE           39
#define MON_IDX_L_PERSON       40
#define MON_IDX_REPTILE        41
#define MON_IDX_SNAKE          42
#define MON_IDX_TROLL          43
#define MON_IDX_MAJOR_DEMON    44
#define MON_IDX_VAMPIRE        45
#define MON_IDX_WIGHT          46
#define MON_IDX_WRAITH         47
#define MON_IDX_XORN           48
#define MON_IDX_YETI           49
#define MON_IDX_MIMIC          50
#define MON_IDX_LURKER         51
#define MON_IDX_SHROOM         52
#define MON_IDX_COIN           53
#define MON_IDX_TOWNSFOLK      54
#define MON_IDX_MAX            55

// Monster flags
#define MON_FLAG_SPORE         0x00000001
#define MON_FLAG_TOUCH         0x00000002
#define MON_FLAG_BITE          0x00000004
#define MON_FLAG_DROOL         0x00000008

#define MON_FLAG_CLAW          0x00000010
#define MON_FLAG_TRAMPLE       0x00000020
#define MON_FLAG_BREATHE       0x00000040
#define MON_FLAG_CRAWL         0x00000080

#define MON_FLAG_BREED         0x00008000
// #define MON_FLAG_x          0x00100000
// #define MON_FLAG_x          0x00200000
// #define MON_FLAG_x          0x00400000
// #define MON_FLAG_x          0x00800000

#define MON_AI_DONTMOVE        0x01000000
#define MON_AI_100RANDOMMOVE   0x02000000
#define MON_AI_75RANDOMMOVE    0x04000000
#define MON_AI_SEEKPLAYER      0x08000000

#define MON_COLOR_MULTI        0x10000000
//#define MON_COLOR_x          0x20000000
//#define MON_COLOR_x          0x40000000
//#define MON_COLOR_x          0x80000000

#define NUM_MON_FLAGS           14

// Effect Flags
#define EFFECT_FLAG_FIRE          0x00000001
#define EFFECT_FLAG_COLD          0x00000002
#define EFFECT_FLAG_ELECTRICITY   0x00000004
#define EFFECT_FLAG_ACID          0x00000008

#define EFFECT_FLAG_POISON        0x00000010
#define EFFECT_FLAG_SPELL         0x00000020
#define EFFECT_FLAG_HP            0x00000040
#define EFFECT_FLAG_XP            0x00000080

#define EFFECT_FLAG_AFRAID        0x00000100
#define EFFECT_FLAG_BLIND         0x00000200
#define EFFECT_FLAG_SLEEP         0x00000400
#define EFFECT_FLAG_CONFUSE       0x00000800

#define EFFECT_FLAG_MP            0x00001000
#define EFFECT_FLAG_IDENTIFY      0x00002000
#define EFFECT_FLAG_LIGHT         0x00004000
#define EFFECT_FLAG_ESP           0x00008000

#define EFFECT_FLAG_LEVITATE      0x00010000
#define EFFECT_FLAG_INVISIBLE     0x00020000
#define EFFECT_FLAG_PARALYZE      0x00040000
#define EFFECT_FLAG_FUEL          0x00080000

#define EFFECT_FLAG_SUMMON        0x00100000
#define EFFECT_FLAG_TOHIT         0x00200000
#define EFFECT_FLAG_TODAM         0x00400000
#define EFFECT_FLAG_AC            0x00800000

#define EFFECT_FLAG_STAT          0x01000000
#define EFFECT_FLAG_DOOR          0x02000000
#define EFFECT_FLAG_TREASURE      0x04000000
#define EFFECT_FLAG_HOLDING       0x08000000

#define EFFECT_FLAG_MAPPING       0x10000000
#define EFFECT_FLAG_RECALL        0x20000000
#define EFFECT_FLAG_STONE_TO_MUD  0x40000000
#define EFFECT_FLAG_SPEED         0x80000000


#define NUM_EFFECT_FLAGS          32

// Effect Modifiers
#define EFFECT_MOD_RESIST        0x000000001
#define EFFECT_MOD_SEE           0x000000002
#define EFFECT_MOD_IMMUNE        0x000000004
#define EFFECT_MOD_WEAK          0x000000008

#define EFFECT_MOD_ENCHANT       0x000000010

#define NUM_EFFECT_MODIFIERS     5

// Effect Types
#define EFFECT_TYPE_HEAL          0x00000001
#define EFFECT_TYPE_HIT           0x00000002
#define EFFECT_TYPE_TIMED         0x00000004
#define EFFECT_TYPE_CAUSE         0x00000008

#define EFFECT_TYPE_INTRINSIC     0x00000010
#define EFFECT_TYPE_RESTORE       0x00000020
#define EFFECT_TYPE_GAIN          0x00000040
#define EFFECT_TYPE_LOSE          0x00000080

#define NUM_EFFECT_TYPES           8

// Types of items
#define ITEM_IDX_INVALID        -1
#define ITEM_IDX_SWORD          0
#define ITEM_IDX_SHIELD         1
#define ITEM_IDX_ARMOR          2
#define ITEM_IDX_HELMET         3
#define ITEM_IDX_CLOAK          4
#define ITEM_IDX_GLOVES         5
#define ITEM_IDX_BOOTS          6
#define ITEM_IDX_AMULET         7
#define ITEM_IDX_RING           8
#define ITEM_IDX_TORCH          9
#define ITEM_IDX_BOW            10
#define ITEM_IDX_ARROW          11
#define ITEM_IDX_XBOW           12
#define ITEM_IDX_BOLT           13
#define ITEM_IDX_CHEST          14
#define ITEM_IDX_SCROLL         15
#define ITEM_IDX_POTION         16
#define ITEM_IDX_WAND           17
#define ITEM_IDX_STAFF          18
#define ITEM_IDX_BOOK           19
#define ITEM_IDX_MONEY          20
#define ITEM_IDX_FOOD           21
#define ITEM_IDX_AXE            22
#define ITEM_IDX_POLEARM        23
#define ITEM_IDX_SHOVEL         24
#define ITEM_IDX_MAX            25

// TODO: Might need to switch from "ITEM/MONSTER"
//       to "types of harm" / "types of aid"
//       like: <hurt>, <blind>, XdY means "blind for x turns"
//             <heal>, <blind> means "cure blindness"
// Item flags

#define ITEM_FLAG_CURSED        0x00000001

#define ITEM_COLOR_MULTI        0x10000000

#define NUM_ITEM_FLAGS  2

// Make sure you change below here if you added any flags.
#define NUM_STRINGS				MON_IDX_MAX + NUM_MON_FLAGS + ITEM_IDX_MAX + NUM_ITEM_FLAGS + NUM_EFFECT_FLAGS + NUM_EFFECT_MODIFIERS + NUM_EFFECT_TYPES
#include "TextEntry.h"
class Constants
{
public:
    Constants():m_StringTable(NULL){};
    ~Constants(){if(m_StringTable!= NULL) delete [] m_StringTable; m_StringTable = NULL;};

    void Init()
    {
        printf("expecting %d strings\n", NUM_STRINGS);
        m_StringTable = new TextEntry[NUM_STRINGS];
        int i=0;
        // Monster flags
        // types
        m_StringTable[i++].Init("MON_IDX_SHROOM",		        MON_IDX_SHROOM);
        m_StringTable[i++].Init("MON_IDX_JELLY",                MON_IDX_JELLY);
        m_StringTable[i++].Init("MON_IDX_ICKY",                 MON_IDX_ICKY);
        m_StringTable[i++].Init("MON_IDX_DRAGON",               MON_IDX_DRAGON);
        m_StringTable[i++].Init("MON_IDX_ANCIENT_DRAGON",       MON_IDX_ANCIENT_DRAGON);
        m_StringTable[i++].Init("MON_IDX_SNAKE",                MON_IDX_SNAKE);
        m_StringTable[i++].Init("MON_IDX_KOBOLD",               MON_IDX_KOBOLD);
        m_StringTable[i++].Init("MON_IDX_SPIDER",               MON_IDX_SPIDER);
        m_StringTable[i++].Init("MON_IDX_WORM",                 MON_IDX_WORM);
        m_StringTable[i++].Init("MON_IDX_DINOSAUR",             MON_IDX_DINOSAUR);
        m_StringTable[i++].Init("MON_IDX_ANT",                  MON_IDX_ANT);
        m_StringTable[i++].Init("MON_IDX_HARPY",                MON_IDX_HARPY);
        m_StringTable[i++].Init("MON_IDX_DOG",                  MON_IDX_DOG);
        m_StringTable[i++].Init("MON_IDX_BAT",                  MON_IDX_BAT);
        m_StringTable[i++].Init("MON_IDX_FROG",                 MON_IDX_FROG);
        m_StringTable[i++].Init("MON_IDX_SKELETON",             MON_IDX_SKELETON);
        m_StringTable[i++].Init("MON_IDX_GHOUL",                MON_IDX_GHOUL);
        m_StringTable[i++].Init("MON_IDX_WIGHT",                MON_IDX_WIGHT);
        m_StringTable[i++].Init("MON_IDX_VAMPIRE",              MON_IDX_VAMPIRE);
        m_StringTable[i++].Init("MON_IDX_LICH",                 MON_IDX_LICH);
        m_StringTable[i++].Init("MON_IDX_FAERIE_DRAGON",        MON_IDX_FAERIE_DRAGON);
        m_StringTable[i++].Init("MON_IDX_EYE",                  MON_IDX_EYE);
        m_StringTable[i++].Init("MON_IDX_CENTIPEDE",            MON_IDX_CENTIPEDE);
        m_StringTable[i++].Init("MON_IDX_CAT",                  MON_IDX_CAT);
        m_StringTable[i++].Init("MON_IDX_GOLEM",                MON_IDX_GOLEM);
        m_StringTable[i++].Init("MON_IDX_HUMANOID",             MON_IDX_HUMANOID);
        m_StringTable[i++].Init("MON_IDX_GIANT",                MON_IDX_GIANT);
        m_StringTable[i++].Init("MON_IDX_LOUSE",                MON_IDX_LOUSE);
        m_StringTable[i++].Init("MON_IDX_LEECH",                MON_IDX_LEECH);
        m_StringTable[i++].Init("MON_IDX_MOLD",                 MON_IDX_MOLD);
        m_StringTable[i++].Init("MON_IDX_NAGA",                 MON_IDX_NAGA);
        m_StringTable[i++].Init("MON_IDX_ORC",                  MON_IDX_ORC);
        m_StringTable[i++].Init("MON_IDX_PERSON",               MON_IDX_PERSON);
        m_StringTable[i++].Init("MON_IDX_L_PERSON",             MON_IDX_L_PERSON);
        m_StringTable[i++].Init("MON_IDX_RAT",                  MON_IDX_RAT);
        m_StringTable[i++].Init("MON_IDX_MINOR_DEMON",          MON_IDX_MINOR_DEMON);
        m_StringTable[i++].Init("MON_IDX_MAJOR_DEMON",          MON_IDX_MAJOR_DEMON);
        m_StringTable[i++].Init("MON_IDX_YEEK",                 MON_IDX_YEEK);
        m_StringTable[i++].Init("MON_IDX_ZOMBIE",               MON_IDX_ZOMBIE);
        m_StringTable[i++].Init("MON_IDX_BALROG",               MON_IDX_BALROG);
        m_StringTable[i++].Init("MON_IDX_FLY",                  MON_IDX_FLY);
        m_StringTable[i++].Init("MON_IDX_DRAGON_FLY",           MON_IDX_DRAGON_FLY);
        m_StringTable[i++].Init("MON_IDX_GHOST",                MON_IDX_GHOST);
        m_StringTable[i++].Init("MON_IDX_INSECT",               MON_IDX_INSECT);
        m_StringTable[i++].Init("MON_IDX_BEETLE",               MON_IDX_BEETLE);
        m_StringTable[i++].Init("MON_IDX_OGRE",                 MON_IDX_OGRE);
        m_StringTable[i++].Init("MON_IDX_REPTILE",              MON_IDX_REPTILE);
        m_StringTable[i++].Init("MON_IDX_TROLL",                MON_IDX_TROLL);
        m_StringTable[i++].Init("MON_IDX_WRAITH",               MON_IDX_WRAITH);
        m_StringTable[i++].Init("MON_IDX_XORN",                 MON_IDX_XORN);
        m_StringTable[i++].Init("MON_IDX_YETI",                 MON_IDX_YETI);
        m_StringTable[i++].Init("MON_IDX_MIMIC",                MON_IDX_MIMIC);
        m_StringTable[i++].Init("MON_IDX_LURKER",               MON_IDX_LURKER);
        m_StringTable[i++].Init("MON_IDX_COIN",                 MON_IDX_COIN);
        m_StringTable[i++].Init("MON_IDX_TOWNSFOLK",            MON_IDX_TOWNSFOLK);

        // Monster flags (attack types, ai types, color types)
        m_StringTable[i++].Init("MON_FLAG_SPORE", MON_FLAG_SPORE);
        m_StringTable[i++].Init("MON_FLAG_TOUCH", MON_FLAG_TOUCH);
        m_StringTable[i++].Init("MON_FLAG_BITE", MON_FLAG_BITE);
        m_StringTable[i++].Init("MON_FLAG_DROOL", MON_FLAG_DROOL);
        m_StringTable[i++].Init("MON_FLAG_CLAW", MON_FLAG_CLAW);
        m_StringTable[i++].Init("MON_FLAG_TRAMPLE", MON_FLAG_TRAMPLE);
        m_StringTable[i++].Init("MON_FLAG_BREATHE", MON_FLAG_BREATHE);
        m_StringTable[i++].Init("MON_FLAG_CRAWL", MON_FLAG_CRAWL);
        m_StringTable[i++].Init("MON_FLAG_BREED", MON_FLAG_BREED);
        m_StringTable[i++].Init("MON_AI_DONTMOVE", MON_AI_DONTMOVE);
        m_StringTable[i++].Init("MON_AI_100RANDOMMOVE", MON_AI_100RANDOMMOVE);
        m_StringTable[i++].Init("MON_AI_75RANDOMMOVE", MON_AI_75RANDOMMOVE);
        m_StringTable[i++].Init("MON_AI_SEEKPLAYER", MON_AI_SEEKPLAYER);
        m_StringTable[i++].Init("MON_COLOR_MULTI", MON_COLOR_MULTI);

        // Effect flags (flags, modifiers, types)
        m_StringTable[i++].Init("EFFECT_FLAG_FIRE", EFFECT_FLAG_FIRE);
        m_StringTable[i++].Init("EFFECT_FLAG_COLD", EFFECT_FLAG_COLD);
        m_StringTable[i++].Init("EFFECT_FLAG_ELECTRICITY", EFFECT_FLAG_ELECTRICITY);
        m_StringTable[i++].Init("EFFECT_FLAG_ACID", EFFECT_FLAG_ACID);
        m_StringTable[i++].Init("EFFECT_FLAG_POISON", EFFECT_FLAG_POISON);
        m_StringTable[i++].Init("EFFECT_FLAG_SPELL", EFFECT_FLAG_SPELL);
        m_StringTable[i++].Init("EFFECT_FLAG_HP", EFFECT_FLAG_HP);
        m_StringTable[i++].Init("EFFECT_FLAG_XP", EFFECT_FLAG_XP);
        m_StringTable[i++].Init("EFFECT_FLAG_AFRAID", EFFECT_FLAG_AFRAID);
        m_StringTable[i++].Init("EFFECT_FLAG_BLIND", EFFECT_FLAG_BLIND);
        m_StringTable[i++].Init("EFFECT_FLAG_SLEEP", EFFECT_FLAG_SLEEP );
        m_StringTable[i++].Init("EFFECT_FLAG_CONFUSE", EFFECT_FLAG_CONFUSE );
        m_StringTable[i++].Init("EFFECT_FLAG_MP", EFFECT_FLAG_MP);
        m_StringTable[i++].Init("EFFECT_FLAG_IDENTIFY", EFFECT_FLAG_IDENTIFY);
        m_StringTable[i++].Init("EFFECT_FLAG_LIGHT", EFFECT_FLAG_LIGHT );
        m_StringTable[i++].Init("EFFECT_FLAG_ESP", EFFECT_FLAG_ESP );
        m_StringTable[i++].Init("EFFECT_FLAG_LEVITATE", EFFECT_FLAG_LEVITATE);
        m_StringTable[i++].Init("EFFECT_FLAG_INVISIBLE", EFFECT_FLAG_INVISIBLE);
        m_StringTable[i++].Init("EFFECT_FLAG_PARALYZE", EFFECT_FLAG_PARALYZE );
        m_StringTable[i++].Init("EFFECT_FLAG_FUEL",          EFFECT_FLAG_FUEL);
        m_StringTable[i++].Init("EFFECT_FLAG_SUMMON",        EFFECT_FLAG_SUMMON);
        m_StringTable[i++].Init("EFFECT_FLAG_TOHIT", EFFECT_FLAG_TOHIT);
        m_StringTable[i++].Init("EFFECT_FLAG_TODAM", EFFECT_FLAG_TODAM);
        m_StringTable[i++].Init("EFFECT_FLAG_AC",    EFFECT_FLAG_AC);
        m_StringTable[i++].Init("EFFECT_FLAG_STAT",  EFFECT_FLAG_STAT);
        m_StringTable[i++].Init("EFFECT_FLAG_DOOR",     EFFECT_FLAG_DOOR);
        m_StringTable[i++].Init("EFFECT_FLAG_TREASURE", EFFECT_FLAG_TREASURE);
        m_StringTable[i++].Init("EFFECT_FLAG_HOLDING",          EFFECT_FLAG_HOLDING);
        m_StringTable[i++].Init("EFFECT_FLAG_MAPPING",       EFFECT_FLAG_MAPPING);
        m_StringTable[i++].Init("EFFECT_FLAG_RECALL",        EFFECT_FLAG_RECALL);
        m_StringTable[i++].Init("EFFECT_FLAG_STONE_TO_MUD",  EFFECT_FLAG_STONE_TO_MUD);
        m_StringTable[i++].Init("EFFECT_FLAG_SPEED",         EFFECT_FLAG_SPEED);

        m_StringTable[i++].Init("EFFECT_MOD_RESIST", EFFECT_MOD_RESIST );
        m_StringTable[i++].Init("EFFECT_MOD_SEE", EFFECT_MOD_SEE);
        m_StringTable[i++].Init("EFFECT_MOD_IMMUNE", EFFECT_MOD_IMMUNE);
        m_StringTable[i++].Init("EFFECT_MOD_WEAK", EFFECT_MOD_WEAK );
        m_StringTable[i++].Init("EFFECT_MOD_ENCHANT", EFFECT_MOD_ENCHANT );

        m_StringTable[i++].Init("EFFECT_TYPE_HEAL", EFFECT_TYPE_HEAL );
        m_StringTable[i++].Init("EFFECT_TYPE_HIT", EFFECT_TYPE_HIT);
        m_StringTable[i++].Init("EFFECT_TYPE_TIMED", EFFECT_TYPE_TIMED);
        m_StringTable[i++].Init("EFFECT_TYPE_CAUSE", EFFECT_TYPE_CAUSE );
        m_StringTable[i++].Init("EFFECT_TYPE_INTRINSIC", EFFECT_TYPE_INTRINSIC );
        m_StringTable[i++].Init("EFFECT_TYPE_RESTORE", EFFECT_TYPE_RESTORE );
        m_StringTable[i++].Init("EFFECT_TYPE_GAIN", EFFECT_TYPE_GAIN );
        m_StringTable[i++].Init("EFFECT_TYPE_LOSE", EFFECT_TYPE_LOSE );

        // Item Types
        m_StringTable[i++].Init("ITEM_IDX_SWORD",        ITEM_IDX_SWORD);
        m_StringTable[i++].Init("ITEM_IDX_SHIELD",       ITEM_IDX_SHIELD);
        m_StringTable[i++].Init("ITEM_IDX_ARMOR",        ITEM_IDX_ARMOR);
        m_StringTable[i++].Init("ITEM_IDX_HELMET",       ITEM_IDX_HELMET);
        m_StringTable[i++].Init("ITEM_IDX_CLOAK",        ITEM_IDX_CLOAK);
        m_StringTable[i++].Init("ITEM_IDX_GLOVES",       ITEM_IDX_GLOVES);
        m_StringTable[i++].Init("ITEM_IDX_BOOTS",        ITEM_IDX_BOOTS);
        m_StringTable[i++].Init("ITEM_IDX_AMULET",       ITEM_IDX_AMULET);
        m_StringTable[i++].Init("ITEM_IDX_RING",         ITEM_IDX_RING);
        m_StringTable[i++].Init("ITEM_IDX_TORCH",        ITEM_IDX_TORCH);
        m_StringTable[i++].Init("ITEM_IDX_BOW",          ITEM_IDX_BOW);
        m_StringTable[i++].Init("ITEM_IDX_ARROW",        ITEM_IDX_ARROW);
        m_StringTable[i++].Init("ITEM_IDX_XBOW",         ITEM_IDX_XBOW);
        m_StringTable[i++].Init("ITEM_IDX_BOLT",         ITEM_IDX_BOLT);
        m_StringTable[i++].Init("ITEM_IDX_CHEST",        ITEM_IDX_CHEST);
        m_StringTable[i++].Init("ITEM_IDX_SCROLL",       ITEM_IDX_SCROLL);
        m_StringTable[i++].Init("ITEM_IDX_POTION",       ITEM_IDX_POTION);
        m_StringTable[i++].Init("ITEM_IDX_WAND",         ITEM_IDX_WAND);
        m_StringTable[i++].Init("ITEM_IDX_STAFF",        ITEM_IDX_STAFF);
        m_StringTable[i++].Init("ITEM_IDX_BOOK",         ITEM_IDX_BOOK);
        m_StringTable[i++].Init("ITEM_IDX_MONEY",        ITEM_IDX_MONEY);
        m_StringTable[i++].Init("ITEM_IDX_FOOD",         ITEM_IDX_FOOD);
        m_StringTable[i++].Init("ITEM_IDX_AXE",          ITEM_IDX_AXE);
        m_StringTable[i++].Init("ITEM_IDX_SHOVEL",       ITEM_IDX_SHOVEL);
        m_StringTable[i++].Init("ITEM_IDX_POLEARM",      ITEM_IDX_POLEARM);

        // Item flags
        m_StringTable[i++].Init("ITEM_FLAG_CURSED",        ITEM_FLAG_CURSED);
        m_StringTable[i++].Init("ITEM_COLOR_MULTI",        ITEM_COLOR_MULTI);

        if( i != NUM_STRINGS ) printf("misconfiguration error, expected %d strings, got %d\n", NUM_STRINGS, i);
    };
    TextEntry	*m_StringTable;

    int LookupString(char *szIn)
    {
        int i;
        for( i=0; i < NUM_STRINGS; i++ )
        {
            char *curr = m_StringTable[i].m_szString;
            if( strcmp( curr, szIn ) == 0 )
            {
                return m_StringTable[i].m_dwValue;
            }
        }

        printf("bad string: %s\n", szIn);

        return -1;
    }
};
#endif // __CONSTANTS_H__
