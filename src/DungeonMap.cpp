
#include "DungeonMap.h"
#include "DisplayText.h"

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
    JRect rcWorld( 0, 0, DUNG_WIDTH - 1, DUNG_HEIGHT - 1 );

    // First, fill the whole dungeon with rock
    FillArea( DUNG_IDX_WALL, &rcWorld, DIR_NONE, false );

    // Do something with the depth, here...
    if( depth > 100 )
    {
        printf( "You have a bad feeling about this level...\n" );
    }
    else if( depth > 75 )
    {
        printf( "Just another walk in the park.\n" );
    }
    else if( depth > 50 )
    {
        printf(
            "It is my firm belief that this level contains monsters, of one kind or another.\n" );
    }
    else if( depth > 25 )
    {
        printf( "This level goes together like wham-a-lamma-lamma and bop-she-bop-she-bop.\n" );
    }
    else if( depth > 10 )
    {
        printf( "What was *that*?!.\n" );
    }
    else if( depth > 5 )
    {
        printf( "Please keep hands and arms inside the carriage.\n" );
    }
    else if( depth <= 0 )
    {
        printf( "Error, levels don't go below 0.\n" );
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

    for( int y = area.top; y <= area.bottom; y++ )
    {
        vCheck.y = y;
        for( int x = area.left; x <= area.right; x++ )
        {
            vCheck.x = x;
            if( GetTile( vCheck )->GetType() != DUNG_IDX_WALL )
            {
                printf( "interior check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n",
                        RECT_EXPAND( area ), x, y, m_dmtTiles[y * DUNG_WIDTH + x].GetType() );
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
    JRect rcCheck( area );
    // TweakBorders(rcCheck, direction);
    if( rcCheck.top <= 0 )
    {
        printf( "border failed: <%d %d, %d %d>, top=0\n", RECT_EXPAND( rcCheck ) );
        return false;
    }
    else if( rcCheck.bottom >= DUNG_HEIGHT - 1 )
    {
        printf( "border failed: <%d %d, %d %d>, bottom=max\n", RECT_EXPAND( rcCheck ) );
        return false;
    }
    else if( rcCheck.left <= 0 )
    {
        printf( "border failed: <%d %d, %d %d>, left=0\n", RECT_EXPAND( rcCheck ) );
        return false;
    }
    else if( rcCheck.right >= DUNG_WIDTH - 1 )
    {
        printf( "border failed: <%d %d, %d %d>, right=max\n", RECT_EXPAND( rcCheck ) );
        return false;
    }

    JRect rcEdges( rcCheck.left - 1, rcCheck.top - 1, rcCheck.right + 1, rcCheck.bottom + 1 );
    if( !rcEdges.IsInWorld() )
    {
        printf( "border failed: <%d %d, %d %d>, edges fail.\n", RECT_EXPAND( rcEdges ) );
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
                printf( "border check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n",
                        RECT_EXPAND( area ), x, y, m_dmtTiles[y * DUNG_WIDTH + x].GetType() );
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

void CDungeonMap::FillArea( const CDungeonCreationStep *pStep )
{
    JRect rcFill( pStep->m_rcArea );
    FillArea( DUNG_IDX_FLOOR, &rcFill, pStep->m_dwDirection,
              pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_HALLWAY );
}
void CDungeonMap::FillArea( const Uint8 type, JRect *rcFill, const int direction, bool bIsHallway )
{
    JRect rcLocal( *rcFill );
    JRect rcEdges( rcLocal.left - 1, rcLocal.top - 1, rcLocal.right + 1, rcLocal.bottom + 1 );
    JIVector vOrigin( rcLocal.left, rcLocal.top );

    if( bIsHallway )
    {
        switch( direction )
        {
        case DIR_NORTH:
            // rcLocal.bottom--;
            break;
        case DIR_SOUTH:
            vOrigin.Init( rcLocal.left, rcLocal.bottom );
            // rcLocal.top++;
            break;
        case DIR_EAST:
            vOrigin.Init( rcLocal.right, rcLocal.top );
            // rcLocal.left++;
            break;
        case DIR_WEST:
            // rcLocal.right--;
            break;
        case DIR_NONE:
            // this doesn't make sense, but whatever.
            break;
        }
    }

    // Do not fill if requested rect is outside the world
    if( !Util::IsInWorld( rcLocal ) )
    {
        return;
    }

    // Do not fill if requested rect would light areas outside the world
    if( type != DUNG_IDX_WALL )
    {
        JRect rcEdges( rcLocal.left - 1, rcLocal.top - 1, rcLocal.right + 1, rcLocal.bottom + 1 );
        if( !Util::IsInWorld( rcEdges ) )
        {
            return;
        }
    }

    int x, y;
    JIVector vCurPos( vOrigin );
    for( y = rcLocal.top; y <= rcLocal.bottom; y++ )
    {
        vCurPos.y = y;
        for( x = rcLocal.left; x <= rcLocal.right; x++ )
        {
            vCurPos.x = x;
            GetTile( vCurPos )->SetType( type );
            if( type != DUNG_IDX_WALL )
            {
                GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );
            }
        }
    }

    // Now light the borders
    if( type != DUNG_IDX_WALL )
    {
        printf( "filled: <%d %d, %d %d>\nlit <%d %d, %d %d>\n", RECT_EXPAND( rcLocal ),
                RECT_EXPAND( rcEdges ) );

        // L and R sides, going over-by-one in each direction
        for( y = rcEdges.top; y <= rcEdges.bottom; y++ )
        {
            vCurPos.Init( rcEdges.left, y );
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );

            vCurPos.Init( rcEdges.right, y );
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );
        }
        // T and B sides, staying between the already-lit corners
        for( x = rcLocal.left; x <= rcLocal.right; x++ )
        {
            vCurPos.Init( x, rcEdges.top );
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );

            vCurPos.Init( x, rcEdges.bottom );
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT );
        }
    }

    // you still filled rcFill squares, just that one of them was a door.
    printf( "filled from <%d %d> to <%d %d>\n", RECT_EXPAND( *rcFill ) );

    if( bIsHallway )
    {
        // GetTile(*vOrigin)->SetType(DUNG_IDX_OPEN_DOOR);
        GetTile( vOrigin )->SetType( DUNG_IDX_FLOOR );
        // GetTile(*vOrigin)->SetFlags(~DUNG_FLAG_LIT);
    }
    else if( type != DUNG_IDX_WALL )
    {
        // GetTile(*vOrigin)->SetType(DUNG_IDX_RUBBLE);
        GetTile( vOrigin )->SetType( DUNG_IDX_FLOOR );
    }
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
        return false;
    }
    CDungeonCreationStep *pCurStep = pLink->m_lpData;
    CDungeonCreationStep *pNewStep = NULL;

    printf( "creating %d %s at <%d %d, %d %d>\n", pCurStep->m_dwDirection,
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
                continue;
            JIVector vHall = GetWallOrigin( pCurStep, dir );
            if( !vHall.IsInWorld() )
                continue;
            pNewStep = MakeHallStep( vHall, dir, pCurStep->m_dwRecurDepth + 1 );
            if( pNewStep != NULL )
                m_stkDungeonMapCreation->Push( pNewStep );
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
            AddDoor( vRoom, dir );
            if( !vRoom.IsInWorld() )
                break;
            pNewStep = MakeRoomStep( vRoom, dir, pCurStep->m_dwRecurDepth + 1 );
            if( pNewStep != NULL )
                m_stkDungeonMapCreation->Push( pNewStep );
        }
        else if( pick_next <= 100 )
        {
            // make a random number of hallways, continuing from this one.
            // push either rooms or hallways onto stack
            int num_halls = Util::GetRandom( 2, 4 );
            num_halls = 4;
            // Use num_rooms instead of forcing all 4 directions
            int dirs[4];
            RandomDirections( dirs );
            for( int index = 0; index <= num_halls; index++ )
            {
                int dir = dirs[index];
                if( pCurStep->m_dwDirection == Opposite( dir ) )
                    continue;
                if( pCurStep->m_dwDirection == dir )
                    continue;
                JIVector vHall = GetHallOrigin( pCurStep, DUNG_CREATE_STEP_MAKE_HALLWAY );
                if( !vHall.IsInWorld() )
                    break;
                pNewStep = MakeHallStep( vHall, dir, pCurStep->m_dwRecurDepth + 1 );
                if( pNewStep != NULL )
                    m_stkDungeonMapCreation->Push( pNewStep );
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
    if( roll <= normal )
    {
        door_type = DUNG_IDX_DOOR;
    }
    else if( roll <= open )
    {
        printf( "Added an open door.\n" );
        door_type = DUNG_IDX_OPEN_DOOR;
    }
    else
    {
        printf( "Added a secret door.\n" );
        door_type = DUNG_IDX_SECRET_DOOR;
    }
    printf( "Placing a %d door at <%d %d>\n", door_type, VEC_EXPAND( vDoor ) );
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
        return NULL;
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
            printf( "un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n",
                    RECT_EXPAND( pStep->m_rcArea ), RECT_EXPAND( rcTry ) );
            pStep->m_rcArea.Init( rcTry );
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this room.
        g_pGame->GetStats()->Printf( "...room <%d %d> conflicts. terminated.\n",
                                     VEC_EXPAND( vPos ) );
        delete pStep;
        return NULL;
    }
    // put a door where the room and hallway meet
    if( recurdepth > 0 )
        AddDoor( pStep->m_vPos, pStep->m_dwDirection );

    return pStep;
}

CDungeonCreationStep *CDungeonMap::MakeHallStep( const JIVector &vPos, const int direction,
                                                 const int recurdepth )
{
    if( recurdepth > MAX_RECURDEPTH )
        return NULL;
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
            printf( "un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n",
                    RECT_EXPAND( pStep->m_rcArea ), RECT_EXPAND( rcTry ) );
            pStep->m_rcArea.Init( rcTry );
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this hallway.
        g_pGame->GetStats()->Printf( "...hall <%d %d> conflicts. terminated.\n",
                                     VEC_EXPAND( vPos ) );
        delete pStep;
        return NULL;
    }
    // put a door where the room and hallway meet
    AddDoor( pStep->m_vPos, pStep->m_dwDirection );

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
    JRect rcEdges( rcRoom.left - 1, rcRoom.top - 1, rcRoom.right + 1, rcRoom.bottom + 1 );
    if( !rcEdges.IsInWorld() )
    {
        rcRoom.top = CLAMP( rcRoom.top, 1, DUNG_HEIGHT - 2 );
        rcRoom.bottom = CLAMP( rcRoom.bottom, 1, DUNG_HEIGHT - 2 );
        rcRoom.left = CLAMP( rcRoom.left, 1, DUNG_WIDTH - 2 );
        rcRoom.right = CLAMP( rcRoom.right, 1, DUNG_WIDTH - 2 );
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
    if( !rcHall.IsInWorld() )
    {
        rcHall.top = CLAMP( rcHall.top, 1, DUNG_HEIGHT - 2 );
        rcHall.bottom = CLAMP( rcHall.bottom, 1, DUNG_HEIGHT - 2 );
        rcHall.left = CLAMP( rcHall.left, 1, DUNG_WIDTH - 2 );
        rcHall.right = CLAMP( rcHall.right, 1, DUNG_WIDTH - 2 );
    }
}

JIVector &CDungeonMap::GetWallOrigin( CDungeonCreationStep *pStep, const int direction )
{
    switch( direction )
    {
    case DIR_NORTH:
        // note: someday, this should be a random spot on the wall, rather than width/2
        pStep->m_vPos.Init(
            pStep->m_rcArea.Left() + ( Util::GetRandom( 0, pStep->m_rcArea.Width() ) ),
            pStep->m_rcArea.Top() - 2 ); // -1... does a room's rect include its walls?
        printf( "[%d>%d]GetWallOrigin creating north %s, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
                pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
                VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_SOUTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() +
                                ( Util::GetRandom( 0, pStep->m_rcArea.Width() ) ),
                            pStep->m_rcArea.Bottom() + 2 );
        printf( "[%d>%d]GetWallOrigin creating south %s, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
                pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
                VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_WEST:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() - 2,
                            pStep->m_rcArea.Top() +
                                ( Util::GetRandom( 0, pStep->m_rcArea.Height() ) ) );
        printf( "[%d>%d]GetWallOrigin creating west %s, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1,
                pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM ? "hall" : "room",
                VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_EAST:
        pStep->m_vPos.Init( pStep->m_rcArea.Right() + 2,
                            pStep->m_rcArea.Top() +
                                ( Util::GetRandom( 0, pStep->m_rcArea.Height() ) ) );
        printf( "[%d>%d]GetWallOrigin creating east %s, starting at <%d %d>\n",
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
        printf( "[%d>%d]GetHallOrigin creating north hall, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_SOUTH:
        pStep->m_vPos.Init( pStep->m_rcArea.Left(), pStep->m_rcArea.Bottom() + modifier );
        printf( "[%d>%d]GetHallOrigin creating south hall, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_WEST:
        pStep->m_vPos.Init( pStep->m_rcArea.Left() - modifier, pStep->m_rcArea.Top() );
        printf( "[%d>%d]GetHallOrigin creating west hall, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    case DIR_EAST:
        pStep->m_vPos.Init( pStep->m_rcArea.Right() + modifier, pStep->m_rcArea.Top() );
        printf( "[%d>%d]GetHallOrigin creating east hall, starting at <%d %d>\n",
                pStep->m_dwRecurDepth, pStep->m_dwRecurDepth + 1, VEC_EXPAND( pStep->m_vPos ) );
        break;
    }

    return pStep->m_vPos;
}
