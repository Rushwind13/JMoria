
#include "DungeonMap.h"
#include "DisplayText.h"

#ifdef FIXED_DUNGEON
Uint8 dungeontiles[DUNG_HEIGHT][DUNG_WIDTH] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,4,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,8,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,5,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,1,1,1,2,1,1,
	1,0,7,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,0,0,6,0,0,0,0,0,0,1,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};/* */
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
void CDungeonMap::CreateDungeon(const int depth)
{
	JIVector vOrigin;
    Term();
	m_dmtTiles = new CDungeonMapTile[DUNG_WIDTH * DUNG_HEIGHT];
    JRect rcWorld(0,0, DUNG_WIDTH-1,DUNG_HEIGHT-1);

	// First, fill the whole dungeon with rock
	FillArea(DUNG_IDX_WALL, &rcWorld, DIR_NONE, &vOrigin, false );

    // Do something with the depth, here...
    if( depth > 100 )
    {
        printf("You have a bad feeling about this level...\n");
    }
    else if( depth > 75 )
    {
        printf("Just another walk in the park.\n");
    }
    else if( depth > 50 )
    {
        printf("It is my firm belief that this level contains monsters, of one kind or another.\n");
    }
    else if( depth > 25 )
    {
        printf("This level goes together like wham-a-lamma-lamma and bop-she-bop-she-bop.\n");
    }
    else if( depth > 10 )
    {
        printf("What was *that*?!.\n");
    }
    else if( depth > 5 )
    {
        printf("Please keep hands and arms inside the carriage.\n");
    }
    else if( depth <= 0 )
    {
        printf("Error, levels don't go below 0.\n");
    }
#ifdef FIXED_DUNGEON
//    For setpiece rooms, treasure rooms, &c
	JIVector vCurPos;
	int type;
	for(vCurPos.y=0;vCurPos.y<DUNG_HEIGHT;vCurPos.y++)
	{
		for(vCurPos.x=0;vCurPos.x<DUNG_WIDTH;vCurPos.x++)
		{
			type = dungeontiles[vCurPos.y][vCurPos.x];
			GetTile(vCurPos)->SetType(type);
			//if( type != DUNG_IDX_WALL )
			{
				GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);
			}
		}
	}
#else

	// Next, carve out a room in the middle
    JIVector vPos( Util::GetRandom(DUNG_ROOM_MAXWIDTH,DUNG_WIDTH-DUNG_ROOM_MAXWIDTH-1), Util::GetRandom(DUNG_ROOM_MAXHEIGHT,DUNG_HEIGHT-DUNG_ROOM_MAXHEIGHT-1) );
    //JIVector vPos(DUNG_WIDTH/2, DUNG_HEIGHT/2);
    //JIVector vPos(20,20);
    InitDungeonCreate(vPos);
#endif

}

bool CDungeonMap::CheckArea( CDungeonCreationStep *pStep )
{
		bool bInteriorOK = false;
		bool bBorderOK = false;

		bInteriorOK = CheckInterior( pStep );
		if( bInteriorOK ) bBorderOK = CheckBorder( pStep );
		return bInteriorOK && bBorderOK;
}

bool CDungeonMap::CheckInterior( CDungeonCreationStep *pStep )
{
		JIVector vCheck;

		for( int x = pStep->m_rcArea.Left(); x <= pStep->m_rcArea.Right(); x++ )
		{
			vCheck.x = x;
			for( int y = pStep->m_rcArea.Top(); y <= pStep->m_rcArea.Bottom(); y++ )
			{
				vCheck.y = y;
				if( GetTile(vCheck)->GetType() != DUNG_IDX_WALL )
				{
					printf("interior check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n", RECT_EXPAND(pStep->m_rcArea),x,y, m_dmtTiles[y * DUNG_WIDTH + x].GetType() );
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
            break;
        case DIR_SOUTH:
            rcIn.top++;
            break;
        case DIR_WEST:
            rcIn.right--;
            break;
        case DIR_EAST:
            rcIn.left++;
            break;
        default:
            break;
    }
}

bool CDungeonMap::CheckBorder( CDungeonCreationStep *pStep )
{
		JRect rcCheck(pStep->m_rcArea);
    
        TweakBorders(rcCheck, pStep->m_dwDirection);
		if( rcCheck.Top() == 0 )
		{
            printf("border failed: <%d %d, %d %d>, top=0\n", RECT_EXPAND(pStep->m_rcArea));
			return false;
		}
		else if( rcCheck.Bottom() == DUNG_HEIGHT )
		{
			printf("border failed: <%d %d, %d %d>, bottom=max\n", RECT_EXPAND(pStep->m_rcArea));
			return false;
		}
		else if( rcCheck.Left() == 0 )
		{
			printf("border failed: <%d %d, %d %d>, left=0\n", RECT_EXPAND(pStep->m_rcArea));
			return false;
		}
		else if( rcCheck.Right() == DUNG_WIDTH )
		{
			printf("border failed: <%d %d, %d %d>, right=max\n", RECT_EXPAND(pStep->m_rcArea));
			return false;
		}

		JIVector vCheck;
		for( int x = rcCheck.Left()-1; x <= rcCheck.Right()+1; x++ )
		{
			vCheck.x = x;
			for( int y = rcCheck.Top()-1; y <= rcCheck.Bottom()+1; y++ )
			{
				vCheck.y = y;
				if( GetTile(vCheck)->GetType() != DUNG_IDX_WALL && GetTile(vCheck)->GetType() != DUNG_IDX_DOOR )
				{
					printf("border check failed. Wanted <%d %d, %d %d>, but <%d %d> was %d\n", RECT_EXPAND(pStep->m_rcArea), x,y, m_dmtTiles[y * DUNG_WIDTH + x].GetType() );
					return false;
				}
			}
		}

		return true;
}

void CDungeonMap::FillArea(const CDungeonCreationStep *pStep)
{
    JRect rcFill(pStep->m_rcArea);
    JIVector vOrigin(pStep->m_vPos);
    FillArea(DUNG_IDX_FLOOR, &rcFill, pStep->m_dwDirection, &vOrigin, pStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_HALLWAY);
}
void CDungeonMap::FillArea( const Uint8 type, JRect *rcFill, const int direction, JIVector *vOrigin, bool bIsHallway )
{
	JRect rcLocal(*rcFill);
	vOrigin->Init( rcLocal.left, rcLocal.top );

	if( bIsHallway)
	{
		JIVector vTail;
		vTail.Init( rcLocal.right, rcLocal.bottom );
		switch( direction )
		{
		case DIR_NORTH:
			//vOrigin->Init( rcLocal.left, rcLocal.top );
			//vTail.Init( rcLocal.right, rcLocal.bottom );
			rcLocal.bottom--;
			break;
		case DIR_SOUTH:
			vOrigin->Init( rcLocal.left, rcLocal.bottom );
			vTail.Init(   rcLocal.right, rcLocal.top );
			rcLocal.top++;
			break;
		case DIR_EAST:
			vOrigin->Init( rcLocal.right, rcLocal.top );
			vTail.Init(   rcLocal.left, rcLocal.bottom );
			rcLocal.left++;
			break;
		case DIR_WEST:
			//vOrigin->Init( rcLocal.left, rcLocal.top );
			//vTail.Init(   rcLocal.right, rcLocal.bottom );
			rcLocal.right--;
			break;
		case DIR_NONE:
			// this doesn't make sense, but whatever.
			//vOrigin->Init( rcLocal.left, rcLocal.top );
			//vTail.Init( rcLocal.right, rcLocal.bottom );
			break;
		}
//        GetTile(vTail)->SetType(DUNG_IDX_DOOR);
	}
	//else
	//{
		//vOrigin->Init( rcLocal.left, rcLocal.top );
	//}

	if( !Util::IsInWorld(rcLocal) )
	{
		return;
	}

	int x,y;
	JIVector vCurPos;
	for( y=rcLocal.top; y <= rcLocal.bottom; y++ )
	{
		vCurPos.y = y;
		for( x=rcLocal.left; x <= rcLocal.right; x++ )
		{
			vCurPos.x = x;
			GetTile(vCurPos)->SetType(type);
			if( type != DUNG_IDX_WALL )
			{
				GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);
			}
		}
	}

	// Now light the borders
	if( type != DUNG_IDX_WALL )
	{
		if( rcLocal.top-1 < 0 || rcLocal.bottom+1 > DUNG_HEIGHT ||
			rcLocal.left-1 < 0 || rcLocal.right-1 > DUNG_WIDTH )
		{
			// badness.
            printf("Badness occurred; fillArea() is outside the world.\n");
		}
		for( y=rcLocal.top-1; y <= rcLocal.bottom+1; y++ )
		{
			vCurPos.Init(rcLocal.left-1,y);
			GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);

			vCurPos.Init(rcLocal.right+1,y);
			GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);
		}

		for( x=rcLocal.left; x <= rcLocal.right; x++ )
		{
			vCurPos.Init(x,rcLocal.top-1);
			GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);

			vCurPos.Init(x,rcLocal.bottom+1);
			GetTile(vCurPos)->SetFlags(DUNG_FLAG_LIT);
		}
	}

	// you still filled rcFill squares, just that one of them was a door.
	printf("filled from <%d %d> to <%d %d>\n", RECT_EXPAND(*rcFill));

	if( bIsHallway )
	{
        GetTile(*vOrigin)->SetType(DUNG_IDX_DOOR);
	}
	else
	{
		GetTile(*vOrigin)->SetType(DUNG_IDX_RUBBLE);
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

void RandomDirections(int r[])
{
    // initial range of numbers
    for(int i=0;i<4;++i){
        r[i]=i;
    }
    
    for (int i = 3; i >= 0; --i){
        //generate a random number [0, n-1]
        int j = rand() % (i+1);

        //swap the last element with element at random index
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
        g_pGame->GetStats()->Printf("Dungeon creation complete.\n");
        return false;
    }
    CDungeonCreationStep *pCurStep = pLink->m_lpData;
    CDungeonCreationStep *pNewStep = NULL;
    
    printf("creating %d %s at <%d %d, %d %d>\n", pCurStep->m_dwDirection, pCurStep->m_dwIndex == DUNG_CREATE_STEP_MAKE_ROOM? "room":"hallway", RECT_EXPAND(pCurStep->m_rcArea));
    // create current entity
    FillArea(pCurStep);

    switch( pCurStep->m_dwIndex )
    {
        case DUNG_CREATE_STEP_MAKE_ROOM:
            {
                // push hallways onto stack
                int num_halls = Util::GetRandom(2,4);
                // num_halls = 1;
                // Use num_halls instead of forcing all 4 directions
                int dirs[4];
                RandomDirections(dirs);
                for( int index = 0; index <= num_halls; index++ )
                {
                    int dir = dirs[index];
                    if( pCurStep->m_dwDirection == Opposite(dir) ) continue;
                    pNewStep = MakeHallStep( GetWallOrigin(pCurStep, dir), dir, pCurStep->m_dwRecurDepth+1 );
                    if( pNewStep != NULL ) m_stkDungeonMapCreation->Push(pNewStep);
                }
            }
            break;
        case DUNG_CREATE_STEP_MAKE_HALLWAY:
            {
                int pick_next = Util::Roll("1d100");
                if( pick_next <= 80 )
                {
                    // Make a (single) room, in the direction of this hallway
                    int dir = pCurStep->m_dwDirection;
                    pNewStep = MakeRoomStep( GetHallOrigin(pCurStep), dir, pCurStep->m_dwRecurDepth+1 );
                    if( pNewStep != NULL ) m_stkDungeonMapCreation->Push(pNewStep);
                }
                else if( pick_next <= 100 )
                {
                    // make a random number of hallways, continuing from this one.
                    // push either rooms or hallways onto stack
                    int num_halls = Util::GetRandom(2,4);
                     num_halls = 4;
                    // Use num_rooms instead of forcing all 4 directions
                    int dirs[4];
                    RandomDirections(dirs);
                    for( int index = 0; index <= num_halls; index++ )
                    {
                        int dir = dirs[index];
                        if( pCurStep->m_dwDirection == Opposite(dir) ) continue;
                        if( pCurStep->m_dwDirection == dir ) continue;
                        pNewStep = MakeHallStep( GetHallOrigin(pCurStep), dir, pCurStep->m_dwRecurDepth+1 );
                        if( pNewStep != NULL ) m_stkDungeonMapCreation->Push(pNewStep);
                    }
                }
            }
            break;
    }

    m_stkDungeonMapCreation->Remove(pLink);
    
    return true;
}

void CDungeonMap::InitDungeonCreate( JIVector &vOrigin )
{
    CDungeonCreationStep *step = MakeRoomStep( vOrigin, DIR_NONE, 0);
    m_stkDungeonMapCreation->Push(step);
}

CDungeonCreationStep *CDungeonMap::MakeRoomStep( const JIVector &vPos, const int direction, const int recurdepth )
{
    if( recurdepth > MAX_RECURDEPTH ) return NULL;
    CDungeonCreationStep *pStep = new CDungeonCreationStep();
    pStep->m_dwIndex = DUNG_CREATE_STEP_MAKE_ROOM;
    pStep->m_dwDirection = direction;
    pStep->m_dwRecurDepth = recurdepth;
    pStep->m_vPos.Init(vPos.x, vPos.y);
    pStep->m_rcArea.Init(pStep->m_vPos, 0, 0);
    // TODO: eventually, put in diff. types of rooms
    // randomly sized rectangular room

    bool dwDone = false;
    int count = 0;

    JRect rcTry(pStep->m_rcArea);
    while( !dwDone && count < 20 )
    {
        GetRoomRect(pStep->m_rcArea, pStep->m_dwDirection);
        dwDone = CheckArea( pStep );
        if(!dwDone)
        {
            // this one didn't work, need to "un-shift" the rect for the next try.
            printf("un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n", RECT_EXPAND(pStep->m_rcArea), RECT_EXPAND(rcTry));
            pStep->m_rcArea.Init(rcTry);
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this room.
        g_pGame->GetStats()->Printf("...room <%d %d> conflicts. terminated.\n", VEC_EXPAND(vPos));
        delete pStep;
        return NULL;
    }

    return pStep;
}

CDungeonCreationStep *CDungeonMap::MakeHallStep( const JIVector &vPos, const int direction, const int recurdepth )
{
    if( recurdepth > MAX_RECURDEPTH ) return NULL;
    CDungeonCreationStep *pStep = new CDungeonCreationStep();
    pStep->m_dwIndex = DUNG_CREATE_STEP_MAKE_HALLWAY;
    pStep->m_dwRecurDepth = recurdepth;
    pStep->m_dwDirection = direction;
    pStep->m_vPos.Init(vPos.x, vPos.y);
    pStep->m_rcArea.Init(pStep->m_vPos, 0, 0);

    bool dwDone = false;
    int count = 0;

    JRect rcTry(pStep->m_rcArea);
    while( !dwDone && count < 20 )
    {
        GetHallRect(pStep->m_rcArea, pStep->m_dwDirection);
        dwDone = CheckArea( pStep );
        if(!dwDone)
        {
            // this one didn't work, need to "un-shift" the rect for the next try.
            printf("un-shifting <%d %d, %d %d> back to <%d %d, %d %d>\n", RECT_EXPAND(pStep->m_rcArea), RECT_EXPAND(rcTry));
            pStep->m_rcArea.Init(rcTry);
        }
        count++;
    }

    if( !dwDone )
    {
        // can't find a good match for this hallway.
        g_pGame->GetStats()->Printf("...hall <%d %d> conflicts. terminated.\n", VEC_EXPAND(vPos));
        delete pStep;
        return NULL;
    }
    // put a door where the room and hallway meet
    GetTile(pStep->m_vPos)->SetType(DUNG_IDX_DOOR);
    
    return pStep;
}

void CDungeonMap::GetRoomRect(JRect &rcRoom, const int direction)
{
    JIVector vSize(0,0);
    vSize.Init( Util::GetRandom(DUNG_ROOM_MINWIDTH,  DUNG_ROOM_MAXWIDTH), Util::GetRandom(DUNG_ROOM_MINHEIGHT, DUNG_ROOM_MAXHEIGHT));
    //vSize.Init(5,5);
    switch( direction )
    {
        case DIR_NORTH:
            rcRoom.left        -= Util::GetRandom(1,vSize.x-1); // TODO: all the /2 are to center hallways on rooms
            rcRoom.SetWidth(vSize.x);
            rcRoom.SetHeight(vSize.y, false);
            break;
        case DIR_SOUTH:
            rcRoom.left        -= Util::GetRandom(1,vSize.x-1);
            rcRoom.SetWidth(vSize.x);
            rcRoom.SetHeight(vSize.y);
            break;
        case DIR_WEST:
            rcRoom.top        -= Util::GetRandom(1,vSize.y-1);
            rcRoom.SetWidth(vSize.x, false);
            rcRoom.SetHeight(vSize.y);
            break;
        case DIR_EAST:
            rcRoom.top        -= Util::GetRandom(1,vSize.y-1);
            rcRoom.SetWidth(vSize.x);
            rcRoom.SetHeight(vSize.y);
            break;
        case DIR_NONE:
            rcRoom.SetWidth(vSize.x);
            rcRoom.SetHeight(vSize.y);
            break;
    }
    if( !rcRoom.IsInWorld() )
    {
        rcRoom.top =    CLAMP( rcRoom.top, 0 ,DUNG_HEIGHT );
        rcRoom.bottom = CLAMP( rcRoom.bottom, 0, DUNG_HEIGHT );
        rcRoom.left =   CLAMP( rcRoom.left, 0, DUNG_WIDTH );
        rcRoom.right =  CLAMP( rcRoom.right, 0, DUNG_WIDTH );
    }
}

void CDungeonMap::GetHallRect(JRect &rcHall, const int direction)
{
    int length = Util::GetRandom(DUNG_HALL_MINLENGTH, DUNG_HALL_MAXLENGTH);
    switch( direction )
    {
        case DIR_NORTH:
            rcHall.SetWidth(0);
            rcHall.SetHeight(length, false);
            break;
        case DIR_SOUTH:
            rcHall.SetWidth(0);
            rcHall.SetHeight(length);
            break;
        case DIR_WEST:
            rcHall.SetWidth(length, false);
            rcHall.SetHeight(0);
            break;
        case DIR_EAST:
            rcHall.SetWidth(length);
            rcHall.SetHeight(0);
            break;
    }
    if( !rcHall.IsInWorld() )
    {
        rcHall.top =    CLAMP( rcHall.top, 0 ,DUNG_HEIGHT );
        rcHall.bottom = CLAMP( rcHall.bottom, 0, DUNG_HEIGHT );
        rcHall.left =   CLAMP( rcHall.left, 0, DUNG_WIDTH );
        rcHall.right =  CLAMP( rcHall.right, 0, DUNG_WIDTH );
    }
}

JIVector &CDungeonMap::GetWallOrigin(CDungeonCreationStep *pStep, const int direction)
{
    switch( direction )
    {
        case DIR_NORTH:
            // note: someday, this should be a random spot on the wall, rather than width/2
            pStep->m_vPos.Init(pStep->m_rcArea.Left() + (Util::GetRandom(1,pStep->m_rcArea.Width()-1)), pStep->m_rcArea.Top()-1); // -1... does a room's rect include its walls?
            printf("[%d>%d]creating north %s, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, pStep->m_dwIndex==DUNG_CREATE_STEP_MAKE_ROOM?"hall":"room", VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_SOUTH:
            pStep->m_vPos.Init(pStep->m_rcArea.Left() + (Util::GetRandom(1,pStep->m_rcArea.Width()-1)), pStep->m_rcArea.Bottom()+1);
            printf("[%d>%d]creating south %s, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, pStep->m_dwIndex==DUNG_CREATE_STEP_MAKE_ROOM?"hall":"room", VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_WEST:
            pStep->m_vPos.Init(pStep->m_rcArea.Left()-1, pStep->m_rcArea.Top() + (Util::GetRandom(1,pStep->m_rcArea.Height()-1)));
            printf("[%d>%d]creating west %s, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, pStep->m_dwIndex==DUNG_CREATE_STEP_MAKE_ROOM?"hall":"room", VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_EAST:
            pStep->m_vPos.Init(pStep->m_rcArea.Right()+1, pStep->m_rcArea.Top() + (Util::GetRandom(1,pStep->m_rcArea.Height()-1)));
            printf("[%d>%d]creating east %s, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, pStep->m_dwIndex==DUNG_CREATE_STEP_MAKE_ROOM?"hall":"room", VEC_EXPAND(pStep->m_vPos) );
            break;
    }
    return pStep->m_vPos;
}

JIVector &CDungeonMap::GetHallOrigin(CDungeonCreationStep *pStep)
{
    switch( pStep->m_dwDirection )
    {
        case DIR_NORTH:
            // note: someday, this should be a random spot on the wall, rather than width/2
            pStep->m_vPos.Init(pStep->m_rcArea.Left(), pStep->m_rcArea.Top()-1); // -1... does a room's rect include its walls?
            printf("[%d>%d]creating north hall, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_SOUTH:
            pStep->m_vPos.Init(pStep->m_rcArea.Left(), pStep->m_rcArea.Bottom()+1);
            printf("[%d>%d]creating south hall, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_WEST:
            pStep->m_vPos.Init(pStep->m_rcArea.Left()-1, pStep->m_rcArea.Top());
            printf("[%d>%d]creating west hall, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, VEC_EXPAND(pStep->m_vPos) );
            break;
        case DIR_EAST:
            pStep->m_vPos.Init(pStep->m_rcArea.Right()+1, pStep->m_rcArea.Top());
            printf("[%d>%d]creating east hall, starting at <%d %d>\n", pStep->m_dwRecurDepth, pStep->m_dwRecurDepth+1, VEC_EXPAND(pStep->m_vPos) );
            break;
    }
    
    return pStep->m_vPos;
}
