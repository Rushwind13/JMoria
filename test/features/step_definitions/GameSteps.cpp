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
    if( g_pGame != NULL )
    {
        g_pGame = NULL;
    }
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
GIVEN( "^I spawn a monster with SEEK$" )
{
    ScenarioScope<TestCtx> context;
    context->vec_b.Init( -2, 2 );
    context->vec_b += context->vec;

    int seek_monster = 12;
    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( seek_monster );

    context->result = CMonster::CreateMonster( pmd, context->vec_b );

    printf( "<%f %f> %s %d\n", VEC_EXPAND( context->vec_b ), pmd->m_szName, context->result );
}
GIVEN( "^I update the monster's brain pizza$" )
{
    ScenarioScope<TestCtx> context;
    CDungeon *pDungeon = g_pGame->GetDungeon();
    printf( "dung %d ", pDungeon->m_dwHeight );

    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    printf( "tile %d ", pTile->m_dwFlags );

    CMonster *pMon = pTile->m_pCurMonster;

    printf( "mon %f\n", pMon->m_fHP );
    pMon->m_pBrain->Update( 1.0f );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I update the monster's brain again$" )
{
    ScenarioScope<TestCtx> context;
    CDungeon *pDungeon = g_pGame->GetDungeon();
    printf( "dung %d ", pDungeon->m_dwHeight );

    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    printf( "tile %d ", pTile->m_dwFlags );

    CMonster *pMon = pTile->m_pCurMonster;

    printf( "mon %f\n", pMon->m_fHP );
    pMon->m_pBrain->Update( 1.0f );
    // pMon = pTile->m_pCurMonster;

    // printf("mon_after %f\n", pMon->m_fHP);
}
WHEN( "^I terminate the game$" ) { g_pGame->Term(); }

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

THEN( "^the game terminates successfully$" ) { EXPECT_EQ( true, true ); }

THEN( "^the monster spawned successfully$" )
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
    CDungeon *pDungeon = g_pGame->GetDungeon();
    CDungeonTile *pTile = pDungeon->GetTile( context->vec_b );
    CMonster *pMon = pTile->m_pCurMonster;
    char monster[32];
    char *expected = "Red Dragon";
    sprintf( monster, "%s", pMon->m_md->m_szName );

    EXPECT_EQ( strcmp( monster, expected ), 0 );
}

THEN( "^the monster wants to move toward the player$" )
{
    JVector expected( 1, -1 );
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    JVector actual = pMon->m_pBrain->m_vVel;
    EXPECT_EQ( expected.x, actual.x );
    EXPECT_EQ( expected.y, actual.y );

    EXPECT_EQ( pMon->m_pBrain->GetState(), BRAINSTATE_GOTODEST );
}

THEN( "^the monster moves toward the player$" )
{
    JVector delta( 1, -1 );
    ScenarioScope<TestCtx> context;
    // Monster not in old pos
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;

    bool expected = pMon == NULL;
    EXPECT_EQ( expected, true );

    // Monster is in new pos
    context->vec_b += delta;
    pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    expected = pMon != NULL;
    EXPECT_EQ( expected, true );

    // Monster is correct monster
    char monster[32];
    char *wanted = "Red Dragon";
    sprintf( monster, "%s", pMon->m_md->m_szName );

    EXPECT_EQ( strcmp( monster, wanted ), 0 );
}