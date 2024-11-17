// Player.cpp
//
// implementation of the Player

#include "Player.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
#include "JLinkList.h"
#include "StateBase.h"
#include "TileSet.h"

extern CGame *g_pGame;

void CPlayer::Init( const char *szBasedir )
{
    // Initialize all the player stuff, baby.
    //    m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );
    m_TileSet = new DUNG_TILESET;

    // This will likely get moved somewhere else. --Jimbo
    // SpawnPlayer();
}

bool CPlayer::Update( float fCurTime )
{
    if( g_pGame->GetTime() > m_fLastHPTime + PLAYER_TURNS_PER_HP )
    {
        m_fCurHitPoints++;
        if( m_fCurHitPoints > m_fHitPoints )
        {
            m_fCurHitPoints = m_fHitPoints;
            m_bIsRested = true;
        }
        else
        {
            m_bIsRested = false;
        }
        m_fLastHPTime = g_pGame->GetTime();
    }

    if( g_pGame->GetTime() > m_fLastMPTime + PLAYER_TURNS_PER_MP )
    {
        // TODO: bump mana by 1
        // m_fCurMagicPoints++;
        // if( m_fCurMagicPoints > m_fMagicPoints )
        // {
        //     m_fCurMagicPoints = m_fMagicPoints;
        // }
        // m_fLastMPTime = g_pGame->GetTime();
    }

    if( g_pGame->GetTime() >= (int)( m_fLastLightTime + 1.0f ) )
    {
        if( LightSource() > 0.0f )
        {
            UpdateLight( -1.0f );
        }
        m_fLastLightTime = (float)g_pGame->GetTime();
    }
    CheckDisturbance();
    DisplayStats();
    DisplayInventory( PLACEMENT_INV );
    DisplayEquipment( PLACEMENT_EQUIP );
    return true;
}

void CPlayer::CheckDisturbance()
{
    JIVector vPlayer( VEC_EXPAND( m_vPos ) );
    JRect rcCheck = Util::Nearby( vPlayer, 1 );

    JIVector vCheck;
    CDungeonTile *pTile;
    for( vCheck.y = rcCheck.top; vCheck.y <= rcCheck.bottom; vCheck.y++ )
    {
        for( vCheck.x = rcCheck.left; vCheck.x <= rcCheck.right; vCheck.x++ )
        {
            pTile = g_pGame->GetDungeon()->GetITile( vCheck );
            // check for interesting dungeon types
            switch( pTile->m_dtd->m_dwType )
            {
            case DUNG_IDX_DOOR:
            case DUNG_IDX_OPEN_DOOR:
            case DUNG_IDX_DOWNSTAIRS:
            case DUNG_IDX_LONG_DOWNSTAIRS:
            case DUNG_IDX_UPSTAIRS:
            case DUNG_IDX_LONG_UPSTAIRS:
            case DUNG_IDX_RUBBLE:
                m_bIsDisturbed = true;
                return;
            }
            // items and monsters already tag themselves as disturbing
            // TODO: traps, altars, water,...
        }
    }
}

void CPlayer::Draw()
{
    Uint8 player_tile = '@' - ' ' - 1; // TileIDs[TILE_IDX_PLAYER] - ' ' - 1;
    JVector DUNG_ASPECT;
    JColor player_color( 255, 255, 255, 255 );

    PreDraw();
    m_TileSet->SetTileColor( player_color );
    m_TileSet->DrawTile( player_tile, m_vPos, vSize, true );
    PostDraw();
}

void CPlayer::PreDraw() { g_pGame->GetDungeon()->PreDraw(); }

void CPlayer::PostDraw() { g_pGame->GetDungeon()->PostDraw(); }

JResult CPlayer::SpawnPlayer()
{
    JLog( LOG_LEVEL_INFO, false, "\nTrying to spawn player..." );
    JVector vTryPos;
    JIVector *vOpen;
    while( !m_bHasSpawned )
    {
        JLog( LOG_LEVEL_INFO, false, "." );
        vOpen = g_pGame->GetDungeon()->AnyOpenTile();
        vTryPos.Init( VEC_EXPAND( *vOpen ) );

        // JLog( LOG_LEVEL_NOISE, true, "Trying to spawn player at <%.2f %.2f>...\n", vTryPos.x,
        //       vTryPos.y );
        // g_pGame->GetMsgs()->Printf( "Trying to spawn player at <%.2f %.2f>...\n", vTryPos.x,
        // vTryPos.y );

        // try changing this to "iswalkable" -- might need to move that to dungeon. --Jimbo
        if( g_pGame->GetDungeon()->IsWalkableFor( vTryPos, true ) == DUNG_COLL_NO_COLLISION )
        {
            m_vPos = vTryPos;
            m_bHasSpawned = true;
            JLog( LOG_LEVEL_INFO, false, "Success! Spawned at <%.2f %.2f>\n",
                  VEC_EXPAND( m_vPos ) );
            // g_pGame->GetMsgs()->Printf( "Success!\n" );
        }
    }

    return JSUCCESS;
}

void CPlayer::DisplayStats()
{
    g_pGame->GetStats()->Clear();
    g_pGame->GetStats()->Printf( "Name: %s\n", m_szName );
    g_pGame->GetStats()->Printf( "Race: %s\n", m_pRace->m_szName );
    g_pGame->GetStats()->Printf( "Class: %s\n", m_pClass->m_szName );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "AC: %d\n", (int)m_fArmorClass );
    g_pGame->GetStats()->Printf( "HP: %d / %d\n", (int)m_fCurHitPoints, (int)m_fHitPoints );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "Damage: %s\n", m_szDamage );
    g_pGame->GetStats()->Printf( "+to Hit: %d\n", (int)m_fToHitModifier );
    g_pGame->GetStats()->Printf( "+to Dam: %d\n", (int)m_fDamageModifier );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "Level: %d\n", (int)m_fLevel );
    g_pGame->GetStats()->Printf( "Exp: %d\n", (int)m_fExperience );
    g_pGame->GetStats()->Printf(
        "Exp to Next: %d\n", (int)( m_pClass->m_fExpNeeded[(int)m_fLevel - 1] - m_fExperience ) );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "Light: %d turns\n", (int)LightSource() );
}

void CPlayer::DisplayInventory( uint8 dwPlacement )
{
    CDisplayMeta meta;
    sprintf( meta.header, "You are Carrying:\n" );
    meta.limit = 'z';
    sprintf( meta.footer, "Inventory past first page not shown.\n" );
    CDisplayText *pDT = NULL;
    switch( dwPlacement )
    {
    case PLACEMENT_INV:
        pDT = g_pGame->GetInv();
        break;
    case PLACEMENT_USE:
        pDT = g_pGame->GetUse();
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "DisplayInventory got bad placement: %d\n", dwPlacement );
        return;
        break;
    }

    pDT->DisplayList( m_llInventory, &meta );
}

void CPlayer::DisplayEquipment( uint8 dwPlacement )
{
    CDisplayMeta meta;
    sprintf( meta.header, "You are wearing:\n" );
    meta.limit = 'm';
    sprintf( meta.footer, "Equipment is limited to 10 items, one each for specific body parts.\n" );
    CDisplayText *pDT = NULL;
    switch( dwPlacement )
    {
    case PLACEMENT_EQUIP:
        pDT = g_pGame->GetEquip();
        break;
    case PLACEMENT_USE:
        pDT = g_pGame->GetUse();
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "DisplayEquipment got bad placement: %d\n", dwPlacement );
        return;
        break;
    }

    pDT->DisplayFixedList( m_llEquipment, &meta );
}

void CPlayer::PickUp( JVector &vPickupPos )
{
    CItem *pItem = g_pGame->GetDungeon()->PickUp( vPickupPos );
    if( pItem )
    {
        pItem->m_pllLink = m_llInventory->Add( pItem, pItem->m_id->m_dwIndex );
        g_pGame->GetDungeon()->GetTile( vPickupPos )->m_pCurItem = NULL;
        g_pGame->GetMsgs()->Printf( "You have a %s.\n", pItem->GetName() );
    }
}

bool CPlayer::IsWieldable( CLink<CItem> *pLink )
{
    return ( pLink->m_lpData->EquipType() != EQUIP_IDX_INVALID );
}

bool CPlayer::Wield( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;

    // You can only wield one thing of a given type at a time
    CLink<CItem> *pCurrEquip = m_llEquipment->GetLink( pItem->EquipType(), true );
    if( pCurrEquip != NULL && pCurrEquip->m_lpData != NULL &&
        pCurrEquip->m_lpData->EquipType() == pItem->EquipType() )
    {
        // So if you're already wearing something of this type, remove it and put it back in
        // inventory
        if( Remove( pCurrEquip ) )
        {
            g_pGame->GetMsgs()->Printf( "You were wielding the %s...",
                                        pCurrEquip->m_lpData->GetName() );
        }
        else
        {
            return false;
        }
    }
    // Now put on the new item.
    m_llInventory->Remove( pLink, false );
    pItem->m_pllLink = m_llEquipment->Add( pItem, pItem->EquipType() );
    m_fArmorClass += pItem->m_id->m_fBaseAC + pItem->m_id->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL )
        strcpy( m_szDamage, pItem->m_id->m_szBaseDamage );
    m_fDamageModifier += pItem->m_id->m_fBonusToDamage;
    m_fToHitModifier += pItem->m_id->m_fBonusToHit;

    return true;
}

bool CPlayer::IsRemovable( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    if( pItem->m_dwFlags & ITEM_FLAG_CURSED )
    {
        return false;
    }
    return true;
}

bool CPlayer::Remove( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    if( pItem->m_dwFlags & ITEM_FLAG_CURSED )
    {
        g_pGame->GetMsgs()->Printf( "You can't remove the %s... it seems to be cursed.\n",
                                    pItem->GetName() );
        return false;
    }
    m_llEquipment->Remove( pLink, false );
    pItem->m_pllLink = m_llInventory->Add( pItem, pItem->m_id->m_dwIndex );
    m_fArmorClass -= pItem->m_id->m_fBaseAC + pItem->m_id->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL )
        strcpy( m_szDamage, PLAYER_BASE_DAMAGE );
    m_fDamageModifier -= pItem->m_id->m_fBonusToDamage;
    m_fToHitModifier -= pItem->m_id->m_fBonusToHit;

    return true;
}

float CPlayer::LightSource()
{
    if( m_llEquipment == NULL )
        return 0.0f;

    CLink<CItem> *pLink = m_llEquipment->GetLink( EQUIP_IDX_TORCH );
    if( pLink == NULL || pLink->m_lpData == NULL )
        return 0.0f;

    CItem *pTorch = pLink->m_lpData;

    return pTorch->GetDuration();
}

void CPlayer::UpdateLight( float fValue, bool bReset )
{
    if( m_llEquipment == NULL )
        return;

    CLink<CItem> *pLink = m_llEquipment->GetLink( EQUIP_IDX_TORCH );
    if( pLink == NULL || pLink->m_lpData == NULL )
        return;

    CItem *pTorch = pLink->m_lpData;

    if( !bReset && fValue >= pTorch->GetDuration() )
    {
        fValue = 0.0f;
        bReset = true;
    }
    pTorch->ChangeDuration( fValue, bReset );

    int duration = (int)pTorch->GetDuration();
    if( duration == 0 )
    {
        g_pGame->GetMsgs()->Printf( "Your light has gone out!\n" );
    }
    else if( duration <= 100 && duration % 25 == 0 )
    {
        g_pGame->GetMsgs()->Printf( "Your light is growing very faint.\n" );
    }
    else if( duration <= 500 && duration % 100 == 0 )
    {
        g_pGame->GetMsgs()->Printf( "Your light is growing faint.\n" );
    }
}

int CPlayer::Move( JVector vDir )
{
    JVector vPos = m_vPos + vDir;

    int dwCollideType = g_pGame->GetDungeon()->IsWalkableFor( vPos, true );
    switch( dwCollideType )
    {
    case DUNG_COLL_NO_COLLISION:
        m_vPos = vPos;
        break;
    case DUNG_COLL_ITEM:
        m_vPos = vPos;
        HandleCollision( vPos, dwCollideType );
        break;
    default:
        HandleCollision( vPos, dwCollideType );
        break;
    }
    return dwCollideType;
}

void CPlayer::HandleCollision( JVector vPos, int dwCollideType )
{
    // There are only 2 things to collide with;
    // Monsters and Walls.
    if( dwCollideType == DUNG_COLL_MONSTER )
    {
        char szStatus[16];
        char *szMonster;
        float fRoll = 0.0f;
        float fDamageMult = 1.0f;
        bool bHit;
        // When Players Attack
        CMonster *pMon = g_pGame->GetDungeon()->GetTile( vPos )->m_pCurMonster;
        szMonster = pMon->GetName();

        fRoll = Attack();
        bHit = pMon->Hit( fRoll );
        if( bHit )
        {
            sprintf( szStatus, "hit" );
        }
        else
        {
            sprintf( szStatus, "miss" );
        }

        g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );

        if( bHit )
        {
            if( fRoll > 80.0f )
            {
                g_pGame->GetMsgs()->Printf( "(It was an excellent hit! (x2 damage)\n" );
                fDamageMult = 2.0f;
            }

            float fDamage = Damage( fDamageMult );

            if( pMon->TakeDamage( fDamage ) == STATUS_DEAD )
            {
                sprintf( szStatus, "have slain" );
                g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );
                OnKillMonster( pMon );
                g_pGame->GetDungeon()->RemoveMonster( pMon );
            }
        }
    }
    else if( dwCollideType == DUNG_COLL_ITEM )
    {
        PickUp( vPos );
    }
    else
    {
        // Ouch, you bumped into a %s.
        char what[16];
        switch( dwCollideType )
        {
        case DUNG_IDX_WALL:
        case DUNG_IDX_SECRET_DOOR:
            sprintf( what, "a wall" );
            break;
        case DUNG_IDX_DOOR:
            sprintf( what, "a door" );
            break;
        case DUNG_IDX_RUBBLE:
            sprintf( what, "some rubble" );
            break;
        default:
            sprintf( what, "um, something?" );
            break;
        }
        g_pGame->GetMsgs()->Printf( "Ouch! You bumped into %s!\n", what );
    }
}

float CPlayer::Attack()
{
    float fRoll = Util::Roll( "1d100" );
    float fHitMod = g_pGame->GetPlayer()->m_fToHitModifier;

    JLog( LOG_LEVEL_INFO, true, "You rolled: %.2f, +to-hit Bonus: %.2f = Total: %.2f\n", fRoll,
          fHitMod, fRoll + fHitMod );

    fRoll += fHitMod;

    return fRoll;
}

float CPlayer::Damage( float fDamageMult )
{
    float fDamage = ( Util::Roll( m_szDamage ) + m_fDamageModifier ) * fDamageMult;
    JLog( LOG_LEVEL_INFO, true, "You did %.2f damage (damagemult: %.2f). ", fDamage, fDamageMult );

    return fDamage;
}

void CPlayer::OnKillMonster( CMonster *pMon )
{
    m_fExperience += pMon->m_md->m_fExpValue / m_fLevel;
    GainLevel();
}

void CPlayer::GainLevel()
{
    while( (int)m_fExperience > (int)m_pClass->m_fExpNeeded[(int)m_fLevel - 1] )
    {
        m_fLevel++;
        float fAddedHP = Util::Roll( m_pClass->m_szHD );
        m_fHitPoints += fAddedHP;
        m_fCurHitPoints += fAddedHP;
        g_pGame->GetMsgs()->Printf( "Welcome to level %d.\n", (int)m_fLevel );
        // TODO: Gain Spells or &c here.
    }
}

bool CPlayer::Hit( float &fRoll ) { return ( fRoll >= m_fArmorClass ); }

int CPlayer::TakeDamage( float fDamage, char *szMon )
{
#ifdef CLOCKSTEP
    return STATUS_ALIVE;
#endif
    int retval = STATUS_INVALID;

    if( (int)fDamage < (int)m_fCurHitPoints )
    {
        m_fCurHitPoints -= fDamage;
        m_bIsDisturbed = true;
        retval = STATUS_ALIVE;

        JLog( LOG_LEVEL_INFO, true, "Remaining HP: %.2f \n", m_fCurHitPoints );
    }
    else
    {
        m_fCurHitPoints = 0;
        retval = STATUS_DEAD;
        m_szKilledBy = new char[strlen( szMon ) + 1];
        memset( m_szKilledBy, 0, strlen( szMon ) + 1 );
        strcpy( m_szKilledBy, szMon );
        // This is the end of the game; make the game end on next update.
        JLog( LOG_LEVEL_INFO, true,
              "\n\n%s died on dungeon level %d, while level %d, killed by a %s.\n\n", m_szName,
              g_pGame->GetDungeon()->depth, (int)m_fLevel, m_szKilledBy );
        g_pGame->SetState( STATE_ENDGAME );
    }
    return retval;
}

bool CPlayer::Drop( CItem *pItem )
{
    m_llInventory->Remove( pItem->m_pllLink, false );
    g_pGame->GetDungeon()->Drop( pItem, m_vPos );

    return true;
}

bool CPlayer::CanDropHere()
{
    if( g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurItem != NULL )
    {
        g_pGame->GetMsgs()->Printf( "There is already an item there.\n" );
        return false;
    }
    return true;
}

bool CPlayer::Quaff( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    m_llInventory->Remove( pItem->m_pllLink, false );
    CEffect *pEffect = pItem->m_id->m_llEffects->GetHead()->m_lpData;
    if( pEffect->m_dwEffect == EFFECT_TYPE_HEAL )
    {
        m_fCurHitPoints += Util::Roll( pEffect->m_szAmount );
        if( m_fCurHitPoints > m_fHitPoints )
        {
            m_fCurHitPoints = m_fHitPoints;
        }
        g_pGame->GetMsgs()->Printf( "You feel a bit better.\n" );
    }

    return true;
}

bool CPlayer::Read( CLink<CItem> *pLink )
{
    bool retval = true;
    CItem *pItem = pLink->m_lpData;
    switch( pLink->m_dwIndex )
    {
    case ITEM_IDX_SCROLL:
    {
        JLog( LOG_LEVEL_DEBUG, true, "Reading the %s\n", pItem->GetName() );
        DoReadScroll( pItem );
        break;
    }
    case ITEM_IDX_BOOK:
    default:
    {
        retval = false;
    }
    }

    return retval;
}

void CPlayer::DoReadScroll( CItem *pItem )
{
    m_llInventory->Remove( pItem->m_pllLink, false );
    CEffect *pEffect = pItem->m_id->m_llEffects->GetHead()->m_lpData;
    // TODO: Fix the effect flag
    // JLog( LOG_LEVEL_DEBUG, true, "Effect on the item is %d\n", pEffect->m_dwEffect );
    // if( pEffect->m_dwEffect == EFFECT_TYPE_RESTORE )
    // {
    CItem *cursed;
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    while( pLink != NULL )
    {
        if( pLink->m_lpData->m_dwFlags & ITEM_FLAG_CURSED )
        {
            JLog( LOG_LEVEL_DEBUG, true, "Item is cursed %s\n", pLink->m_lpData->GetName() );
            pLink->m_lpData->m_dwFlags &= ~ITEM_FLAG_CURSED;
            break;
        }
        pLink = pLink->next;
    }
    g_pGame->GetMsgs()->Printf( "It is no longer cursed.\n" );
    // }
}

bool CPlayer::IsDrinkable( CLink<CItem> *pLink )
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

bool CPlayer::IsReadable( CLink<CItem> *pLink )
{
    bool retval = false;
    switch( pLink->m_lpData->m_id->m_dwIndex )
    {
    case ITEM_IDX_BOOK:
    case ITEM_IDX_SCROLL:
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
    if( strlen( szName ) > MAX_STRING_LENGTH - 1 )
    {
        return false;
    }

    strcpy( m_szName, szName );

    return true;
}

void CPlayer::SetWizard()
{
    if( !m_bWizardMode )
    {
        g_pGame->GetMsgs()->Printf( "*** Wizard Mode: On *** your score will not be saved.\n" );
        JLog( LOG_LEVEL_WARN, true, "*** Wizard Mode: On *** your score will not be saved.\n" );
    }
    m_bWizardMode = true;
}
