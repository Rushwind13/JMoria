
#include "DungeonMap.h"
//#define MAX_RECURDEPTH 7
#define MAX_RECURDEPTH 7
#define DIR_NONE	-1
#define DIR_NORTH	 0
#define DIR_SOUTH	 1
#define DIR_WEST	 2
#define DIR_EAST	 3
#define DUNG_HALL_MINLENGTH	2
#define DUNG_HALL_MAXLENGTH	8

#define FIXED_DUNGEON
#ifdef FIXED_DUNGEON
Uint8 dungeontiles[DUNG_HEIGHT][DUNG_WIDTH] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,6,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,1,1,1,2,1,1,
	1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
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
void CDungeonMap::CreateDungeon()
{
	JIVector vOrigin;
	m_dmtTiles = new CDungeonMapTile[DUNG_WIDTH * DUNG_HEIGHT];
	JRect rcWorld(0,0, DUNG_WIDTH-1,DUNG_HEIGHT-1);

	// First, fill the whole dungeon with rock
	FillArea(DUNG_IDX_WALL, &rcWorld, DIR_NONE, &vOrigin, false );

#ifdef FIXED_DUNGEON
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
	//JIVector vPos( Util::GetRandom(0,DUNG_WIDTH-1), Util::GetRandom(0,DUNG_HEIGHT-1) );
	//JIVector vPos(DUNG_WIDTH/2, DUNG_HEIGHT/2);
	JIVector vPos(100,98);
	MakeRoom( &vPos, DIR_NONE, 0 );
#endif

}

void CDungeonMap::MakeRoom( const JIVector *vPos, const int direction, const int recurdepth )
{
	if( recurdepth > MAX_RECURDEPTH )
	{
		return;
	}
	// TODO: eventually, put in diff. types of rooms
	// randomly sized rectangular room

	JIVector vOrigin(*vPos);
	JIVector vSize(0,0);
	JRect rcRoom( *vPos, 0,0 );

	int dwDone = -1;
	int count = 0;

	while( dwDone == -1 && count < 100 )
	{
		vSize.Init( Util::GetRandom(DUNG_ROOM_MINWIDTH,  DUNG_ROOM_MAXWIDTH), Util::GetRandom(DUNG_ROOM_MINHEIGHT, DUNG_ROOM_MAXHEIGHT));
		//vSize.Init(5,5);
		switch( direction )
		{
		case DIR_NORTH:
			rcRoom.left		-= (vSize.x / 2);
			rcRoom.right	 = rcRoom.left + (vSize.x - 1);

			rcRoom.bottom	 = rcRoom.top;
			rcRoom.top		 = rcRoom.bottom - (vSize.y-1);
			break;
		case DIR_SOUTH:
			rcRoom.left		-= (vSize.x / 2);
			rcRoom.right	 = rcRoom.left + (vSize.x - 1);

			rcRoom.bottom	 = rcRoom.top + (vSize.y-1);
			break;
		case DIR_WEST:
			rcRoom.top		-= (vSize.y / 2);
			rcRoom.bottom	 = rcRoom.top + (vSize.y - 1);

			rcRoom.right	 = rcRoom.left;
			rcRoom.left		 = rcRoom.right - (vSize.x-1);
			break;
		case DIR_EAST:
			rcRoom.top		-= (vSize.y / 2);
			rcRoom.bottom	 = rcRoom.top + (vSize.y - 1);

			rcRoom.right	 = rcRoom.left + (vSize.x - 1);
			break;
		case DIR_NONE:
			rcRoom.bottom	 = rcRoom.top + (vSize.y - 1);
			rcRoom.right	 = rcRoom.left + (vSize.x - 1);
			break;
		}

		dwDone = CheckArea( &rcRoom, direction, false );
		count++;
	}

	if( dwDone == -1 )
	{
		// can't find a good match for this hallway.
		printf("...room <%d %d> conflicts. terminated.\n", VEC_EXPAND(*vPos));
		return;
	}

	// found a good spot
		FillArea( DUNG_IDX_FLOOR, &rcRoom, direction, &vOrigin, false );
		printf("[%d]Created Room from <%d %d> to <%d %d>\n", recurdepth, RECT_EXPAND(rcRoom));

	int new_direction;
	int num_halls;
	
	num_halls = Util::GetRandom(1,4);
	//num_halls = 1;

	JIVector vHall;
	for(count=0; count < num_halls; count++ )
	{
		new_direction = Util::GetRandom(0,3);
		//new_direction = DIR_WEST;
		switch( new_direction )
		{
		case DIR_NORTH:
			// note: someday, this should be a random spot on the wall, rather than width/2
			vHall.Init(rcRoom.left + (vSize.x / 2), rcRoom.top-1);
			printf("[%d>%d]creating north hall, starting at <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vHall) );
			break;
		case DIR_SOUTH:
			vHall.Init(rcRoom.left + (vSize.x / 2), rcRoom.bottom+1);
			printf("[%d>%d]creating south hall, starting at <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vHall) );
			break;
		case DIR_WEST:
			vHall.Init(rcRoom.left-1, rcRoom.top + (vSize.y / 2));
			printf("[%d>%d]creating west hall, starting at <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vHall) );
			break;
		case DIR_EAST:
			vHall.Init(rcRoom.right+1, rcRoom.top + (vSize.y / 2));
			printf("[%d>%d]creating east hall, starting at <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vHall) );
			break;
		}
		MakeHall( &vHall, new_direction, recurdepth+1 );
	}
}

void CDungeonMap::MakeHall( const JIVector *vPos, const int direction, const int recurdepth )
{
	if( recurdepth > MAX_RECURDEPTH )
	{
		printf( "Hallway at <%d %d> hit recur depth. terminating.\n", VEC_EXPAND(*vPos));
		return;
	}

	int new_direction = direction;
	// random length hallway
	int length;
	JIVector vOrigin;
	JRect rcHall( *vPos, 0,0 );
	bool bEndHallway = false; // this gets set if you get back a hallway that's shorter than you asked for (you hit a room).

	int dwLength = INVALID_LENGTH;
	int count = 0;

	while( dwLength == INVALID_LENGTH && count < 10 )
	{
		length = Util::GetRandom(DUNG_HALL_MINLENGTH, DUNG_HALL_MAXLENGTH);
		//length = 5;
		switch(new_direction)
		{
		case DIR_NORTH:
			rcHall.right = rcHall.left;
			// flop bottom for top on north hallways
			rcHall.bottom	 = rcHall.top;
			rcHall.top		-= (length-1);
			break;
		case DIR_SOUTH:
			rcHall.right	 = rcHall.left;
			rcHall.bottom	 = rcHall.top + (length-1);
			break;
		case DIR_WEST:
			rcHall.bottom	 = rcHall.top;
			// flop left for right on west hallways
			rcHall.right	 = rcHall.left;
			rcHall.left		-= (length-1);
			break;
		case DIR_EAST:
			rcHall.bottom	 = rcHall.top;
			rcHall.right	 = rcHall.left + (length-1);
			break;
		}

		dwLength = CheckArea( &rcHall, direction, true );
		if( dwLength == 0 )
		{
			// this is a dead-end.
			printf("...hallway <%d %d> dead-ends in dir: %d. terminated.\n", VEC_EXPAND(*vPos), direction);
			return;
		}
		if( dwLength > 0 )
		{
			// You got a shorter match than you wanted.
			// flag it (probably close it off with a 
			// door and kill it)
			bEndHallway = true;
			if( direction == DIR_NORTH || direction == DIR_SOUTH )
			{
				rcHall.bottom = rcHall.top + dwLength;
			}
			else
			{
				rcHall.right = rcHall.left + dwLength;
			}
			break;
		}
		count++;
	}

	if( dwLength == INVALID_LENGTH )
	{
		// can't find a good match for this hallway.
		printf("...hallway <%d %d> conflicts. terminated.\n", VEC_EXPAND(*vPos));
		return;
	}
	
	// found a good spot
	printf(" ending at <%d %d>\n", rcHall.right, rcHall.bottom );
	FillArea( DUNG_IDX_FLOOR, &rcHall, direction, &vOrigin, true );

	int what_next = Util::GetRandom(0,2);

	switch( what_next )
	{
	// make more hallways
	case 1:
	{
		int num_halls; 
		num_halls = Util::GetRandom(1,3);
		//num_halls = 1;
		
		int count;
		for(count=0; count < num_halls; count++ )
		{
			new_direction = Util::GetRandom(0,3);
			if( new_direction == direction )
			{
				new_direction = Util::GetRandom(0,3);
			}			
			//new_direction = DIR_SOUTH;
			switch( new_direction )
			{
			case DIR_NORTH:
				printf( "[%d>%d]New north Hallway starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
				vOrigin.y--;
				MakeHall( &vOrigin, new_direction, recurdepth+1 );
				vOrigin.y++;
				break;
			case DIR_SOUTH:
				printf( "[%d>%d]New south Hallway starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
				vOrigin.y++;
				MakeHall( &vOrigin, new_direction, recurdepth+1 );
				vOrigin.y--;
				break;
			case DIR_EAST:
				printf( "[%d>%d]New east Hallway starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
				vOrigin.x++;
				MakeHall( &vOrigin, new_direction, recurdepth+1 );
				vOrigin.x--;
				break;
			case DIR_WEST:
				printf( "[%d>%d]New west Hallway starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
				vOrigin.x--;
				MakeHall( &vOrigin, new_direction, recurdepth+1 );
				vOrigin.x++;
				break;
			}
		}
		// the other hallways will fill in the doors.
		GetTile(vOrigin)->SetType(DUNG_IDX_FLOOR);
		if( dwLength > 1 )
		{
			GetTile(vOrigin - g_vDirDelta[direction])->SetType(DUNG_IDX_DOOR);
		}
	}

	break;

	// end hallway
	case 0:
		//printf("[%d]Hallway ends.\n",recurdepth);
		//return;


	// make another room
	case 2:
		switch( direction )
		{
		case DIR_NORTH:
			vOrigin.y--;
			printf("[%d>%d]Making a north room starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
			break;
		case DIR_SOUTH:
			vOrigin.y++;
			printf("[%d>%d]Making a south room starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
			break;
		case DIR_EAST:
			vOrigin.x++;
			printf("[%d>%d]Making an east room starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
			break;
		case DIR_WEST:
			vOrigin.x--;
			printf("[%d>%d]Making a west room starting at: <%d %d>\n", recurdepth, recurdepth+1, VEC_EXPAND(vOrigin) );
			break;
		}
		MakeRoom( &vOrigin, direction, recurdepth+1 );
		break;
	}
}

// When checking the area, check one unit radius larger than the requested area, so you 
// don't eat someone else's wall.
int CDungeonMap::CheckArea( const JRect *rcCheck, int direction, bool bIsHallway )
{
	JRect rcLocal(*rcCheck);

	if( !bIsHallway )
	{		
		// expand the search so you don't eat someone else's wall
		// (leave ## between rooms)
		switch( direction )
		{
		case DIR_NORTH:
			rcLocal.left-=1;
			rcLocal.right+=1;
			rcLocal.top-=1;
			//rcLocal.bottom+=1;
			break;
		case DIR_SOUTH:
			rcLocal.left-=1;
			rcLocal.right+=1;
			//rcLocal.top-=1;
			rcLocal.bottom+=1;
			break;
		case DIR_EAST:
			//rcLocal.left-=1;
			rcLocal.right+=1;
			rcLocal.top-=1;
			rcLocal.bottom+=1;
			break;
		case DIR_WEST:
			rcLocal.left-=1;
			//rcLocal.right+=1;
			rcLocal.top-=1;
			rcLocal.bottom+=1;
			break;
		case DIR_NONE:
			break;
		}
	}
	
	if( !rcLocal.IsInWorld() )
	{
		return INVALID_LENGTH;
	}
	printf("checking from <%d %d> to <%d %d>...\n", RECT_EXPAND(*rcCheck));
	
	int x,y;
	JIVector vCurPos;
	if( bIsHallway )
	{
		//Hallways do this differently than rooms do.
		// If they find an open floor spot, they just 
		// place a door and terminate.
		if( direction == DIR_NORTH || direction == DIR_SOUTH )
		{
			vCurPos.x = rcLocal.left;
			for( y=rcLocal.top; y <=rcLocal.bottom; y++ )
			{
				vCurPos.y = y;
				if( GetTile(vCurPos)->GetType() != DUNG_IDX_WALL )
				{
					printf( "Hallway <%d %d> got %d. (wanted %d)\n", rcCheck->left, rcCheck->top, y - rcLocal.top, rcLocal.bottom - rcLocal.top );

					return y - rcLocal.top;
				}
			}

			return rcLocal.bottom - rcLocal.top;
		}
		else
		{
			y = rcLocal.top;
			for( x=rcLocal.left; x <=rcLocal.right; x++ )
			{
				if( m_dmtTiles[y * DUNG_WIDTH + x].GetType() != DUNG_IDX_WALL )
				{
					printf( "Hallway <%d %d> got %d. (wanted %d)\n", rcCheck->left, rcCheck->top, x - rcLocal.left, rcLocal.right - rcLocal.left );
					
					return x - rcLocal.left;
				}
			}

			return rcLocal.right - rcLocal.left;
		}
	}
	else
	{
		for( y = rcLocal.top; y <= rcLocal.bottom; y++ )
		{
			for( x = rcLocal.left; x <= rcLocal.right; x++ )
			{
				if( m_dmtTiles[y * DUNG_WIDTH + x].GetType() != DUNG_IDX_WALL )
				{
					printf("nope. <%d %d> was %d\n", x,y, m_dmtTiles[y * DUNG_WIDTH + x].GetType() );
					return INVALID_LENGTH;
				}
			}
		}
	}
	
	printf("success!\n");
	return 0;
}

void CDungeonMap::FillArea( const Uint8 type, const JRect *rcFill, const int direction, JIVector *vOrigin, bool bIsHallway )
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
		GetTile(vTail)->SetType(DUNG_IDX_DOOR);
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
			int foo=1;
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

