// Player.cpp
//
// implementation of the Player

#include "Player.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
#include "JLinkList.h"
#include "MonsterRecall.h"
#include "StateBase.h"
#include "TileSet.h"
#include <cmath>

extern CGame *g_pGame;

static bool ShowPlayerPosInStats()
{
    const char *flag = getenv( "JMORIA_SHOW_PLAYER_POS" );
    if( flag == NULL )
        return false;
    return ( Util::jstrcmp( flag, "1" ) == 0 || Util::jstrcmp( flag, "true" ) == 0 ||
             Util::jstrcmp( flag, "yes" ) == 0 || Util::jstrcmp( flag, "on" ) == 0 );
}

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
    UpdateActiveEffects( fCurTime );
    UpdateVisibleMonsters();
    CheckDisturbance();
    PassiveSearch();
    DisplayStats();
    DisplayInventory( PLACEMENT_INV );
    DisplayEquipment( PLACEMENT_EQUIP );
    DisplayVisibleMonsters();
    DisplayMonsterRecall();
    DisplayItemRecall();
    DisplayMap();
    return true;
}

void CPlayer::UpdateActiveEffects( float fCurTime )
{
    CLink<CEffect> *pActive = m_llActiveEffects->GetHead();
    CLink<CEffect> *pDelete = NULL;
    CEffect *pEffect = NULL;
    while( pActive != NULL )
    {
        pEffect = pActive->m_lpData;
        pEffect->m_fDuration -= fCurTime;
        if( pEffect->m_fDuration <= 0.0f )
        {
            UndoIntrinsicEffects( pEffect );
            pDelete = pActive;
            pActive = pActive->next;
            m_llActiveEffects->Remove( pDelete, true );
            continue;
        }
        pActive = pActive->next;
    }
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

void CPlayer::Search()
{
    JIVector vPlayer( VEC_EXPAND( m_vPos ) );
    JRect rcCheck = Util::Nearby( vPlayer, 1 );

    JIVector vCheck;
    CDungeonTile *pTile;
    bool bFound = false;
    for( vCheck.y = rcCheck.top; vCheck.y <= rcCheck.bottom; vCheck.y++ )
    {
        for( vCheck.x = rcCheck.left; vCheck.x <= rcCheck.right; vCheck.x++ )
        {
            if( vCheck.x == vPlayer.x && vCheck.y == vPlayer.y )
                continue;
            pTile = g_pGame->GetDungeon()->GetITile( vCheck );
            if( pTile && pTile->m_dtd->m_dwType == DUNG_IDX_SECRET_DOOR )
            {
                if( Util::GetRandom( 1, 100 ) <= CHANCE_SEARCH_ACTIVE )
                {
                    JVector vPos( vCheck.x, vCheck.y );
                    g_pGame->GetDungeon()->Modify( vPos );
                    g_pGame->GetMsgs()->Printf( "You have found a secret door!\n" );
                    bFound = true;
                }
            }
        }
    }
    if( !bFound )
    {
        g_pGame->GetMsgs()->Printf( "You found nothing.\n" );
    }
}

void CPlayer::PassiveSearch()
{
    JIVector vPlayer( VEC_EXPAND( m_vPos ) );
    JRect rcCheck = Util::Nearby( vPlayer, 1 );

    JIVector vCheck;
    CDungeonTile *pTile;
    for( vCheck.y = rcCheck.top; vCheck.y <= rcCheck.bottom; vCheck.y++ )
    {
        for( vCheck.x = rcCheck.left; vCheck.x <= rcCheck.right; vCheck.x++ )
        {
            if( vCheck.x == vPlayer.x && vCheck.y == vPlayer.y )
                continue;
            pTile = g_pGame->GetDungeon()->GetITile( vCheck );
            if( pTile && pTile->m_dtd->m_dwType == DUNG_IDX_SECRET_DOOR )
            {
                if( Util::GetRandom( 1, 100 ) <= CHANCE_SEARCH_PASSIVE )
                {
                    JVector vPos( vCheck.x, vCheck.y );
                    g_pGame->GetDungeon()->Modify( vPos );
                    g_pGame->GetMsgs()->Printf( "You have found a secret door!\n" );
                    m_bIsDisturbed = true;
                }
            }
        }
    }
}

void CPlayer::Draw()
{
    // Don't draw if player hasn't been spawned yet (e.g., in CLOCKSTEP mode)
    if( !m_bHasSpawned )
        return;

    char player_char = '@';
    JVector DUNG_ASPECT;
    JColor player_color( 255, 255, 255, 255 );

    PreDraw();
    m_TileSet->SetTileColor( player_color );
    m_TileSet->DrawChar( player_char, m_vPos, vSize );
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
            m_pTarget = NULL;
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
    {
        // Speed display: hidden at base (1.0); show Fast(+N) or Slow(-N) as integer offset from 10
        int nSpeedOffset = (int)roundf( m_fSpeed * 10.0f ) - 10;
        if( nSpeedOffset > 0 )
            g_pGame->GetStats()->Printf( "Speed: Fast(+%d)\n", nSpeedOffset );
        else if( nSpeedOffset < 0 )
            g_pGame->GetStats()->Printf( "Speed: Slow(%d)\n", nSpeedOffset );
    }
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "Level: %d\n", (int)m_fLevel );
    g_pGame->GetStats()->Printf( "Depth: %d'\n", g_pGame->GetDungeon()->depth * 50 );
    if( IsWizard() || ShowPlayerPosInStats() )
        g_pGame->GetStats()->Printf( "Pos: <%.0f %.0f>\n", VEC_EXPAND( m_vPos ) );
    g_pGame->GetStats()->Printf( "Exp: %d\n", (int)m_fExperience );
    g_pGame->GetStats()->Printf(
        "Exp to Next: %d\n", (int)( m_pClass->m_fExpNeeded[(int)m_fLevel - 1] - m_fExperience ) );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    if( GetIntrinsic( EFFECT_FLAG_INFRA ) != 0 )
        g_pGame->GetStats()->Printf( "Infravision\n" );
    if( GetIntrinsic( EFFECT_FLAG_ESP ) != 0 )
        g_pGame->GetStats()->Printf( "Telepathy\n" );
    if( GetIntrinsic( EFFECT_FLAG_POISON ) != 0 )
        g_pGame->GetStats()->Printf( "Poisoned\n" );
    if( GetIntrinsic( EFFECT_FLAG_PARALYZE ) != 0 )
        g_pGame->GetStats()->Printf( "Paralyzed\n" );
    if( GetIntrinsic( EFFECT_FLAG_AFRAID ) != 0 )
        g_pGame->GetStats()->Printf( "Afraid\n" );
    if( GetIntrinsic( EFFECT_FLAG_BLIND ) != 0 )
        g_pGame->GetStats()->Printf( "Blind\n" );
    if( GetIntrinsic( EFFECT_FLAG_SLEEP ) != 0 )
        g_pGame->GetStats()->Printf( "Asleep\n" );
    if( GetIntrinsic( EFFECT_FLAG_CONFUSE ) != 0 )
        g_pGame->GetStats()->Printf( "Confused\n" );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "Light: %d turns\n", (int)LightSource() );
    g_pGame->GetStats()->Printf( "\n" );
    g_pGame->GetStats()->Printf( "\n" );
    if( m_pTarget )
    {
        g_pGame->GetStats()->Printf( "Target: %s\n", m_pTarget->GetName() );
        g_pGame->GetStats()->Printf( "\n" );
        g_pGame->GetStats()->Printf( "\n" );
        if( IsWizard() )
        {
            g_pGame->GetStats()->Printf( "Target Pos: <%.0f %.0f>\n",
                                         VEC_EXPAND( m_pTarget->GetPos() ) );
        }
    }
    if( IsWizard() )
    {
        g_pGame->GetStats()->Printf( "** WIZARD MODE **\n" );
        g_pGame->GetStats()->Printf( "\n" );
        g_pGame->GetStats()->Printf( "Player Pos: <%.0f %.0f>\n", VEC_EXPAND( m_vPos ) );
    }
}

void CPlayer::DisplayInventory( uint8 dwPlacement, eInvFilter filter )
{
    CDisplayMeta meta;
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

    switch( filter )
    {
    case INV_QUAFF:
        sprintf( meta.header, "Quaff which potion?\n" );
        break;
    case INV_READ:
        sprintf( meta.header, "Read which scroll?\n" );
        break;
    case INV_WIELD:
        sprintf( meta.header, "Wield which item?\n" );
        break;
    case INV_ZAP:
        sprintf( meta.header, "Zap which wand?\n" );
        break;
    case INV_FIRE:
        sprintf( meta.header, "Fire which ammo?\n" );
        break;
    default:
        sprintf( meta.header, "You are Carrying:\n" );
        break;
    }

    if( filter == INV_COMPLETE )
    {
        pDT->DisplayList( m_llInventory, &meta );
        return;
    }

    pDT->Clear();
    pDT->Printf( meta.header );
    CLink<CItem> *pLink = m_llInventory->GetHead();
    char cListId = 'a';
    while( pLink != NULL )
    {
        bool show = false;
        switch( filter )
        {
        case INV_QUAFF:
            show = IsDrinkable( pLink );
            break;
        case INV_READ:
            show = IsReadable( pLink );
            break;
        case INV_WIELD:
            show = IsWieldable( pLink );
            break;
        case INV_ZAP:
            show = IsZappable( pLink );
            break;
        case INV_FIRE:
            show = IsCompatibleAmmo( pLink );
            break;
        default:
            show = true;
            break;
        }
        if( show )
        {
            CItem *pItem = pLink->m_lpData;
            if( pItem->IsStackable() && pItem->m_dwCount > 1 )
            {
                pDT->Printf( "%c - %d %s\n", cListId, pItem->m_dwCount, pItem->GetPlural() );
            }
            else
            {
                pDT->Printf( "%c - %s\n", cListId, pItem->GetName() );
            }
        }
        if( cListId < meta.limit )
        {
            cListId++;
        }
        else
        {
            pDT->Printf( meta.footer );
            break;
        }
        pLink = m_llInventory->GetNext( pLink );
    }
}

void CPlayer::DisplayEquipment( uint8 dwPlacement, eInvFilter filter )
{
    CDisplayMeta meta;
    meta.limit = 'm';
    sprintf( meta.footer, "Equipment is limited to 10 items, one each for specific body parts.\n" );

    switch( filter )
    {
    case INV_FIRE:
        sprintf( meta.header, "Fire which weapon?\n" );
        break;
    default:
        sprintf( meta.header, "You are wearing:\n" );
        break;
    }

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

    if( filter == INV_COMPLETE )
    {
        pDT->DisplayFixedList( m_llEquipment, &meta );
        return;
    }

    pDT->Clear();
    pDT->Printf( meta.header );
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    char cListId = 'a';
    while( cListId <= meta.limit )
    {
        if( pLink != NULL && pLink->m_dwIndex == cListId - 'a' )
        {
            bool show = false;
            switch( filter )
            {
            case INV_FIRE:
                show = IsFireable( pLink );
                break;
            default:
                show = true;
                break;
            }
            if( show )
            {
                pDT->Printf( "%c - %s\n", cListId, pLink->m_lpData->GetName() );
            }
            pLink = m_llEquipment->GetNext( pLink );
        }
        cListId++;
    }
}

void CPlayer::DisplayVisibleMonsters()
{
    if( !g_pGame->IsShowingMonsters() )
        return;

    CDisplayText *pDT = g_pGame->GetMonsters();
    pDT->Clear();
    pDT->Printf( "Visible monsters:\n" );

    JLinkList<CMonster> *pList = GetVisibleMonsters();

    // First pass: count each monster type (m_md->m_dwIndex = type slot)
    int counts[MON_IDX_MAX] = {};
    CLink<CMonster> *pLink = pList->GetHead();
    while( pLink != NULL )
    {
        CMonster *pMon = pLink->m_lpData;
        if( pMon && pMon->m_md && pMon->m_md->m_dwIndex >= 0 &&
            pMon->m_md->m_dwIndex < MON_IDX_MAX )
        {
            counts[pMon->m_md->m_dwIndex]++;
        }
        pLink = pLink->next;
    }

    // Second pass: list is sorted by distance (ascending), so the first
    // occurrence of each type is the closest one — use that ordering.
    bool seen[MON_IDX_MAX] = {};
    pLink = pList->GetHead();
    while( pLink != NULL )
    {
        CMonster *pMon = pLink->m_lpData;
        if( pMon && pMon->m_md && pMon->m_md->m_dwIndex >= 0 &&
            pMon->m_md->m_dwIndex < MON_IDX_MAX )
        {
            int idx = pMon->m_md->m_dwIndex;
            if( !seen[idx] )
            {
                seen[idx] = true;
                if( counts[idx] > 1 )
                    pDT->Printf( "%s (%d)\n", pMon->GetName(), counts[idx] );
                else
                    pDT->Printf( "%s\n", pMon->GetName() );
            }
        }
        pLink = pLink->next;
    }
}

void CPlayer::DisplayMonsterRecall()
{
    if( !g_pGame->IsShowingMonsterRecall() )
        return;

    CDisplayText *pDT = g_pGame->GetMonsterRecall();
    pDT->Clear();

    // Display recall info for the currently targeted monster
    CMonster *pMon = GetTarget();
    if( pMon && pMon->m_md )
    {
        g_pGame->RecallMonster()->PrintRecall( pMon->m_md, pDT );
    }
    else
    {
        pDT->Printf( "No current target.\n" );
    }
}

void CPlayer::DisplayItemRecall()
{
    if( !g_pGame->IsShowingItemRecall() )
        return;

    CDisplayText *pDT = g_pGame->GetItemRecall();
    pDT->Clear();

    // TODO: Display item recall information
    pDT->Printf( "[Item Recall]\n" );
    pDT->Printf( "Not yet implemented\n" );
}

void CPlayer::DisplayMap()
{
    if( !g_pGame->IsShowingMap() )
        return;

    CDisplayText *pDT = g_pGame->GetMap();
    pDT->Clear();
    pDT->Printf( "[Map]\n" );
    pDT->Printf( "Not yet implemented\n" );
}

void CPlayer::PickUp( JVector &vPickupPos )
{
    CItem *pItem = g_pGame->GetDungeon()->PickUp( vPickupPos );
    if( pItem )
    {
        if( pItem->IsStackable() )
        {
            CLink<CItem> *pExists = m_llInventory->GetLink( pItem->m_id->m_dwIndex );
            while( pExists != NULL )
            {
                // we have an item of that type in inventory -- is it the correct item?
                CItem *pExisting = pExists->m_lpData;

                // Stack only if: name matches, identification status matches, and charges match (if
                // applicable)
                bool nameMatches = ( Util::jstrcmp( pExisting->GetName(), pItem->GetName() ) == 0 );
                bool identStatusMatches = ( ( pExisting->m_dwFlags & ITEM_FLAG_IDENTIFIED ) ==
                                            ( pItem->m_dwFlags & ITEM_FLAG_IDENTIFIED ) );

                // For wands/staffs (charged items), also check charge count
                bool chargesMatch = true;
                if( pItem->m_id->m_dwIndex == ITEM_IDX_WAND ||
                    pItem->m_id->m_dwIndex == ITEM_IDX_STAFF )
                {
                    chargesMatch = ( pExisting->m_dwCharges == pItem->m_dwCharges );
                }

                if( nameMatches && identStatusMatches && chargesMatch )
                {
                    pExisting->m_dwCount++;
                    g_pGame->GetMsgs()->Printf( "You have %d %s.\n", pExisting->m_dwCount,
                                                pExisting->GetPlural() );

                    g_pGame->GetDungeon()->GetTile( vPickupPos )->m_pCurItem = NULL;
                    return;
                }
                pExists = pExists->next;
            }
        }
        pItem->m_pllLink =
            m_llInventory->Add( pItem, pItem->m_id->m_dwIndex, pItem->GetInstanceId() );
        g_pGame->GetMsgs()->Printf( "You have a %s.\n", pItem->GetName() );

        g_pGame->GetDungeon()->GetTile( vPickupPos )->m_pCurItem = NULL;
    }
}

bool CPlayer::IsWieldable( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    if( pItem == NULL )
        return false;
    // Exclude ammo items (arrows, bolts) from wield list
    if( pItem->m_id->m_dwIndex == ITEM_IDX_ARROW || pItem->m_id->m_dwIndex == ITEM_IDX_BOLT )
        return false;
    return ( pItem->EquipType() != EQUIP_IDX_INVALID );
}

JResult CPlayer::Wield( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    // Debug dump of current equipment slots (index -> item name)
    {
        CLink<CItem> *pL = m_llEquipment->GetHead();
        while( pL )
        {
            if( pL->m_lpData )
            {
                JLog( LOG_LEVEL_DEBUG, true, "Wield(): equipment slot %d = %s\n", pL->m_dwIndex,
                      pL->m_lpData->GetName() );
            }
            else
            {
                JLog( LOG_LEVEL_DEBUG, true, "Wield(): equipment slot %d = (empty)\n",
                      pL->m_dwIndex );
            }
            pL = m_llEquipment->GetNext( pL );
        }
    }
    // If the new item is two-handed, we must ensure both hands are free.
    // Attempt to remove any existing main-hand and off-hand equipment first.
    if( pItem->m_id && ( pItem->m_id->m_dwFlags & ITEM_FLAG_2HANDED ) )
    {
        // Remove any existing main-hand weapons and any off-hand items (shields)
        // Iterate over equipment and remove matching links. Capture next link before removal.
        CLink<CItem> *pL = m_llEquipment->GetHead();
        while( pL )
        {
            CLink<CItem> *pNext = m_llEquipment->GetNext( pL );
            if( pL->m_lpData )
            {
                CItem *pEquipped = pL->m_lpData;
                bool isMain = ( pEquipped->EquipType() == EQUIP_IDX_MAIN_HAND );
                bool isOffHandFlag =
                    ( pEquipped->m_id && ( pEquipped->m_id->m_dwFlags & ITEM_FLAG_OFFHAND ) );
                if( isMain || isOffHandFlag )
                {
                    JLog( LOG_LEVEL_DEBUG, true,
                          "Wield(): removing equipped %s (main=%d offflag=%d)\n",
                          pEquipped->GetName(), isMain, (int)isOffHandFlag );
                    bool removed = RemoveEquipment( pL );
                    if( !removed )
                    {
                        JLog( LOG_LEVEL_INFO, true, "Could not remove %s to equip two-handed %s\n",
                              pEquipped->GetName(), pItem->GetName() );
                        return JBOGUSKEY;
                    }
                    else
                    {
                        g_pGame->GetMsgs()->Printf( "You were wielding the %s...",
                                                    pEquipped->GetName() );
                    }
                }
            }
            pL = pNext;
        }
    }

    // You can only wield one thing of a given type at a time
    CLink<CItem> *pCurrEquip = m_llEquipment->GetLink( pItem->EquipType() );
    if( pCurrEquip != NULL && pCurrEquip->m_lpData != NULL &&
        pCurrEquip->m_lpData->EquipType() == pItem->EquipType() )
    {
        // So if you're already wearing something of this type, remove it and put it back in
        // inventory
        bool removed = RemoveEquipment( pCurrEquip );
        if( removed )
        {
            g_pGame->GetMsgs()->Printf( "You were wielding the %s...",
                                        pCurrEquip->m_lpData->GetName() );
            JLog( LOG_LEVEL_INFO, true, "You were wielding the %s\n",
                  pCurrEquip->m_lpData->GetName() );
        }
        else
        {
            JLog( LOG_LEVEL_INFO, true, "could not remove the %s, got %d\n",
                  pCurrEquip->m_lpData->GetName(), removed );
            return JBOGUSKEY;
        }
    }
    // Now put on the new item.
    m_llInventory->Remove( pLink, false );
    pItem->m_pllLink = m_llEquipment->Add( pItem, pItem->EquipType(), pItem->GetInstanceId() );
    m_fArmorClass += pItem->m_id->m_fBaseAC + pItem->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL )
        Util::jstrcpy( m_szDamage, pItem->m_id->m_szBaseDamage );
    m_fDamageModifier += pItem->m_fBonusToDamage;
    m_fToHitModifier += pItem->m_fBonusToHit;
    m_fSpeed += pItem->m_fSpeedBonus;

    // Defensive: if this is a two-handed weapon, ensure off-hand is clear.
    if( pItem->m_id && ( pItem->m_id->m_dwFlags & ITEM_FLAG_2HANDED ) )
    {
        CLink<CItem> *pOffCheck = NULL;
        {
            CLink<CItem> *pL = m_llEquipment->GetHead();
            while( pL )
            {
                if( pL->m_lpData && pL->m_dwIndex == EQUIP_IDX_OFF_HAND )
                {
                    pOffCheck = pL;
                    break;
                }
                pL = m_llEquipment->GetNext( pL );
            }
        }
        if( pOffCheck != NULL && pOffCheck->m_lpData != NULL )
        {
            JLog( LOG_LEVEL_WARN, true,
                  "Two-handed equip: off-hand still occupied by %s; removing now.\n",
                  pOffCheck->m_lpData->GetName() );
            RemoveEquipment( pOffCheck );
        }
    }

    return JSUCCESS;
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

bool CPlayer::RemoveEquipment( CLink<CItem> *pLink )
{
    if( !pLink )
        return false;

    CItem *pItem = pLink->m_lpData;
    if( pItem && pItem->m_dwFlags & ITEM_FLAG_CURSED )
    {
        pItem->RevealProperty( KNOWN_CURSED );
        g_pGame->GetMsgs()->Printf( "You can't remove the %s... it seems to be cursed.\n",
                                    pItem->GetName() );

        JLog( LOG_LEVEL_INFO, true, "You can't remove the %s... it seems to be cursed.\n",
              pItem->GetName() );
        return false;
    }

    m_llEquipment->Remove( pLink, false );
    pItem->m_pllLink = m_llInventory->Add( pItem, pItem->m_id->m_dwIndex, pItem->GetInstanceId() );
    m_fArmorClass -= pItem->m_id->m_fBaseAC + pItem->m_fACBonus;
    if( pItem->m_id->m_szBaseDamage != NULL )
        Util::jstrcpy( m_szDamage, PLAYER_BASE_DAMAGE );
    m_fDamageModifier -= pItem->m_fBonusToDamage;
    m_fToHitModifier -= pItem->m_fBonusToHit;
    m_fSpeed -= pItem->m_fSpeedBonus;

    return true;
}

void CPlayer::XchangeWeapons()
{
    // Look up the four equipment slot nodes
    CLink<CItem> *pMainHand = m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND );
    CLink<CItem> *pOffHand = m_llEquipment->GetLink( EQUIP_IDX_OFF_HAND );
    CLink<CItem> *p2ndMain = m_llEquipment->GetLink( EQUIP_IDX_2ND_MAIN );
    CLink<CItem> *p2ndOff = m_llEquipment->GetLink( EQUIP_IDX_2ND_OFF );

    // If there are no secondary slots yet, create them
    if( !p2ndMain )
    {
        CItem *pEmpty = NULL; // SwapData will handle null data
        p2ndMain = m_llEquipment->Add( pEmpty, EQUIP_IDX_2ND_MAIN, -1 );
    }
    if( !p2ndOff )
    {
        CItem *pEmpty = NULL;
        p2ndOff = m_llEquipment->Add( pEmpty, EQUIP_IDX_2ND_OFF, -1 );
    }

    // Swap the main and secondary weapon sets
    m_llEquipment->SwapData( pMainHand, p2ndMain );
    m_llEquipment->SwapData( pOffHand, p2ndOff );

    // Recalculate all combat stats from scratch
    // Start with base values
    m_fArmorClass = 10.0f; // Base AC
    Util::jstrcpy( m_szDamage, PLAYER_BASE_DAMAGE );
    m_fDamageModifier = 0.0f;
    m_fToHitModifier = 0.0f;
    m_fSpeed = 1.0f; // Base speed multiplier

    // Re-add contributions from all equipped items
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    while( pLink )
    {
        if( pLink->m_lpData )
        {
            CItem *pItem = pLink->m_lpData;
            // Only count items in active slots (not secondary slots)
            bool isActive =
                ( pLink->m_dwIndex == EQUIP_IDX_MAIN_HAND ||
                  pLink->m_dwIndex == EQUIP_IDX_OFF_HAND || pLink->m_dwIndex == EQUIP_IDX_HELMET ||
                  pLink->m_dwIndex == EQUIP_IDX_AMULET || pLink->m_dwIndex == EQUIP_IDX_ARMOR ||
                  pLink->m_dwIndex == EQUIP_IDX_CLOAK || pLink->m_dwIndex == EQUIP_IDX_GLOVES ||
                  pLink->m_dwIndex == EQUIP_IDX_BELT || pLink->m_dwIndex == EQUIP_IDX_BOOTS ||
                  pLink->m_dwIndex == EQUIP_IDX_LRING || pLink->m_dwIndex == EQUIP_IDX_RRING ||
                  pLink->m_dwIndex == EQUIP_IDX_TORCH );

            if( isActive && pItem->m_id )
            {
                m_fArmorClass += pItem->m_id->m_fBaseAC + pItem->m_fACBonus;
                if( pItem->m_id->m_szBaseDamage != NULL )
                    Util::jstrcpy( m_szDamage, pItem->m_id->m_szBaseDamage );
                m_fDamageModifier += pItem->m_fBonusToDamage;
                m_fToHitModifier += pItem->m_fBonusToHit;
                m_fSpeed += pItem->m_fSpeedBonus;
            }
        }
        pLink = m_llEquipment->GetNext( pLink );
    }

    // Print feedback message
    CLink<CItem> *pNewMain = m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND );
    if( pNewMain && pNewMain->m_lpData )
    {
        g_pGame->GetMsgs()->Printf( "You switch to your %s.\n", pNewMain->m_lpData->GetName() );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You switch to your bare hands.\n" );
    }
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

float CPlayer::LightRadius()
{
    if( m_llEquipment == NULL )
        return 0.0f;

    CLink<CItem> *pLink = m_llEquipment->GetLink( EQUIP_IDX_TORCH );
    if( pLink == NULL || pLink->m_lpData == NULL )
        return 0.0f;

    CItem *pTorch = pLink->m_lpData;

    return pTorch->GetRadius();
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
        const char *szMonster;
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

            if( DamageMonster( pMon, fDamage ) )
            {
                sprintf( szStatus, "have slain" );
                g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );
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

float CPlayer::RangedAttack( CLink<CItem> *pArrow )
{
    // Calculate ranged attack to-hit roll combining bow and arrow bonuses
    float fRoll = Util::Roll( "1d100" );

    // Get the equipped primary weapon (bow)
    CLink<CItem> *pBow = m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND );
    float fBowBonus = 0.0f;
    if( pBow && pBow->m_lpData && pBow->m_lpData->m_id &&
        ( pBow->m_lpData->m_id->m_dwFlags & ITEM_FLAG_NEEDSAMMO ) )
    {
        fBowBonus = pBow->m_lpData->m_fBonusToHit;
    }

    // Get arrow bonuses
    float fArrowBonus = 0.0f;
    if( pArrow && pArrow->m_lpData )
    {
        fArrowBonus = pArrow->m_lpData->m_fBonusToHit;
    }

    float fTotalBonus = fBowBonus + fArrowBonus;
    fRoll += fTotalBonus;

    JLog( LOG_LEVEL_INFO, true,
          "Ranged: rolled: %.2f, bow: %.2f, arrow: %.2f, total bonus: %.2f = Total: %.2f\n",
          fRoll - fTotalBonus, fBowBonus, fArrowBonus, fTotalBonus, fRoll );

    return fRoll;
}

float CPlayer::Damage( float fDamageMult )
{
    float fDamage = ( Util::Roll( m_szDamage ) + m_fDamageModifier ) * fDamageMult;
    JLog( LOG_LEVEL_INFO, true, "You did %.2f damage (damagemult: %.2f). ", fDamage, fDamageMult );

    return fDamage;
}

void CPlayer::OnKillMonster( CMonster *pMon, float fKillingBlow )
{
    if( g_pGame->RecallMonster() )
    {
        // The player doesn't know exactly how much HP the monster had —
        // only that fKillingBlow was enough to finish it off.  The estimate
        // is: actual_max_HP minus at most (fKillingBlow - 1) remaining HP.
        float fEstimatedHP = pMon->m_fHP - ( fKillingBlow - 1.0f );
        g_pGame->RecallMonster()->RecordKill( pMon->m_md->m_szName, fEstimatedHP );
    }
    m_fExperience += pMon->m_md->m_fExpValue / m_fLevel;
    GainLevel();
    m_pTarget = NULL;
}

bool CPlayer::DamageMonster( CMonster *pMon, float fDamage )
{
    if( pMon->TakeDamage( fDamage ) == STATUS_DEAD )
    {
        OnKillMonster( pMon, fDamage );
        g_pGame->GetDungeon()->RemoveMonster( pMon );
        return true;
    }
    return false;
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

float CPlayer::Resist( uint32 dwElement )
{
    if( dwElement == 0 )
        return 1.0f;

    // Check each element bit in the flags
    uint32 elementMask =
        EFFECT_FLAG_FIRE | EFFECT_FLAG_COLD | EFFECT_FLAG_ELECTRICITY | EFFECT_FLAG_ACID;
    uint32 elements = dwElement & elementMask;
    if( elements == 0 )
        return 1.0f;

    if( !GetIntrinsic( elements ) )
        return 1.0f;

    // We have the intrinsic — look up what modifier (RESIST, IMMUNE, WEAK) in active effects
    CLink<CEffect> *pLink = m_llActiveEffects->GetHead();
    while( pLink )
    {
        CEffect *pEffect = pLink->m_lpData;
        if( pEffect->m_dwFlags & elements )
            return pEffect->Resist();
        pLink = pLink->next;
    }

    // Check equipped items for permanent resistances
    CLink<CItem> *pEquip = m_llEquipment->GetHead();
    while( pEquip )
    {
        CItem *pItem = pEquip->m_lpData;
        if( pItem->m_id->m_llEffects )
        {
            CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
            while( plEffect )
            {
                CEffect *pEffect = plEffect->m_lpData;
                if( ( pEffect->m_dwEffect == EFFECT_TYPE_INTRINSIC ) &&
                    ( pEffect->m_dwFlags & elements ) )
                    return pEffect->Resist();
                plEffect = plEffect->next;
            }
        }
        pEquip = pEquip->next;
    }

    // Intrinsic set but no modifier found — default to resist
    return 0.5f;
}

int CPlayer::TakeDamage( float fDamage, const char *szMon, uint32 dwElement )
{
#ifdef CLOCKSTEP
    return STATUS_ALIVE;
#endif
    int retval = STATUS_INVALID;

    // Apply elemental resistance
    float fMult = Resist( dwElement );
    if( fMult != 1.0f )
    {
        if( fMult == 0.0f )
            g_pGame->GetMsgs()->Printf( "You are immune!\n" );
        else if( fMult < 1.0f )
            g_pGame->GetMsgs()->Printf( "You resist!\n" );
        fDamage *= fMult;
    }

    // Elemental attacks may destroy vulnerable inventory items or degrade equipment
    DoDamageInventory( dwElement );
    DoDamageEquipment( dwElement );

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
        m_szKilledBy = new char[Util::jstrlen( szMon ) + 1];
        memset( m_szKilledBy, 0, Util::jstrlen( szMon ) + 1 );
        Util::jstrcpy( m_szKilledBy, szMon );
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

bool CPlayer::Drop( CItem *pItem, int quantity )
{
    // If dropping the entire stack, use normal Drop
    if( quantity >= pItem->m_dwCount )
    {
        return Drop( pItem );
    }

    // Dropping a partial stack: create a copy with the specified quantity
    CItem *pDropped = pItem->Copy( quantity );

    // Reduce the carried stack
    pItem->m_dwCount -= quantity;

    // Drop the item
    g_pGame->GetDungeon()->Drop( pDropped, m_vPos );

    return true;
}

bool CPlayer::CanDropHere()
{
    // With enhanced Drop() handling stacking and scattering, we can always attempt to drop.
    // Drop() itself will handle placement logic (stacking, scatter to adjacent, or fail with
    // message).
    return true;
}

JResult CPlayer::Quaff( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    bool wasIdentified = pItem->IsIdentified();
    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );
    if( !wasIdentified && m_bLastEffectNoticed )
    {
        pItem->Identify();
        g_pGame->GetMsgs()->Printf( "You recognize it as a %s.\n", pItem->GetName() );
    }
    else if( !wasIdentified && !m_bLastEffectNoticed )
    {
        pItem->m_id->m_bTried = true;
    }
    ConsumeItem( pLink ); // Unified consumption
    return retval;
}

JResult CPlayer::Read( CLink<CItem> *pLink )
{
    CItem *pItem = pLink->m_lpData;
    bool wasIdentified = pItem->IsIdentified();
    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );
    if( !wasIdentified && m_bLastEffectNoticed )
    {
        pItem->Identify();
        g_pGame->GetMsgs()->Printf( "You recognize it as a %s.\n", pItem->GetName() );
    }
    else if( !wasIdentified && !m_bLastEffectNoticed )
    {
        pItem->m_id->m_bTried = true;
    }
    ConsumeItem( pLink ); // Unified consumption
    return retval;
}

JResult CPlayer::Zap( CLink<CItem> *pLink )
{
    // this will get called multiple times for a single shot, if EFFECT_FLAG_NO_COLLIDE is set,
    // this function is to do damage to the monster in the current position
    CItem *pItem = pLink->m_lpData;
    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );
    return retval;
}

JResult CPlayer::Fire( CLink<CItem> *pLink )
{
    // Apply effects on impact (called when projectile hits a monster)
    // NOTE: Consumption happens in RangedState::DoLaunch(), not here
    CItem *pItem = pLink->m_lpData;

    // Track this ammo for ranged to-hit calculations in DoElementalHit
    m_pCurrentRangedAmmo = pItem;

    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );

    // Clear the ranged ammo tracker
    m_pCurrentRangedAmmo = NULL;

    return retval;
}

void CPlayer::ConsumeItem( CLink<CItem> *pLink )
{
    // Unified consumption: remove item from inventory when used
    // Stack decrements are handled by CItem::Consume() (for charges/counts)
    // This handles inventory removal when the last unit is consumed
    if( pLink == NULL || pLink->m_lpData == NULL )
    {
        return;
    }

    CItem *pItem = pLink->m_lpData;
    if( pItem->m_dwCount > 1 )
    {
        pItem->m_dwCount--;
    }
    else
    {
        m_llInventory->Remove( pItem->m_pllLink, false );
    }
}

void CPlayer::ConsumeAndRemoveIfEmpty( CLink<CItem> *pLink )
{
    // Consume an item and remove it from inventory if it becomes empty
    if( pLink == NULL || pLink->m_lpData == NULL )
    {
        return;
    }

    pLink->m_lpData->Consume();
    if( pLink->m_lpData->IsConsumed() )
    {
        m_llInventory->Remove( pLink, false );
    }
}

void CPlayer::ConsolidateInventory()
{
    // Auto-consolidate inventory stacks with matching charges/identification
    // After items are consumed or equipment is modified, stacks may need reorganization
    // This runs between turns and silently merges matching stacks

    bool anyMerged = false;

    CLink<CItem> *pOuter = m_llInventory->GetHead();
    while( pOuter != NULL )
    {
        CItem *pOuterItem = pOuter->m_lpData;
        CLink<CItem> *pInner = m_llInventory->GetNext( pOuter );

        while( pInner != NULL )
        {
            CItem *pInnerItem = pInner->m_lpData;
            CLink<CItem> *pNext = m_llInventory->GetNext( pInner );

            // Check if these items can stack together
            if( pOuterItem->m_id->m_dwIndex == pInnerItem->m_id->m_dwIndex &&
                pOuterItem->IsStackable() &&
                Util::jstrcmp( pOuterItem->GetName(), pInnerItem->GetName() ) == 0 &&
                ( ( pOuterItem->m_dwFlags & ITEM_FLAG_IDENTIFIED ) ==
                  ( pInnerItem->m_dwFlags & ITEM_FLAG_IDENTIFIED ) ) )
            {
                // For charged items, also check charge count matches
                bool chargesMatch = true;
                if( pOuterItem->m_id->m_dwIndex == ITEM_IDX_WAND ||
                    pOuterItem->m_id->m_dwIndex == ITEM_IDX_STAFF )
                {
                    chargesMatch = ( pOuterItem->m_dwCharges == pInnerItem->m_dwCharges );
                }

                if( chargesMatch )
                {
                    // Merge: add inner count to outer
                    pOuterItem->m_dwCount += pInnerItem->m_dwCount;
                    m_llInventory->Remove( pInner, false );
                    anyMerged = true;

                    // Don't advance pInner since we just removed it
                    pInner = pNext;
                    continue;
                }
            }

            pInner = pNext;
        }

        pOuter = m_llInventory->GetNext( pOuter );
    }

    if( anyMerged )
    {
        g_pGame->GetMsgs()->Printf( "You organize your pack.\n" );
    }
}

JResult CPlayer::Magic( CLink<CItem> *pLink )
{
    // TODO: Make magic actually work -- probably should not be item-based (it's a choose-from-menu
    // type) and will probably pass a CSpell rather than a CItem This was to demonstrate that Read
    // Quaff and Magic are similar but magic is multi-use
    CItem *pItem = pLink->m_lpData;
    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    JResult retval = DoEffects( plEffect, pItem->m_id->m_fDuration, pItem->m_dwFlags );
    return retval;
}

bool CPlayer::IsFuel( CLink<CItem> *pLink )
{
    return ( pLink->m_lpData->m_id->m_dwIndex == ITEM_IDX_FUEL );
}

#define LANTERN_MAX_FUEL 15000.0f

JResult CPlayer::Fuel( CLink<CItem> *pLink )
{
    CLink<CItem> *pLantern = m_llEquipment->GetLink( EQUIP_IDX_TORCH );
    if( pLantern == NULL || pLantern->m_lpData == NULL )
        return JERROR();

    CItem *pTorch = pLantern->m_lpData;
    if( !( pTorch->m_id->m_dwFlags & ITEM_FLAG_NEEDSAMMO ) )
    {
        g_pGame->GetMsgs()->Printf( "Your %s cannot be refueled.\n", pTorch->GetName() );
        return JERROR();
    }

    CItem *pFuel = pLink->m_lpData;
    float fFuelAmount = pFuel->m_id->m_fDuration;
    float fCurrent = pTorch->GetDuration();
    float fNew = fCurrent + fFuelAmount;
    if( fNew > LANTERN_MAX_FUEL )
        fNew = LANTERN_MAX_FUEL;
    pTorch->ChangeDuration( fNew, true );

    // Consume the flask
    if( pFuel->m_dwCount > 1 )
    {
        pFuel->m_dwCount--;
    }
    else
    {
        m_llInventory->Remove( pFuel->m_pllLink, false );
    }
    return JSUCCESS;
}

JResult CPlayer::DoEffects( CLink<CEffect> *plEffect, float fDuration, int dwItemFlags )
{
    CEffect *pEffect;
    m_bLastEffectNoticed = false;
    while( plEffect != NULL )
    {
        pEffect = plEffect->m_lpData;
        JLog( LOG_LEVEL_DEBUG, true, "Effect: %s Flag: %s Mod: %s\n",
              g_Constants.IndexToString( EFFECT_TYPE, pEffect->m_dwEffect ),
              g_Constants.EffectFlagToString( pEffect->m_dwFlags, pEffect->m_dwFlags2 ),
              g_Constants.IndexToString( EFFECT_MOD, pEffect->m_dwModifier ) );
        switch( pEffect->m_dwEffect )
        {
        case EFFECT_TYPE_HEAL:
            DoHealEffects( pEffect );
            break;
        case EFFECT_TYPE_HIT:
            DoHitEffects( pEffect );
            break;
        case EFFECT_TYPE_CREATE:
            DoCreateEffects( pEffect );
            break;
        case EFFECT_TYPE_DESTROY:
            JLog( LOG_LEVEL_DEBUG, true, "Destroying\n" );
            DoDestroyEffects( pEffect, dwItemFlags );
            break;
        case EFFECT_TYPE_INTRINSIC:
            JLog( LOG_LEVEL_DEBUG, true, "Setting intrinsic\n" );
            DoIntrinsicEffects( pEffect, fDuration );
            break;
        case EFFECT_TYPE_RESTORE:
            DoRestoreEffects( pEffect );
            break;
        case EFFECT_TYPE_GAIN:
            DoGainEffects( pEffect );
            break;
        case EFFECT_TYPE_LOSE:
            DoLoseEffects( pEffect );
            break;
        case EFFECT_TYPE_SEE:
            DoSeeEffects( pEffect );
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true, "bad effect type: %d\n", pEffect->m_dwEffect );
            break;
        }
        plEffect = plEffect->next;
    }
    return JSUCCESS;
}

JResult CPlayer::DoHealEffects( CEffect *pEffect )
{
    char effect[32];
    bool needsHeal = GetIntrinsic( pEffect->m_dwFlags ) != 0;
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_HP:
        DoHealHP( pEffect );
        m_bLastEffectNoticed = true;
        break;
    case EFFECT_FLAG_AFRAID:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You are no longer afraid.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    case EFFECT_FLAG_BLIND:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You can see again.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    case EFFECT_FLAG_CONFUSE:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You can think clearly again.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    case EFFECT_FLAG_POISON:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You are no longer poisoned.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    case EFFECT_FLAG_PARALYZE:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You can move again.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    case EFFECT_FLAG_SLEEP:
        if( needsHeal )
        {
            g_pGame->GetMsgs()->Printf( "You wake up.\n" );
            UnsetIntrinsic( pEffect->m_dwFlags );
            m_bLastEffectNoticed = true;
        }
        break;
    default:
        break;
    }
    return JSUCCESS;
}

JResult CPlayer::DoHealHP( CEffect *pEffect )
{
    float fHeal = Util::Roll( pEffect->m_szAmount );
    m_fCurHitPoints += fHeal;
    if( m_fCurHitPoints > m_fHitPoints )
    {
        m_fCurHitPoints = m_fHitPoints;
    }
    if( fHeal > 100.0f )
    {
        g_pGame->GetMsgs()->Printf( "You feel amazing!\n" );
    }
    else if( fHeal > 50.0f )
    {
        g_pGame->GetMsgs()->Printf( "You feel a lot better.\n" );
    }
    else if( fHeal > 25.0f )
    {
        g_pGame->GetMsgs()->Printf( "You feel better.\n" );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You feel a bit better.\n" );
    }
    return JSUCCESS;
}

JResult CPlayer::DoHitEffects( CEffect *pEffect )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_LIGHT:
        return DoLightRay( pEffect );
        break;
    case EFFECT_FLAG_FIRE:
    case EFFECT_FLAG_COLD:
    case EFFECT_FLAG_ELECTRICITY:
    case EFFECT_FLAG_ACID:
        return DoElementalHit( pEffect );
        break;
    default:
        return DoPhysicalHit( pEffect );
        break;
    }
    return JSUCCESS;
}

JResult CPlayer::DoPhysicalHit( CEffect *pEffect )
{
    bool bCriticalHit = false;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vRangedHitPosition );
    if( !pTile )
    {
        return JBOGUSKEY;
    }
    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }
    // Save monster name before it's potentially deleted
    const char *szMonName = pMon->GetName();

    // If this is a ranged attack (arrow/ammo with bow), perform to-hit check
    if( m_pCurrentRangedAmmo )
    {
        // Get the arrow as a CLink
        CLink<CItem> *pArrowLink = NULL;
        CLink<CItem> *pInvItem = m_llInventory->GetHead();
        while( pInvItem )
        {
            if( pInvItem->m_lpData == m_pCurrentRangedAmmo )
            {
                pArrowLink = pInvItem;
                break;
            }
            pInvItem = m_llInventory->GetNext( pInvItem );
        }

        float fRoll = RangedAttack( pArrowLink );
        bool bHit = pMon->Hit( fRoll );

        if( !bHit )
        {
            g_pGame->GetMsgs()->Printf( "You miss the %s.\n", szMonName );
            JLog( LOG_LEVEL_INFO, true, "Ranged miss: roll %.2f vs AC %d\n", fRoll,
                  (int)pMon->m_fCurAC );
            return JSUCCESS; // Arrow missed, no damage
        }

        if( fRoll > 80.0f )
        {
            g_pGame->GetMsgs()->Printf( "(Critical hit!)\n" );
            bCriticalHit = true;
        }
    }
    const char *szAmount = pEffect->m_szAmount;
    if( !szAmount && pEffect->m_ed )
        szAmount = pEffect->m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "1d2";

    float fDamage = Util::Roll( szAmount );

    // For ranged attacks, add arrow's damage bonus
    if( m_pCurrentRangedAmmo )
    {
        fDamage += m_pCurrentRangedAmmo->m_fBonusToDamage;
    }

    if( bCriticalHit )
    {
        fDamage *= 2.0f;
    }

    if( DamageMonster( pMon, fDamage ) )
    {
        g_pGame->GetMsgs()->Printf( "The %s dies.\n", szMonName );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );
    }
    return JSUCCESS;
}

JResult CPlayer::DoLightRay( CEffect *pEffect )
{
    // if there is a monster at the position
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vRangedHitPosition );
    if( !pTile )
    {
        return JBOGUSKEY;
    }

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );

        return JBOGUSKEY;
    }

    // Save monster name before it's potentially deleted
    const char *szMonName = pMon->GetName();

    JLog( LOG_LEVEL_INFO, true, "monster: %s\n", szMonName );
    // TODO: this should be "weaknesses" and re-use effect_flag_light instead of new "general flag"
    if( ( pMon->m_md->m_dwFlags & MON_FLAG_HURT_BY_LIGHT ) == MON_FLAG_HURT_BY_LIGHT )
    {
        JLog( LOG_LEVEL_NOISE, true, "that's gonna hurt\n" );

        float fDamage = Util::Roll( "1d5" ); // pEffect->m_szAmount );

        if( DamageMonster( pMon, fDamage ) )
        {
            g_pGame->GetMsgs()->Printf( "The %s shrivels away in the bright light!\n", szMonName );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "The %s screams in agony.\n", szMonName );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s is unaffected.\n", szMonName );
    }

    return JSUCCESS;
}

JResult CPlayer::DoElementalHit( CEffect *pEffect )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vRangedHitPosition );
    if( !pTile )
    {
        return JBOGUSKEY;
    }

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    const char *szElement = g_Constants.IndexToString( EFFECT_FLAG, pEffect->m_dwFlags );
    JLog( LOG_LEVEL_INFO, true, "elemental hit (%s) on %s\n", szElement, pMon->GetName() );

    // Save monster name before it's potentially deleted
    const char *szMonName = pMon->GetName();
    // Print effect description message
    if( pEffect->m_ed && pEffect->m_ed->m_szName )
    {
        g_pGame->GetMsgs()->Printf( "The %s strikes the %s with %s.\n", pEffect->m_ed->m_szName,
                                    szMonName, szElement );
    }

    const char *szAmount = pEffect->m_szAmount;
    if( !szAmount && pEffect->m_ed )
        szAmount = pEffect->m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "1d6";

    float fDamage = Util::Roll( szAmount );

    // For ranged attacks, add arrow's damage bonus
    if( m_pCurrentRangedAmmo )
    {
        fDamage += m_pCurrentRangedAmmo->m_fBonusToDamage;
    }

    if( DamageMonster( pMon, fDamage ) )
    {
        g_pGame->GetMsgs()->Printf( "The %s is destroyed!\n", szMonName );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );
    }

    return JSUCCESS;
}

JResult CPlayer::DoDamageInventory( uint32 dwElement )
{
    uint32 elementMask =
        EFFECT_FLAG_FIRE | EFFECT_FLAG_COLD | EFFECT_FLAG_ELECTRICITY | EFFECT_FLAG_ACID;
    uint32 element = dwElement & elementMask;
    if( element == 0 )
        return JBOGUSKEY;

    float fResistMult = Resist( dwElement );
    // Immunity protects items
    if( fResistMult == 0.0f )
        return JBOGUSKEY;
#ifdef UNIT_TEST
    float fChance = 1.1f;
#else
    // Base 3% chance per inventory slot; halved when player resists
    float fChance = ( fResistMult < 1.0f ) ? 0.015f : 0.03f;
#endif

    const char *szVerb = "are destroyed";
    if( element == EFFECT_FLAG_FIRE )
        szVerb = "catch fire";
    else if( element == EFFECT_FLAG_COLD )
        szVerb = "shatter in the cold";
    else if( element == EFFECT_FLAG_ACID )
        szVerb = "are dissolved by acid";
    else if( element == EFFECT_FLAG_ELECTRICITY )
        szVerb = "are blasted by lightning";

    CLink<CItem> *pLink = m_llInventory->GetHead();
    while( pLink != NULL )
    {
        CLink<CItem> *pNext = pLink->next;
        CItem *pItem = pLink->m_lpData;

        if( pItem->IsWeakTo( element ) && Util::GetRandom( 0.0f, 1.0f ) < fChance )
        {
            g_pGame->GetMsgs()->Printf( "Your %s %s!\n", pItem->GetPlural(), szVerb );
            if( pItem->m_dwCount > 1 )
                pItem->m_dwCount--;
            else
                m_llInventory->Remove( pLink, false );
        }

        pLink = pNext;
    }
    return JSUCCESS;
}

JResult CPlayer::DoDamageEquipment( uint32 dwElement )
{
    uint32 elementMask = EFFECT_FLAG_FIRE | EFFECT_FLAG_ACID;
    uint32 element = dwElement & elementMask;
    if( element == 0 )
        return JBOGUSKEY;

    float fResistMult = Resist( dwElement );
    if( fResistMult == 0.0f )
        return JBOGUSKEY;
#ifdef UNIT_TEST
    float fChance = 1.1;
#else
    float fChance = ( fResistMult < 1.0f ) ? 0.015f : 0.03f;
#endif
    const char *szElement = ( element == EFFECT_FLAG_FIRE ) ? "fire" : "acid";

    CLink<CItem> *pLink = m_llEquipment->GetHead();
    while( pLink != NULL )
    {
        CItem *pItem = pLink->m_lpData;

        if( pItem->IsWeakTo( element ) && Util::GetRandom( 0.0f, 1.0f ) < fChance )
        {
            // Weapons: randomly reduce to-hit or to-damage bonus
            bool bIsWeapon = ( pItem->EquipType() == EQUIP_IDX_MAIN_HAND );
            if( bIsWeapon )
            {
                bool bReduceToHit = ( Util::GetRandom( 0, 1 ) == 0 );
                if( bReduceToHit )
                {
                    pItem->m_fBonusToHit -= 1.0f;
                    g_pGame->GetMsgs()->Printf( "Your %s is pitted by %s! (to-hit reduced)\n",
                                                pItem->GetName(), szElement );
                }
                else
                {
                    pItem->m_fBonusToDamage -= 1.0f;
                    g_pGame->GetMsgs()->Printf( "Your %s is pitted by %s! (to-damage reduced)\n",
                                                pItem->GetName(), szElement );
                }
            }
            else
            {
                // Armor/clothing: reduce AC bonus, floor at -(baseAC)
                float fFloor = -pItem->m_id->m_fBaseAC;
                if( pItem->m_fACBonus > fFloor )
                {
                    pItem->m_fACBonus -= 1.0f;
                    g_pGame->GetMsgs()->Printf( "Your %s is damaged by %s!\n", pItem->GetName(),
                                                szElement );
                }
            }
        }

        pLink = pLink->next;
    }
    return JSUCCESS;
}

JResult CPlayer::DoCreateEffects( CEffect *pEffect )
{
    JResult retval = JBOGUSKEY;
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_LIGHT:
        retval = DoLightArea();
        break;
    case EFFECT_FLAG_TELEPORT:
        retval = DoTeleport( pEffect );
        break;
    case EFFECT_FLAG_MAPPING:
        retval = DoMagicMapping( pEffect );
        break;
    case EFFECT_FLAG_RECALL:
        retval = DoRecall();
        break;
    case EFFECT_FLAG_SUMMON:
        retval = DoSummonMonsters();
        break;
    }
    if( retval == JSUCCESS )
        m_bLastEffectNoticed = true;
    return retval;
}

JResult CPlayer::DoLightArea()
{
    CRoom *pRoom = g_pGame->GetDungeon()->InRoom( m_vPos );
    if( pRoom )
    {
        pRoom->SetFlags( DUNG_FLAG_LIT );
        g_pGame->GetDungeon()->LightRoom( pRoom );
        return JSUCCESS;
    }
    return JBOGUSKEY;
}

JResult CPlayer::DoTeleport( CEffect *pEffect )
{
    if( pEffect->m_dwModifier & EFFECT_MOD_AREA )
    {
        // Phase Door: limited range teleport
        for( int attempt = 0; attempt < 100; attempt++ )
        {
            int dx = ( rand() % ( PHASE_DOOR_RANGE * 2 + 1 ) ) - PHASE_DOOR_RANGE;
            int dy = ( rand() % ( PHASE_DOOR_RANGE * 2 + 1 ) ) - PHASE_DOOR_RANGE;
            JVector vTarget( m_vPos.x + dx, m_vPos.y + dy );
            if( vTarget.x >= 0 && vTarget.x < DUNG_WIDTH && vTarget.y >= 0 &&
                vTarget.y < DUNG_HEIGHT )
            {
                if( g_pGame->GetDungeon()->IsWalkableFor( vTarget, true ) ==
                    DUNG_COLL_NO_COLLISION )
                {
                    m_vPos = vTarget;
                    g_pGame->GetMsgs()->Printf( "You feel a brief shimmer.\n" );
                    return JSUCCESS;
                }
            }
        }
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
        return JSUCCESS;
    }
    // Full teleport: reuse SpawnPlayer logic
    m_bHasSpawned = false;
    SpawnPlayer();
    g_pGame->GetMsgs()->Printf( "You feel a wrenching sensation.\n" );
    return JSUCCESS;
}

JResult CPlayer::DoMagicMapping( CEffect *pEffect )
{
    if( pEffect->m_dwModifier & EFFECT_MOD_AREA )
    {
        // Limited range mapping
        int xMin = (int)m_vPos.x - MAGIC_MAPPING_RANGE;
        int xMax = (int)m_vPos.x + MAGIC_MAPPING_RANGE;
        int yMin = (int)m_vPos.y - MAGIC_MAPPING_RANGE;
        int yMax = (int)m_vPos.y + MAGIC_MAPPING_RANGE;
        g_pGame->GetDungeon()->RevealMap( xMin, yMin, xMax, yMax );
        g_pGame->GetMsgs()->Printf( "The area around you is revealed.\n" );
        return JSUCCESS;
    }
    // Full dungeon mapping
    g_pGame->GetDungeon()->RevealMap( 0, 0, DUNG_WIDTH - 1, DUNG_HEIGHT - 1 );
    g_pGame->GetMsgs()->Printf( "The dungeon is revealed to you.\n" );
    return JSUCCESS;
}

JResult CPlayer::DoRecall()
{
    CDungeon *pDungeon = g_pGame->GetDungeon();
    if( pDungeon->depth > 0 )
    {
        // In dungeon: save current depth, return to town
        if( m_dwRecallDepth > (uint8)pDungeon->depth )
        {
            g_pGame->GetMsgs()->Printf( "Recall depth reset (was: %dft)\n", m_dwRecallDepth * 50 );
        }
        m_dwRecallDepth = pDungeon->depth;
        int delta = -pDungeon->depth; // go to depth 0
        g_pGame->GetMsgs()->Printf( "The world spins and you find yourself in town.\n" );
        pDungeon->OnChangeLevel( delta );
    }
    else
    {
        // In town: return to last-visited dungeon depth
        int delta = m_dwRecallDepth; // go from 0 to recall depth
        g_pGame->GetMsgs()->Printf( "The world spins and you are back at %d ft.\n",
                                    m_dwRecallDepth * 50 );
        pDungeon->OnChangeLevel( delta );
    }
    return JSUCCESS;
}

JResult CPlayer::DoSummonMonsters()
{
    CDungeon *pDungeon = g_pGame->GetDungeon();
    JIVector vPlayerPos( (int)m_vPos.x, (int)m_vPos.y );
    int count = Util::Roll( 1, 3 );

    // Summon as if 20 levels deeper — punishes careless reading
    int effectiveDepth = pDungeon->depth + 20;
    if( effectiveDepth > DUNG_MAXDEPTH )
        effectiveDepth = DUNG_MAXDEPTH;

    for( int i = 0; i < count; i++ )
    {
        int which = pDungeon->ChooseMonsterForDepth( effectiveDepth, 15.0f );
        if( which == MON_IDX_INVALID )
            continue;
        CMonsterDef *pDef = pDungeon->GetMonsterDef( which );
        if( pDef )
            CMonster::CreateMonster( pDef, vPlayerPos, true );
    }

    g_pGame->GetMsgs()->Printf( "Monsters appear around you!\n" );
    return JSUCCESS;
}

JResult CPlayer::DoDestroyEffects( CEffect *pEffect, int dwItemFlags )
{
    if( pEffect->HasFlag( "EFFECT_FLAG_CURSE" ) )
    {
        if( dwItemFlags & ITEM_FLAG_CURSED )
        {
            JLog( LOG_LEVEL_DEBUG, true, "Cursing\n" );
            return DoApplyCurse();
        }
        else
        {
            JLog( LOG_LEVEL_DEBUG, true, "Uncursing\n" );
            return DoRemoveCurse();
        }
    }
    return JBOGUSKEY;
}

JResult CPlayer::DoRemoveCurse()
{
    CItem *cursed;
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    while( pLink != NULL )
    {
        if( pLink->m_lpData->m_dwFlags & ITEM_FLAG_CURSED )
        {
            JLog( LOG_LEVEL_DEBUG, true, "Item is cursed %s, uncursing it.\n",
                  pLink->m_lpData->GetName() );
            pLink->m_lpData->m_dwFlags &= ~ITEM_FLAG_CURSED;
            break;
        }
        pLink = pLink->next;
    }
    g_pGame->GetMsgs()->Printf( "It is no longer cursed.\n" );

    return JSUCCESS;
}

JResult CPlayer::DoApplyCurse()
{
    CItem *cursed;
    CLink<CItem> *pLink = m_llEquipment->GetHead();
    while( pLink != NULL )
    {
        if( ( pLink->m_lpData->m_dwFlags & ITEM_FLAG_CURSED ) == 0 )
        {
            JLog( LOG_LEVEL_NOISE, true, "Item is not cursed %s, cursing it.\n",
                  pLink->m_lpData->GetName() );
            pLink->m_lpData->m_dwFlags |= ITEM_FLAG_CURSED;
            g_pGame->GetMsgs()->Printf( "It is now cursed.\n" );
            break;
        }
        pLink = pLink->next;
    }

    return JSUCCESS;
}

JResult CPlayer::DoIntrinsicEffects( CEffect *pEffect, float fDuration )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_AFRAID:
        g_pGame->GetMsgs()->Printf( "You are afraid!\n" );
        break;
    case EFFECT_FLAG_BLIND:
        g_pGame->GetMsgs()->Printf( "You are blind.\n" );
        break;
    case EFFECT_FLAG_CONFUSE:
        g_pGame->GetMsgs()->Printf( "You are confused.\n" );
        break;
    case EFFECT_FLAG_POISON:
        g_pGame->GetMsgs()->Printf( "You are poisoned.\n" );
        break;
    case EFFECT_FLAG_PARALYZE:
        g_pGame->GetMsgs()->Printf( "You can't move!\n" );
        break;
    case EFFECT_FLAG_SLEEP:
        g_pGame->GetMsgs()->Printf( "You fall asleep.\n" );
        break;
    case EFFECT_FLAG_INFRA:
        g_pGame->GetMsgs()->Printf( "Your eyes feel tingly.\n" );
        break;
    case EFFECT_FLAG_ESP:
        g_pGame->GetMsgs()->Printf( "You sense stray thoughts around you.\n" );
        break;
    case EFFECT_FLAG_FIRE:
        if( pEffect->m_dwModifier & EFFECT_MOD_RESIST )
            g_pGame->GetMsgs()->Printf( "You feel resistant to fire.\n" );
        break;
    case EFFECT_FLAG_COLD:
        if( pEffect->m_dwModifier & EFFECT_MOD_RESIST )
            g_pGame->GetMsgs()->Printf( "You feel resistant to cold.\n" );
        break;
    case EFFECT_FLAG_ELECTRICITY:
        if( pEffect->m_dwModifier & EFFECT_MOD_RESIST )
            g_pGame->GetMsgs()->Printf( "You feel resistant to electricity.\n" );
        break;
    case EFFECT_FLAG_ACID:
        if( pEffect->m_dwModifier & EFFECT_MOD_RESIST )
            g_pGame->GetMsgs()->Printf( "You feel resistant to acid.\n" );
        break;
    case EFFECT_FLAG_INVISIBLE:
        g_pGame->GetMsgs()->Printf( "You fade from view.\n" );
        break;
    case EFFECT_FLAG_LEVITATE:
        g_pGame->GetMsgs()->Printf( "You feel light on your feet.\n" );
        break;
    case EFFECT_FLAG_FREE_ACTION:
        g_pGame->GetMsgs()->Printf( "You feel free to move.\n" );
        break;
    case EFFECT_FLAG_SPEED:
        g_pGame->GetMsgs()->Printf( "You feel yourself moving faster.\n" );
        m_fSpeed += 1.0f;
        break;
    case EFFECT_FLAG_LIGHT:
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "unknown intrinsic type: %d\n", pEffect->m_dwFlags );
        return JBOGUSKEY;
    }
    CEffect *pActive = new CEffect( *pEffect );
    SetIntrinsic( pActive->m_dwFlags );
    m_bLastEffectNoticed = true;
    if( ( pActive->m_dwModifier & EFFECT_MOD_TIMED ) != 0 )
    {
        pActive->m_fDuration = (int)fDuration;
        m_llActiveEffects->Add( pActive, pActive->m_dwFlags );
    }
    return JSUCCESS;
}

JResult CPlayer::UndoIntrinsicEffects( CEffect *pEffect )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_AFRAID:
    case EFFECT_FLAG_BLIND:
    case EFFECT_FLAG_CONFUSE:
    case EFFECT_FLAG_POISON:
    case EFFECT_FLAG_PARALYZE:
    case EFFECT_FLAG_SLEEP:
        return DoHealEffects( pEffect );
        break;
    case EFFECT_FLAG_INFRA:
        g_pGame->GetMsgs()->Printf( "Your eyes stop tinging.\n" );
        break;
    case EFFECT_FLAG_ESP:
        g_pGame->GetMsgs()->Printf( "You no longer sense stray thoughts.\n" );
        break;
    case EFFECT_FLAG_FIRE:
        g_pGame->GetMsgs()->Printf( "You no longer feel resistant to fire.\n" );
        break;
    case EFFECT_FLAG_COLD:
        g_pGame->GetMsgs()->Printf( "You no longer feel resistant to cold.\n" );
        break;
    case EFFECT_FLAG_ELECTRICITY:
        g_pGame->GetMsgs()->Printf( "You no longer feel resistant to electricity.\n" );
        break;
    case EFFECT_FLAG_ACID:
        g_pGame->GetMsgs()->Printf( "You no longer feel resistant to acid.\n" );
        break;
    case EFFECT_FLAG_INVISIBLE:
        g_pGame->GetMsgs()->Printf( "You reappear.\n" );
        break;
    case EFFECT_FLAG_LEVITATE:
        g_pGame->GetMsgs()->Printf( "You float gently to the ground.\n" );
        break;
    case EFFECT_FLAG_FREE_ACTION:
        g_pGame->GetMsgs()->Printf( "You feel sluggish.\n" );
        break;
    case EFFECT_FLAG_SPEED:
        g_pGame->GetMsgs()->Printf( "You feel yourself slowing down.\n" );
        m_fSpeed -= 1.0f;
        break;
    case EFFECT_FLAG_LIGHT:
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "unknown intrinsic type: %d\n", pEffect->m_dwFlags );
        return JBOGUSKEY;
    }
    UnsetIntrinsic( pEffect->m_dwFlags );
    return JSUCCESS;
}

JResult CPlayer::DoRestoreEffects( CEffect *pEffect )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_IDENTIFY:
        m_bLastEffectNoticed = true;
        return DoIdentify();
    case EFFECT_FLAG_HP:
        if( m_fCurHitPoints < m_fHitPoints )
        {
            m_fCurHitPoints = m_fHitPoints;
            g_pGame->GetMsgs()->Printf( "You feel completely healthy.\n" );
            m_bLastEffectNoticed = true;
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
        }
        return JSUCCESS;
    default:
        break;
    }
    return JSUCCESS;
}

JResult CPlayer::DoIdentify()
{
    m_bPendingIdentify = true;
    return JSUCCESS;
}

JResult CPlayer::DoGainEffects( CEffect *pEffect )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_FUEL:
        // Fuel gain is handled by the Fuel command in UseState
        m_bLastEffectNoticed = true;
        return JSUCCESS;
    case EFFECT_FLAG_XP:
    {
        float fGain = pEffect->m_szAmount ? Util::Roll( pEffect->m_szAmount ) : 50.0f;
        m_fExperience += fGain;
        g_pGame->GetMsgs()->Printf( "You feel more experienced.\n" );
        m_bLastEffectNoticed = true;
        return JSUCCESS;
    }
    default:
        break;
    }
    return JSUCCESS;
}

JResult CPlayer::DoLoseEffects( CEffect *pEffect )
{
    switch( pEffect->m_dwFlags )
    {
    case EFFECT_FLAG_XP:
    {
        float fLoss = pEffect->m_szAmount ? Util::Roll( pEffect->m_szAmount ) : 50.0f;
        m_fExperience -= fLoss;
        if( m_fExperience < 0.0f )
            m_fExperience = 0.0f;
        g_pGame->GetMsgs()->Printf( "You feel less experienced.\n" );
        m_bLastEffectNoticed = true;
        return JSUCCESS;
    }
    case EFFECT_FLAG_HP:
    {
        float fLoss = pEffect->m_szAmount ? Util::Roll( pEffect->m_szAmount ) : 10.0f;
        m_fHitPoints -= fLoss;
        if( m_fHitPoints < 1.0f )
            m_fHitPoints = 1.0f;
        if( m_fCurHitPoints > m_fHitPoints )
            m_fCurHitPoints = m_fHitPoints;
        g_pGame->GetMsgs()->Printf( "You feel weakened.\n" );
        m_bLastEffectNoticed = true;
        return JSUCCESS;
    }
    default:
        break;
    }
    return JSUCCESS;
}

JResult CPlayer::DoSeeEffects( CEffect *pEffect )
{
    CDungeon *pDungeon = g_pGame->GetDungeon();
    int range = ( pEffect->m_ed && pEffect->m_ed->m_fRange > 0 ) ? (int)pEffect->m_ed->m_fRange
                                                                 : MAGIC_MAPPING_RANGE;
    JIVector vPlayer( VEC_EXPAND( m_vPos ) );
    JRect rcCheck = Util::Nearby( vPlayer, range );

    if( pEffect->m_dwFlags2 & EFFECT_FLAG_DOOR )
    {
        // Reveal doors (secret and non-secret) and stairs within range.
        // Permanent for this level — once you know where they are, they stay on the map.
        bool bFoundDoors = false;
        bool bFoundStairs = false;
        JIVector vCheck;
        for( vCheck.y = rcCheck.top; vCheck.y <= rcCheck.bottom; vCheck.y++ )
        {
            for( vCheck.x = rcCheck.left; vCheck.x <= rcCheck.right; vCheck.x++ )
            {
                CDungeonTile *pTile = pDungeon->GetITile( vCheck );
                if( !pTile || !pTile->m_dtd )
                    continue;
                switch( pTile->m_dtd->m_dwType )
                {
                case DUNG_IDX_SECRET_DOOR:
                {
                    JVector vPos( (float)vCheck.x, (float)vCheck.y );
                    pDungeon->Modify( vPos );
                    bFoundDoors = true;
                    break;
                }
                case DUNG_IDX_DOOR:
                case DUNG_IDX_OPEN_DOOR:
                    pTile->SetFlags( DUNG_FLAG_SEEN );
                    bFoundDoors = true;
                    break;
                case DUNG_IDX_UPSTAIRS:
                case DUNG_IDX_LONG_UPSTAIRS:
                case DUNG_IDX_DOWNSTAIRS:
                case DUNG_IDX_LONG_DOWNSTAIRS:
                    pTile->SetFlags( DUNG_FLAG_SEEN );
                    bFoundStairs = true;
                    break;
                }
            }
        }
        if( bFoundDoors )
            g_pGame->GetMsgs()->Printf( "You sense the presence of doors!\n" );
        if( bFoundStairs )
            g_pGame->GetMsgs()->Printf( "You sense the presence of stairs!\n" );
        m_bLastEffectNoticed = true;
    }

    if( pEffect->m_dwFlags2 & EFFECT_FLAG_TRAP )
    {
        // Reveal traps within range. Permanent for this level.
        bool bFound = false;
        JIVector vCheck;
        for( vCheck.y = rcCheck.top; vCheck.y <= rcCheck.bottom; vCheck.y++ )
        {
            for( vCheck.x = rcCheck.left; vCheck.x <= rcCheck.right; vCheck.x++ )
            {
                CDungeonTile *pTile = pDungeon->GetITile( vCheck );
                if( pTile && ( pTile->m_dwFlags & DUNG_FLAG_TRAP ) )
                {
                    pTile->SetFlags( DUNG_FLAG_SEEN );
                    bFound = true;
                }
            }
        }
        if( bFound )
            g_pGame->GetMsgs()->Printf( "You sense traps.\n" );
        m_bLastEffectNoticed = true;
    }

    if( pEffect->m_dwFlags2 & EFFECT_FLAG_MONSTERS )
    {
        // Detect monsters within range. One-turn duration: m_bDetected is cleared
        // at the start of the next UpdateVisibleMonsters() call.
        CLink<CMonster> *pLink = pDungeon->m_llMonsters->GetHead();
        bool bFound = false;
        while( pLink )
        {
            CMonster *pMon = pLink->m_lpData;
            if( pMon )
            {
                JVector vMonPos = pMon->GetPos();
                int dx = abs( (int)vMonPos.x - vPlayer.x );
                int dy = abs( (int)vMonPos.y - vPlayer.y );
                if( dx <= range && dy <= range )
                {
                    pMon->m_bDetected = true;
                    CDungeonTile *pTile = pDungeon->GetTile( vMonPos );
                    if( pTile )
                        pTile->SetFlags( DUNG_FLAG_SEEN );
                    bFound = true;
                }
            }
            pLink = pLink->next;
        }
        if( bFound )
            g_pGame->GetMsgs()->Printf( "You sense the presence of monsters!\n" );
        m_bLastEffectNoticed = true;
    }

    return JSUCCESS;
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

bool CPlayer::IsFireable( CLink<CItem> *pLink )
{
    bool retval = false;
    CItem *pItem = pLink->m_lpData;
    if( pItem == NULL || pItem->m_id == NULL )
        return false;

    switch( pItem->m_id->m_dwIndex )
    {
    case ITEM_IDX_ARROW:
    case ITEM_IDX_BOLT:
        // Only fireable if we have ammo remaining
        retval = ( pItem->m_dwCount > 0 );
        break;
    default:
        break;
    }
    return retval;
}

bool CPlayer::IsCompatibleAmmo( CLink<CItem> *pLink )
{
    // Check if this ammo is compatible with the equipped primary weapon
    if( !IsFireable( pLink ) )
    {
        return false;
    }

    CLink<CItem> *pMainWeapon = m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND );
    if( pMainWeapon == NULL )
    {
        return false;
    }

    uint32 weaponType = pMainWeapon->m_lpData->m_id->m_dwIndex;
    uint32 ammoType = pLink->m_lpData->m_id->m_dwIndex;

    // BOW uses ARROW, XBOW uses BOLT
    if( weaponType == ITEM_IDX_BOW && ammoType == ITEM_IDX_ARROW )
    {
        return true;
    }
    if( weaponType == ITEM_IDX_XBOW && ammoType == ITEM_IDX_BOLT )
    {
        return true;
    }

    return false;
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

bool CPlayer::IsZappable( CLink<CItem> *pLink )
{
    bool retval = false;
    switch( pLink->m_lpData->m_id->m_dwIndex )
    {
    case ITEM_IDX_WAND:
        retval = true;
        break;
    default:
        break;
    }
    return retval;
}

bool CPlayer::SetName( const char *szName )
{
    if( Util::jstrlen( szName ) > MAX_STRING_LENGTH - 1 )
    {
        return false;
    }

    Util::jstrcpy( m_szName, szName );

    return true;
}

void CPlayer::SetWizard()
{
    if( !m_bWizardMode )
    {
        SetName( "** Wizard Mode **" );
        g_pGame->GetMsgs()->Printf( "*** Wizard Mode: On *** your score will not be saved.\n" );
        JLog( LOG_LEVEL_WARN, true, "*** Wizard Mode: On *** your score will not be saved.\n" );
    }
    m_bWizardMode = true;
}

void CPlayer::ClearWizard() { m_bWizardMode = false; }

void CPlayer::ClearVisibleMonsters()
{
    if( m_llVisibleMonsters )
    {
        m_llVisibleMonsters->Terminate();
        delete m_llVisibleMonsters;
        m_llVisibleMonsters = NULL;
    }
}

void CPlayer::UpdateVisibleMonsters()
{
    ClearVisibleMonsters();
    m_llVisibleMonsters = new JLinkList<CMonster>( false );

    CDungeon *pDungeon = g_pGame->GetDungeon();
    CLink<CMonster> *pLink = pDungeon->m_llMonsters->GetHead();
    while( pLink != NULL )
    {
        if( !pLink->m_lpData )
        {
            pLink = pLink->next;
            continue;
        }
        CMonster *pMon = pLink->m_lpData;

        // Clear one-turn detection flag from previous turn
        pMon->m_bDetected = false;

        bool bPlayerSees = pDungeon->PlayerCanSee(
            pMon->GetPos(), pMon->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND ) );
        if( bPlayerSees )
        {
            JVector vMonPos = pMon->GetPos();
            int dist =
                abs( (int)vMonPos.x - (int)m_vPos.x ) + abs( (int)vMonPos.y - (int)m_vPos.y );
            m_llVisibleMonsters->Add( pMon, dist, pMon->GetInstanceId() );

            if( g_pGame->RecallMonster() )
            {
                int depthFeet = pDungeon->depth * 50;
                g_pGame->RecallMonster()->RecordSighting( pMon->m_md->m_szName, depthFeet,
                                                          pMon->GetInstanceId() );
            }
        }
        pLink = pLink->next;
    }
}

JLinkList<CMonster> *CPlayer::GetVisibleMonsters()
{
    if( !m_llVisibleMonsters )
        UpdateVisibleMonsters();
    return m_llVisibleMonsters;
}
