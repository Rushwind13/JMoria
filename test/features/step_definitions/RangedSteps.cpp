#include "TestContext.hpp"
using cucumber::ScenarioScope;

#include "Game.h"
#include "JKeys.h"
#include "RangedState.h"

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^the player has a ([-A-Za-z ]+) in slot ([a-z])$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, slot );
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, nullptr );

    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    EXPECT_EQ( compare, 0 );

    // Create item at player position, then pick it up into inventory
    context->result = CItem::CreateItem( pid, playerPos );
    EXPECT_EQ( context->result, JSUCCESS );

    g_pGame->GetPlayer()->PickUp( playerPos );

    // Verify item is in inventory
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    ASSERT_NE( pLink, nullptr );
}

GIVEN( "^the player has a ([-A-Za-z ]+) in slot ([a-z]) with ([0-9]+) charges$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, slot );
    REGEX_PARAM( int, charges );
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, nullptr );

    context->result = CItem::CreateItem( pid, playerPos );
    EXPECT_EQ( context->result, JSUCCESS );

    g_pGame->GetPlayer()->PickUp( playerPos );

    // Set charges to specified amount
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    ASSERT_NE( pLink, nullptr );
    pLink->m_lpData->m_dwCharges = charges;
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I zap slot ([a-z])$" )
{
    REGEX_PARAM( std::string, slot );
    ScenarioScope<TestCtx> context;

    // Enter ranged state with 'z' command
    g_pGame->SetState( STATE_RANGED );

    JKeysym keysym;
    keysym.sym = JKEY_z;
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );

    // Select inventory slot
    keysym.sym = slot[0];
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^I fire slot ([a-z])$" )
{
    REGEX_PARAM( std::string, slot );
    ScenarioScope<TestCtx> context;

    // Enter ranged state with 'f' command
    g_pGame->SetState( STATE_RANGED );

    JKeysym keysym;
    keysym.sym = JKEY_f;
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );

    // Select equipment slot
    keysym.sym = slot[0];
    keysym.mod = 0;
    g_pGame->GetGameState()->HandleKey( &keysym );
}

WHEN( "^the projectile completes its trajectory$" )
{
    ScenarioScope<TestCtx> context;

    // Drive the trajectory animation to completion by calling OnUpdate
    // with sufficient time steps. The failsafe triggers at 20 steps.
    for( int i = 0; i < 25; i++ )
    {
        if( g_pGame->GetGameStateIndex() != STATE_RANGED )
            break;
        g_pGame->GetGameState()->Update( PROJECTILE_UPDATE_INTERVAL );
    }
}

/*#######
##
## THEN
##
#######*/
THEN( "^the ranged hit position matches the target$" )
{
    ScenarioScope<TestCtx> context;

    JVector vHit = g_pGame->GetPlayer()->GetRangedHitPosition();
    // Hit position should be non-zero (a valid hit occurred)
    EXPECT_FALSE( vHit.x == 0 && vHit.y == 0 );
}

THEN( "^the game is in ranged trajectory state$" )
{
    ScenarioScope<TestCtx> context;

    int state = g_pGame->GetGameStateIndex();
    EXPECT_EQ( state, STATE_RANGED );
}

GIVEN( "^a wall exists one tile above the beam path$" )
{
    ScenarioScope<TestCtx> context;

    // The beam travels east from the player; place a wall tile one tile north
    // of the player's position (y-1). The beam will light it as a neighbour.
    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    JVector vWall( playerPos.x + 1, playerPos.y - 1 );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vWall );
    ASSERT_NE( pTile, nullptr );
    pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_WALL );
    pTile->m_dwFlags &= ~( DUNG_FLAG_LIT | DUNG_FLAG_SEEN );
    context->vec = playerPos; // remember for THEN step
}

THEN( "^the tiles between the player and the target are lit$" )
{
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    JVector targetPos = context->vec_b;

    // All floor tiles between player and target (exclusive of player tile)
    // should have DUNG_FLAG_LIT set by LightPosition()
    int minX = (int)playerPos.x + 1;
    int maxX = (int)targetPos.x;
    for( int x = minX; x <= maxX; x++ )
    {
        JVector vCheck( x, playerPos.y );
        CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vCheck );
        ASSERT_NE( pTile, nullptr );
        EXPECT_NE( pTile->m_dwFlags & DUNG_FLAG_LIT, 0u )
            << "Tile at x=" << x << " y=" << (int)playerPos.y << " is not lit";
    }
}

THEN( "^the wall tile adjacent to the beam is lit$" )
{
    ScenarioScope<TestCtx> context;

    // Wall was placed at (player.x+1, player.y-1) in the GIVEN step
    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    JVector vWall( playerPos.x + 1, playerPos.y - 1 );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vWall );
    ASSERT_NE( pTile, nullptr );
    EXPECT_NE( pTile->m_dwFlags & DUNG_FLAG_LIT, 0u ) << "Wall tile adjacent to beam is not lit";
}

THEN( "^the player's current room is lit$" )
{
    ScenarioScope<TestCtx> context;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    CRoom *pRoom = g_pGame->GetDungeon()->InRoom( playerPos );
    ASSERT_NE( pRoom, nullptr ) << "Player is not in a room";
    EXPECT_NE( pRoom->HasFlags( DUNG_FLAG_LIT ), 0u )
        << "Player's room does not have DUNG_FLAG_LIT after wand beam";
}
