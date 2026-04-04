
#include "DungeonMap.h"
#include "DisplayText.h"
#include <ctime>

// Uncomment or set via -DDUNGEN_DEBUG compiler flag to enable detailed diagnostics
// #define DUNGEN_DEBUG

#ifdef FIXED_DUNGEON
Uint8 dungeontiles[DUNG_HEIGHT][DUNG_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 8, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2,
    1, 1, 1, 0, 7, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 6, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; /* */
#endif

// Use the following algorithm to create a randomized dungeon
// filled with rooms (of various types) and corridors:
// 1.  Fill the whole map with solid earth
// 2.  Dig out a single room in the centre of the map
// 3.  Pick a wall of any room
// 4.  Decide upon a new feature to build
// 5.  See if there is room to add the new feature through the chosen wall
// 6.  If yes, continue. If no, go back to step 3
// 7.  Add the feature through the chosen wall
// 8.  Go back to step 3, until the dungeon is complete
// 9.  Add the up and down staircases at random points in map
// 10. Finally, sprinkle some monsters and items liberally over dungeon
void CDungeonMap::CreateDungeon( const int depth )
{
    Term();
    m_dmtTiles = new CDungeonMapTile[DUNG_WIDTH * DUNG_HEIGHT];
    m_llRooms = new JLinkList<CRoom>;
    m_llHallways = new JLinkList<CRoom>;
    JRect rcWorld( 0, 0, DUNG_WIDTH - 1, DUNG_HEIGHT - 1 );
    m_dwDepth = depth;

    // Capture current RNG seed for determinism and debugging
    m_dwSeed = Util::GetRandomSeed();

    // Warn if seed appears uninitialized (could cause subtle determinism issues)
    if( m_dwSeed == 0 )
    {
        JLog(
            LOG_LEVEL_WARN, false,
            "[DUNGEN] Warning: RNG seed is 0, generation may be unintentionally deterministic\n" );
    }

    // First, fill the whole dungeon with rock
    FillDungeonArea( DUNG_IDX_WALL, rcWorld, false );

    // Do something with the depth, here...
    if( depth > 100 )
    {
        JLog( LOG_LEVEL_INFO, false, "You have a bad feeling about this level...\n" );
    }
    else if( depth > 75 )
    {
        JLog( LOG_LEVEL_INFO, false, "Just another walk in the park.\n" );
    }
    else if( depth > 50 )
    {
        JLog( LOG_LEVEL_INFO, false,
              "It is my firm belief that this level contains monsters, of one kind or another.\n" );
    }
    else if( depth > 25 )
    {
        JLog( LOG_LEVEL_INFO, false,
              "This level goes together like wham-a-lamma-lamma and bop-she-bop-she-bop.\n" );
    }
    else if( depth > 10 )
    {
        JLog( LOG_LEVEL_INFO, false, "What was *that*?!.\n" );
    }
    else if( depth > 5 )
    {
        JLog( LOG_LEVEL_INFO, false, "Please keep hands and arms inside the carriage.\n" );
    }
    else if( depth <= 0 )
    {
        JLog( LOG_LEVEL_ERROR, false, "Error, levels don't go below 0.\n" );
    }
#ifdef FIXED_DUNGEON
    //    For setpiece rooms, treasure rooms, &c
    JIVector vCurPos;
    int type;
    for( vCurPos.y = 0; vCurPos.y < DUNG_HEIGHT; vCurPos.y++ )
    {
        for( vCurPos.x = 0; vCurPos.x < DUNG_WIDTH; vCurPos.x++ )
        {
            type = dungeontiles[vCurPos.y][vCurPos.x];
            GetTile( vCurPos )->SetType( type );
            // if( type != DUNG_IDX_WALL )
            {
                GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );
            }
        }
    }
#else

    // Next, carve out a room in the middle
    JIVector vPos( Util::GetRandom( DUNG_ROOM_MAXWIDTH, DUNG_WIDTH - DUNG_ROOM_MAXWIDTH - 1 ),
                   Util::GetRandom( DUNG_ROOM_MAXHEIGHT, DUNG_HEIGHT - DUNG_ROOM_MAXHEIGHT - 1 ) );
    InitDungeonCreate( vPos );
#endif
}

// Deterministic overload for testing - seeds RNG with explicit value
void CDungeonMap::CreateDungeon( const int depth, const unsigned int seed )
{
    // Seed RNG for deterministic generation
    Util::SeedRandom( seed );

    // Call regular CreateDungeon which will pick up the seed
    CreateDungeon( depth );
}

bool CDungeonMap::CheckArea( CDungeonCreationStep *pStep )
{
    bool bInteriorOK = false;
    bool bBorderOK = false;
    bool bIsHallway = pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_HALLWAY;
    int direction = bIsHallway ? pStep->m_dwDirection : DIR_NONE;

    bInteriorOK = CheckInterior( pStep->m_rcArea );
    if( bInteriorOK )
        bBorderOK = CheckBorder( pStep->m_rcArea, direction );
    return bInteriorOK && bBorderOK;
}

bool CDungeonMap::CheckInterior( const JRect area )
{
    JIVector vCheck;

    for( vCheck.y = area.top; vCheck.y <= area.bottom; vCheck.y++ )
    {
        for( vCheck.x = area.left; vCheck.x <= area.right; vCheck.x++ )
        {
            if( GetTile( vCheck )->GetType() != DUNG_IDX_WALL )
            {
                JLog( LOG_LEVEL_NOISE, true,
                      "interior check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n",
                      RECT_EXPAND( area ), VEC_EXPAND( vCheck ), GetTile( vCheck )->GetType() );
                return false;
            }
        }
    }
    return true;
}

void TweakBorders( JRect &rcIn, int direction )
{
    switch( direction )
    {
    case DIR_NORTH:
        rcIn.bottom--;
        rcIn.top--;
        break;
    case DIR_SOUTH:
        rcIn.top++;
        rcIn.bottom++;
        break;
    case DIR_WEST:
        rcIn.right--;
        rcIn.left--;
        break;
    case DIR_EAST:
        rcIn.left++;
        rcIn.right++;
        break;
    default:
        break;
    }
}

// CheckBorder: Validates that a 1-tile border around a proposed area is suitable
// for room/hallway placement. Border tiles must be either:
// - DUNG_IDX_WALL (solid rock): Indicates space for new construction
// - Doors: Allows connecting to existing rooms/hallways through doorways
// This enables natural dungeon connectivity while preventing room overlaps.
bool CDungeonMap::CheckBorder( const JRect area, int direction )
{
    JRect rcEdges( area.left - 1, area.top - 1, area.right + 1, area.bottom + 1 );
    if( !rcEdges.IsInWorld() )
    {
        JLog( LOG_LEVEL_NOISE, true, "border failed: <%d %d, %d %d>, edges fail.\n",
              RECT_EXPAND( rcEdges ) );
        return false;
    }
    JIVector vCheck( rcEdges.left, rcEdges.top );
    for( int y = rcEdges.top; y <= rcEdges.bottom; y++ )
    {
        vCheck.y = y;
        for( int x = rcEdges.left; x <= rcEdges.right; x++ )
        {
            vCheck.x = x;
            int type = GetTile( vCheck )->GetType();
            // Allow walls (space for new construction) and doors (connecting points)
            if( type != DUNG_IDX_WALL && !IsDoor( type ) )
            {
                JLog( LOG_LEVEL_NOISE, true,
                      "border check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n",
                      RECT_EXPAND( area ), VEC_EXPAND( vCheck ), GetTile( vCheck )->GetType() );
                // Reject if border contains non-wall, non-door tiles (floor, stairs, etc.)
                // This prevents room overlaps while allowing door connections
                return false;
            }
        }
    }

    return true;
}

bool CDungeonMap::IsDoor( int type )
{
    switch( type )
    {
    case DUNG_IDX_DOOR:
    case DUNG_IDX_OPEN_DOOR:
    case DUNG_IDX_SECRET_DOOR:
        return true;
    default:
        return false;
    }
}

JResult CDungeonMap::LightArea( CRoom *pRoom )
{
    if( !Util::IsInWorld( pRoom->GetEdges() ) )
    {
        return JBOGUSKEY;
    }

    JIVector vCurPos;
    for( vCurPos.y = pRoom->GetEdges().top; vCurPos.y <= pRoom->GetEdges().bottom; vCurPos.y++ )
    {
        for( vCurPos.x = pRoom->GetEdges().left; vCurPos.x <= pRoom->GetEdges().right; vCurPos.x++ )
        {
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );
        }
    }
    pRoom->SetFlags( DUNG_FLAG_LIT );
    return JSUCCESS;
}

JResult CDungeonMap::FillDungeonArea( Uint8 type, JRect rcFill, bool bBoundsCheck )
{
    if( bBoundsCheck && !Util::IsWithinWorld( rcFill ) )
    {
        return JBOGUSKEY;
    }

    // Pre-fill invariant: if we're filling with non-wall, verify all interior tiles were walls
    if( type != DUNG_IDX_WALL )
    {
        for( int y = rcFill.top; y <= rcFill.bottom; y++ )
        {
            for( int x = rcFill.left; x <= rcFill.right; x++ )
            {
                JIVector vPos( x, y );
                if( vPos.IsInWorld() && GetTile( vPos )->GetType() != DUNG_IDX_WALL )
                {
                    JLog( LOG_LEVEL_WARN, true,
                          "[DUNGEN] Pre-fill invariant violation at <%d %d>: "
                          "expected wall, found type %d\n",
                          x, y, GetTile( vPos )->GetType() );
                }
            }
        }
    }
    JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] FillArea type=%d <%d %d, %d %d>\n", type,
          RECT_EXPAND( rcFill ) );

    JIVector vCurPos;
    for( vCurPos.y = rcFill.top; vCurPos.y <= rcFill.bottom; vCurPos.y++ )
    {
        for( vCurPos.x = rcFill.left; vCurPos.x <= rcFill.right; vCurPos.x++ )
        {
            GetTile( vCurPos )->SetType( type );
        }
    }

    m_diagnostics.fill_operations++;

    return JSUCCESS;
}
void CDungeonMap::FillArea( const CDungeonCreationStep *pStep )
{
    CRoom *pRoom = new CRoom( pStep->m_rcArea );

    if( pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM )
    {
        pRoom->SetFlags( DUNG_FLAG_ROOM );
        m_llRooms->Add( pRoom );
    }
    else
    {
        pRoom->SetFlags( DUNG_FLAG_HALL );
        m_llHallways->Add( pRoom );
    }

    FillArea( DUNG_IDX_FLOOR, pRoom );
}
void CDungeonMap::FillArea( const Uint8 type, CRoom *pRoom )
{
    FillDungeonArea( type, pRoom->GetArea() );

    if( type != DUNG_IDX_WALL && pRoom->HasFlags( DUNG_FLAG_ROOM ) != 0 )
    {
        if( Util::GetRandom( 1, 100 ) < LitChance() )
        {
            LightArea( pRoom );
        }
        else
        {
            JLog( LOG_LEVEL_WARN, true, "Created unlit room\n" );
        }
    }

    // you still filled rcFill squares, just that one of them was a door.
    JLog( LOG_LEVEL_DEBUG, true, "filled from <%d %d> to <%d %d>\n",
          RECT_EXPAND( pRoom->GetArea() ) );
}

int CDungeonMap::LitChance()
{
    const float maxChance = 0.75f;
    const int deepestLit = 20;
    if( m_dwDepth > deepestLit )
        return 0;
    float chance =
        maxChance - ( ( (float)( m_dwDepth - 1 ) * maxChance ) / (float)( deepestLit - 1 ) );
    return (int)( chance * 100.0f );
}

int CDungeonMap::Opposite( int direction )
{
    switch( direction )
    {
    case DIR_NORTH:
        return DIR_SOUTH;
        break;
    case DIR_SOUTH:
        return DIR_NORTH;
        break;
    case DIR_WEST:
        return DIR_EAST;
        break;
    case DIR_EAST:
        return DIR_WEST;
        break;
    case DIR_NONE:
        return DIR_NONE;
        break;
    }
    return DIR_NONE;
}

// Get the two adjacent directions to a given cardinal direction
// For example, DIR_NORTH has adjacent directions DIR_WEST and DIR_EAST
void CDungeonMap::GetAdjacentDirections( int primary_dir, int &adj1, int &adj2 ) const
{
    switch( primary_dir )
    {
    case DIR_NORTH:
        adj1 = DIR_WEST;
        adj2 = DIR_EAST;
        break;
    case DIR_SOUTH:
        adj1 = DIR_EAST;
        adj2 = DIR_WEST;
        break;
    case DIR_WEST:
        adj1 = DIR_NORTH;
        adj2 = DIR_SOUTH;
        break;
    case DIR_EAST:
        adj1 = DIR_SOUTH;
        adj2 = DIR_NORTH;
        break;
    default:
        adj1 = DIR_NONE;
        adj2 = DIR_NONE;
        break;
    }
}

void RandomDirections( int r[] )
{
    // Cardinal directions only: NORTH=0, EAST=2, SOUTH=4, WEST=6
    static const int cardinals[4] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };
    for( int i = 0; i < 4; ++i )
    {
        r[i] = cardinals[i];
    }

    for( int i = 3; i >= 0; --i )
    {
        // generate a random number [0, n-1]
        int j = rand() % ( i + 1 );

        // swap the last element with element at random index
        int temp = r[i];
        r[i] = r[j];
        r[j] = temp;
    }
}
bool CDungeonMap::ProcessStep()
{
    CLink<CDungeonCreationStep> *pLink = m_stkDungeonMapCreation->Pop();
    if( pLink == NULL || pLink->m_lpData == NULL )
    {
        // Finalize timing when generation completes
        m_diagnostics.end_time_ms = Util::GetTimeInMillis();
        m_diagnostics.total_time_ms = m_diagnostics.end_time_ms - m_diagnostics.start_time_ms;

        if( g_pGame )
            g_pGame->GetStats()->Printf( "Dungeon creation complete.\n" );
        JLog( LOG_LEVEL_INFO, true,
              "[DUNGEN] Generation complete: %d steps, %d rooms, %d halls, "
              "%d skipped, %d fill ops, %d abandoned, %d truncated\n",
              m_diagnostics.steps_created, m_diagnostics.rooms_created,
              m_diagnostics.hallways_created, m_diagnostics.steps_skipped,
              m_diagnostics.fill_operations, m_diagnostics.repeated_failures,
              m_diagnostics.hallways_truncated );
        JLog( LOG_LEVEL_INFO, true, "[DUNGEN] Total generation time: %.2f ms\n",
              m_diagnostics.total_time_ms );
        return false;
    }
    CDungeonCreationStep *pCurStep = pLink->m_lpData;
    CDungeonCreationStep *pNewStep = NULL;

    m_diagnostics.steps_created++;
    JLog( LOG_LEVEL_NOISIER, true,
          "[DUNGEN] Step %d: creating %s at <%d %d, %d %d> (depth=%d, fail_count=%d)\n",
          m_diagnostics.steps_created,
          pCurStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hallway",
          RECT_EXPAND( pCurStep->m_rcArea ), pCurStep->m_dwRecurDepth, pCurStep->m_dwFailureCount );

    JLog( LOG_LEVEL_DEBUG, true, "creating %d %s at <%d %d, %d %d>\n", pCurStep->m_dwDirection,
          pCurStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hallway",
          RECT_EXPAND( pCurStep->m_rcArea ) );
    // create current entity
    FillArea( pCurStep );

    // Dispatch to appropriate processor based on step type
    switch( pCurStep->m_dwIndex )
    {
    case DUNG_CREATE_STEP_MAKE_ROOM:
        ProcessRoom( pCurStep );
        break;
    case DUNG_CREATE_STEP_MAKE_HALLWAY:
        ProcessHallway( pCurStep );
        break;
    }

    m_stkDungeonMapCreation->Remove( pLink );

    return true;
}

// Process room creation: attempt to create 2-4 hallways from random walls
// If all attempts fail (dead-end), try one backtracking hallway
// Process room creation: attempt to create 2-4 hallways from random walls
// If all attempts fail (dead-end), try one backtracking hallway
void CDungeonMap::ProcessRoom( CDungeonCreationStep *pCurStep )
{
    // Create 2-4 hallways in random directions
    int num_halls = Util::GetRandom( 2, 4 );
    int halls_created = 0;
    int halls_failed = 0;

    ExpandInRandomDirections( pCurStep, num_halls, DUNG_CREATE_STEP_MAKE_HALLWAY, false,
                              &halls_created, &halls_failed );

    // Track repeated failures: if all hallway attempts failed, this is a dead-end branch
    if( halls_created == 0 && halls_failed > 0 )
    {
        pCurStep->m_dwFailureCount++;
        m_diagnostics.repeated_failures++;
        JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] Dead-end room: all %d hallway attempts failed\n",
              halls_failed );

        // Mitigation: allow a single backtracking hallway in the opposite direction
        // This provides an alternate growth path to reduce tails-out dead ends.
        if( pCurStep->m_dwDirection != DIR_NONE )
        {
            int back_dir = Opposite( pCurStep->m_dwDirection );
            JIVector vHallBack = GetWallOrigin( pCurStep, back_dir );
            if( vHallBack.IsWithinWorld() )
            {
                CDungeonCreationStep *pBackStep =
                    CreateHallway( vHallBack, back_dir, pCurStep->m_dwRecurDepth + 1 );
                if( pBackStep != NULL )
                {
                    AddDoor( vHallBack, back_dir );
                    m_stkDungeonMapCreation->Push( pBackStep );
                    m_diagnostics.hallways_created++;
                    JLog( LOG_LEVEL_NOISIER, true,
                          "[DUNGEN] Backtracking hallway created to mitigate dead-end room\n" );
                }
                else
                {
                    m_diagnostics.steps_skipped++;
                    JLog( LOG_LEVEL_NOISIER, true,
                          "[DUNGEN] Backtracking hallway creation failed\n" );
                }
            }
        }
    }
}

// Process hallway creation: 80% chance to create a room, 20% chance to branch hallways
// Uses alternate direction fallback for room creation
void CDungeonMap::ProcessHallway( CDungeonCreationStep *pCurStep )
{
    int pick_next = Util::Roll( "1d100" );

    if( pick_next <= HALLWAY_LEADS_TO_ROOM_PERCENT )
    {
        // Make a (single) room, with alternate direction fallback if needed
        TryCreateRoomWithFallback( pCurStep );
    }
    else if( pick_next <= 100 )
    {
        // Make 2-4 branch hallways in random directions
        int num_halls = Util::GetRandom( 2, 4 );
        ExpandInRandomDirections( pCurStep, num_halls, DUNG_CREATE_STEP_MAKE_HALLWAY, false );
    }
}

void CDungeonMap::AddDoor( const JIVector vHall, int direction )
{
    // room-to-hallway transition means skip over room wall
    // ...#
    // ...####
    // ...X... need to turn X into a DOOR type
    // ...####
    // ...#
    const int normal = 80;
    const int open = 95;
    JIVector vDoor( vHall.x, vHall.y );
    switch( direction )
    {
    case DIR_NORTH:
        vDoor.y += DOOR_OFFSET; // Move 1 tile inward from hallway start to room wall
        break;
    case DIR_SOUTH:
        vDoor.y -= DOOR_OFFSET; // Move 1 tile inward from hallway start to room wall
        break;
    case DIR_WEST:
        vDoor.x += DOOR_OFFSET; // Move 1 tile inward from hallway start to room wall
        break;
    case DIR_EAST:
        vDoor.x -= DOOR_OFFSET; // Move 1 tile inward from hallway start to room wall
        break;
    case DIR_NONE:
        break;
    }

    if( !vDoor.IsInWorld() )
        return;

    // What kind of door?
    int door_type = DUNG_IDX_DOOR;
    int roll = Util::Roll( "1d100" );
    char flavor[32];
    if( roll <= normal )
    {
        sprintf( flavor, "" );
        door_type = DUNG_IDX_DOOR;
    }
    else if( roll <= open )
    {
        sprintf( flavor, "open " );
        door_type = DUNG_IDX_OPEN_DOOR;
    }
    else
    {
        sprintf( flavor, "secret " );
        door_type = DUNG_IDX_SECRET_DOOR;
    }
    JLog( LOG_LEVEL_DEBUG, false, "Placing a %sdoor at <%d %d>\n", flavor, VEC_EXPAND( vDoor ) );
    GetTile( vDoor )->SetType( door_type );
}

void CDungeonMap::InitDungeonCreate( JIVector &vOrigin )
{
    // Start timing for performance measurement
    m_diagnostics.start_time_ms = Util::GetTimeInMillis();

    CDungeonCreationStep *step = CreateRoom( vOrigin, DIR_NONE, 0 );
    m_stkDungeonMapCreation->Push( step );
}

// Helper: Try to create a room from a hallway with alternate direction fallback
// Returns true if room was successfully created and pushed to stack
bool CDungeonMap::TryCreateRoomWithFallback( CDungeonCreationStep *pCurStep )
{
    int dir = pCurStep->m_dwDirection;
    JIVector vRoom = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_ROOM );

    if( !vRoom.IsWithinWorld() )
        return false;

    // Try primary direction first
    CDungeonCreationStep *pNewStep = CreateRoom( vRoom, dir, pCurStep->m_dwRecurDepth + 1 );

    // If primary fails, try adjacent directions
    if( pNewStep == NULL )
    {
        int adj1, adj2;
        GetAdjacentDirections( dir, adj1, adj2 );

        // Try first adjacent direction
        JIVector vRoomAdj1 = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_ROOM );
        if( vRoomAdj1.IsWithinWorld() )
        {
            pNewStep = CreateRoom( vRoomAdj1, adj1, pCurStep->m_dwRecurDepth + 1 );
            if( pNewStep != NULL )
                dir = adj1;
        }

        // Try second adjacent direction if first failed
        if( pNewStep == NULL )
        {
            JIVector vRoomAdj2 = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_ROOM );
            if( vRoomAdj2.IsWithinWorld() )
            {
                pNewStep = CreateRoom( vRoomAdj2, adj2, pCurStep->m_dwRecurDepth + 1 );
                if( pNewStep != NULL )
                    dir = adj2;
            }
        }

        if( pNewStep != NULL )
        {
            JLog( LOG_LEVEL_NOISIER, true,
                  "[DUNGEN] Room creation succeeded via alternate direction fallback\n" );
        }
    }

    // If we got a room, push it to stack
    if( pNewStep != NULL )
    {
        AddDoor( vRoom, dir );
        m_stkDungeonMapCreation->Push( pNewStep );
        m_diagnostics.rooms_created++;
        JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] Room created from hallway, pushed to stack\n" );
        return true;
    }
    else
    {
        m_diagnostics.steps_skipped++;
        JLog( LOG_LEVEL_NOISIER, true,
              "[DUNGEN] Room creation failed after trying primary and adjacent directions\n" );
        return false;
    }
}

// Helper: Expand a parent step by creating N children in random directions
// Handles the common pattern of: random directions, opposite check, create, push to stack
void CDungeonMap::ExpandInRandomDirections( CDungeonCreationStep *pParent, int num_children,
                                            int child_step_type, bool allow_backtracking,
                                            int *out_created, int *out_failed )
{
    const char *child_type_name =
        ( child_step_type == DUNG_CREATE_STEP_MAKE_ROOM ) ? "Room" : "Hallway";
    int dirs[4];
    RandomDirections( dirs );
    int created = 0;
    int failed = 0;

    for( int index = 0; index <= num_children; index++ )
    {
        int dir = dirs[index];

        // Skip opposite direction unless backtracking is allowed
        if( !allow_backtracking && pParent->m_dwDirection == Opposite( dir ) )
        {
            if( num_children < 4 )
                num_children++;
            continue;
        }

        // Get origin point based on child type
        JIVector vOrigin = ( child_step_type == DUNG_CREATE_STEP_MAKE_ROOM )
                               ? GetHallOrigin( pParent, DUNG_CREATE_STEP_MAKE_ROOM )
                               : GetWallOrigin( pParent, dir );

        if( !vOrigin.IsWithinWorld() )
            continue;

        // Create child step
        CDungeonCreationStep *pChild =
            CreateStep( child_step_type, vOrigin, dir, pParent->m_dwRecurDepth + 1 );

        if( pChild != NULL )
        {
            AddDoor( vOrigin, dir );
            m_stkDungeonMapCreation->Push( pChild );
            created++;

            // Update diagnostics
            if( child_step_type == DUNG_CREATE_STEP_MAKE_ROOM )
                m_diagnostics.rooms_created++;
            else
                m_diagnostics.hallways_created++;

            JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] %s created, pushed to stack\n",
                  child_type_name );
        }
        else
        {
            failed++;
            m_diagnostics.steps_skipped++;
            JLog( LOG_LEVEL_NOISIER, true,
                  "[DUNGEN] %s creation failed (conflict or depth limit)\n", child_type_name );
        }
    }

    // Return counts if requested
    if( out_created )
        *out_created = created;
    if( out_failed )
        *out_failed = failed;
}

// Unified step creation with type-specific behavior
// Handles depth checking, initialization, placement loop, and conflict resolution
CDungeonCreationStep *CDungeonMap::CreateStep( int step_type, const JIVector &vPos,
                                               const int direction, const int recurdepth )
{
    const char *type_name = ( step_type == DUNG_CREATE_STEP_MAKE_ROOM ) ? "Room" : "Hall";

    // Check recursion depth
    if( recurdepth > MAX_RECURDEPTH )
    {
        JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] Create%s rejected: recursion depth %d > %d\n",
              type_name, recurdepth, MAX_RECURDEPTH );
        return NULL;
    }

    JLog( LOG_LEVEL_DEBUG, true, "Creating a %s step\n",
          step_type == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hall" );

    // Initialize step
    CDungeonCreationStep *pStep = new CDungeonCreationStep();
    pStep->m_dwIndex = step_type;
    pStep->m_dwDirection = direction;
    pStep->m_dwRecurDepth = recurdepth;
    pStep->m_vPos.Init( vPos.x, vPos.y );
    pStep->m_rcArea.Init( pStep->m_vPos, 0, 0 );

    // Placement attempt loop
    bool bPlacementSucceeded = false;
    int attempt_count = 0;
    JRect rcTry( pStep->m_rcArea );

    while( !bPlacementSucceeded && attempt_count < MAX_TRIES )
    {
        // Get geometry based on step type
        JResult rectResult = ( step_type == DUNG_CREATE_STEP_MAKE_ROOM )
                                 ? GetRoomRect( pStep->m_rcArea, pStep->m_dwDirection )
                                 : GetHallRect( pStep->m_rcArea, pStep->m_dwDirection );

        if( rectResult != JSUCCESS )
        {
            // Geometry generation failed (clamping or degenerate rect)
            JLog( LOG_LEVEL_WARN, true, "[DUNGEN] Create%s: Get%sRect failed on attempt %d\n",
                  type_name, type_name, attempt_count + 1 );
            pStep->m_rcArea.Init( rcTry );
            attempt_count++;
            continue;
        }

        // Check for conflicts
        bPlacementSucceeded = CheckArea( pStep );
        if( !bPlacementSucceeded )
        {
            // For hallways, try truncating to connect to the existing structure
            if( step_type == DUNG_CREATE_STEP_MAKE_HALLWAY )
            {
                JRect rcBeforeTruncate( pStep->m_rcArea );
                if( TruncateHallway( pStep ) == JSUCCESS )
                {
                    bPlacementSucceeded = true;
                }
                else
                {
                    pStep->m_rcArea.Init( rcBeforeTruncate );
                }
            }
        }
        if( !bPlacementSucceeded )
        {
            // Log conflict and restore rect for next attempt
            JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] %s attempt %d conflict at <%d %d, %d %d>\n",
                  type_name, attempt_count + 1, RECT_EXPAND( pStep->m_rcArea ) );
            JLog( LOG_LEVEL_NOISE, true, "un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n",
                  RECT_EXPAND( pStep->m_rcArea ), RECT_EXPAND( rcTry ) );
            pStep->m_rcArea.Init( rcTry );
        }
        attempt_count++;
    }

    // Handle placement failure
    if( !bPlacementSucceeded )
    {
        JLog( LOG_LEVEL_NOISIER, true, "[DUNGEN] %s <%d %d> failed after %d attempts (conflicts)\n",
              type_name, vPos.x, vPos.y, attempt_count );
        g_pGame->GetStats()->Printf( "...%s <%d %d> conflicts. terminated.\n",
                                     step_type == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hall",
                                     VEC_EXPAND( vPos ) );
        JLog( LOG_LEVEL_DEBUG, true, "...%s <%d %d> conflicts. terminated.\n",
              step_type == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hall", VEC_EXPAND( vPos ) );
        delete pStep;
        return NULL;
    }

    JLog( LOG_LEVEL_DEBUG, true, "success!\n" );
    return pStep;
}

// Public wrapper for creating room steps
CDungeonCreationStep *CDungeonMap::CreateRoom( const JIVector &vPos, const int direction,
                                               const int recurdepth )
{
    return CreateStep( DUNG_CREATE_STEP_MAKE_ROOM, vPos, direction, recurdepth );
}

// Public wrapper for creating hallway steps
CDungeonCreationStep *CDungeonMap::CreateHallway( const JIVector &vPos, const int direction,
                                                  const int recurdepth )
{
    return CreateStep( DUNG_CREATE_STEP_MAKE_HALLWAY, vPos, direction, recurdepth );
}

JResult CDungeonMap::GetRoomRect( JRect &rcRoom, const int direction )
{
    JIVector vSize( 0, 0 );
    vSize.Init( Util::GetRandom( DUNG_ROOM_MINWIDTH, DUNG_ROOM_MAXWIDTH ),
                Util::GetRandom( DUNG_ROOM_MINHEIGHT, DUNG_ROOM_MAXHEIGHT ) );
    switch( direction )
    {
    case DIR_NORTH:
        rcRoom.left -=
            Util::GetRandom( 1, vSize.x - 1 ); // TODO: all the /2 are to center hallways on rooms
        rcRoom.SetWidth( vSize.x );
        rcRoom.SetHeight( vSize.y, false );
        break;
    case DIR_SOUTH:
        rcRoom.left -= Util::GetRandom( 1, vSize.x - 1 );
        rcRoom.SetWidth( vSize.x );
        rcRoom.SetHeight( vSize.y );
        break;
    case DIR_WEST:
        rcRoom.top -= Util::GetRandom( 1, vSize.y - 1 );
        rcRoom.SetWidth( vSize.x, false );
        rcRoom.SetHeight( vSize.y );
        break;
    case DIR_EAST:
        rcRoom.top -= Util::GetRandom( 1, vSize.y - 1 );
        rcRoom.SetWidth( vSize.x );
        rcRoom.SetHeight( vSize.y );
        break;
    case DIR_NONE:
        rcRoom.SetWidth( vSize.x );
        rcRoom.SetHeight( vSize.y );
        break;
    }

    // Check if clamping is needed (indicates out-of-bounds geometry)
    bool bClamped = rcRoom.ClampToWorld( true );

    // Check for degenerate rectangles (0 or negative width/height)
    if( rcRoom.Width() <= 0 || rcRoom.Height() <= 0 )
    {
        JLog( LOG_LEVEL_WARN, true,
              "[DUNGEN] GetRoomRect produced degenerate rect <%d %d, %d %d> (w=%d h=%d)\n",
              RECT_EXPAND( rcRoom ), rcRoom.Width(), rcRoom.Height() );
        return -1;
    }

    // If clamping occurred, the geometry may be corrupted
    if( bClamped )
    {
        JLog( LOG_LEVEL_WARN, true,
              "[DUNGEN] GetRoomRect required clamping - geometry may be corrupted\n" );
        return -1;
    }

    return JSUCCESS;
}

JResult CDungeonMap::GetHallRect( JRect &rcHall, const int direction )
{
    int length = Util::GetRandom( DUNG_HALL_MINLENGTH, DUNG_HALL_MAXLENGTH );
    switch( direction )
    {
    case DIR_NORTH:
        rcHall.SetWidth( 0 );
        rcHall.SetHeight( length, false );
        break;
    case DIR_SOUTH:
        rcHall.SetWidth( 0 );
        rcHall.SetHeight( length );
        break;
    case DIR_WEST:
        rcHall.SetWidth( length, false );
        rcHall.SetHeight( 0 );
        break;
    case DIR_EAST:
        rcHall.SetWidth( length );
        rcHall.SetHeight( 0 );
        break;
    }

    // Check if clamping is needed (indicates out-of-bounds geometry)
    bool bClamped = rcHall.ClampToWorld( true );

    // Hallways can have 0 width or 0 height (they're 1-dimensional corridors)
    // But they shouldn't have negative dimensions
    if( rcHall.Width() < 0 || rcHall.Height() < 0 )
    {
        JLog( LOG_LEVEL_WARN, true,
              "[DUNGEN] GetHallRect produced invalid rect <%d %d, %d %d> (w=%d h=%d)\n",
              RECT_EXPAND( rcHall ), rcHall.Width(), rcHall.Height() );
        return -1;
    }

    // If clamping occurred, the geometry may be corrupted
    if( bClamped )
    {
        JLog( LOG_LEVEL_WARN, true,
              "[DUNGEN] GetHallRect required clamping - geometry may be corrupted\n" );
        return -1;
    }

    return JSUCCESS;
}

// TruncateHallway: When a hallway's full length would collide with existing structure,
// scan along the growth direction to find where the collision starts, shorten the
// hallway to stop just before it, and place a connecting door.
//
// Layout for a southbound hallway connecting to an existing room:
//   hallway floor  (y = rc.top ... newBottom)
//   DOOR           (y = newBottom + 1, on the existing room's wall)
//   room floor     (y = newBottom + 2, existing room interior)
//
// Returns JSUCCESS if truncation produced a valid hallway, JERROR otherwise.
JResult CDungeonMap::TruncateHallway( CDungeonCreationStep *pStep )
{
    if( pStep->m_dwIndex != DUNG_CREATE_STEP_MAKE_HALLWAY )
        return JERROR();

    int dir = pStep->m_dwDirection;
    JRect &rc = pStep->m_rcArea;
    bool horizontal = ( dir == DIR_EAST || dir == DIR_WEST );
    bool positive = ( dir == DIR_EAST || dir == DIR_SOUTH );

    // Scan coordinates: fixedCoord is the thin axis, scan along the growth axis
    int fixedCoord = horizontal ? rc.top : rc.left;
    int scanFrom =
        positive ? ( horizontal ? rc.left : rc.top ) : ( horizontal ? rc.right : rc.bottom );
    int scanTo =
        positive ? ( horizontal ? rc.right : rc.bottom ) : ( horizontal ? rc.left : rc.top );
    int scanStep = positive ? 1 : -1;

    // Scan along the growth direction to find the first non-wall tile
    int collisionPos = 0;
    bool foundCollision = false;
    for( int pos = scanFrom; pos != scanTo + scanStep; pos += scanStep )
    {
        JIVector vTile;
        if( horizontal )
            vTile.Init( pos, fixedCoord );
        else
            vTile.Init( fixedCoord, pos );

        if( !vTile.IsInWorld() )
            break;

        if( GetTile( vTile )->GetType() != DUNG_IDX_WALL )
        {
            collisionPos = pos;
            foundCollision = true;
            break;
        }
    }

    if( !foundCollision )
    {
        // No interior collision — must be a border-only failure, can't truncate
        return JERROR();
    }

    // The collision tile is existing structure (floor/door).
    // One tile before it (toward us) is the room/hallway wall — place a door there.
    // Two tiles before it is our last hallway floor tile.
    int doorPos = collisionPos - scanStep;
    int newEnd = doorPos - scanStep;

    // Calculate truncated length
    int newLength = ( newEnd - scanFrom ) * scanStep + 1; // works for both +/- directions
    if( newLength < DUNG_HALL_MINLENGTH )
    {
        JLog( LOG_LEVEL_NOISIER, true,
              "[DUNGEN] TruncateHallway: length %d < min %d, cannot truncate\n", newLength,
              DUNG_HALL_MINLENGTH );
        return JERROR();
    }

    // Verify the door position is a wall tile (the boundary we're connecting through)
    JIVector vDoor;
    if( horizontal )
        vDoor.Init( doorPos, fixedCoord );
    else
        vDoor.Init( fixedCoord, doorPos );

    if( !vDoor.IsInWorld() || GetTile( vDoor )->GetType() != DUNG_IDX_WALL )
    {
        JLog( LOG_LEVEL_NOISIER, true,
              "[DUNGEN] TruncateHallway: door position <%d %d> is not wall (type=%d)\n",
              VEC_EXPAND( vDoor ), vDoor.IsInWorld() ? GetTile( vDoor )->GetType() : -1 );
        return JERROR();
    }

    // Truncate the rect
    if( dir == DIR_EAST )
        rc.right = newEnd;
    else if( dir == DIR_WEST )
        rc.left = newEnd;
    else if( dir == DIR_SOUTH )
        rc.bottom = newEnd;
    else // DIR_NORTH
        rc.top = newEnd;

    // Validate the truncated hallway (interior + border)
    if( !CheckArea( pStep ) )
    {
        JLog( LOG_LEVEL_NOISIER, true,
              "[DUNGEN] TruncateHallway: truncated rect <%d %d, %d %d> still conflicts\n",
              RECT_EXPAND( rc ) );
        return JERROR();
    }

    // Place the connecting door
    JLog( LOG_LEVEL_DEBUG, true,
          "[DUNGEN] TruncateHallway: connecting at <%d %d>, hallway <%d %d, %d %d> (len=%d)\n",
          VEC_EXPAND( vDoor ), RECT_EXPAND( rc ), newLength );
    GetTile( vDoor )->SetType( DUNG_IDX_DOOR );
    m_diagnostics.hallways_truncated++;

    return JSUCCESS;
}

JIVector &CDungeonMap::GetWallOrigin( CDungeonCreationStep *pStep, const int direction )
{
    // Pick a random interior point along the parent feature's wall in the given direction,
    // then offset outward by WALL_OFFSET to leave a gap for the door tile.
    //
    // For rooms (Width >= 3, Height >= 3): random interior point along the wall edge.
    // For hallways (Width == 0 or Height == 0): the thin dimension has only one position,
    // so no random offset is applied; the length dimension is randomized normally.
    int wallW = pStep->m_rcArea.Width();
    int wallH = pStep->m_rcArea.Height();
    int xAlongWall = ( wallW >= 2 ) ? Util::GetRandom( 1, wallW - 1 ) : 0;
    int yAlongWall = ( wallH >= 2 ) ? Util::GetRandom( 1, wallH - 1 ) : 0;

    switch( direction )
    {
    case DIR_NORTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() + xAlongWall,
                            pStep->m_rcArea.Top() - WALL_OFFSET );
        break;
    case DIR_SOUTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() + xAlongWall,
                            pStep->m_rcArea.Bottom() + WALL_OFFSET );
        break;
    case DIR_WEST:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() - WALL_OFFSET,
                            pStep->m_rcArea.Top() + yAlongWall );
        break;
    case DIR_EAST:
        pStep->m_vPos.Init( pStep->m_rcArea.Right() + WALL_OFFSET,
                            pStep->m_rcArea.Top() + yAlongWall );
        break;
    }

    JLog( LOG_LEVEL_NOISE, true, "[%d>%d]GetWallOrigin creating %s %s, starting at <%d %d>\n",
          pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
          direction == DIR_NORTH   ? "north"
          : direction == DIR_SOUTH ? "south"
          : direction == DIR_WEST  ? "west"
                                   : "east",
          pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
          VEC_EXPAND( pStep->m_vPos ) );

    return pStep->m_vPos;
}

// Export dungeon layout to fixture file for regression testing
// Format: CSV with header row, then grid of tile types
bool CDungeonMap::ExportDungeon( const char *pszFilename ) const
{
    if( !pszFilename || !m_dmtTiles )
        return false;

    FILE *fp = fopen( pszFilename, "w" );
    if( !fp )
        return false;

    // Write header with metadata
    fprintf( fp, "JMORIA_FIXTURE_v1\n" );
    fprintf( fp, "seed=%u,depth=%u,width=%d,height=%d,rooms=%d,hallways=%d\n", m_dwSeed, m_dwDepth,
             DUNG_WIDTH, DUNG_HEIGHT, m_llRooms ? m_llRooms->length() : 0,
             m_llHallways ? m_llHallways->length() : 0 );

    // Write tile grid: each row separated by newline
    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        for( int x = 0; x < DUNG_WIDTH; x++ )
        {
            CDungeonMapTile *pTile = &m_dmtTiles[y * DUNG_WIDTH + x];
            Uint8 type = pTile->GetType();
            uint32 flags = pTile->GetFlags();

            // Format: type or type:flags (hex) if flags present
            if( flags != 0 )
                fprintf( fp, "%d:%x", type, flags );
            else
                fprintf( fp, "%d", type );

            // Comma-separated within row, newline at end
            if( x < DUNG_WIDTH - 1 )
                fprintf( fp, "," );
        }
        fprintf( fp, "\n" );
    }

    fclose( fp );
    return true;
}

// Import dungeon layout from fixture file
bool CDungeonMap::ImportDungeon( const char *pszFilename )
{
    if( !pszFilename )
        return false;

    FILE *fp = fopen( pszFilename, "r" );
    if( !fp )
        return false;

    char buf[4096];

    // Read and validate header
    if( !fgets( buf, sizeof( buf ), fp ) )
    {
        fclose( fp );
        return false;
    }

    // Check header (account for newline at end of fgets)
    char *newline = strchr( buf, '\n' );
    if( newline )
        *newline = '\0';

    if( strcmp( buf, "JMORIA_FIXTURE_v1" ) != 0 )
    {
        fclose( fp );
        return false;
    }

    // Read metadata line
    unsigned int seed, depth;
    int width, height, rooms, hallways;
    if( !fgets( buf, sizeof( buf ), fp ) ||
        sscanf( buf, "seed=%u,depth=%u,width=%d,height=%d,rooms=%d,hallways=%d", &seed, &depth,
                &width, &height, &rooms, &hallways ) != 6 )
    {
        fclose( fp );
        return false;
    }

    // Validate dimensions
    if( width != DUNG_WIDTH || height != DUNG_HEIGHT )
    {
        fclose( fp );
        return false;
    }

    // Allocate tiles if needed
    if( !m_dmtTiles )
    {
        m_dmtTiles = new CDungeonMapTile[DUNG_WIDTH * DUNG_HEIGHT];
    }

    m_dwSeed = seed;
    m_dwDepth = depth;

    // Read tile grid
    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        if( !fgets( buf, sizeof( buf ), fp ) )
        {
            fclose( fp );
            return false;
        }

        int x = 0;
        char *ptr = buf;
        while( x < DUNG_WIDTH && ptr )
        {
            Uint8 type = 0;
            uint32 flags = 0;
            int type_int = 0;

            // Parse type and optional flags
            if( sscanf( ptr, "%d", &type_int ) == 1 )
            {
                type = (Uint8)type_int;
            }
            else
            {
                fclose( fp );
                return false;
            }

            // Check for flags (after colon)
            char *colon = strchr( ptr, ':' );
            if( colon )
            {
                if( sscanf( colon + 1, "%x", (unsigned int *)&flags ) == 1 )
                {
                    // successfully parsed flags
                }
            }

            CDungeonMapTile *pTile = &m_dmtTiles[y * DUNG_WIDTH + x];
            pTile->SetType( type );
            if( flags != 0 )
                pTile->SetFlags( flags );

            // Move to next tile (comma or newline)
            ptr = strchr( ptr, ',' );
            if( ptr )
                ptr++;
            x++;
        }

        if( x != DUNG_WIDTH )
        {
            fclose( fp );
            return false;
        }
    }

    fclose( fp );
    return true;
}

// Compare current dungeon with another dungeon
// Returns true if layout (tiles only) matches
bool CDungeonMap::CompareDungeon( const CDungeonMap &other ) const
{
    if( !m_dmtTiles || !other.m_dmtTiles )
        return false;

    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        for( int x = 0; x < DUNG_WIDTH; x++ )
        {
            const CDungeonMapTile *pTile1 = &m_dmtTiles[y * DUNG_WIDTH + x];
            const CDungeonMapTile *pTile2 = &other.m_dmtTiles[y * DUNG_WIDTH + x];

            if( pTile1->GetType() != pTile2->GetType() || pTile1->GetFlags() != pTile2->GetFlags() )
            {
                return false;
            }
        }
    }

    return true;
}

JIVector &CDungeonMap::GetHallOrigin( CDungeonCreationStep *pStep, int step_type )
{
    int modifier = 1;
    if( step_type == DUNG_CREATE_STEP_MAKE_ROOM )
    {
        modifier++;
    }
    switch( pStep->m_dwDirection )
    {
    case DIR_NORTH:
        // note: someday, this should be a random spot on the wall, rather than width/2
        pStep->m_vPos.Init( pStep->m_rcArea.Left(),
                            pStep->m_rcArea.Top() -
                                modifier ); // -1... does a room's rect include its walls?
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetHallOrigin creating north hall, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_SOUTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left(), pStep->m_rcArea.Bottom() + modifier );
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetHallOrigin creating south hall, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_WEST:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() - modifier, pStep->m_rcArea.Top() );
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetHallOrigin creating west hall, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_EAST:
        pStep->m_vPos.Init( pStep->m_rcArea.Right() + modifier, pStep->m_rcArea.Top() );
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetHallOrigin creating east hall, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    }

    return pStep->m_vPos;
}

// Flood-fill connectivity validation
// Returns true if the dungeon is fully connected
// Sets reachable_tiles to the number of walkable tiles reached
// Sets total_walkable_tiles to the total number of walkable tiles in the dungeon
bool CDungeonMap::ValidateConnectivity( int &reachable_tiles, int &total_walkable_tiles ) const
{
    if( !m_dmtTiles )
        return false;

    // Create visited array
    bool *visited = new bool[DUNG_WIDTH * DUNG_HEIGHT];
    memset( visited, false, DUNG_WIDTH * DUNG_HEIGHT );

    // Count total walkable tiles (floors, doors, stairs)
    total_walkable_tiles = 0;
    JIVector start_pos;
    bool found_start = false;

    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        for( int x = 0; x < DUNG_WIDTH; x++ )
        {
            const CDungeonMapTile *pTile = &m_dmtTiles[y * DUNG_WIDTH + x];
            Uint8 type = pTile->GetType();

            // Count walkable tiles
            if( type == DUNG_IDX_FLOOR || type == DUNG_IDX_DOOR || type == DUNG_IDX_OPEN_DOOR ||
                type == DUNG_IDX_SECRET_DOOR || type == DUNG_IDX_UPSTAIRS ||
                type == DUNG_IDX_LONG_UPSTAIRS || type == DUNG_IDX_DOWNSTAIRS ||
                type == DUNG_IDX_LONG_DOWNSTAIRS )
            {
                total_walkable_tiles++;

                // Remember first walkable tile as starting point
                if( !found_start )
                {
                    start_pos.Init( x, y );
                    found_start = true;
                }
            }
        }
    }

    // If no walkable tiles, dungeon is empty (technically "connected")
    if( total_walkable_tiles == 0 )
    {
        delete[] visited;
        reachable_tiles = 0;
        return true;
    }

    // Flood-fill using a queue (BFS)
    JIVector *queue = new JIVector[DUNG_WIDTH * DUNG_HEIGHT];
    int queue_head = 0;
    int queue_tail = 0;

    // Start flood-fill from first walkable tile
    queue[queue_tail++] = start_pos;
    visited[start_pos.y * DUNG_WIDTH + start_pos.x] = true;
    reachable_tiles = 1;

    // BFS flood-fill
    while( queue_head < queue_tail )
    {
        JIVector current = queue[queue_head++];

        // Check all 4 directions
        static const int dx[] = { 0, 0, -1, 1 };
        static const int dy[] = { -1, 1, 0, 0 };

        for( int i = 0; i < 4; i++ )
        {
            JIVector neighbor( current.x + dx[i], current.y + dy[i] );

            // Skip if out of bounds
            if( !neighbor.IsInWorld() )
                continue;

            int idx = neighbor.y * DUNG_WIDTH + neighbor.x;

            // Skip if already visited
            if( visited[idx] )
                continue;

            // Check if tile is walkable
            const CDungeonMapTile *pTile = &m_dmtTiles[idx];
            Uint8 type = pTile->GetType();

            if( type == DUNG_IDX_FLOOR || type == DUNG_IDX_DOOR || type == DUNG_IDX_OPEN_DOOR ||
                type == DUNG_IDX_SECRET_DOOR || type == DUNG_IDX_UPSTAIRS ||
                type == DUNG_IDX_LONG_UPSTAIRS || type == DUNG_IDX_DOWNSTAIRS ||
                type == DUNG_IDX_LONG_DOWNSTAIRS )
            {
                visited[idx] = true;
                queue[queue_tail++] = neighbor;
                reachable_tiles++;
            }
        }
    }

    delete[] queue;
    delete[] visited;

    // Dungeon is connected if all walkable tiles are reachable
    bool is_connected = ( reachable_tiles == total_walkable_tiles );

    JLog( LOG_LEVEL_INFO, true, "Connectivity: %d/%d tiles reachable (%s)\n", reachable_tiles,
          total_walkable_tiles, is_connected ? "CONNECTED" : "DISCONNECTED" );

    return is_connected;
}

// Simpler interface: returns true if all rooms are reachable
bool CDungeonMap::ValidateAllRoomsReachable() const
{
    int reachable = 0;
    int total = 0;
    return ValidateConnectivity( reachable, total );
}
