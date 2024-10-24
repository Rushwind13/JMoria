#ifndef __TEST_CONTEXT__
#define __TEST_CONTEXT__

// #include <JMDefs.h>
#include "FileParse.h"
#include "Item.h"
#include "Monster.h"
#include <Constants.h>
#include <DungeonMap.h>
#include <JRect.h>
#include <JVector.h>
#include <TextEntry.h>
#include <TileSet.h>

// main game pointer
CGame *g_pGame = NULL;

/*#######
##
## CONTEXT
##
#######*/
struct TestCtx
{
    JVector vec;
    JVector vec_b;

    JRect area;

    // Tileset
    CTileset *tileset;
    char szTileset[1024];
    JIVector aspect;
    JIVector vTile;

    JIVector origin;

    // TextEntry
    TextEntry pair;

    // TextEntry
    Constants constants;

    // DungeonMap
    CDungeonMap map;
    CDungeonCreationStep *dungeonCreationStep;
    bool areaChecked;
    int direction;
    JRect room;

    // MonsterDef
    CDataFile dfMonsters;
    CMonsterDef *monsterDef;
    CMonster *monster;

    // All Monsters
    JLinkList<CMonsterDef> *m_llMonsterDefs = new JLinkList<CMonsterDef>;
    JLinkList<CMonster> *m_llMonsters = new JLinkList<CMonster>;

    // ItemDef
    CItemDef *ItemDef;
    JResult Success;

    int index;
    int lo;
    int hi;
    int lo_f;
    int hi_f;

    // RESULTS
    int result_int;
    float result_float;
    JResult result;
};

#endif // __TEST_CONTEXT__
