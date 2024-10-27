#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I have a game$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame = new CGame;
}
GIVEN("^I initialize the game$")
{
    ScenarioScope<TestCtx> context;
    context->result = g_pGame->Init("../../JMoria/");
}
GIVEN( "^the game has a player$" )
{
    ScenarioScope<TestCtx> context;
    context->vec = g_pGame->GetPlayer()->m_vPos;
    printf("player pos <%.2f %.2f>\n", VEC_EXPAND(context->vec));
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}

/*#######
##
## WHEN
##
#######*/
WHEN("^I spawn a monster with SEEK$")
{
    ScenarioScope<TestCtx> context;
    context->vec_b.Init(2, 0);
    context->vec_b += context->vec;
    printf("monster pos <%.2f %.2f>\n", VEC_EXPAND(context->vec_b));

    int seek_monster = 10;
    CDungeon *pDungeon = g_pGame->GetDungeon();
    CMonsterDef *pmd = pDungeon->GetMonsterDef(seek_monster);

    context->result = CMonster::CreateMonster(pmd, context->vec_b);

    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    CMonster *pMon = pTile->m_pCurMonster;
    printf("monster spawn pos <%.2f %.2f>, result: %d dungeon: %d, tile: %d, monster: %f\n", VEC_EXPAND(context->vec_b), context->result_bool, pDungeon->m_dwWidth, pTile->m_dwFlags, pMon->m_fHP);

}

/*#######
##
## THEN
##
#######*/

THEN( "^the game initalized successfully$" )
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}

THEN("^the monster spawned successfully$")
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
    CDungeon *pDungeon = g_pGame->GetDungeon();
    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    CMonster *pMon = pTile->m_pCurMonster;
    char monster[32];
    char *expected = "Giant Snake";
    sprintf(monster,"%s", pMon->m_md->m_szName);
    printf("monster spawn pos <%.2f %.2f>, dungeon: %d, tile: %d, monster: %f name: %s\n", VEC_EXPAND(context->vec_b), pDungeon->m_dwWidth, pTile->m_dwFlags, pMon->m_fHP, monster);

    EXPECT_EQ( strcmp(monster, expected),0);
}