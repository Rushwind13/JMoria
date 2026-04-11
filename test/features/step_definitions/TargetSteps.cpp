#include "TestContext.hpp"
using cucumber::ScenarioScope;

#include "Game.h"
#include "JKeys.h"
#include "TargetState.h"

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^I spawn a ([-A-Za-z ]+), a monster behind a wall$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;

    // Place monster 3 tiles to the right of the player
    context->vec_b = playerPos;
    context->vec_b.x += 3;

    // Ensure monster position is open floor
    CDungeonTile *pMonTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    pMonTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );

    // Place a wall between player and monster (1 tile right of player)
    JVector vWall = playerPos;
    vWall.x += 1;
    CDungeonTile *pWallTile = g_pGame->GetDungeon()->GetTile( vWall );
    pWallTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_WALL );

    // Also wall the tile at +2 to ensure full blockage
    JVector vWall2 = playerPos;
    vWall2.x += 2;
    CDungeonTile *pWallTile2 = g_pGame->GetDungeon()->GetTile( vWall2 );
    pWallTile2->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_WALL );

    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( monster.c_str() );
    ASSERT_NE( pmd, nullptr );

    int compare = Util::jstrcmp( monster.c_str(), pmd->m_szName );
    EXPECT_EQ( compare, 0 );

    JIVector vI( VEC_EXPAND( context->vec_b ) );
    context->result = CMonster::CreateMonster( pmd, vI );
}

GIVEN( "^I spawn a ([-A-Za-z ]+), a monster at distance ([0-9]+)$" )
{
    REGEX_PARAM( std::string, monster );
    REGEX_PARAM( int, distance );
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;

    // Place monster 'distance' tiles to the right of the player on open floor
    context->vec_b = playerPos;
    context->vec_b.x += distance;

    // Ensure all tiles between player and monster are open floor (clear LOS)
    for( int i = 1; i <= distance; i++ )
    {
        JVector vClear = playerPos;
        vClear.x += i;
        CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vClear );
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );
        pTile->m_dwFlags |= DUNG_FLAG_SEEN;
    }

    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( monster.c_str() );
    ASSERT_NE( pmd, nullptr );

    int compare = Util::jstrcmp( monster.c_str(), pmd->m_szName );
    EXPECT_EQ( compare, 0 );

    JIVector vI( VEC_EXPAND( context->vec_b ) );
    context->result = CMonster::CreateMonster( pmd, vI );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I enter targeting mode$" )
{
    ScenarioScope<TestCtx> context;

    // Transition to target state, same as CmdState does
    g_pGame->SetState( STATE_TARGET );

    // Send the * key (JKEY_8 + JMOD_SHIFT) to initialize targeting
    JKeysym keysym;
    keysym.sym = JKEY_8;
    keysym.mod = JMOD_SHIFT;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^I cycle to the next target$" )
{
    ScenarioScope<TestCtx> context;

    // Send * key to cycle to next target
    JKeysym keysym;
    keysym.sym = JKEY_8;
    keysym.mod = JMOD_SHIFT;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^I confirm the target$" )
{
    ScenarioScope<TestCtx> context;

    // Send . key to confirm target
    JKeysym keysym;
    keysym.sym = JKEY_PERIOD;
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^I cancel targeting$" )
{
    ScenarioScope<TestCtx> context;

    // Send ESC to cancel targeting
    JKeysym keysym;
    keysym.sym = JKEY_ESCAPE;
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^the targeted monster is removed$" )
{
    ScenarioScope<TestCtx> context;

    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    ASSERT_NE( pTarget, nullptr );
    g_pGame->GetDungeon()->RemoveMonster( pTarget );
}

/*#######
##
## THEN
##
#######*/
THEN( "^the player has a target$" )
{
    ScenarioScope<TestCtx> context;

    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    EXPECT_NE( pTarget, nullptr );
}

THEN( "^the target is a ([-A-Za-z ]+)$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;

    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    ASSERT_NE( pTarget, nullptr );
    int compare = Util::jstrcmp( monster.c_str(), pTarget->GetName() );
    EXPECT_EQ( compare, 0 );
}

THEN( "^targeting mode exits with no targets$" )
{
    ScenarioScope<TestCtx> context;

    // When no targets are found, TargetState resets back to command
    int state = g_pGame->GetGameStateIndex();
    EXPECT_EQ( state, STATE_COMMAND );
}

THEN( "^the game is in command state$" )
{
    ScenarioScope<TestCtx> context;

    int state = g_pGame->GetGameStateIndex();
    EXPECT_EQ( state, STATE_COMMAND );
}

THEN( "^a LOS line is drawn$" )
{
    ScenarioScope<TestCtx> context;

    JLinkList<JIVector> *pLine = g_pGame->GetDungeon()->GetLOSLine();
    ASSERT_NE( pLine, nullptr );
    EXPECT_GT( (int)pLine->length(), 0 );
}

THEN( "^no LOS line is drawn$" )
{
    ScenarioScope<TestCtx> context;

    JLinkList<JIVector> *pLine = g_pGame->GetDungeon()->GetLOSLine();
    EXPECT_EQ( pLine, nullptr );
}

THEN( "^the player has no target$" )
{
    ScenarioScope<TestCtx> context;

    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    EXPECT_EQ( pTarget, nullptr );
}
