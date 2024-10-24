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

    context->result_bool = g_pGame->GetDungeon()->SpawnMonster(10, context->vec_b);
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
    int actual = context->result_bool;
    EXPECT_EQ( actual, true );
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    printf("monster spawn pos <%.2f %.2f> %s\n", VEC_EXPAND(context->vec_b), pMon->GetName());

    EXPECT_EQ( pMon->GetName(), "Giant Snake");
}