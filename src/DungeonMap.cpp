
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

uint8 MAX_TRIES = 2;

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
    
    // Use current RNG seed (seeded in main)
    m_dwSeed = Util::GetRandomSeed();

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
    // JIVector vPos(DUNG_WIDTH/2, DUNG_HEIGHT/2);
    // JIVector vPos(20,20);
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
            if( type != DUNG_IDX_WALL && !IsDoor( type ) )
            {
                JLog( LOG_LEVEL_NOISE, true,
                      "border check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n",
                      RECT_EXPAND( area ), VEC_EXPAND( vCheck ), GetTile( vCheck )->GetType() );
                // TODO: ... what if we allow overlaps? This is just a border check... the interior
                // is OK.
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

#ifdef DUNGEN_DEBUG
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
                    JLog( LOG_LEVEL_WARN, true, "[DUNGEN] Pre-fill invariant violation at <%d %d>: "
                          "expected wall, found type %d\n", x, y, GetTile( vPos )->GetType() );
                }
            }
        }
    }
    JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] FillArea type=%d <%d %d, %d %d>\n", type,
          RECT_EXPAND( rcFill ) );
#endif

    JIVector vCurPos;
    for( vCurPos.y = rcFill.top; vCurPos.y <= rcFill.bottom; vCurPos.y++ )
    {
        for( vCurPos.x = rcFill.left; vCurPos.x <= rcFill.right; vCurPos.x++ )
        {
            GetTile( vCurPos )->SetType( type );
        }
    }
    
#ifdef DUNGEN_DEBUG
    m_diagnostics.fill_operations++;
#endif

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

void RandomDirections( int r[] )
{
    // initial range of numbers
    for( int i = 0; i < 4; ++i )
    {
        r[i] = i;
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
bool CDungeonMap::CreateOneStep()
{
    CLink<CDungeonCreationStep> *pLink = m_stkDungeonMapCreation->Pop();
    if( pLink == NULL || pLink->m_lpData == NULL )
    {
        g_pGame->GetStats()->Printf( "Dungeon creation complete.\n" );
#ifdef DUNGEN_DEBUG
        JLog( LOG_LEVEL_INFO, true, "[DUNGEN] Generation complete: %d steps, %d rooms, %d halls, "
              "%d skipped, %d fill ops\n", m_diagnostics.steps_created, m_diagnostics.rooms_created,
              m_diagnostics.hallways_created, m_diagnostics.steps_skipped, m_diagnostics.fill_operations );
#endif
        return false;
    }
    CDungeonCreationStep *pCurStep = pLink->m_lpData;
    CDungeonCreationStep *pNewStep = NULL;

#ifdef DUNGEN_DEBUG
    m_diagnostics.steps_created++;
    JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Step %d: creating %s at <%d %d, %d %d> (depth=%d)\n", 
          m_diagnostics.steps_created,
          pCurStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hallway",
          RECT_EXPAND( pCurStep->m_rcArea ), pCurStep->m_dwRecurDepth );
#endif

    JLog( LOG_LEVEL_DEBUG, true, "creating %d %s at <%d %d, %d %d>\n", pCurStep->m_dwDirection,
          pCurStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "room" : "hallway",
          RECT_EXPAND( pCurStep->m_rcArea ) );
    // create current entity
    FillArea( pCurStep );

    switch( pCurStep->m_dwIndex )
    {
    case DUNG_CREATE_STEP_MAKE_ROOM:
    {
        // push hallways onto stack
        int num_halls = Util::GetRandom( 2, 4 );
        // num_halls = 1;
        // Use num_halls instead of forcing all 4 directions
        int dirs[4];
        RandomDirections( dirs );
        for( int index = 0; index <= num_halls; index++ )
        {
            int dir = dirs[index];
            if( pCurStep->m_dwDirection == Opposite( dir ) )
            {
                if( num_halls < 4 )
                    num_halls++;
                continue;
            }
            JIVector vHall = GetWallOrigin( pCurStep, dir );
            if( !vHall.IsWithinWorld() )
                continue;
            pNewStep = MakeHallStep( vHall, dir, pCurStep->m_dwRecurDepth + 1 );
            if( pNewStep != NULL )
            {
                AddDoor( vHall, dir );
                m_stkDungeonMapCreation->Push( pNewStep );
#ifdef DUNGEN_DEBUG
                m_diagnostics.hallways_created++;
                JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Hallway created, pushed to stack\n" );
#endif
            }
#ifdef DUNGEN_DEBUG
            else
            {
                m_diagnostics.steps_skipped++;
                JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Hallway creation failed (conflict or depth limit)\n" );
            }
#endif
        }
    }
    break;
    case DUNG_CREATE_STEP_MAKE_HALLWAY:
    {
        int pick_next = Util::Roll( "1d100" );
        if( pick_next <= 80 )
        {
            // Make a (single) room, in the direction of this hallway
            int dir = pCurStep->m_dwDirection;
            JIVector vRoom = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_ROOM );
            if( !vRoom.IsWithinWorld() )
                break;
            pNewStep = MakeRoomStep( vRoom, dir, pCurStep->m_dwRecurDepth + 1 );
            if( pNewStep != NULL )
            {
                AddDoor( vRoom, dir );
                m_stkDungeonMapCreation->Push( pNewStep );
#ifdef DUNGEN_DEBUG
                m_diagnostics.rooms_created++;
                JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Room created from hallway, pushed to stack\n" );
#endif
            }
#ifdef DUNGEN_DEBUG
            else
            {
                m_diagnostics.steps_skipped++;
                JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Room creation failed (conflict or depth limit)\n" );
            }
#endif
        }
        else if( pick_next <= 100 )
        {
            // make a random number of hallways, continuing from this one.
            // push either rooms or hallways onto stack
            int num_halls = Util::GetRandom( 2, 4 );
            // num_halls = 4;
            // Use num_rooms instead of forcing all 4 directions
            int dirs[4];
            RandomDirections( dirs );
            for( int index = 0; index <= num_halls; index++ )
            {
                int dir = dirs[index];
                if( pCurStep->m_dwDirection == Opposite( dir ) )
                {
                    if( num_halls < 4 )
                        num_halls++;
                    continue;
                }
                // if( pCurStep->m_dwDirection == dir )
                //     continue;
                JIVector vHall = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_HALLWAY );
                if( !vHall.IsWithinWorld() )
                    break;
                pNewStep = MakeHallStep( vHall, dir, pCurStep->m_dwRecurDepth + 1 );
                if( pNewStep != NULL )
                {
                    AddDoor( vHall, pCurStep->m_dwDirection );
                    m_stkDungeonMapCreation->Push( pNewStep );
#ifdef DUNGEN_DEBUG
                    m_diagnostics.hallways_created++;
                    JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Branch hallway created, pushed to stack\n" );
#endif
                }
#ifdef DUNGEN_DEBUG
                else
                {
                    m_diagnostics.steps_skipped++;
                    JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Branch hallway creation failed\n" );
                }
#endif
            }
        }
    }
    break;
    }

    m_stkDungeonMapCreation->Remove( pLink );

    return true;
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
        vDoor.y++;
        break;
    case DIR_SOUTH:
        vDoor.y--;
        break;
    case DIR_WEST:
        vDoor.x++;
        break;
    case DIR_EAST:
        vDoor.x--;
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
    CDungeonCreationStep *step = MakeRoomStep( vOrigin, DIR_NONE, 0 );
    m_stkDungeonMapCreation->Push( step );
}

CDungeonCreationStep *CDungeonMap::MakeRoomStep( const JIVector &vPos, const int direction,
                                                 const int recurdepth )
{
    if( recurdepth > MAX_RECURDEPTH )
    {
#ifdef DUNGEN_DEBUG
        JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] MakeRoomStep rejected: recursion depth %d > %d\n", 
              recurdepth, MAX_RECURDEPTH );
#endif
        return NULL;
    }
    JLog( LOG_LEVEL_DEBUG, true, "Creating a room step\n" );
    CDungeonCreationStep *pStep = new CDungeonCreationStep();
    pStep->m_dwIndex = DUNG_CREATE_STEP_MAKE_ROOM;
    pStep->m_dwDirection = direction;
    pStep->m_dwRecurDepth = recurdepth;
    pStep->m_vPos.Init( vPos.x, vPos.y );
    pStep->m_rcArea.Init( pStep->m_vPos, 0, 0 );
    // TODO: eventually, put in diff. types of rooms
    // randomly sized rectangular room

    bool dwDone = false;
    int count = 0;

    JRect rcTry( pStep->m_rcArea );
    while( !dwDone && count < MAX_TRIES )
    {
        GetRoomRect( pStep->m_rcArea, pStep->m_dwDirection );
        dwDone = CheckArea( pStep );
        if( !dwDone )
        {
            // this one didn't work, need to "un-shift" the rect for the next try.
#ifdef DUNGEN_DEBUG
            JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Room attempt %d conflict at <%d %d, %d %d>\n",
                  count + 1, RECT_EXPAND( pStep->m_rcArea ) );
#endif
            JLog( LOG_LEVEL_NOISE, true, "un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n",
                  RECT_EXPAND( pStep->m_rcArea ), RECT_EXPAND( rcTry ) );
            pStep->m_rcArea.Init( rcTry );
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this room.
#ifdef DUNGEN_DEBUG
        JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Room <%d %d> failed after %d attempts (conflicts)\n",
              vPos.x, vPos.y, count );
#endif
        g_pGame->GetStats()->Printf( "...room <%d %d> conflicts. terminated.\n",
                                     VEC_EXPAND( vPos ) );
        JLog( LOG_LEVEL_DEBUG, true, "...room <%d %d> conflicts. terminated.\n",
              VEC_EXPAND( vPos ) );
        delete pStep;
        return NULL;
    }
    // put a door where the room and hallway meet
    // if( recurdepth > 0 )
    //     AddDoor( pStep->m_vPos, pStep->m_dwDirection );

    JLog( LOG_LEVEL_DEBUG, true, "success!\n" );
    return pStep;
}

CDungeonCreationStep *CDungeonMap::MakeHallStep( const JIVector &vPos, const int direction,
                                                 const int recurdepth )
{
    if( recurdepth > MAX_RECURDEPTH )
    {
#ifdef DUNGEN_DEBUG
        JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] MakeHallStep rejected: recursion depth %d > %d\n",
              recurdepth, MAX_RECURDEPTH );
#endif
        return NULL;
    }
    JLog( LOG_LEVEL_DEBUG, true, "Creating a hall step\n" );
    CDungeonCreationStep *pStep = new CDungeonCreationStep();
    pStep->m_dwIndex = DUNG_CREATE_STEP_MAKE_HALLWAY;
    pStep->m_dwRecurDepth = recurdepth;
    pStep->m_dwDirection = direction;
    pStep->m_vPos.Init( vPos.x, vPos.y );
    pStep->m_rcArea.Init( pStep->m_vPos, 0, 0 );

    bool dwDone = false;
    int count = 0;

    JRect rcTry( pStep->m_rcArea );
    while( !dwDone && count < MAX_TRIES )
    {
        GetHallRect( pStep->m_rcArea, pStep->m_dwDirection );
        dwDone = CheckArea( pStep );
        if( !dwDone )
        {
            // this one didn't work, need to "un-shift" the rect for the next try.
#ifdef DUNGEN_DEBUG
            JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Hall attempt %d conflict at <%d %d, %d %d>\n",
                  count + 1, RECT_EXPAND( pStep->m_rcArea ) );
#endif
            JLog( LOG_LEVEL_NOISE, true, "un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n",
                  RECT_EXPAND( pStep->m_rcArea ), RECT_EXPAND( rcTry ) );
            pStep->m_rcArea.Init( rcTry );
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this hallway.
#ifdef DUNGEN_DEBUG
        JLog( LOG_LEVEL_NOISE, true, "[DUNGEN] Hall <%d %d> failed after %d attempts (conflicts)\n",
              vPos.x, vPos.y, count );
#endif
        g_pGame->GetStats()->Printf( "...hall <%d %d> conflicts. terminated.\n",
                                     VEC_EXPAND( vPos ) );
        JLog( LOG_LEVEL_DEBUG, true, "...hall <%d %d> conflicts. terminated.\n",
              VEC_EXPAND( vPos ) );
        delete pStep;
        return NULL;
    }
    // put a door where the room and hallway meet
    // AddDoor( pStep->m_vPos, pStep->m_dwDirection );

    JLog( LOG_LEVEL_DEBUG, true, "success!\n" );
    return pStep;
}

void CDungeonMap::GetRoomRect( JRect &rcRoom, const int direction )
{
    JIVector vSize( 0, 0 );
    vSize.Init( Util::GetRandom( DUNG_ROOM_MINWIDTH, DUNG_ROOM_MAXWIDTH ),
                Util::GetRandom( DUNG_ROOM_MINHEIGHT, DUNG_ROOM_MAXHEIGHT ) );
    // vSize.Init(5,5);
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
    if( !rcRoom.IsWithinWorld() )
    {
        rcRoom.Init(
            CLAMP( rcRoom.left, 1, DUNG_WIDTH - 2 ), CLAMP( rcRoom.top, 1, DUNG_HEIGHT - 2 ),
            CLAMP( rcRoom.right, 1, DUNG_WIDTH - 2 ), CLAMP( rcRoom.bottom, 1, DUNG_HEIGHT - 2 ) );
    }
}

void CDungeonMap::GetHallRect( JRect &rcHall, const int direction )
{
    int length = Util::GetRandom( DUNG_HALL_MINLENGTH, DUNG_HALL_MAXLENGTH );
    // length=5;
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
    if( !rcHall.IsWithinWorld() )
    {
        rcHall.Init(
            CLAMP( rcHall.left, 1, DUNG_WIDTH - 2 ), CLAMP( rcHall.top, 1, DUNG_HEIGHT - 2 ),
            CLAMP( rcHall.right, 1, DUNG_WIDTH - 2 ), CLAMP( rcHall.bottom, 1, DUNG_HEIGHT - 2 ) );
    }
}

JIVector &CDungeonMap::GetWallOrigin( CDungeonCreationStep *pStep, const int direction )
{
    switch( direction )
    {
    case DIR_NORTH:
        // note: someday, this should be a random spot on the wall, rather than width/2
        pStep->m_vPos.Init(
            pStep->m_rcArea.Left() + ( Util::GetRandom( 1, pStep->m_rcArea.Width() - 1 ) ),
            pStep->m_rcArea.Top() - 2 ); // -1... does a room's rect include its walls?
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetWallOrigin creating north %s, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
              pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
              VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_SOUTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() +
                                ( Util::GetRandom( 1, pStep->m_rcArea.Width() - 1 ) ),
                            pStep->m_rcArea.Bottom() + 2 );
        JLog( LOG_LEVEL_NOISE, true,
              "[%d>%d]GetWallOrigin creating south %s, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
              pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
              VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_WEST:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() - 2,
                            pStep->m_rcArea.Top() +
                                ( Util::GetRandom( 1, pStep->m_rcArea.Height() - 1 ) ) );
        JLog( LOG_LEVEL_NOISE, true, "[%d>%d]GetWallOrigin creating west %s, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
              pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
              VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_EAST:
        pStep->m_vPos.Init( pStep->m_rcArea.Right() + 2,
                            pStep->m_rcArea.Top() +
                                ( Util::GetRandom( 1, pStep->m_rcArea.Height() - 1 ) ) );
        JLog( LOG_LEVEL_NOISE, true, "[%d>%d]GetWallOrigin creating east %s, starting at <%d %d>\n",
              pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
              pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
              VEC_EXPAND( pStep->m_vPos ) );
        break;
    }
    return pStep->m_vPos;
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
