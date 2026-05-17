#include "TestContext.hpp"
using cucumber::ScenarioScope;

// Carve a flat floor area and plant a closed door so we can test LOS
// without needing a generated dungeon.
//
// Layout: 13-wide x 8-tall block (cols 44-56, rows 19-26) all floor,
// with a closed door at (doorX, doorY).  The item position (itemX, itemY)
// is one tile north of the door and stays floor — no real CItem is needed
// because the test exercises PlayerCanSee() directly.
static void BuildLOSGeometry( int itemX, int itemY, int doorX, int doorY )
{
    CDungeon *pDung = g_pGame->GetDungeon();
    CDungeonTileDef *pFloor = pDung->GetTileDef( DUNG_IDX_FLOOR );
    CDungeonTileDef *pDoor = pDung->GetTileDef( DUNG_IDX_DOOR );

    // Clear a generously-sized block to floor so every Bresenham path tile
    // between the player positions and the item is passable.
    int x0 = itemX - 6, x1 = itemX + 6;
    int y0 = itemY - 1, y1 = doorY + 5;

    for( int y = y0; y <= y1; y++ )
    {
        for( int x = x0; x <= x1; x++ )
        {
            JIVector v( x, y );
            CDungeonTile *pTile = pDung->GetITile( v );
            if( pTile )
                pTile->m_dtd = pFloor;
        }
    }

    // Plant the closed door — this is the single LOS blocker.
    JIVector vDoor( doorX, doorY );
    CDungeonTile *pDoorTile = pDung->GetITile( vDoor );
    if( pDoorTile )
        pDoorTile->m_dtd = pDoor;
}

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^the LOS geometry has item at ([0-9]+),([0-9]+) and door at ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, itemX );
    REGEX_PARAM( int, itemY );
    REGEX_PARAM( int, doorX );
    REGEX_PARAM( int, doorY );
    BuildLOSGeometry( itemX, itemY, doorX, doorY );
}

GIVEN( "^the player is at ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, px );
    REGEX_PARAM( int, py );
    g_pGame->GetPlayer()->m_vPos.Init( px, py );
}

GIVEN( "^the door at ([0-9]+),([0-9]+) is removed$" )
{
    REGEX_PARAM( int, dx );
    REGEX_PARAM( int, dy );
    JIVector vDoor( dx, dy );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetITile( vDoor );
    if( pTile )
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^I check LOS to ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, tx );
    REGEX_PARAM( int, ty );
    ScenarioScope<TestCtx> context;
    JVector vTarget( tx, ty );
    context->result_bool = g_pGame->GetDungeon()->PlayerCanSee( vTarget );
}

/*#######
##
## THEN
##
#######*/

THEN( "^the LOS result is (true|false)$" )
{
    REGEX_PARAM( std::string, expected );
    ScenarioScope<TestCtx> context;
    if( expected == "true" )
        EXPECT_TRUE( context->result_bool );
    else
        EXPECT_FALSE( context->result_bool );
}
