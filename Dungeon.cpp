// Dungeon.cpp
//
// implementation of the Dungeon

#include "Dungeon.h"

// TODO: this is for the collision defines; should move those someplace more useful --Jimbo
#include "CmdState.h"
#include "Render.h"
#include "Player.h"
#include "FileParse.h"
#include "DisplayText.h"

unsigned char TileIDs[DUNG_IDX_MAX+1]			= ".#+'<<>>:@";
int			  ModifiedTileTypes[DUNG_IDX_MAX+1] =
{
	DUNG_IDX_INVALID,
	DUNG_IDX_INVALID,
	DUNG_IDX_OPEN_DOOR,
	DUNG_IDX_DOOR,
	DUNG_IDX_INVALID,
	DUNG_IDX_INVALID,
	DUNG_IDX_FLOOR,
	DUNG_IDX_INVALID
};
//extern Uint8 dungeontiles[DUNG_HEIGHT][DUNG_WIDTH];

// Setup - the one-time-run stuff to set up the Dungeon
//
// create all the dungeon tile types,
// read the monsters config data

void CDungeon::Init()
{
	int i;
	// Initialize all the Dungeon stuff, baby.
	m_dtdlist = new CDungeonTileDef[DUNG_IDX_MAX];

	// Create the dungeon tile defs (at the moment, there are 7 or so entries in this array)
	for( i = DUNG_IDX_FLOOR; i < DUNG_IDX_MAX; i++ )
	{
		m_dtdlist[i].m_dwType = i;
		m_dtdlist[i].m_dwModifiedType = ModifiedTileTypes[i];
		m_dtdlist[i].m_dwIndex = TileIDs[i] - ' ' - 1;
		switch(m_dtdlist[i].m_dwType)
		{
		case DUNG_IDX_FLOOR:
			m_dtdlist[i].m_Color.SetColor(192,192,192,255);
			break;
		case DUNG_IDX_WALL:
			m_dtdlist[i].m_Color.SetColor(64,64,64,255);
			break;
		case DUNG_IDX_DOOR:
			m_dtdlist[i].m_Color.SetColor(64,32,128,255);
			break;
		case DUNG_IDX_OPEN_DOOR:
			m_dtdlist[i].m_Color.SetColor(192,192,192,255);
                break;
        case DUNG_IDX_UPSTAIRS:
        case DUNG_IDX_LONG_UPSTAIRS:
			m_dtdlist[i].m_Color.SetColor(175,175,175,255);
                break;
        case DUNG_IDX_DOWNSTAIRS:
        case DUNG_IDX_LONG_DOWNSTAIRS:
			m_dtdlist[i].m_Color.SetColor(195,195,195,255);
			break;
		case DUNG_IDX_RUBBLE:
			m_dtdlist[i].m_Color.SetColor(200,100,64,255);
			break;
		case DUNG_IDX_PLAYER:
			m_dtdlist[i].m_Color.SetColor(255,255,255,255);
			break;
		default:
			m_dtdlist[i].m_Color.SetColor(0,0,0,255);
			break;
		}
	}

    // Load the monster list from config
    // TODO: Make this a method on CMonsterDef.
    m_llMonsterDefs = new JLinkList<CMonsterDef>;
    
    CMonsterDef *pmd;
    CDataFile dfMonsters;
    dfMonsters.Open("Resources/Monsters.txt");
    
    pmd = new CMonsterDef;
    while( dfMonsters.ReadMonster(*pmd) )
    {
        m_llMonsterDefs->Add(pmd);
        pmd = new CMonsterDef;
    }
    
    delete pmd;
    
    // Load the item list from config
    // TODO: Make this a method on CItemDef.
    m_llItemDefs = new JLinkList<CItemDef>;
    
    CItemDef *pid;
    CDataFile dfItems;
    dfItems.Open("Resources/Items.txt");
    
    pid = new CItemDef;
    while( dfItems.ReadItem(*pid) )
    {
        m_llItemDefs->Add(pid);
        pid = new CItemDef;
    }
    
    delete pid;
    
    // Load the graphics
    // Just one tile set at the moment.
	m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );
    
    CreateNewLevel(DUNG_CFG_START_LEVEL);
}

JResult CDungeon::TerminateLevel()
{
    if( m_Tiles )
    {
        delete [] m_Tiles;
        m_Tiles = NULL;
    }
    if( m_dmCurLevel )
    {
        delete m_dmCurLevel;
        m_dmCurLevel = NULL;
    }
    if( m_llMonsters )
    {
        m_llMonsters->Terminate();
        delete m_llMonsters;
        m_llMonsters = NULL;
    }
    if( m_llItems )
    {
        m_llItems->Terminate();
        delete m_llItems;
        m_llItems = NULL;
    }
    
    return JSUCCESS;
}

JResult CDungeon::CreateNewLevel(const int delta)
{
    depth += delta;
    if( depth < 1 ) depth = 1;
    if( depth > DUNG_MAXDEPTH ) depth = DUNG_MAXDEPTH;
    
    CreateMap();
    
    // Place items appropriate to this level.
    PlaceItems(depth);
    
    // Spawn monsters appropriate to this level.
    SpawnMonsters(depth);
    
    return JSUCCESS;
}

JResult CDungeon::CreateMap()
{
    m_Tiles = new CDungeonTile[DUNG_HEIGHT*DUNG_WIDTH];
    // Create the randomized dungeon
    m_dmCurLevel = new CDungeonMap;
    m_dmCurLevel->CreateDungeon(depth);
    
    JIVector vDungeon(DUNG_HEIGHT,DUNG_WIDTH);
    Uint8 dung_tile_type = DUNG_IDX_INVALID;
    float open_area = 0.0f;
    // Create the dungeon array (at the moment, there are 16*16 entries in this list)
    // Set position, Tile type, and Flags for entire dungeon
    for( vDungeon.y = 0; vDungeon.y < DUNG_HEIGHT; vDungeon.y++ )
    {
        for( vDungeon.x = 0; vDungeon.x < DUNG_WIDTH; vDungeon.x++ )
        {
            GetITile(vDungeon)->m_vPos.x = (float)vDungeon.x;
            GetITile(vDungeon)->m_vPos.y = (float)vDungeon.y;
            dung_tile_type = m_dmCurLevel->GetdtdIndex(vDungeon);
            if( dung_tile_type == DUNG_IDX_FLOOR ) open_area++;
            GetITile(vDungeon)->m_dtd = &m_dtdlist[dung_tile_type];
            GetITile(vDungeon)->m_dwFlags = m_dmCurLevel->GetFlags(vDungeon);
        }
    }
    
    m_fOpenFloorArea = open_area;
    return JSUCCESS;
}

JResult CDungeon::PlaceItems(const int depth)
{
    m_llItems = new JLinkList<CItem>;
    
    int desired_items = int (m_fOpenFloorArea * DUNG_CFG_MONSTERS_PER_LEVEL);
    printf("Possible spawn points: %0.2f  desired items: %d\n", m_fOpenFloorArea, desired_items);
    
    while( desired_items > 0 )
    {
        int which_item = ChooseItemForDepth(depth);
        
        CItemDef *chosen_item = m_llItemDefs->GetLink(which_item)->m_lpData;
        printf("Choosing item %d, called %s\n", which_item, chosen_item->m_szName);
        
        CItem::CreateItem(chosen_item);
        
        desired_items--;
    }
    
    return JSUCCESS;
}

JResult CDungeon::SpawnMonsters(const int depth)
{
    m_llMonsters = new JLinkList<CMonster>;
    
    int desired_monsters = int (m_fOpenFloorArea * DUNG_CFG_MONSTERS_PER_LEVEL);
    printf("Possible spawn points: %0.2f  desired monsters: %d\n", m_fOpenFloorArea, desired_monsters);
    
    while( desired_monsters > 0 )
    {
        
        // Spawn a monster into dungeon
        // TODO: make this into a method on CMonster.
#define RANDOM_MONSTER
#ifdef RANDOM_MONSTER
        int which_monster = ChooseMonsterForDepth(depth);
#else
        int which_monster = m_llMonsterDefs->length()-1;
        //which_monster = 0;
#endif // RANDOM_MONSTER
        if( which_monster == MON_IDX_INVALID )
        {
            printf("Couldn't find a suitable monster.\n");
            return JERROR();
        }
        CMonsterDef *chosen_monster = m_llMonsterDefs->GetLink(which_monster)->m_lpData;
        printf("Choosing monster %d, called %s\n", which_monster, chosen_monster->m_szName);
        
        CMonster::CreateMonster(chosen_monster);
        
        desired_monsters--;
    }
    
    return JSUCCESS;
}

int CDungeon::ChooseItemForDepth(const int depth)
{
    int which_item = ITEM_IDX_INVALID;
    int count = 0;
    while( count < DUNG_CFG_MAX_SPAWN_TRIES )
    {
        which_item = Util::GetRandom(0, m_llItemDefs->length()-1);
        CItemDef *chosen_item = m_llItemDefs->GetLink(which_item)->m_lpData;
        if( abs(depth - chosen_item->m_dwLevel) < 5 )
        {
            break;
        }
        count++;
    }
    
    return which_item;
}

int CDungeon::ChooseMonsterForDepth(const int depth)
{
    int which_monster = MON_IDX_INVALID;
    int count = 0;
    while( count < DUNG_CFG_MAX_SPAWN_TRIES )
    {
        which_monster = Util::GetRandom(0, m_llMonsterDefs->length()-1);
        CMonsterDef *chosen_monster = m_llMonsterDefs->GetLink(which_monster)->m_lpData;
        if( abs(depth - chosen_monster->m_dwLevel) < 5 )
        {
            break;
        }
        count++;
    }
    
    return which_monster;
}

JResult CDungeon::OnChangeLevel(const int delta)
{
    printf("Changing level...");
    // Clean up old level, then
    TerminateLevel();
    
    // Create new level
    CreateNewLevel(delta);
    printf("done.\n");
    printf("You pass through a one-way door, to arrive on level %d.\n", depth);
    g_pGame->GetMsgs()->Printf("You pass through a one-way door, to arrive on level %d.\n", depth);
    
    return JSUCCESS;
}

bool CDungeon::Update(float fCurTime)
{
	return true;
}

bool CDungeon::IsOnScreen(JVector vPos)
{
    // 12.7.2017 - the below code is totally broken and
    // was causing monsters/items not to display if they were "too far"
    // from the player. MIN/MAX offsets are set to totally bogus values
    // which is the actual bug, but revisit this if you are either
    // 1) trying to do sight-distance, or
    // 2) trying to speed things up by not drawing off-screen stuff,
    //    once big dungeons are happening.
//    if( g_pGame->GetPlayer() == NULL )
//    {
//        return false;
//    }
//
//    JVector vPlayer = g_pGame->GetPlayer()->m_vPos;
//    if( vPos.x < vPlayer.x - SCREEN_MIN_XOFF ||
//        vPos.x > vPlayer.x + SCREEN_MAX_XOFF ||
//        vPos.y < vPlayer.y - SCREEN_MIN_YOFF ||
//        vPos.y > vPlayer.y + SCREEN_MAX_YOFF )
//    {
//        return false;
//    }
	return true;
}

void CDungeon::Draw()
{
	
	PreDraw();
    
    // Select Our Texture
    DrawDungeon();
    
    DrawItems();
    
    DrawMonsters();
    
	PostDraw();
}

void CDungeon::DrawDungeon()
{
    // Brute force method; optimize this later
    JVector vScreen;
    JVector vSize(1,1);
    JVector vPlayer = g_pGame->GetPlayer()->m_vPos;
    
    for( vScreen.x = 0; vScreen.x < DUNG_WIDTH; vScreen.x++ )
    {
        for( vScreen.y = 0; vScreen.y < DUNG_HEIGHT; vScreen.y++ )
        {/* */
            
            /*for( vScreen.x = vPlayer.x - SCREEN_MIN_XOFF; vScreen.x < vPlayer.x + SCREEN_MAX_XOFF; vScreen.x++ )
             {
             for( vScreen.y = vPlayer.y - SCREEN_MIN_YOFF; vScreen.y < vPlayer.y + SCREEN_MAX_YOFF; vScreen.y++ )
             {/* */
            CDungeonTile *curTile = GetTile(vScreen);
            
            // this tile doesn't exist, or it's not lit
            // or something else is standing there
            if( curTile == NULL ||
               ( (curTile->m_dwFlags & DUNG_FLAG_LIT) == 0 ) ||
               vScreen == vPlayer ||
               curTile->m_pCurItem != NULL ||
               curTile->m_pCurMonster != NULL )
            {
                continue;
            }
            m_TileSet->SetTileColor( curTile->m_dtd->m_Color );
            m_TileSet->DrawTile( curTile->m_dtd->m_dwIndex, vScreen, vSize, true );
        }
    }

}

void CDungeon::DrawItems()
{
    CLink <CItem> *pLink = m_llItems->GetHead();
    CItem *pItem;
    
    while(pLink != NULL)
    {
        pItem = pLink->m_lpData;
        if( pItem && IsOnScreen(pItem->m_vPos) )
        {
            pItem->Draw();
        }
        pLink = m_llItems->GetNext(pLink);
    }
}

void CDungeon::DrawMonsters()
{
    CLink <CMonster> *pLink = m_llMonsters->GetHead();
    CMonster *pMon;
    
    while(pLink != NULL)
    {
        pMon = pLink->m_lpData;
        if( pMon && IsOnScreen(pMon->GetPos()) )
        {
            pMon->Draw();
        }
        pLink = m_llMonsters->GetNext(pLink);
    }
}

void CDungeon::PreDraw()
{
	if( g_pGame->GetPlayer() != NULL )
	{
        int xinitval = m_dwZoom;
        //int xinitval = 16;
		int yinitval = xinitval;

//#define ORIGIN_PLAYER
#ifdef ORIGIN_PLAYER
		int xorigin = (int)g_pGame->GetPlayer()->m_vPos.x - DUNG_WIDTH/2;
		int yorigin = (int)g_pGame->GetPlayer()->m_vPos.y - DUNG_HEIGHT/2;
#else
		int xorigin = 0; // + is left (?!)
		int yorigin = 0; // + is up
#endif
        m_Rect.Init( xorigin-xinitval,yorigin+yinitval,xorigin+xinitval,yorigin-yinitval);
	}
	g_pGame->GetRender()->PreDrawObjects( m_Rect, m_TileSet->Texture(), true, false, &m_vfTranslate );

	// Do any external setup that needs doing.
	m_TileSet->PreDrawTile();
}

void CDungeon::PostDraw()
{
	// Do any external teardown that needs doing.
	m_TileSet->PostDrawTile();

	g_pGame->GetRender()->PostDrawObjects();
}

void CDungeon::Term()
{
	if( m_Tiles )
	{
		delete [] m_Tiles;
		m_Tiles = NULL;
    }
    
    if( m_dtdlist )
    {
        delete [] m_dtdlist;
        m_dtdlist = NULL;
    }
    
    if( m_llMonsterDefs )
    {
        m_llMonsterDefs->Terminate();
        delete m_llMonsterDefs;
        m_llMonsterDefs = NULL;
    }
    
    if( m_llItemDefs )
    {
        m_llItemDefs->Terminate();
        delete m_llItemDefs;
        m_llItemDefs = NULL;
    }
}

void CDungeon::RemoveMonster( CMonster *pMon )
{
	CLink <CMonster> *pLink;
	pLink = pMon->m_pllLink;
	
	GetTile(pMon->GetPos())->m_pCurMonster = NULL;
	m_llMonsters->Remove(pLink);
}

int CDungeon::IsWalkableFor( JVector &vPos, bool isPlayer )
{
	// Check for someone else standing there first (handles things that can walk thru walls)
	CDungeonTile *curTile = GetTile(vPos);
	if( curTile == NULL )
	{
		printf("Hey! That's a bad tile.\n");
		return false;
	}
	if( curTile->m_pCurMonster != NULL )
	{
        // Monsters can collide with other monsters
		return DUNG_COLL_MONSTER;
	}
    else if( !isPlayer
            && g_pGame->GetPlayer()
            && g_pGame->GetPlayer()->m_bHasSpawned
            && g_pGame->GetPlayer()->m_vPos == vPos )
    {
        // Monsters colliding with the player can be hazardous to your health.
        printf("Monster attacking not implemented yet.\n");
        return DUNG_COLL_PLAYER;
    }
    else if( isPlayer && curTile->m_pCurItem != NULL )
    {
        return DUNG_COLL_ITEM;
    }
	
	// If you get here, the square was unoccupied. Now check for running into inanimates...
	int type = curTile->m_dtd->m_dwType;
	switch( type )
	{
	case DUNG_IDX_WALL:
	case DUNG_IDX_DOOR:
	case DUNG_IDX_RUBBLE:
		return type;
		break;
	default:
		return DUNG_COLL_NO_COLLISION;
		break;
	}
}

int CDungeon::CanPlaceItemAt(JVector &vPos)
{
    CDungeonTile *curTile = GetTile(vPos);
    if( curTile == NULL )
    {
        printf("Hey! That's a bad tile.\n");
        return false;
    }
    if( curTile->m_pCurItem != NULL )
    {
        // Items can collide with other Items
        return DUNG_COLL_ITEM;
    }
    
    // If you get here, the square was unoccupied. Now check for running into inanimates...
    int type = curTile->m_dtd->m_dwType;
    switch( type )
    {
        case DUNG_IDX_WALL:
        case DUNG_IDX_DOOR:
        case DUNG_IDX_OPEN_DOOR:
        case DUNG_IDX_RUBBLE:
        case DUNG_IDX_UPSTAIRS:
        case DUNG_IDX_LONG_UPSTAIRS:
        case DUNG_IDX_DOWNSTAIRS:
        case DUNG_IDX_LONG_DOWNSTAIRS:
            return type;
            break;
        default:
            return DUNG_COLL_NO_COLLISION;
            break;
    }
}

bool CDungeon::IsOpenable( JVector &vPos )
{
	// trivial check; is this a modifiable tile at all?
	CDungeonTile *curTile = GetTile(vPos);
	if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
	{
		return false;
	}
	// Check for someone else standing there first (handles things that can walk thru walls)
	
	if( curTile->m_pCurItem )
	{
		return( curTile->m_pCurItem->IsOpenable() );
	}
	
	// If you get here, the square was unoccupied. Now check for running into inanimates...
	return( curTile->m_dtd->m_dwType == DUNG_IDX_DOOR);
}

bool CDungeon::IsTunnelable( JVector &vPos )
{
	// trivial check; is this a modifiable tile at all?
	CDungeonTile *curTile = GetTile(vPos);
	if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
	{
		return false;
	}
	// Check for someone else standing there first (handles things that can walk thru walls)
	
	if( curTile->m_pCurItem )
	{
		return( curTile->m_pCurItem->IsTunnelable() );
	}
	
	// If you get here, the square was unoccupied. Now check for running into inanimates...
	return( curTile->m_dtd->m_dwType == DUNG_IDX_RUBBLE);
}

bool CDungeon::IsCloseable( JVector &vPos )
{
	// trivial check; is this a modifiable tile at all?
	CDungeonTile *curTile = GetTile(vPos);
	if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
	{
		return false;
	}
	// Check for someone else standing there first (handles things that can walk thru walls)
	
	if( curTile->m_pCurItem )
	{
		return( curTile->m_pCurItem->IsCloseable() );
	}
	
	// If you get here, the square was unoccupied. Now check for running into inanimates...
	return( curTile->m_dtd->m_dwType == DUNG_IDX_OPEN_DOOR);
}

int CDungeon::IsStairs( JVector &vPos )
{
    // trivial check; is this a modifiable tile at all?
    CDungeonTile *curTile = GetTile(vPos);
    if( curTile == NULL )
    {
        return DUNG_IDX_INVALID;
    }
    
    if( curTile->m_dtd->m_dwType == DUNG_IDX_UPSTAIRS ||
       curTile->m_dtd->m_dwType == DUNG_IDX_LONG_UPSTAIRS ||
       curTile->m_dtd->m_dwType == DUNG_IDX_DOWNSTAIRS ||
       curTile->m_dtd->m_dwType == DUNG_IDX_LONG_DOWNSTAIRS )
    {
        return curTile->m_dtd->m_dwType;
    }
    
    // If you get here, the square was not a staircase
    return DUNG_IDX_INVALID;
}

JResult CDungeon::Modify( JVector &vPos )
{
	if( GetTile(vPos)->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
	{
		// hey! you can't modify that tile! How did you get here?!
		return JERROR();
	}

	GetTile( vPos )->m_dtd = &m_dtdlist[GetTile(vPos)->m_dtd->m_dwModifiedType];

	return JSUCCESS;
}
