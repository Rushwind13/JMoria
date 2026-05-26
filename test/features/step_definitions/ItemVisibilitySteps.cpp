#include "TestContext.hpp"
using cucumber::ScenarioScope;

// Carve a clear floor region centered at (cx, cy) with radius 12 so that all
// PlayerCanSee paths between player and items within sight distance are
// unobstructed.  No rooms are registered — these are raw floor tiles only.
static void BuildItemVisibilityGeometry( int cx, int cy )
{
    CDungeon *pDung = g_pGame->GetDungeon();
    CDungeonTileDef *pFloor = pDung->GetTileDef( DUNG_IDX_FLOOR );

    const int radius = 12;
    for( int y = cy - radius; y <= cy + radius; y++ )
    {
        for( int x = cx - radius; x <= cx + radius; x++ )
        {
            JIVector v( x, y );
            CDungeonTile *pTile = pDung->GetITile( v );
            if( pTile )
                pTile->m_dtd = pFloor;
        }
    }
}

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^the item visibility geometry is centered at ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, cx );
    REGEX_PARAM( int, cy );
    BuildItemVisibilityGeometry( cx, cy );
}
