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

void CPlayer::PickUp( JVector &vPickupPos )
{
    CItem *pItem = g_pGame->GetDungeon()->PickUp(vPickupPos);
    pItem->m_pllLink = m_llInventory->Add(pItem, pItem->m_id->m_dwIndex);
    g_pGame->GetDungeon()->GetTile(vPickupPos)->m_pCurItem = NULL;
    g_pGame->GetMsgs()->Printf( "You have a %s.\n", pItem->GetName() );
}

bool CPlayer::IsWieldable(CLink<CItem> *pItem)
{
    bool retval = false;
    switch( pItem->m_lpData->m_id->m_dwIndex )
    {
        case ITEM_IDX_SWORD:
        case ITEM_IDX_SHIELD:
        case ITEM_IDX_ARMOR:
        case ITEM_IDX_BOW:
        case ITEM_IDX_XBOW:
        case ITEM_IDX_CLOAK:
        case ITEM_IDX_GLOVES:
        case ITEM_IDX_RING:
        case ITEM_IDX_BOOTS:
        case ITEM_IDX_TORCH:
        case ITEM_IDX_AMULET:
            retval = true;
            break;
        default:
            retval = false;
            break;
    }
    return retval;
}

bool CPlayer::Wield(CLink<CItem> *pLink)
{
    CItem *pItem = pLink->m_lpData;
    
    // You can only wield one thing of a given type at a time
    CLink<CItem> *pCurrEquip = m_llEquipment->GetLink(pItem->m_id->m_dwIndex, true);
    if( pCurrEquip != NULL && pCurrEquip->m_dwIndex == pLink->m_dwIndex )
    {
        // So if you're already wearing something of this type, remove it and put it back in inventory
        if( Remove(pCurrEquip) )
        {
            g_pGame->GetMsgs()->Printf("You were wielding the %s...", pCurrEquip->m_lpData->GetName());
        }
        else
        {
            return false;
        }
    }
    // Now put on the new item.
    m_llInventory->Remove(pLink, false);
    pItem->m_pllLink = m_llEquipment->Add(pItem, pItem->m_id->m_dwIndex);
    
    return true;
}

bool CPlayer::IsRemovable(CLink<CItem> *pLink)
{
    CItem *pItem = pLink->m_lpData;
    if( pItem->m_dwFlags & ITEM_FLAG_CURSED )
    {
        return false;
    }
    return true;
}

bool CPlayer::Remove(CLink<CItem> *pLink)
{
    CItem *pItem = pLink->m_lpData;
    if(pItem->m_dwFlags & ITEM_FLAG_CURSED)
    {
        g_pGame->GetMsgs()->Printf("You can't remove the %s... it seems to be cursed.\n", pItem->GetName());
        return false;
    }
    m_llEquipment->Remove(pLink, false);
    pItem->m_pllLink = m_llInventory->Add(pItem, pItem->m_id->m_dwIndex);
    
    return true;
}
