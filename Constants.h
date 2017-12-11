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

#define VERSION "0.06"

// all the states which the game can run in
// this modifies the event handling
#define STATE_INVALID		-1
#define STATE_COMMAND		 0
#define STATE_MODIFY		 1
#define STATE_MENU           2
#define STATE_MAX            3

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
#define DUNG_CFG_MONSTERS_PER_LEVEL 0.05f
#define DUNG_CFG_ITEMS_PER_LEVEL 0.05f
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

#define MON_FLAG_FIRE          0x00000100
#define MON_FLAG_COLD          0x00000200
#define MON_FLAG_ELECTRICITY   0x00000400
#define MON_FLAG_ACID          0x00000800

#define MON_FLAG_POISON        0x00001000
#define MON_FLAG_SPELL         0x00002000
#define MON_FLAG_DRAIN_XP      0x00004000
#define MON_FLAG_BREED         0x00008000

#define MON_FLAG_AFRAID        0x00010000
#define MON_FLAG_BLIND         0x00020000
#define MON_FLAG_SLEEP         0x00040000
#define MON_FLAG_CONFUSE       0x00080000

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

#define NUM_MON_FLAGS           25

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
#define ITEM_FLAG_CURE_POISON   0x00000001
#define ITEM_FLAG_CURE_BLIND    0x00000002
#define ITEM_FLAG_CURE_CONFUSE  0x00000004
#define ITEM_FLAG_CURE_FEAR     0x00000008

#define ITEM_FLAG_HEAL_HP       0x00000010
#define ITEM_FLAG_HEAL_MP       0x00000020
#define ITEM_FLAG_IDENTIFY      0x00000040
#define ITEM_FLAG_LIGHT         0x00000080

#define ITEM_FLAG_SUMMON        0x00000100
#define ITEM_FLAG_ENCHANT_TOHIT 0x00000200
#define ITEM_FLAG_ENCHANT_TODAM 0x00000400
#define ITEM_FLAG_ENCHANT_AC    0x00000800

#define ITEM_FLAG_RESIST_FIRE   0x00001000
#define ITEM_FLAG_RESIST_COLD   0x00002000
#define ITEM_FLAG_RESIST_ELEC   0x00004000
#define ITEM_FLAG_RESIST_ACID   0x00008000

#define ITEM_FLAG_LEVITATE      0x00010000
#define ITEM_FLAG_RESTORE_STAT  0x00020000
#define ITEM_FLAG_GAIN_STAT     0x00040000
#define ITEM_FLAG_INVISIBLE     0x00080000

#define ITEM_FLAG_MAPPING       0x00100000
#define ITEM_FLAG_RECALL        0x00200000
#define ITEM_FLAG_STONE_TO_MUD  0x00400000
#define ITEM_FLAG_SPEED         0x00800000

#define ITEM_FLAG_ESP           0x01000000
#define ITEM_FLAG_CONFUSE       0x02000000
#define ITEM_FLAG_FIND_DOOR     0x04000000
#define ITEM_FLAG_FIND_TREASURE 0x08000000

#define ITEM_FLAG_CURSED        0x10000000
#define ITEM_FLAG_SPELL         0x20000000
#define ITEM_FLAG_HOLDING       0x40000000
#define ITEM_COLOR_MULTI        0x80000000

#define NUM_ITEM_FLAGS  32

// Make sure you change below here if you added any flags.
#define NUM_STRINGS				MON_IDX_MAX + NUM_MON_FLAGS + ITEM_IDX_MAX + NUM_ITEM_FLAGS
#include "TextEntry.h"
class Constants
{
public:
    Constants():m_StringTable(NULL){};
    ~Constants(){if(m_StringTable!= NULL) delete [] m_StringTable; m_StringTable = NULL;};

    void Init()
    {
        m_StringTable = new TextEntry[NUM_STRINGS];
        int i=0;
        // Monster flags
        // types
        m_StringTable[i++].Init("MON_IDX_SHROOM",		MON_IDX_SHROOM);
        m_StringTable[i++].Init("MON_IDX_JELLY",		MON_IDX_JELLY);
        m_StringTable[i++].Init("MON_IDX_ICKY",			MON_IDX_ICKY);
        m_StringTable[i++].Init("MON_IDX_DRAGON",       MON_IDX_DRAGON);
        m_StringTable[i++].Init("MON_IDX_ANCIENT_DRAGON",       MON_IDX_ANCIENT_DRAGON);
        m_StringTable[i++].Init("MON_IDX_SNAKE",        MON_IDX_SNAKE);
        m_StringTable[i++].Init("MON_IDX_KOBOLD",       MON_IDX_KOBOLD);
        m_StringTable[i++].Init("MON_IDX_SPIDER",       MON_IDX_SPIDER);
        m_StringTable[i++].Init("MON_IDX_WORM",         MON_IDX_WORM);
        m_StringTable[i++].Init("MON_IDX_DINOSAUR",     MON_IDX_DINOSAUR);
        m_StringTable[i++].Init("MON_IDX_ANT",          MON_IDX_ANT);
        m_StringTable[i++].Init("MON_IDX_HARPY",        MON_IDX_HARPY);
        m_StringTable[i++].Init("MON_IDX_DOG",          MON_IDX_DOG);
        m_StringTable[i++].Init("MON_IDX_BAT",          MON_IDX_BAT);
        m_StringTable[i++].Init("MON_IDX_FROG",         MON_IDX_FROG);
        m_StringTable[i++].Init("MON_IDX_SKELETON", MON_IDX_SKELETON);
        m_StringTable[i++].Init("MON_IDX_GHOUL", MON_IDX_GHOUL);
        m_StringTable[i++].Init("MON_IDX_WIGHT", MON_IDX_WIGHT);
        m_StringTable[i++].Init("MON_IDX_VAMPIRE", MON_IDX_VAMPIRE);
        m_StringTable[i++].Init("MON_IDX_LICH", MON_IDX_LICH);
        m_StringTable[i++].Init("MON_IDX_FAERIE_DRAGON", MON_IDX_FAERIE_DRAGON);
        m_StringTable[i++].Init("MON_IDX_EYE", MON_IDX_EYE);
        m_StringTable[i++].Init("MON_IDX_CENTIPEDE", MON_IDX_CENTIPEDE);
        m_StringTable[i++].Init("MON_IDX_CAT", MON_IDX_CAT);
        m_StringTable[i++].Init("MON_IDX_GOLEM", MON_IDX_GOLEM);
        m_StringTable[i++].Init("MON_IDX_HUMANOID", MON_IDX_HUMANOID);
        m_StringTable[i++].Init("MON_IDX_GIANT", MON_IDX_GIANT);
        m_StringTable[i++].Init("MON_IDX_LOUSE", MON_IDX_LOUSE);
        m_StringTable[i++].Init("MON_IDX_LEECH", MON_IDX_LEECH);
        m_StringTable[i++].Init("MON_IDX_MOLD", MON_IDX_MOLD);
        m_StringTable[i++].Init("MON_IDX_NAGA", MON_IDX_NAGA);
        m_StringTable[i++].Init("MON_IDX_ORC", MON_IDX_ORC);
        m_StringTable[i++].Init("MON_IDX_PERSON", MON_IDX_PERSON);
        m_StringTable[i++].Init("MON_IDX_L_PERSON", MON_IDX_L_PERSON);
        m_StringTable[i++].Init("MON_IDX_RAT", MON_IDX_RAT);
        m_StringTable[i++].Init("MON_IDX_MINOR_DEMON", MON_IDX_MINOR_DEMON);
        m_StringTable[i++].Init("MON_IDX_MAJOR_DEMON", MON_IDX_MAJOR_DEMON);
        m_StringTable[i++].Init("MON_IDX_YEEK", MON_IDX_YEEK);
        m_StringTable[i++].Init("MON_IDX_ZOMBIE", MON_IDX_ZOMBIE);
        m_StringTable[i++].Init("MON_IDX_BALROG", MON_IDX_BALROG);
        m_StringTable[i++].Init("MON_IDX_FLY", MON_IDX_FLY);
        m_StringTable[i++].Init("MON_IDX_DRAGON_FLY", MON_IDX_DRAGON_FLY);
        m_StringTable[i++].Init("MON_IDX_GHOST", MON_IDX_GHOST);
        m_StringTable[i++].Init("MON_IDX_INSECT", MON_IDX_INSECT);
        m_StringTable[i++].Init("MON_IDX_BEETLE", MON_IDX_BEETLE);
        m_StringTable[i++].Init("MON_IDX_OGRE", MON_IDX_OGRE);
        m_StringTable[i++].Init("MON_IDX_REPTILE", MON_IDX_REPTILE);
        m_StringTable[i++].Init("MON_IDX_TROLL", MON_IDX_TROLL);
        m_StringTable[i++].Init("MON_IDX_WRAITH", MON_IDX_WRAITH);
        m_StringTable[i++].Init("MON_IDX_XORN", MON_IDX_XORN);
        m_StringTable[i++].Init("MON_IDX_YETI", MON_IDX_YETI);
        m_StringTable[i++].Init("MON_IDX_MIMIC", MON_IDX_MIMIC);
        m_StringTable[i++].Init("MON_IDX_TOWNSFOLK", MON_IDX_TOWNSFOLK);

        // Monster flags (attack types, ai types, color types)
        m_StringTable[i++].Init("MON_FLAG_SPORE", MON_FLAG_SPORE);
        m_StringTable[i++].Init("MON_FLAG_TOUCH", MON_FLAG_TOUCH);
        m_StringTable[i++].Init("MON_FLAG_BITE", MON_FLAG_BITE);
        m_StringTable[i++].Init("MON_FLAG_DROOL", MON_FLAG_DROOL);
        m_StringTable[i++].Init("MON_FLAG_CLAW", MON_FLAG_CLAW);
        m_StringTable[i++].Init("MON_FLAG_TRAMPLE", MON_FLAG_TRAMPLE);
        m_StringTable[i++].Init("MON_FLAG_BREATHE", MON_FLAG_BREATHE);
        m_StringTable[i++].Init("MON_FLAG_CRAWL", MON_FLAG_CRAWL);
        m_StringTable[i++].Init("MON_FLAG_FIRE", MON_FLAG_FIRE);
        m_StringTable[i++].Init("MON_FLAG_COLD", MON_FLAG_COLD);
        m_StringTable[i++].Init("MON_FLAG_ELECTRICITY", MON_FLAG_ELECTRICITY);
        m_StringTable[i++].Init("MON_FLAG_ACID", MON_FLAG_ACID);
        m_StringTable[i++].Init("MON_FLAG_POISON", MON_FLAG_POISON);
        m_StringTable[i++].Init("MON_FLAG_SPELL", MON_FLAG_SPELL);
        m_StringTable[i++].Init("MON_FLAG_DRAIN_XP", MON_FLAG_DRAIN_XP);
        m_StringTable[i++].Init("MON_FLAG_BREED", MON_FLAG_BREED);
        m_StringTable[i++].Init("MON_FLAG_AFRAID", MON_FLAG_AFRAID);
        m_StringTable[i++].Init("MON_FLAG_BLIND", MON_FLAG_BLIND);
        m_StringTable[i++].Init("MON_FLAG_SLEEP", MON_FLAG_SLEEP);
        m_StringTable[i++].Init("MON_FLAG_CONFUSE", MON_FLAG_CONFUSE);
        m_StringTable[i++].Init("MON_AI_DONTMOVE", MON_AI_DONTMOVE);
        m_StringTable[i++].Init("MON_AI_100RANDOMMOVE", MON_AI_100RANDOMMOVE);
        m_StringTable[i++].Init("MON_AI_75RANDOMMOVE", MON_AI_75RANDOMMOVE);
        m_StringTable[i++].Init("MON_AI_SEEKPLAYER", MON_AI_SEEKPLAYER);
        m_StringTable[i++].Init("MON_COLOR_MULTI", MON_COLOR_MULTI);
        
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
        m_StringTable[i++].Init("ITEM_IDX_FOOD",        ITEM_IDX_FOOD);
        m_StringTable[i++].Init("ITEM_IDX_AXE",          ITEM_IDX_AXE);
        // Item flags
        m_StringTable[i++].Init("ITEM_FLAG_CURE_POISON", ITEM_FLAG_CURE_POISON);
        m_StringTable[i++].Init("ITEM_FLAG_CURE_BLIND", ITEM_FLAG_CURE_BLIND);
        m_StringTable[i++].Init("ITEM_FLAG_CURE_CONFUSE", ITEM_FLAG_CURE_CONFUSE);
        m_StringTable[i++].Init("ITEM_FLAG_CURE_FEAR", ITEM_FLAG_CURE_FEAR);
        m_StringTable[i++].Init("ITEM_FLAG_HEAL_HP", ITEM_FLAG_HEAL_HP);
        m_StringTable[i++].Init("ITEM_FLAG_HEAL_MP", ITEM_FLAG_HEAL_MP);
        m_StringTable[i++].Init("ITEM_FLAG_IDENTIFY", ITEM_FLAG_IDENTIFY);
        m_StringTable[i++].Init("ITEM_FLAG_LIGHT", ITEM_FLAG_LIGHT);
        m_StringTable[i++].Init("ITEM_FLAG_SUMMON", ITEM_FLAG_SUMMON);
        m_StringTable[i++].Init("ITEM_FLAG_ENCHANT_TOHIT", ITEM_FLAG_ENCHANT_TOHIT);
        m_StringTable[i++].Init("ITEM_FLAG_ENCHANT_TODAM", ITEM_FLAG_ENCHANT_TODAM);
        m_StringTable[i++].Init("ITEM_FLAG_ENCHANT_AC", ITEM_FLAG_ENCHANT_AC);
        m_StringTable[i++].Init("ITEM_FLAG_RESIST_FIRE", ITEM_FLAG_RESIST_FIRE);
        m_StringTable[i++].Init("ITEM_FLAG_RESIST_COLD", ITEM_FLAG_RESIST_COLD);
        m_StringTable[i++].Init("ITEM_FLAG_RESIST_ELEC", ITEM_FLAG_RESIST_ELEC);
        m_StringTable[i++].Init("ITEM_FLAG_RESIST_ACID", ITEM_FLAG_RESIST_ACID);
        m_StringTable[i++].Init("ITEM_FLAG_LEVITATE", ITEM_FLAG_LEVITATE);
        m_StringTable[i++].Init("ITEM_FLAG_RESTORE_STAT", ITEM_FLAG_RESTORE_STAT);
        m_StringTable[i++].Init("ITEM_FLAG_GAIN_STAT", ITEM_FLAG_GAIN_STAT);
        m_StringTable[i++].Init("ITEM_FLAG_INVISIBLE", ITEM_FLAG_INVISIBLE);
        m_StringTable[i++].Init("ITEM_FLAG_MAPPING", ITEM_FLAG_MAPPING);
        m_StringTable[i++].Init("ITEM_FLAG_RECALL", ITEM_FLAG_RECALL);
        m_StringTable[i++].Init("ITEM_FLAG_STONE_TO_MUD", ITEM_FLAG_STONE_TO_MUD);
        m_StringTable[i++].Init("ITEM_FLAG_SPEED", ITEM_FLAG_SPEED);
        m_StringTable[i++].Init("ITEM_FLAG_ESP", ITEM_FLAG_ESP);
        m_StringTable[i++].Init("ITEM_FLAG_CONFUSE", ITEM_FLAG_CONFUSE);
        m_StringTable[i++].Init("ITEM_FLAG_FIND_DOOR", ITEM_FLAG_FIND_DOOR);
        m_StringTable[i++].Init("ITEM_FLAG_FIND_TREASURE", ITEM_FLAG_FIND_TREASURE);
        m_StringTable[i++].Init("ITEM_FLAG_CURSED", ITEM_FLAG_CURSED);
        m_StringTable[i++].Init("ITEM_FLAG_SPELL", ITEM_FLAG_SPELL);
        m_StringTable[i++].Init("ITEM_G_HOLDING", ITEM_FLAG_HOLDING);
        m_StringTable[i++].Init("ITEM_COLOR_MULTI", ITEM_COLOR_MULTI);
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
