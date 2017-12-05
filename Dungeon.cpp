// Dungeon.cpp
//
// implementation of the Dungeon

#include "Dungeon.h"

// TODO: this is for the collision defines; should move those someplace more useful --Jimbo
#include "CmdState.h"
#include "Render.h"
#include "Player.h"
#include "FileParse.h"

unsigned char TileIDs[DUNG_IDX_MAX+1]			= ".#+'<>:@";
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
			m_dtdlist[i].m_Color.SetColor(175,175,175,255);
			break;
		case DUNG_IDX_DOWNSTAIRS:
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

	m_Tiles = new CDungeonTile[DUNG_HEIGHT*DUNG_WIDTH];
	JIVector vDungeon(DUNG_HEIGHT,DUNG_WIDTH);

	// Create the randomized dungeon
	m_dmCurLevel = new CDungeonMap;
	m_dmCurLevel->CreateDungeon();

	// Create the dungeon array (at the moment, there are 16*16 entries in this list)
	for( vDungeon.y = 0; vDungeon.y < DUNG_HEIGHT; vDungeon.y++ )
	{
		for( vDungeon.x = 0; vDungeon.x < DUNG_WIDTH; vDungeon.x++ )
		{
			GetITile(vDungeon)->m_vPos.x = (float)vDungeon.x;
			GetITile(vDungeon)->m_vPos.y = (float)vDungeon.y;
			GetITile(vDungeon)->m_dtd = &m_dtdlist[m_dmCurLevel->GetdtdIndex(vDungeon)];
			GetITile(vDungeon)->m_dwFlags = m_dmCurLevel->GetFlags(vDungeon);
		}
	}

	// Create the monster list
	m_llMonsters = new JLinkList<CMonster>;
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

#ifdef RANDOM_MONSTER
    int which_monster = Util::GetRandom(0, m_llMonsterDefs->length()-1);
    printf("trying to spawn monster %d\n", which_monster);
#else
    int which_monster = m_llMonsterDefs->length()-1;
#endif // RANDOM_MONSTER
    CMonsterDef *chosen_monster = m_llMonsterDefs->GetLink(which_monster)->m_lpData;
    
    CMonster *pMon;
    pMon = new CMonster;
    pMon->Init(chosen_monster);
    
    
    pMon->m_pllLink = m_llMonsters->Add(pMon);
    g_pGame->GetAIMgr()->m_llAIBrains->Add(pMon->m_pBrain);
    pMon->m_pBrain->SetParent(pMon);
    
	/*
	CMonster *pMon, *pMon2, *pMon3;
	CMonsterDef *pmd, *pmd2, *pmd3;

	pMon = new CMonster;
	pMon2 = new CMonster;
	pMon3 = new CMonster;

	pmd = new CMonsterDef;
	pmd2 = new CMonsterDef;
	pmd3 = new CMonsterDef;

	// jelly; monster 1
	pMon2->m_pBrain->m_fSpeed = 0.0f;
	pMon2->m_pBrain->m_dwMoveType = MON_AI_DONTMOVE;
	pMon2->m_pBrain->SetState(BRAINSTATE_REST);
	pmd2->m_fBaseHP = 10.0f;
	pmd2->m_fBaseAC = 10.0f;
	pmd2->m_dwIndex = MonIDs[MON_IDX_JELLY] - ' ' - 1;
	pmd2->m_dwType = MON_IDX_JELLY;
	pmd2->m_dwFlags = MON_FLAG_TOUCH;
	pmd2->m_szName = new char[32];
	pmd2->m_Color.SetColor(255,0,0,255);
	sprintf( pmd2->m_szName, "Red Jelly" );

	pMon2->Init(pmd2);

	pMon2->m_pllLink = m_llMonsters->Add(pMon2);
	g_pGame->GetAIMgr()->m_llAIBrains->Add(pMon2->m_pBrain);
	pMon2->m_pBrain->SetParent(pMon2);

	// mushroom patch; monster 2
	pMon->m_pBrain->m_fSpeed = 0.0f;
	pMon->m_pBrain->m_dwMoveType = MON_AI_DONTMOVE;
	pMon->m_pBrain->SetState(BRAINSTATE_REST);
	pmd->m_fBaseHP = 4.0f;
	pmd->m_fBaseAC = 50.0f;
	pmd->m_dwIndex = MonIDs[MON_IDX_SHROOM] - ' ' - 1;
	pmd->m_dwType = MON_IDX_SHROOM;
	pmd->m_dwFlags = MON_FLAG_SPORE;
	pmd->m_szName = new char[32];
	pmd->m_Color.SetColor(255,255,0,255);
	sprintf( pmd->m_szName, "Yellow Mushroom Patch" );

	pMon->Init(pmd);

	pMon->m_pllLink = m_llMonsters->Add(pMon);
	g_pGame->GetAIMgr()->m_llAIBrains->Add(pMon->m_pBrain);
	pMon->m_pBrain->SetParent(pMon);

	// icky thing; random mover
	pMon3->m_pBrain->m_fSpeed = 1.0f;
	pMon3->m_pBrain->m_dwMoveType = MON_AI_100RANDOMMOVE;
	pMon3->m_pBrain->SetState(BRAINSTATE_SEEK);
	pmd3->m_fBaseHP = 4.0f;
	pmd3->m_fBaseAC = 50.0f;
	pmd3->m_dwIndex = MonIDs[MON_IDX_ICKY] - ' ' - 1;
	pmd3->m_dwType = MON_IDX_ICKY;
	pmd3->m_dwFlags = MON_FLAG_TOUCH|MON_AI_100RANDOMMOVE;
	pmd3->m_szName = new char[32];
	pmd3->m_Color.SetColor(0,0,255,255);
	sprintf( pmd3->m_szName, "Blue Icky Thing" );

	pMon3->Init(pmd3);

	pMon3->m_pllLink = m_llMonsters->Add(pMon3);
	g_pGame->GetAIMgr()->m_llAIBrains->Add(pMon3->m_pBrain);
	pMon3->m_pBrain->SetParent(pMon3);
	/* */
	//

	m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );
}

bool CDungeon::Update(float fCurTime)
{
	return true;
}

bool CDungeon::IsOnScreen(JVector vPos)
{
	if( g_pGame->GetPlayer() == NULL )
	{
		return false;
	}

	JVector vPlayer = g_pGame->GetPlayer()->m_vPos;
	if( vPos.x < vPlayer.x - SCREEN_MIN_XOFF ||
		vPos.x > vPlayer.x + SCREEN_MAX_XOFF ||
		vPos.y < vPlayer.y - SCREEN_MIN_YOFF ||
		vPos.y > vPlayer.y + SCREEN_MAX_YOFF )
	{
		return false;
	}

	return true;
}

void CDungeon::Draw()
{
	// Brute force method; optimize this later
	JVector vScreen;
	JVector vSize(1,1);
	JVector vPlayer = g_pGame->GetPlayer()->m_vPos;
	
	PreDraw();
    // Select Our Texture
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
			if( curTile == NULL || 
				( (curTile->m_dwFlags & DUNG_FLAG_LIT) == 0 ) )
			{
				continue;
			}
			m_TileSet->SetTileColor( curTile->m_dtd->m_Color );
			m_TileSet->DrawTile( curTile->m_dtd->m_dwIndex, vScreen, vSize, true );
		}
	}

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
	PostDraw();
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
}

void CDungeon::RemoveMonster( CMonster *pMon )
{
	CLink <CMonster> *pLink;
	pLink = pMon->m_pllLink;
	
	GetTile(pMon->GetPos())->m_pCurMonster = NULL;
	m_llMonsters->Remove(pLink);
}


int CDungeon::IsWalkable( JVector &vPos )
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
		return DUNG_COLL_MONSTER;
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
