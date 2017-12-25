// Player.cpp
//
// implementation of the Player

#include "Player.h"
#include "TileSet.h"
#include "Game.h"
#include "Dungeon.h"
#include "DisplayText.h"
#include "JLinkList.h"
#include "StateBase.h"

extern CGame *g_pGame;

void CPlayer::Init()
{
	// Initialize all the player stuff, baby.
//    m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );
    m_TileSet = new DUNG_TILESET;

	// This will likely get moved somewhere else. --Jimbo
    SpawnPlayer();
}

bool CPlayer::Update(float fCurTime)
{
    DisplayStats();
    DisplayInventory(PLACEMENT_INV);
    DisplayEquipment(PLACEMENT_EQUIP);
	return true;
}

void CPlayer::Draw()
{
	Uint8 player_tile = '@' - ' ' - 1;//TileIDs[TILE_IDX_PLAYER] - ' ' - 1;
	JVector DUNG_ASPECT;
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

void CPlayer::DisplayStats()
{
    g_pGame->GetStats()->Clear();
    g_pGame->GetStats()->Printf("Name: %s\n", m_szName);
    g_pGame->GetStats()->Printf("Race: %s\n", m_pRace->m_szName);
    g_pGame->GetStats()->Printf("Class: %s\n", m_pClass->m_szName);
    g_pGame->GetStats()->Printf("\n");
    g_pGame->GetStats()->Printf("AC: %d\n", (int)m_fArmorClass);
    g_pGame->GetStats()->Printf("HP: %d / %d\n", (int)m_fCurHitPoints, (int)m_fHitPoints);
    g_pGame->GetStats()->Printf("\n");
    g_pGame->GetStats()->Printf("Damage: %s\n", m_szDamage);
    g_pGame->GetStats()->Printf("+to Hit: %d\n", (int)m_fToHitModifier);
    g_pGame->GetStats()->Printf("+to Dam: %d\n", (int)m_fDamageModifier);
    g_pGame->GetStats()->Printf("\n");
    g_pGame->GetStats()->Printf("\n");
    g_pGame->GetStats()->Printf("Level: %d\n", (int)m_fLevel);
    g_pGame->GetStats()->Printf("Exp: %d\n", (int)m_fExperience);
    g_pGame->GetStats()->Printf("Exp to Next: %d\n", (int)(m_pClass->m_fExpNeeded[(int)m_fLevel-1] - m_fExperience));
}

void CPlayer::DisplayInventory( uint8 dwPlacement )
{
    CDisplayMeta meta;
    sprintf( meta.header, "You are Carrying:\n");
    meta.limit = 'z';
    sprintf( meta.footer, "Inventory past first page not shown.\n");
    CDisplayText *pDT = NULL;
    switch(dwPlacement)
    {
        case PLACEMENT_INV:
            pDT = g_pGame->GetInv();
            break;
        case PLACEMENT_USE:
            pDT = g_pGame->GetUse();
            break;
        default:
            printf("DisplayInventory got bad placement: %d\n", dwPlacement);
            return;
            break;
    }
    
    pDT->DisplayList( m_llInventory, &meta );
}

void CPlayer::DisplayEquipment( uint8 dwPlacement )
{
    CDisplayMeta meta;
    sprintf( meta.header, "You are wearing:\n");
    meta.limit = 'j';
    sprintf( meta.footer, "Equipment is limited to 10 items, one each for specific body parts.\n");
    CDisplayText *pDT = NULL;
    switch(dwPlacement)
    {
        case PLACEMENT_EQUIP:
            pDT = g_pGame->GetEquip();
            break;
        case PLACEMENT_USE:
            pDT = g_pGame->GetUse();
            break;
        default:
            printf("DisplayEquipment got bad placement: %d\n", dwPlacement);
            return;
            break;
    }
    
    pDT->DisplayList( m_llEquipment, &meta );
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
        case ITEM_IDX_HELMET:
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
    m_fArmorClass += pItem->m_id->m_fBaseAC + pItem->m_id->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL ) strcpy(m_szDamage, pItem->m_id->m_szBaseDamage);
    m_fDamageModifier += pItem->m_id->m_fBonusToDamage;
    m_fToHitModifier += pItem->m_id->m_fBonusToHit;

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
    m_fArmorClass -= pItem->m_id->m_fBaseAC + pItem->m_id->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL ) strcpy(m_szDamage, PLAYER_BASE_DAMAGE);
    m_fDamageModifier -= pItem->m_id->m_fBonusToDamage;
    m_fToHitModifier -= pItem->m_id->m_fBonusToHit;


    return true;
}

float CPlayer::Attack()
{
    float fRoll = Util::Roll( "1d100" );
    float fHitMod = g_pGame->GetPlayer()->m_fToHitModifier;

    printf( "You rolled: %.2f, +to-hit Bonus: %.2f = Total: %.2f\n", fRoll, fHitMod, fRoll + fHitMod );

    fRoll += fHitMod;

    return fRoll;
}

float CPlayer::Damage( float fDamageMult )
{
    float fDamage = ( Util::Roll( m_szDamage ) + m_fDamageModifier ) * fDamageMult;
    printf( "You did %.2f damage (damagemult: %.2f). ", fDamage, fDamageMult );

    return fDamage;
}

void CPlayer::OnKillMonster( CMonster *pMon )
{
    m_fExperience += pMon->m_md->m_fExpValue / m_fLevel;
    GainLevel();
}

void CPlayer::GainLevel()
{
    while( (int) m_fExperience > (int) m_pClass->m_fExpNeeded[(int)m_fLevel-1] )
    {
        m_fLevel++;
        float fAddedHP = Util::Roll(m_pClass->m_szHD);
        m_fHitPoints += fAddedHP;
        m_fCurHitPoints += fAddedHP;
        g_pGame->GetMsgs()->Printf("Welcome to level %d.\n", (int)m_fLevel);
        // TODO: Gain Spells or &c here.
    }

}

bool CPlayer::Hit( float &fRoll )
{
    return ( fRoll >= m_fArmorClass );
}

int CPlayer::TakeDamage( float fDamage, char *szMon )
{
#ifdef CLOCKSTEP
    return STATUS_ALIVE;
#endif
    int retval = STATUS_INVALID;

    if( (int)fDamage < (int)m_fCurHitPoints )
    {
        m_fCurHitPoints -= fDamage;
        retval = STATUS_ALIVE;
    }
    else
    {
        m_fCurHitPoints = 0;
        retval = STATUS_DEAD;
        m_szKilledBy = new char[strlen(szMon)+1];
        memset(m_szKilledBy,0,strlen(szMon)+1);
        strcpy(m_szKilledBy, szMon);
        // This is the end of the game; make the game end on next update.
        printf("%s died on dungeon level %d, while level %d, killed by a %s.\n", m_szName, g_pGame->GetDungeon()->depth, (int)m_fLevel, m_szKilledBy );
        g_pGame->SetState(STATE_ENDGAME);
    }

    printf( "Remaining HP: %.2f \n", m_fCurHitPoints );

    return retval;
}

bool CPlayer::Drop( CItem *pItem )
{
    m_llInventory->Remove(pItem->m_pllLink, false);
    g_pGame->GetDungeon()->Drop(pItem, m_vPos);
    
    return true;
}

bool CPlayer::CanDropHere()
{
    if( g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurItem != NULL )
    {
        g_pGame->GetMsgs()->Printf("There is already an item there.\n");
        return false;
    }
    return true;
}

bool CPlayer::Quaff( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    m_llInventory->Remove(pItem->m_pllLink, false);
    CEffect *pEffect = pItem->m_id->m_llEffects->GetHead()->m_lpData;
    if( pEffect->m_dwEffect == EFFECT_TYPE_HEAL )
    {
        m_fCurHitPoints += Util::Roll(pEffect->m_szAmount);
        if( m_fCurHitPoints > m_fHitPoints )
        {
            m_fCurHitPoints = m_fHitPoints;
        }
        g_pGame->GetMsgs()->Printf("You feel a bit better.\n");
    }
    
    return true;
}

bool CPlayer::IsDrinkable(CLink<CItem> *pLink)
{
    bool retval = false;
    switch( pLink->m_lpData->m_id->m_dwIndex )
    {
        case ITEM_IDX_POTION:
            retval = true;
            break;
        default:
            retval = false;
            break;
    }
    return retval;
}

bool CPlayer::SetName( const char *szName )
{
    if(strlen(szName) > MAX_STRING_LENGTH-1)
    {
        return false;
    }
    
    strcpy( m_szName, szName );
    
    return true;
}

