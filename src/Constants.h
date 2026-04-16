// Constants.h
// Global constants for JMoria
//
// Created 4/11/3
//
// Note: wanted to use enums, but couldn't figure out
// how to make them work with CL /EP for monsters.dat
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__
#include "Util.h"

#define VERSION "0.61"
#define COPYRIGHT "2002-2026"
#define AUTHOR "Jimbo S. Harris"

// all the states which the game can run in
// this modifies the event handling
#define STATE_INVALID -1
#define STATE_COMMAND 0
#define STATE_MODIFY 1
#define STATE_MENU 2
#define STATE_USE 3
#define STATE_STRINGINPUT 4
#define STATE_ENDGAME 5
#define STATE_CLOCKSTEP 6
#define STATE_REST 7
#define STATE_RUN 8
#define STATE_INTRO 9
#define STATE_LOOK 10
#define STATE_TARGET 11
#define STATE_RANGED 12
#define STATE_MAX 13

// Various statuses that someone could have
#define STATUS_INVALID -1
#define STATUS_DEAD 0
#define STATUS_ALIVE 1
#define STATUS_GORGED 2
#define STATUS_FULL 3
#define STATUS_HUNGRY 4
#define STATUS_STARVING 5
// #define STATUS_CONFUSED 6
// #define STATUS_STUNNED 7
// #define STATUS_AFRAID 8
// #define STATUS_SLEEPING 9
#define STATUS_MAX 6

#define DIR_INVALID -1
#define DIR_NONE -1
#define DIR_NORTH 0
#define DIR_NE 1
#define DIR_EAST 2
#define DIR_SE 3
#define DIR_SOUTH 4
#define DIR_SW 5
#define DIR_WEST 6
#define DIR_NW 7
#define DIR_MAX 8

// Types of Dungeon Tiles
#define DUNG_IDX_INVALID -1
#define DUNG_IDX_FLOOR 0
#define DUNG_IDX_WALL 1
#define DUNG_IDX_DOOR 2
#define DUNG_IDX_OPEN_DOOR 3
#define DUNG_IDX_UPSTAIRS 4
#define DUNG_IDX_LONG_UPSTAIRS 5
#define DUNG_IDX_DOWNSTAIRS 6
#define DUNG_IDX_LONG_DOWNSTAIRS 7
#define DUNG_IDX_RUBBLE 8
#define DUNG_IDX_SECRET_DOOR 9
// JUNK,
#define DUNG_IDX_PLAYER 10
#define DUNG_IDX_MAX 11

// Dungeon Tile flags
#define DUNG_FLAG_LIT 0x00000001
#define DUNG_FLAG_SEEN 0x00000002
#define DUNG_FLAG_ROOM 0x00000004
#define DUNG_FLAG_HALL 0x00000008

#define DUNG_FLAG_TRAP 0x00000010
#define DUNG_FLAG_LOCKED 0x00000020
#define DUNG_FLAG_VISIBLE 0x00000040

// Door and search chance percentages (1-100)
#define CHANCE_PICK_LOCK 75
#define CHANCE_CLOSE_DOOR 90
#define CHANCE_FIND_SECRET_BUMP 20
#define CHANCE_SEARCH_ACTIVE 25
#define CHANCE_SEARCH_PASSIVE 5
// #define DUNG_FLAG_x  0x00000080

// Dungeon Flags
#ifdef FIXED_DUNGEON
#define DUNG_CFG_MONSTERS_PER_LEVEL 0.05f
#define DUNG_CFG_ITEMS_PER_LEVEL 0.05f
#else
#define DUNG_CFG_MONSTERS_PER_LEVEL 0.01f
#define DUNG_CFG_ITEMS_PER_LEVEL 0.02f
#endif // FIXED_DUNGEON
#define DUNG_CFG_START_LEVEL 1
#define DUNG_CFG_MAX_SPAWN_TRIES 10

// Types of Monsters
#define MON_IDX_INVALID -1
#define MON_IDX_ANT 0
#define MON_IDX_BAT 1
#define MON_IDX_CENTIPEDE 2
#define MON_IDX_DRAGON 3
#define MON_IDX_DINOSAUR 4
#define MON_IDX_EYE 5
#define MON_IDX_CAT 6
#define MON_IDX_GOLEM 7
#define MON_IDX_HUMANOID 8
#define MON_IDX_HARPY 9
#define MON_IDX_ICKY 10
#define MON_IDX_KOBOLD 11
#define MON_IDX_LOUSE 12
#define MON_IDX_LEECH 13
#define MON_IDX_MOLD 14
#define MON_IDX_NAGA 15
#define MON_IDX_ORC 16
#define MON_IDX_PERSON 17
#define MON_IDX_RAT 18
#define MON_IDX_SKELETON 19
#define MON_IDX_MINOR_DEMON 20
#define MON_IDX_WORM 21
#define MON_IDX_SPIDER 22
#define MON_IDX_YEEK 23
#define MON_IDX_ZOMBIE 24
#define MON_IDX_FROG 25
#define MON_IDX_BALROG 26
#define MON_IDX_DOG 27
#define MON_IDX_ANCIENT_DRAGON 28
#define MON_IDX_FLY 29
#define MON_IDX_DRAGON_FLY 30
#define MON_IDX_FAERIE_DRAGON 31
#define MON_IDX_GHOUL 32
#define MON_IDX_GHOST 33
#define MON_IDX_GIANT 34
#define MON_IDX_INSECT 35
#define MON_IDX_JELLY 36
#define MON_IDX_BEETLE 37
#define MON_IDX_LICH 38
#define MON_IDX_OGRE 39
#define MON_IDX_L_PERSON 40
#define MON_IDX_REPTILE 41
#define MON_IDX_SNAKE 42
#define MON_IDX_TROLL 43
#define MON_IDX_MAJOR_DEMON 44
#define MON_IDX_VAMPIRE 45
#define MON_IDX_WIGHT 46
#define MON_IDX_WRAITH 47
#define MON_IDX_XORN 48
#define MON_IDX_YETI 49
#define MON_IDX_MIMIC 50
#define MON_IDX_LURKER 51
#define MON_IDX_SHROOM 52
#define MON_IDX_COIN 53
#define MON_IDX_TOWNSFOLK 54
#define MON_IDX_MAX 55

// Monster flags
#define MON_FLAG_SPORE 0x00000001
#define MON_FLAG_TOUCH 0x00000002
#define MON_FLAG_BITE 0x00000004
#define MON_FLAG_DROOL 0x00000008

#define MON_FLAG_CLAW 0x00000010
#define MON_FLAG_TRAMPLE 0x00000020
#define MON_FLAG_BREATHE 0x00000040
#define MON_FLAG_CRAWL 0x00000080

#define MON_FLAG_WARM 0x00000100
#define MON_FLAG_EMPTY_MIND 0x00000200
#define MON_FLAG_REGENERATE 0x00000400
#define MON_FLAG_HURT_BY_LIGHT 0x00000800

// #define MON_FLAG_x          0x00001000
// #define MON_FLAG_x          0x00002000
// #define MON_FLAG_x          0x00004000
#define MON_FLAG_BREED 0x00008000

// #define MON_FLAG_x          0x00100000
// #define MON_FLAG_x          0x00200000
// #define MON_FLAG_x          0x00400000
// #define MON_FLAG_x          0x00800000

#define MON_AI_DONTMOVE 0x01000000
#define MON_AI_100RANDOMMOVE 0x02000000
#define MON_AI_75RANDOMMOVE 0x04000000
#define MON_AI_SEEKPLAYER 0x08000000

#define MON_COLOR_MULTI 0x10000000
// #define MON_COLOR_x          0x20000000
// #define MON_COLOR_x          0x40000000
// #define MON_COLOR_x          0x80000000

#define NUM_MON_FLAGS 18

// Effect Flags
#define EFFECT_FLAG_FIRE 0x00000001
#define EFFECT_FLAG_COLD 0x00000002
#define EFFECT_FLAG_ELECTRICITY 0x00000004
#define EFFECT_FLAG_ACID 0x00000008

#define EFFECT_FLAG_POISON 0x00000010
#define EFFECT_FLAG_LIGHT 0x00000020
#define EFFECT_FLAG_PARALYZE 0x00000040
#define EFFECT_FLAG_TREASURE 0x00000080

#define EFFECT_FLAG_AFRAID 0x00000100
#define EFFECT_FLAG_BLIND 0x00000200
#define EFFECT_FLAG_SLEEP 0x00000400
#define EFFECT_FLAG_CONFUSE 0x00000800

#define EFFECT_FLAG_STONE_TO_MUD 0x00001000
#define EFFECT_FLAG_FUEL 0x00002000
#define EFFECT_FLAG_INFRA 0x00004000
#define EFFECT_FLAG_ESP 0x00008000

#define EFFECT_FLAG_IDENTIFY 0x00010000
#define EFFECT_FLAG_RECALL 0x00020000
#define EFFECT_FLAG_MAPPING 0x00040000
#define EFFECT_FLAG_SUMMON 0x00080000

#define EFFECT_FLAG_STAT 0x00100000
#define EFFECT_FLAG_TOHIT 0x00200000
#define EFFECT_FLAG_TODAM 0x00400000
#define EFFECT_FLAG_AC 0x00800000

#define EFFECT_FLAG_XP 0x01000000
#define EFFECT_FLAG_HP 0x02000000
#define EFFECT_FLAG_MP 0x04000000
#define EFFECT_FLAG_TELEPORT 0x08000000

#define EFFECT_FLAG_FREE_ACTION 0x10000000
#define EFFECT_FLAG_INVISIBLE 0x20000000
#define EFFECT_FLAG_LEVITATE 0x40000000
#define EFFECT_FLAG_SPEED 0x80000000

#define NUM_EFFECT_FLAGS 32
// probably need a second set of effects
// cursed
// trap
// holding might be an item thing

// Effect Modifiers
#define EFFECT_MOD_RESIST 0x000000001
#define EFFECT_MOD_SEE 0x000000002
#define EFFECT_MOD_IMMUNE 0x000000004
#define EFFECT_MOD_WEAK 0x000000008

#define EFFECT_MOD_TIMED 0x000000010
#define EFFECT_MOD_AREA 0x000000020
#define EFFECT_MOD_LINE 0x000000040
#define EFFECT_MOD_BALL 0x000000080

#define EFFECT_MOD_ENCHANT 0x000000100
// #define EFFECT_MOD_x 0x000000200
// #define EFFECT_MOD_x 0x000000400
// #define EFFECT_MOD_x 0x000000800

#define NUM_EFFECT_MODIFIERS 9

// Effect Types
#define EFFECT_TYPE_HEAL 0x00000001
#define EFFECT_TYPE_HIT 0x00000002
#define EFFECT_TYPE_CREATE 0x00000004
#define EFFECT_TYPE_DESTROY 0x00000008

#define EFFECT_TYPE_INTRINSIC 0x00000010
#define EFFECT_TYPE_RESTORE 0x00000020
#define EFFECT_TYPE_GAIN 0x00000040
#define EFFECT_TYPE_LOSE 0x00000080

// #define EFFECT_TYPE_x 0x00000100
// #define EFFECT_TYPE_x 0x00000200
// #define EFFECT_TYPE_x 0x00000400
// #define EFFECT_TYPE_x 0x00000800

#define NUM_EFFECT_TYPES 8

// Character equipment slots
// index for m_llEquipment
#define EQUIP_IDX_INVALID -1
#define EQUIP_IDX_MAIN_HAND 0
#define EQUIP_IDX_OFF_HAND 1
#define EQUIP_IDX_HELMET 2
#define EQUIP_IDX_AMULET 3
#define EQUIP_IDX_ARMOR 4
#define EQUIP_IDX_CLOAK 5
#define EQUIP_IDX_GLOVES 6
#define EQUIP_IDX_BELT 7
#define EQUIP_IDX_BOOTS 8
#define EQUIP_IDX_RING 9 // duplicate on purpose: readability
#define EQUIP_IDX_LRING 9
#define EQUIP_IDX_RRING 10
#define EQUIP_IDX_TORCH 11
#define EQUIP_IDX_AMMO 12
#define EQUIP_IDX_MAX 13 // adding a new type to equipment could be tricky; avoid

// Types of items
// see ItemIDs
#define ITEM_IDX_INVALID -1
#define ITEM_IDX_SWORD 0
#define ITEM_IDX_SHIELD 1
#define ITEM_IDX_ARMOR 2
#define ITEM_IDX_HELMET 3
#define ITEM_IDX_CLOAK 4
#define ITEM_IDX_GLOVES 5
#define ITEM_IDX_BOOTS 6
#define ITEM_IDX_AMULET 7
#define ITEM_IDX_RING 8
#define ITEM_IDX_TORCH 9
#define ITEM_IDX_BOW 10
#define ITEM_IDX_ARROW 11
#define ITEM_IDX_XBOW 12
#define ITEM_IDX_BOLT 13
#define ITEM_IDX_CHEST 14
#define ITEM_IDX_SCROLL 15
#define ITEM_IDX_POTION 16
#define ITEM_IDX_WAND 17
#define ITEM_IDX_STAFF 18
#define ITEM_IDX_BOOK 19
#define ITEM_IDX_MONEY 20
#define ITEM_IDX_FOOD 21
#define ITEM_IDX_SPEAR 22
#define ITEM_IDX_AXE 23
#define ITEM_IDX_POLEARM 24
#define ITEM_IDX_SHOVEL 25
#define ITEM_IDX_DAGGER 26
#define ITEM_IDX_MACE 27
#define ITEM_IDX_2H_SWORD 28
#define ITEM_IDX_BELT 29
#define ITEM_IDX_FUEL 30
#define ITEM_IDX_MAX 31

// TODO: Might need to switch from "ITEM/MONSTER"
//       to "types of harm" / "types of aid"
//       like: <hurt>, <blind>, XdY means "blind for x turns"
//             <heal>, <blind> means "cure blindness"
// Item flags

#define ITEM_FLAG_CURSED 0x00000001
#define ITEM_FLAG_STACKS 0x00000002
#define ITEM_FLAG_IDENTIFIED 0x00000004
#define ITEM_FLAG_MAGIC 0x00000008

#define ITEM_FLAG_2HANDED 0x00000010
#define ITEM_FLAG_OFFHAND 0x00000020
#define ITEM_FLAG_MAINHAND 0x00000040
#define ITEM_FLAG_NEEDSAMMO 0x00000080

#define ITEM_FLAG_NO_COLLIDE 0x00000100
// #define ITEM_FLAG_x 0x00000200
// #define ITEM_FLAG_x 0x00000400
// #define ITEM_FLAG_x 0x00000800

// #define ITEM_FLAG_x 0x00001000
// #define ITEM_FLAG_x 0x00002000
// #define ITEM_FLAG_x 0x00004000
#define ITEM_FLAG_HOLDING 0x00008000

#define ITEM_COLOR_MULTI 0x10000000

// Known-property flags for CItem::m_dwKnownProps
#define KNOWN_CURSED 0x00000001
#define KNOWN_BONUSES 0x00000002
#define KNOWN_CHARGES 0x00000004
#define KNOWN_TRIED 0x00000008

#define NUM_ITEM_FLAGS 11

// Make sure you change below here if you added any flags.
#define NUM_STRINGS                                                                                \
    MON_IDX_MAX + NUM_MON_FLAGS + EQUIP_IDX_MAX + ITEM_IDX_MAX + NUM_ITEM_FLAGS +                  \
        NUM_EFFECT_FLAGS + NUM_EFFECT_MODIFIERS + NUM_EFFECT_TYPES

#define MON_IDX 0
#define MON_FLAG 1
#define EQUIP_IDX 2
#define ITEM_IDX 3
#define ITEM_FLAG 4
#define EFFECT_FLAG 5
#define EFFECT_MOD 6
#define MON_AI 7
#define EFFECT_TYPE 8

#define NUM_POTION_TYPES 32
#define NUM_SCROLL_TYPES 32
#define NUM_LUMBER_TYPES 32

#include "TextEntry.h"
class Constants
{
public:
    Constants() : m_StringTable( NULL ) {}
    ~Constants()
    {
        if( m_StringTable != NULL )
            delete[] m_StringTable;
        m_StringTable = NULL;
    }

    void Init()
    {
        JLog( LOG_LEVEL_INFO, true, "expecting %d strings...", NUM_STRINGS );
        m_StringTable = new TextEntry[NUM_STRINGS];
        int i = 0;
        // Monster flags
        // types
        m_StringTable[i++].Init( "MON_IDX_SHROOM", MON_IDX_SHROOM );
        m_StringTable[i++].Init( "MON_IDX_JELLY", MON_IDX_JELLY );
        m_StringTable[i++].Init( "MON_IDX_ICKY", MON_IDX_ICKY );
        m_StringTable[i++].Init( "MON_IDX_DRAGON", MON_IDX_DRAGON );
        m_StringTable[i++].Init( "MON_IDX_ANCIENT_DRAGON", MON_IDX_ANCIENT_DRAGON );
        m_StringTable[i++].Init( "MON_IDX_SNAKE", MON_IDX_SNAKE );
        m_StringTable[i++].Init( "MON_IDX_KOBOLD", MON_IDX_KOBOLD );
        m_StringTable[i++].Init( "MON_IDX_SPIDER", MON_IDX_SPIDER );
        m_StringTable[i++].Init( "MON_IDX_WORM", MON_IDX_WORM );
        m_StringTable[i++].Init( "MON_IDX_DINOSAUR", MON_IDX_DINOSAUR );
        m_StringTable[i++].Init( "MON_IDX_ANT", MON_IDX_ANT );
        m_StringTable[i++].Init( "MON_IDX_HARPY", MON_IDX_HARPY );
        m_StringTable[i++].Init( "MON_IDX_DOG", MON_IDX_DOG );
        m_StringTable[i++].Init( "MON_IDX_BAT", MON_IDX_BAT );
        m_StringTable[i++].Init( "MON_IDX_FROG", MON_IDX_FROG );
        m_StringTable[i++].Init( "MON_IDX_SKELETON", MON_IDX_SKELETON );
        m_StringTable[i++].Init( "MON_IDX_GHOUL", MON_IDX_GHOUL );
        m_StringTable[i++].Init( "MON_IDX_WIGHT", MON_IDX_WIGHT );
        m_StringTable[i++].Init( "MON_IDX_VAMPIRE", MON_IDX_VAMPIRE );
        m_StringTable[i++].Init( "MON_IDX_LICH", MON_IDX_LICH );
        m_StringTable[i++].Init( "MON_IDX_FAERIE_DRAGON", MON_IDX_FAERIE_DRAGON );
        m_StringTable[i++].Init( "MON_IDX_EYE", MON_IDX_EYE );
        m_StringTable[i++].Init( "MON_IDX_CENTIPEDE", MON_IDX_CENTIPEDE );
        m_StringTable[i++].Init( "MON_IDX_CAT", MON_IDX_CAT );
        m_StringTable[i++].Init( "MON_IDX_GOLEM", MON_IDX_GOLEM );
        m_StringTable[i++].Init( "MON_IDX_HUMANOID", MON_IDX_HUMANOID );
        m_StringTable[i++].Init( "MON_IDX_GIANT", MON_IDX_GIANT );
        m_StringTable[i++].Init( "MON_IDX_LOUSE", MON_IDX_LOUSE );
        m_StringTable[i++].Init( "MON_IDX_LEECH", MON_IDX_LEECH );
        m_StringTable[i++].Init( "MON_IDX_MOLD", MON_IDX_MOLD );
        m_StringTable[i++].Init( "MON_IDX_NAGA", MON_IDX_NAGA );
        m_StringTable[i++].Init( "MON_IDX_ORC", MON_IDX_ORC );
        m_StringTable[i++].Init( "MON_IDX_PERSON", MON_IDX_PERSON );
        m_StringTable[i++].Init( "MON_IDX_L_PERSON", MON_IDX_L_PERSON );
        m_StringTable[i++].Init( "MON_IDX_RAT", MON_IDX_RAT );
        m_StringTable[i++].Init( "MON_IDX_MINOR_DEMON", MON_IDX_MINOR_DEMON );
        m_StringTable[i++].Init( "MON_IDX_MAJOR_DEMON", MON_IDX_MAJOR_DEMON );
        m_StringTable[i++].Init( "MON_IDX_YEEK", MON_IDX_YEEK );
        m_StringTable[i++].Init( "MON_IDX_ZOMBIE", MON_IDX_ZOMBIE );
        m_StringTable[i++].Init( "MON_IDX_BALROG", MON_IDX_BALROG );
        m_StringTable[i++].Init( "MON_IDX_FLY", MON_IDX_FLY );
        m_StringTable[i++].Init( "MON_IDX_DRAGON_FLY", MON_IDX_DRAGON_FLY );
        m_StringTable[i++].Init( "MON_IDX_GHOST", MON_IDX_GHOST );
        m_StringTable[i++].Init( "MON_IDX_INSECT", MON_IDX_INSECT );
        m_StringTable[i++].Init( "MON_IDX_BEETLE", MON_IDX_BEETLE );
        m_StringTable[i++].Init( "MON_IDX_OGRE", MON_IDX_OGRE );
        m_StringTable[i++].Init( "MON_IDX_REPTILE", MON_IDX_REPTILE );
        m_StringTable[i++].Init( "MON_IDX_TROLL", MON_IDX_TROLL );
        m_StringTable[i++].Init( "MON_IDX_WRAITH", MON_IDX_WRAITH );
        m_StringTable[i++].Init( "MON_IDX_XORN", MON_IDX_XORN );
        m_StringTable[i++].Init( "MON_IDX_YETI", MON_IDX_YETI );
        m_StringTable[i++].Init( "MON_IDX_MIMIC", MON_IDX_MIMIC );
        m_StringTable[i++].Init( "MON_IDX_LURKER", MON_IDX_LURKER );
        m_StringTable[i++].Init( "MON_IDX_COIN", MON_IDX_COIN );
        m_StringTable[i++].Init( "MON_IDX_TOWNSFOLK", MON_IDX_TOWNSFOLK );

        // Monster flags (attack types, ai types, color types)
        m_StringTable[i++].Init( "MON_FLAG_SPORE", MON_FLAG_SPORE );
        m_StringTable[i++].Init( "MON_FLAG_TOUCH", MON_FLAG_TOUCH );
        m_StringTable[i++].Init( "MON_FLAG_BITE", MON_FLAG_BITE );
        m_StringTable[i++].Init( "MON_FLAG_DROOL", MON_FLAG_DROOL );
        m_StringTable[i++].Init( "MON_FLAG_CLAW", MON_FLAG_CLAW );
        m_StringTable[i++].Init( "MON_FLAG_TRAMPLE", MON_FLAG_TRAMPLE );
        m_StringTable[i++].Init( "MON_FLAG_BREATHE", MON_FLAG_BREATHE );
        m_StringTable[i++].Init( "MON_FLAG_CRAWL", MON_FLAG_CRAWL );
        m_StringTable[i++].Init( "MON_FLAG_WARM", MON_FLAG_WARM );
        m_StringTable[i++].Init( "MON_FLAG_EMPTY_MIND", MON_FLAG_EMPTY_MIND );
        m_StringTable[i++].Init( "MON_FLAG_REGENERATE", MON_FLAG_REGENERATE );
        m_StringTable[i++].Init( "MON_FLAG_HURT_BY_LIGHT", MON_FLAG_HURT_BY_LIGHT );
        m_StringTable[i++].Init( "MON_FLAG_BREED", MON_FLAG_BREED );
        m_StringTable[i++].Init( "MON_AI_DONTMOVE", MON_AI_DONTMOVE );
        m_StringTable[i++].Init( "MON_AI_100RANDOMMOVE", MON_AI_100RANDOMMOVE );
        m_StringTable[i++].Init( "MON_AI_75RANDOMMOVE", MON_AI_75RANDOMMOVE );
        m_StringTable[i++].Init( "MON_AI_SEEKPLAYER", MON_AI_SEEKPLAYER );
        m_StringTable[i++].Init( "MON_COLOR_MULTI", MON_COLOR_MULTI );

        // Effect flags (flags, modifiers, types)
        m_StringTable[i++].Init( "EFFECT_FLAG_FIRE", EFFECT_FLAG_FIRE );
        m_StringTable[i++].Init( "EFFECT_FLAG_COLD", EFFECT_FLAG_COLD );
        m_StringTable[i++].Init( "EFFECT_FLAG_ELECTRICITY", EFFECT_FLAG_ELECTRICITY );
        m_StringTable[i++].Init( "EFFECT_FLAG_ACID", EFFECT_FLAG_ACID );

        m_StringTable[i++].Init( "EFFECT_FLAG_POISON", EFFECT_FLAG_POISON );
        m_StringTable[i++].Init( "EFFECT_FLAG_LIGHT", EFFECT_FLAG_LIGHT );
        m_StringTable[i++].Init( "EFFECT_FLAG_PARALYZE", EFFECT_FLAG_PARALYZE );
        m_StringTable[i++].Init( "EFFECT_FLAG_TREASURE", EFFECT_FLAG_TREASURE );

        m_StringTable[i++].Init( "EFFECT_FLAG_AFRAID", EFFECT_FLAG_AFRAID );
        m_StringTable[i++].Init( "EFFECT_FLAG_BLIND", EFFECT_FLAG_BLIND );
        m_StringTable[i++].Init( "EFFECT_FLAG_SLEEP", EFFECT_FLAG_SLEEP );
        m_StringTable[i++].Init( "EFFECT_FLAG_CONFUSE", EFFECT_FLAG_CONFUSE );

        m_StringTable[i++].Init( "EFFECT_FLAG_STONE_TO_MUD", EFFECT_FLAG_STONE_TO_MUD );
        m_StringTable[i++].Init( "EFFECT_FLAG_FUEL", EFFECT_FLAG_FUEL );
        m_StringTable[i++].Init( "EFFECT_FLAG_INFRA", EFFECT_FLAG_INFRA );
        m_StringTable[i++].Init( "EFFECT_FLAG_ESP", EFFECT_FLAG_ESP );

        m_StringTable[i++].Init( "EFFECT_FLAG_IDENTIFY", EFFECT_FLAG_IDENTIFY );
        m_StringTable[i++].Init( "EFFECT_FLAG_RECALL", EFFECT_FLAG_RECALL );
        m_StringTable[i++].Init( "EFFECT_FLAG_MAPPING", EFFECT_FLAG_MAPPING );
        m_StringTable[i++].Init( "EFFECT_FLAG_SUMMON", EFFECT_FLAG_SUMMON );

        m_StringTable[i++].Init( "EFFECT_FLAG_STAT", EFFECT_FLAG_STAT );
        m_StringTable[i++].Init( "EFFECT_FLAG_TOHIT", EFFECT_FLAG_TOHIT );
        m_StringTable[i++].Init( "EFFECT_FLAG_TODAM", EFFECT_FLAG_TODAM );
        m_StringTable[i++].Init( "EFFECT_FLAG_AC", EFFECT_FLAG_AC );

        m_StringTable[i++].Init( "EFFECT_FLAG_HP", EFFECT_FLAG_HP );
        m_StringTable[i++].Init( "EFFECT_FLAG_XP", EFFECT_FLAG_XP );
        m_StringTable[i++].Init( "EFFECT_FLAG_MP", EFFECT_FLAG_MP );
        m_StringTable[i++].Init( "EFFECT_FLAG_TELEPORT", EFFECT_FLAG_TELEPORT );

        m_StringTable[i++].Init( "EFFECT_FLAG_FREE_ACTION", EFFECT_FLAG_FREE_ACTION );
        m_StringTable[i++].Init( "EFFECT_FLAG_INVISIBLE", EFFECT_FLAG_INVISIBLE );
        m_StringTable[i++].Init( "EFFECT_FLAG_LEVITATE", EFFECT_FLAG_LEVITATE );
        m_StringTable[i++].Init( "EFFECT_FLAG_SPEED", EFFECT_FLAG_SPEED );

        // Effect Modifiers
        m_StringTable[i++].Init( "EFFECT_MOD_RESIST", EFFECT_MOD_RESIST );
        m_StringTable[i++].Init( "EFFECT_MOD_SEE", EFFECT_MOD_SEE );
        m_StringTable[i++].Init( "EFFECT_MOD_IMMUNE", EFFECT_MOD_IMMUNE );
        m_StringTable[i++].Init( "EFFECT_MOD_WEAK", EFFECT_MOD_WEAK );

        m_StringTable[i++].Init( "EFFECT_MOD_TIMED", EFFECT_MOD_TIMED );
        m_StringTable[i++].Init( "EFFECT_MOD_AREA", EFFECT_MOD_AREA );
        m_StringTable[i++].Init( "EFFECT_MOD_LINE", EFFECT_MOD_LINE );
        m_StringTable[i++].Init( "EFFECT_MOD_BALL", EFFECT_MOD_BALL );

        m_StringTable[i++].Init( "EFFECT_MOD_ENCHANT", EFFECT_MOD_ENCHANT );

        // Effect types
        m_StringTable[i++].Init( "EFFECT_TYPE_HEAL", EFFECT_TYPE_HEAL );
        m_StringTable[i++].Init( "EFFECT_TYPE_HIT", EFFECT_TYPE_HIT );
        m_StringTable[i++].Init( "EFFECT_TYPE_CREATE", EFFECT_TYPE_CREATE );
        m_StringTable[i++].Init( "EFFECT_TYPE_DESTROY", EFFECT_TYPE_DESTROY );
        m_StringTable[i++].Init( "EFFECT_TYPE_INTRINSIC", EFFECT_TYPE_INTRINSIC );
        m_StringTable[i++].Init( "EFFECT_TYPE_RESTORE", EFFECT_TYPE_RESTORE );
        m_StringTable[i++].Init( "EFFECT_TYPE_GAIN", EFFECT_TYPE_GAIN );
        m_StringTable[i++].Init( "EFFECT_TYPE_LOSE", EFFECT_TYPE_LOSE );

        // Equipment slots
        m_StringTable[i++].Init( "EQUIP_IDX_MAIN_HAND", EQUIP_IDX_MAIN_HAND );
        m_StringTable[i++].Init( "EQUIP_IDX_OFF_HAND", EQUIP_IDX_OFF_HAND );
        m_StringTable[i++].Init( "EQUIP_IDX_HELMET", EQUIP_IDX_HELMET );
        m_StringTable[i++].Init( "EQUIP_IDX_AMULET", EQUIP_IDX_AMULET );
        m_StringTable[i++].Init( "EQUIP_IDX_ARMOR", EQUIP_IDX_ARMOR );
        m_StringTable[i++].Init( "EQUIP_IDX_CLOAK", EQUIP_IDX_CLOAK );
        m_StringTable[i++].Init( "EQUIP_IDX_GLOVES", EQUIP_IDX_GLOVES );
        m_StringTable[i++].Init( "EQUIP_IDX_BELT", EQUIP_IDX_BELT );
        m_StringTable[i++].Init( "EQUIP_IDX_BOOTS", EQUIP_IDX_BOOTS );
        m_StringTable[i++].Init( "EQUIP_IDX_LRING", EQUIP_IDX_LRING );
        m_StringTable[i++].Init( "EQUIP_IDX_RRING", EQUIP_IDX_RRING );
        m_StringTable[i++].Init( "EQUIP_IDX_TORCH", EQUIP_IDX_TORCH );
        m_StringTable[i++].Init( "EQUIP_IDX_AMMO", EQUIP_IDX_AMMO );

        // Item Types
        m_StringTable[i++].Init( "ITEM_IDX_SWORD", ITEM_IDX_SWORD );
        m_StringTable[i++].Init( "ITEM_IDX_SHIELD", ITEM_IDX_SHIELD );
        m_StringTable[i++].Init( "ITEM_IDX_ARMOR", ITEM_IDX_ARMOR );
        m_StringTable[i++].Init( "ITEM_IDX_HELMET", ITEM_IDX_HELMET );
        m_StringTable[i++].Init( "ITEM_IDX_CLOAK", ITEM_IDX_CLOAK );
        m_StringTable[i++].Init( "ITEM_IDX_GLOVES", ITEM_IDX_GLOVES );
        m_StringTable[i++].Init( "ITEM_IDX_BOOTS", ITEM_IDX_BOOTS );
        m_StringTable[i++].Init( "ITEM_IDX_AMULET", ITEM_IDX_AMULET );
        m_StringTable[i++].Init( "ITEM_IDX_RING", ITEM_IDX_RING );
        m_StringTable[i++].Init( "ITEM_IDX_TORCH", ITEM_IDX_TORCH );
        m_StringTable[i++].Init( "ITEM_IDX_BOW", ITEM_IDX_BOW );
        m_StringTable[i++].Init( "ITEM_IDX_ARROW", ITEM_IDX_ARROW );
        m_StringTable[i++].Init( "ITEM_IDX_XBOW", ITEM_IDX_XBOW );
        m_StringTable[i++].Init( "ITEM_IDX_BOLT", ITEM_IDX_BOLT );
        m_StringTable[i++].Init( "ITEM_IDX_CHEST", ITEM_IDX_CHEST );
        m_StringTable[i++].Init( "ITEM_IDX_SCROLL", ITEM_IDX_SCROLL );
        m_StringTable[i++].Init( "ITEM_IDX_POTION", ITEM_IDX_POTION );
        m_StringTable[i++].Init( "ITEM_IDX_WAND", ITEM_IDX_WAND );
        m_StringTable[i++].Init( "ITEM_IDX_STAFF", ITEM_IDX_STAFF );
        m_StringTable[i++].Init( "ITEM_IDX_BOOK", ITEM_IDX_BOOK );
        m_StringTable[i++].Init( "ITEM_IDX_MONEY", ITEM_IDX_MONEY );
        m_StringTable[i++].Init( "ITEM_IDX_FOOD", ITEM_IDX_FOOD );
        m_StringTable[i++].Init( "ITEM_IDX_DAGGER", ITEM_IDX_DAGGER );
        m_StringTable[i++].Init( "ITEM_IDX_MACE", ITEM_IDX_MACE );
        m_StringTable[i++].Init( "ITEM_IDX_2H_SWORD", ITEM_IDX_2H_SWORD );
        m_StringTable[i++].Init( "ITEM_IDX_SPEAR", ITEM_IDX_SPEAR );
        m_StringTable[i++].Init( "ITEM_IDX_AXE", ITEM_IDX_AXE );
        m_StringTable[i++].Init( "ITEM_IDX_POLEARM", ITEM_IDX_POLEARM );
        m_StringTable[i++].Init( "ITEM_IDX_SHOVEL", ITEM_IDX_SHOVEL );
        m_StringTable[i++].Init( "ITEM_IDX_BELT", ITEM_IDX_BELT );
        m_StringTable[i++].Init( "ITEM_IDX_FUEL", ITEM_IDX_FUEL );

        // Item flags
        m_StringTable[i++].Init( "ITEM_FLAG_CURSED", ITEM_FLAG_CURSED );
        m_StringTable[i++].Init( "ITEM_FLAG_STACKS", ITEM_FLAG_STACKS );
        m_StringTable[i++].Init( "ITEM_FLAG_IDENTIFIED", ITEM_FLAG_IDENTIFIED );
        m_StringTable[i++].Init( "ITEM_FLAG_MAGIC", ITEM_FLAG_MAGIC );
        m_StringTable[i++].Init( "ITEM_FLAG_2HANDED", ITEM_FLAG_2HANDED );
        m_StringTable[i++].Init( "ITEM_FLAG_OFFHAND", ITEM_FLAG_OFFHAND );
        m_StringTable[i++].Init( "ITEM_FLAG_MAINHAND", ITEM_FLAG_MAINHAND );
        m_StringTable[i++].Init( "ITEM_FLAG_NEEDSAMMO", ITEM_FLAG_NEEDSAMMO );
        m_StringTable[i++].Init( "ITEM_FLAG_NO_COLLIDE", ITEM_FLAG_NO_COLLIDE );
        m_StringTable[i++].Init( "ITEM_FLAG_HOLDING", ITEM_FLAG_HOLDING );
        m_StringTable[i++].Init( "ITEM_COLOR_MULTI", ITEM_COLOR_MULTI );

        if( i != NUM_STRINGS )
        {
            JLog( LOG_LEVEL_ERROR, true,
                  "FATAL: StringTable expected %d entries but got %d. "
                  "NUM_ITEM_FLAGS or another NUM_ constant is out of sync!\n",
                  NUM_STRINGS, i );
            assert( i == NUM_STRINGS && "StringTable entry count mismatch" );
        }
        else
        {
            JLog( LOG_LEVEL_INFO, false, "Success!\n" );
        }
    };

    bool CompareType( const char *type, const char *szIn )
    {
        if( Util::jstrncmp( szIn, type, Util::jstrlen( type ) ) )
        {
            JLog( LOG_LEVEL_WARN, true, "wanted type %s but got %s\n", type, szIn );
            return false;
        }

        return true;
    }

    int LookupString( const char *szIn )
    {
        int i;
        for( i = 0; i < NUM_STRINGS; i++ )
        {
            const char *curr = m_StringTable[i].m_szString;
            if( Util::jstrcmp( curr, szIn ) == 0 )
            {
                return m_StringTable[i].m_dwValue;
            }
        }

        JLog( LOG_LEVEL_WARN, true, "bad string: %s\n", szIn );

        return -1;
    }

    const char *IndexToString( const int flag_set, const int dwIndex )
    {
        if( dwIndex < 0 || dwIndex >= NUM_STRINGS )
            return "";
        int offset = 0;
        int index = dwIndex;

        switch( flag_set )
        {
        case ITEM_FLAG:
            offset += ITEM_IDX_MAX;
        case ITEM_IDX:
            offset += EQUIP_IDX_MAX;
        case EQUIP_IDX:
            offset += NUM_EFFECT_TYPES;
        case EFFECT_TYPE:
            offset += NUM_EFFECT_MODIFIERS;
        case EFFECT_MOD:
            offset += NUM_EFFECT_FLAGS;
        case EFFECT_FLAG:
            offset += NUM_MON_FLAGS;
        case MON_FLAG:
            offset += MON_IDX_MAX;
        case MON_IDX:
            break;
        default:
            break;
        }
        switch( flag_set )
        {
        case ITEM_IDX:
        case EQUIP_IDX:
        case MON_IDX:
            break;
        default:
            index = Util::jlog2( dwIndex );
        }
        return m_StringTable[offset + index].m_szString;
    }

    const char *PotionColor( const uint32 dwIndex )
    {
        if( dwIndex >= NUM_POTION_TYPES )
            return "";
        const char *PotionColors[] = {
            "Clear",      "White",   "Black",    "Red",    "Pink",    "Orange",  "Yellow",
            "Green",      "Blue",    "Purple",   "Brown",  "Gray",    "Golden",  "Silver",
            "Ruby",       "Emerald", "Sapphire", "Amber",  "Rose",    "Lilac",   "Teal",
            "Turquoise",  "Navy",    "Olive",    "Maroon", "Crimson", "Fuchsia", "Lavender",
            "Chartreuse", "Gloopy",  "Bubbling", "Glowing" };
        JLog( LOG_LEVEL_NOISE, true, "index %d value %s\n", dwIndex, PotionColors[dwIndex] );
        return PotionColors[dwIndex];
    }

    const char *PotionRGBA( const uint32 dwIndex )
    {
        if( dwIndex >= NUM_POTION_TYPES )
            return "0,0,0,0";

        const char *PotionRGBAs[] = {
            "255,255,255,10",  // Clear
            "255,255,255,255", // White
            "0,0,0,255",       // Black
            "255,0,0,255",     // Red
            "255,192,203,255", // Pink
            "255,165,0,255",   // Orange
            "255,255,0,255",   // Yellow
            "0,128,0,255",     // Green
            "0,0,255,255",     // Blue
            "128,0,128,255",   // Purple
            "165,42,42,255",   // Brown
            "128,128,128,255", // Gray
            "255,215,0,255",   // Golden
            "192,192,192,255", // Silver
            "220,20,60,255",   // Ruby
            "0,128,0,255",     // Emerald
            "0,0,255,255",     // Sapphire
            "255,191,0,255",   // Amber
            "255,228,225,255", // Rose
            "221,160,221,255", // Lilac
            "0,128,128,255",   // Teal
            "64,224,208,255",  // Turquoise
            "0,0,128,255",     // Navy
            "128,128,0,255",   // Olive
            "128,0,0,255",     // Maroon
            "220,20,60,255",   // Crimson
            "255,0,255,255",   // Fuchsia
            "230,230,250,255", // Lavender
            "127,255,0,255",   // Chartreuse
            "127,127,127,255", //  "Gloopy"
            "225,225,255,255", // "Bubbling"
            "127,255,0,255"    // "Glowing"
        };

        JLog( LOG_LEVEL_NOISE, true, "index %d value %s\n", dwIndex, PotionRGBAs[dwIndex] );
        return PotionRGBAs[dwIndex];
    }

    const char *ScrollName( const uint32 dwIndex )
    {
        if( dwIndex >= NUM_SCROLL_TYPES )
            return "";

        const char *ScrollNames[] = {
            "pizza",          "dolphin",       "coffee",       "doggos",       "dolor sit",
            "amet consect",   "etur adipis",   "elit sed",     "do eius",      "mod tempor",
            "didunt utbore",  "et aliqua",     "ut enim ad",   "veniam quis",  "nostrud exer",
            "ullamco labo",   "ris nisi ut",   "aliquip ex",   "comm sequat",  "duis aute ir",
            "uredo lorin",    "epre deriti",   "volute velit", "esse cillum",  "eu fugiat",
            "nulla pariatur", "excesint occa", "ecat pidatat", "proident sun", "incul pafic",
            "des erumol",     "id est laborum" };

        return ScrollNames[dwIndex];
    }

    const char *Lumber( const uint32 dwIndex )
    {
        if( dwIndex >= NUM_LUMBER_TYPES )
            return "";

        const char *myLumber[] = {
            "Oak",      "Ash",       "Willow",   "Hazel", "Hawthorn", "Ebony",      "Yew",
            "Maple",    "Birch",     "Pine",     "Cedar", "Walnut",   "Elm",        "Spruce",
            "Cherry",   "Applewood", "Pearwood", "Holly", "Platinum", "Blackthorn", "Alder",
            "Ironwood", "Heartwood", "Steel",    "Glass", "Plastic",  "Aluminum",   "Dragonbone",
            "Iron",     "Silver",    "Gold",     "Bronze" };

        return myLumber[dwIndex];
    }

    const char *LumberRGBA( const uint32 dwIndex )
    {
        if( dwIndex >= NUM_LUMBER_TYPES )
            return "0,0,0,0";

        const char *LumberRGBAs[] = {
            "160,82,45,255",   // Oak
            "150,100,80,255",  // Ash
            "190,180,160,255", // Willow
            "120,90,70,255",   // Hazel
            "180,150,130,255", // Hawthorn
            "30,15,10,255",    // Ebony
            "100,80,60,255",   // Yew
            "200,180,150,255", // Maple
            "220,200,180,255", // Birch
            "190,170,150,255", // Pine
            "150,120,90,255",  // Cedar
            "100,60,30,255",   // Walnut
            "160,120,90,255",  // Elm
            "180,160,140,255", // Spruce
            "200,150,120,255", // Cherry
            "220,180,150,255", // Applewood
            "200,180,160,255", // Pearwood
            "180,160,140,255", // Holly
            "200,200,200,255", // Platinum
            "100,80,60,255",   // Blackthorn
            "180,160,140,255", // Alder
            "120,100,80,255",  // Ironwood
            "150,100,80,255",  // Heartwood
            "160,160,160,255", // Steel
            "200,200,200,255", // Glass
            "200,200,200,255", // Plastic
            "192,192,192,255", // Aluminum
            "150,100,80,255",  // Dragonbone
            "160,160,160,255", // Iron
            "192,192,192,255", // Silver
            "255,215,0,255",   // Gold
            "205,127,50,255"   // Bronze
        };

        return LumberRGBAs[dwIndex];
    }

public:
    TextEntry *m_StringTable;

protected:
};
#endif // __CONSTANTS_H__
