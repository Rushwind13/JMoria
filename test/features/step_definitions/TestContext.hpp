#ifndef __TEST_CONTEXT__
#define __TEST_CONTEXT__
#include <JMDefs.h>

#include "FileParse.h"
#include "Item.h"
#include "Monster.h"
#include <AIMgr.h>
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

    // TextEntry
    TextEntry pair;

    // TextEntry
    Constants constants;

    // DungeonMap
    CDungeonMap map;

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
};

#endif // __TEST_CONTEXT__
