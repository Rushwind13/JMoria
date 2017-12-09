// Player.cpp
//
// implementation of the Player

#include "Player.h"
#include "TileSet.h"
#include "Game.h"
#include "Dungeon.h"
#include "DisplayText.h"

extern CGame *g_pGame;

void CPlayer::Init()
{
	// Initialize all the player stuff, baby.
	m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );

	// This will likely get moved somewhere else. --Jimbo
	SpawnPlayer();
}

bool CPlayer::Update(float fCurTime)
{
    DisplayInventory();
    DisplayEquipment();
	return true;
}

void CPlayer::Draw()
{
	Uint8 player_tile = '@' - ' ' - 1;//TileIDs[TILE_IDX_PLAYER] - ' ' - 1;
	JVector vSize(1,1);
    JColor player_color(255,255,255,255);

	PreDraw();
    m_TileSet->SetTileColor(player_color);
	m_TileSet->DrawTile( player_tile, m_vPos, vSize, true );
	PostDraw();
}

void CPlayer::PreDraw()
{
	g_pGame->GetDungeon()->PreDraw();
}

void CPlayer::PostDraw()
{
	g_pGame->GetDungeon()->PostDraw();
}

JResult CPlayer::SpawnPlayer()
{
	printf("Trying to spawn player...");
	JVector vTryPos;
	while( !m_bHasSpawned )
	{
		vTryPos.Init( (float)Util::GetRandom(0, DUNG_WIDTH-1), (float)Util::GetRandom(0, DUNG_HEIGHT-1) );

		//printf("Trying to spawn player at <%.2f %.2f>...\n", vTryPos.x, vTryPos.y);
		//g_pGame->GetMsgs()->Printf( "Trying to spawn player at <%.2f %.2f>...\n", vTryPos.x, vTryPos.y );

		// try changing this to "iswalkable" -- might need to move that to dungeon. --Jimbo
		if( g_pGame->GetDungeon()->IsWalkableFor(vTryPos, true) == DUNG_COLL_NO_COLLISION )
		{
			m_vPos = vTryPos;
			m_bHasSpawned = true;
			printf("Success!\n");
			//g_pGame->GetMsgs()->Printf( "Success!\n" );
		}
	}

	return JSUCCESS;
}

void CPlayer::DisplayInventory()
{
    CLink<CItem> *pLink = m_llInventory->GetHead();
    CItem *pItem;
    g_pGame->GetInv()->Clear();
    g_pGame->GetInv()->Printf("You are carrying:\n");
    char cInventoryId = 'a';
    while(pLink != NULL)
    {
        pItem = pLink->m_lpData;
        g_pGame->GetInv()->Printf("%c - %s\n", cInventoryId, pItem->GetName());
        
        if( cInventoryId < 'z' )
        {
            cInventoryId++;
        }
        else
        {
            printf("Inventory past first page not shown.\n");
            break;
        }
        pLink = m_llInventory->GetNext(pLink);
    }
    
}

void CPlayer::DisplayEquipment()
{
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    CItem *pItem;
    g_pGame->GetEquip()->Clear();
    g_pGame->GetEquip()->Printf("You are wearing:\n");
    char cEquipmentId = 'a';
    while(pLink != NULL)
    {
        pItem = pLink->m_lpData;
        g_pGame->GetEquip()->Printf("%c - %s\n", cEquipmentId, pItem->GetName());
        
        if( cEquipmentId < 'z' )
        {
            cEquipmentId++;
        }
        else
        {
            printf("Equipment is limited to N items, one each for specific body parts.\n");
            break;
        }
        pLink = m_llEquipment->GetNext(pLink);
    }
    
}
