#ifndef __TEST_CONTEXT__
#define __TEST_CONTEXT__
#include <gtest/gtest.h>

#include <cucumber-cpp/autodetect.hpp>

#include <JMDefs.h>

#include "FileParse.h"
#include "Item.h"
#include "Monster.h"
#include <AIMgr.h>
#include <Constants.h>
#include <Dungeon.h>
#include <DungeonMap.h>
#include <JRect.h>
#include <JVector.h>
#include <Player.h>
#include <TextEntry.h>
#include <TileSet.h>

#include <vector>

// main game pointer
CGame *g_pGame = NULL;
eLogLevel g_eLogLevel = LOG_LEVEL_DEBUG;

/*#######
##
## CONTEXT
##
#######*/
struct DungeonSnapshot
{
    int rooms;
    int hallways;
    int stack_depth;
};

struct TestCtx
{
    JVector vec;
    JVector vec_b;
    JIVector vec_i;

    JRect area;

    // Tileset
    CTileset *tileset;
    char szTileset[1024];
    JIVector aspect;
    JIVector vTile;

    // TextEntry
    TextEntry pair;

    // TextEntry
    Constants constants;

    // DungeonMap
    CDungeonMap map;
    CDungeonCreationStep *pStep;
    CRoom *pRoom;
    
    // Stress testing
    std::vector<DungeonSnapshot> dungeon_history;

    // AI Brain
    CAIBrain *brain;
    // ItemDef
    CItemDef *ItemDef;
    JResult Success;

    // MonsterDef
    CDataFile dfMonsters;
    CMonsterDef *monsterDef;
    CMonster *monster;

    // All Monsters
    JLinkList<CMonsterDef> *m_llMonsterDefs = new JLinkList<CMonsterDef>;
    JLinkList<CMonster> *m_llMonsters = new JLinkList<CMonster>;

    // RESULTS
    bool result_bool;
    int result_int;
    float result_float;
    JResult result;

    int index;
    int lo;
    int hi;
    int lo_f;
    int hi_f;

    char szBuffer[1024];
};

#endif // __TEST_CONTEXT__
