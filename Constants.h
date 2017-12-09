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
#define MON_IDX_SHROOM          0
#define MON_IDX_JELLY           1
#define MON_IDX_ICKY            2
#define MON_IDX_DRAGON          3
#define MON_IDX_ANCIENT_DRAGON  4
#define MON_IDX_SNAKE           5
#define MON_IDX_KOBOLD          6
#define MON_IDX_SPIDER          7
#define MON_IDX_WORM            8
#define MON_IDX_DINOSAUR        9
#define MON_IDX_ANT            10
#define MON_IDX_HARPY          11
#define MON_IDX_DOG            12
#define MON_IDX_MAX            13

#define MON_FLAG_SPORE			0x00000001
#define MON_FLAG_TOUCH			0x00000002
#define MON_FLAG_BITE			0x00000004
#define MON_FLAG_DROOL			0x00000008

#define MON_FLAG_CLAW			0x00000010
#define MON_FLAG_POISON			0x00000020
#define MON_FLAG_FIRE           0x00000040
#define MON_FLAG_BREED          0x00000080

#define MON_FLAG_BREATHE        0x00000100

#define MON_AI_DONTMOVE			0x00010000
#define MON_AI_100RANDOMMOVE	0x00020000
#define MON_AI_75RANDOMMOVE		0x00040000

#define MON_COLOR_MULTI			0x01000000

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
#define ITEM_IDX_MAX            23

#define ITEM_FLAG_CURSED        0x00001000

#define ITEM_COLOR_MULTI        0x01000000

// Make sure you change below here if you added any flags.
#define NUM_STRINGS				51
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
        // attack types
        m_StringTable[i++].Init("MON_FLAG_SPORE",		MON_FLAG_SPORE);
        m_StringTable[i++].Init("MON_FLAG_TOUCH",		MON_FLAG_TOUCH);
        m_StringTable[i++].Init("MON_FLAG_BITE",		MON_FLAG_BITE);
        m_StringTable[i++].Init("MON_FLAG_DROOL",		MON_FLAG_DROOL);
        m_StringTable[i++].Init("MON_FLAG_CLAW",		MON_FLAG_CLAW);
        m_StringTable[i++].Init("MON_FLAG_POISON",		MON_FLAG_POISON);
        m_StringTable[i++].Init("MON_FLAG_FIRE",        MON_FLAG_FIRE);
        m_StringTable[i++].Init("MON_FLAG_BREED",       MON_FLAG_BREED);
        m_StringTable[i++].Init("MON_FLAG_BREATHE",     MON_FLAG_BREATHE);
        // AI flags
        // movement flags
        m_StringTable[i++].Init("MON_AI_DONTMOVE",		MON_AI_DONTMOVE);
        m_StringTable[i++].Init("MON_AI_100RANDOMMOVE",	MON_AI_100RANDOMMOVE);
        m_StringTable[i++].Init("MON_AI_75RANDOMMOVE",    MON_AI_75RANDOMMOVE);
        // Color flags
        m_StringTable[i++].Init("MON_COLOR_MULTI",    MON_COLOR_MULTI);
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
        m_StringTable[i++].Init("ITEM_FLAG_CURSED",    ITEM_FLAG_CURSED);
        // Color flags
        m_StringTable[i++].Init("ITEM_COLOR_MULTI",    ITEM_COLOR_MULTI);
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
