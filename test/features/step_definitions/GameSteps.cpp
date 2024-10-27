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
GIVEN( "^I initialize the game$" )
{
    ScenarioScope<TestCtx> context;
    context->result = g_pGame->Init( "../../JMoria/" );
}
GIVEN( "^the game has a player$" )
{
    ScenarioScope<TestCtx> context;
    context->vec = g_pGame->GetPlayer()->m_vPos;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I spawn a monster with SEEK$" )
{
    ScenarioScope<TestCtx> context;
    context->vec_b.Init( 2, 0 );
    context->vec_b += context->vec;

    int seek_monster = 10;
    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( seek_monster );

    context->result = CMonster::CreateMonster( pmd, context->vec_b );
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

THEN( "^the monster spawned successfully$" )
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
    CDungeon *pDungeon = g_pGame->GetDungeon();
    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    CMonster *pMon = pTile->m_pCurMonster;
    char monster[32];
    char *expected = "Giant Snake";
    sprintf( monster, "%s", pMon->m_md->m_szName );

    EXPECT_EQ( strcmp( monster, expected ), 0 );
}