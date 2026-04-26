//
//  Item.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/7/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "Item.h"
#include "Dungeon.h"
#include "Player.h"

// Simple instance id generator for items
static uint32 s_nextItemInstanceId = 1;

// --- Magic item spawn tuning ---
#define IMBUE_CHANCE_MIN 5    // % magic chance at depth 0
#define IMBUE_CHANCE_MAX 85   // % magic chance at depth 80
#define IMBUE_CHANCE_DEPTH 80 // depth at which max chance is reached
#define IMBUE_WEAPON_MAX 7    // max weapon bonus pool
#define IMBUE_WEAPON_SCALE 10 // depth divisor for weapon pool
#define IMBUE_ARMOR_MAX 4     // max armor AC bonus
#define IMBUE_ARMOR_SCALE 15  // depth divisor for armor pool
#define IMBUE_CURSED_CHANCE 5 // % chance a magic item spawns cursed

// Roll a bonus from 1..(1 + depth/scale), capped at cap
static int BonusForDepth( int depth, int scale, int cap )
{
    int maxVal = 1 + depth / scale;
    if( maxVal > cap )
        maxVal = cap;
    return Util::GetRandom( 1, maxVal );
}

JResult CItem::CreateItem( CItemDef *pid, JVector vSpawnPoint, bool bNear )
{
    //    int desired = Util::Roll(pid->m_szAppear);
    //    for( int count=0; count < desired; count++ )
    {
        CItem *pItem;
        pItem = new CItem;

        // Initialize the Item from the ItemDef
        pItem->Init( pid );

        // Imbue weapons/armor with magic bonuses based on dungeon depth
        int depth =
            ( g_pGame && g_pGame->GetDungeon() ) ? g_pGame->GetDungeon()->depth : pid->m_dwLevel;
        pItem->Imbue( depth );

        if( g_pGame )
        {
            // Put the item in the world
            pItem->SpawnItem( /*bNear? Util::Near(vSpawnPoint): /**/ vSpawnPoint );

            // Now that the item is set up, add it to the global list of items
            pItem->m_pllLink = g_pGame->GetDungeon()->m_llItems->Add( pItem );
        }
    }

    return JSUCCESS;
}

void CItem::Init( CItemDef *pid )
{
    m_id = pid;
    // assign a unique instance id when initializing the item
    if( m_dwInstanceId == 0 )
    {
        m_dwInstanceId = s_nextItemInstanceId++;
    }
    m_Color.SetColor( m_id->m_Color );

    // Roll per-instance bonuses from CItemDef NdM dice strings
    m_fACBonus = m_id->m_szACBonus ? Util::Roll( m_id->m_szACBonus ) : 0.0f;
    m_fBonusToHit = m_id->m_szBonusToHit ? Util::Roll( m_id->m_szBonusToHit ) : 0.0f;
    m_fBonusToDamage = m_id->m_szBonusToDamage ? Util::Roll( m_id->m_szBonusToDamage ) : 0.0f;

    // Speed bonus: rings randomize 0.1-1.0; other equipment use fixed m_id->m_fSpeed
    if( m_id->m_dwIndex == ITEM_IDX_RING )
    {
        bool hasSpeedEffect = false;
        if( m_id->m_llEffects )
        {
            CLink<CEffect> *pLink = m_id->m_llEffects->GetHead();
            while( pLink )
            {
                if( pLink->m_lpData->m_dwFlags & EFFECT_FLAG_SPEED )
                {
                    hasSpeedEffect = true;
                    break;
                }
                pLink = pLink->next;
            }
        }
        m_fSpeedBonus = hasSpeedEffect ? (float)Util::GetRandom( 1, 20 ) / 10.0f : 0.0f;
    }
    else
    {
        m_fSpeedBonus = m_id->m_fSpeed;
    }

    switch( m_id->m_dwIndex )
    {
    case ITEM_IDX_POTION:
    case ITEM_IDX_SCROLL:
    case ITEM_IDX_TORCH:
        m_fRemainingDuration = Util::GetRandom( 0.0f, m_id->m_fDuration );
        break;
    case ITEM_IDX_STAFF:
    case ITEM_IDX_WAND:
        if( m_id->m_szCharges )
        {
            m_dwCharges = (uint32)Util::Roll( m_id->m_szCharges );
        }
        else
        {
            m_dwCharges = (uint32)Util::Roll( "1d20" );
        }
        m_dwMaxCharges = m_dwCharges * 2;
        break;
    }
}

CItem *CItem::Copy( int quantity )
{
    CItem *pCopy = new CItem();
    pCopy->Init( m_id );
    pCopy->m_dwCount = ( quantity > 0 ) ? quantity : m_dwCount;

    // Copy all instance state
    pCopy->m_dwKnownProps = m_dwKnownProps;
    pCopy->m_dwFlags = m_dwFlags;
    pCopy->m_fACBonus = m_fACBonus;
    pCopy->m_fBonusToHit = m_fBonusToHit;
    pCopy->m_fBonusToDamage = m_fBonusToDamage;
    pCopy->m_fSpeedBonus = m_fSpeedBonus;
    pCopy->m_dwCharges = m_dwCharges;
    pCopy->m_dwMaxCharges = m_dwMaxCharges;

    return pCopy;
}

void CItem::SetCursed( bool bCursed )
{
    if( bCursed )
    {
        m_dwFlags &= ~ITEM_FLAG_CURSED;
        m_dwFlags |= ITEM_FLAG_CURSED;
        m_Color.SetColor( 255, 0, 0, 255 );
    }
    else
    {
        m_dwFlags &= ~ITEM_FLAG_CURSED;
        m_Color.SetColor( m_id->m_Color );
    }
}

void CItem::SetCursed( int likelihood )
{
    int rolled = (int)Util::GetRandom( 1.0f, 100.0f );
    SetCursed( rolled < likelihood );
}

void CItem::Imbue( int depth )
{
    // Items already flagged MAGIC in their definition (e.g., Helm of Infravision)
    // keep their data-defined bonuses — skip the random magic roll
    if( m_id->m_dwFlags & ITEM_FLAG_MAGIC )
        return;

    // Only equipment slots that benefit from magic bonuses
    int slot = EquipType();
    bool isRanged = ( m_id->m_dwIndex == ITEM_IDX_BOW || m_id->m_dwIndex == ITEM_IDX_XBOW );
    bool isWeapon = ( slot == EQUIP_IDX_MAIN_HAND ) && !isRanged;
    bool isAmmo = ( slot == EQUIP_IDX_AMMO );
    bool isArmor =
        ( slot == EQUIP_IDX_ARMOR || slot == EQUIP_IDX_OFF_HAND || slot == EQUIP_IDX_HELMET ||
          slot == EQUIP_IDX_CLOAK || slot == EQUIP_IDX_GLOVES || slot == EQUIP_IDX_BOOTS );

    if( !isWeapon && !isRanged && !isAmmo && !isArmor )
        return;

    // Magic chance scales with depth: 5% at depth 0, up to 85% at depth 80
    int magicChance =
        IMBUE_CHANCE_MIN + ( depth * ( IMBUE_CHANCE_MAX - IMBUE_CHANCE_MIN ) ) / IMBUE_CHANCE_DEPTH;
    if( magicChance > IMBUE_CHANCE_MAX )
        magicChance = IMBUE_CHANCE_MAX;

    int roll = Util::GetRandom( 1, 100 );
    if( roll > magicChance )
        return; // mundane: +0, +0

    // -- This item is magical --
    m_dwFlags |= ITEM_FLAG_MAGIC;

    if( isWeapon )
    {
        int pool = BonusForDepth( depth, IMBUE_WEAPON_SCALE, IMBUE_WEAPON_MAX );
        // Split pool between to-hit and to-damage
        int toHit = Util::GetRandom( 0, pool );
        int toDam = pool - toHit;
        m_fBonusToHit = (float)toHit;
        m_fBonusToDamage = (float)toDam;
    }
    else if( isRanged )
    {
        // Ranged weapons (bows/xbows) get to-hit only
        m_fBonusToHit = (float)BonusForDepth( depth, IMBUE_WEAPON_SCALE, IMBUE_WEAPON_MAX );
    }
    else if( isAmmo )
    {
        // Ammo (arrows/bolts) get to-dam only
        m_fBonusToDamage = (float)BonusForDepth( depth, IMBUE_WEAPON_SCALE, IMBUE_WEAPON_MAX );
    }
    else if( isArmor )
    {
        m_fACBonus = (float)BonusForDepth( depth, IMBUE_ARMOR_SCALE, IMBUE_ARMOR_MAX );
    }

    // ~5% chance the magic item is cursed — bonuses become penalties
    if( Util::GetRandom( 1, 100 ) <= IMBUE_CURSED_CHANCE )
    {
        SetCursed( true );
        m_fBonusToHit = -m_fBonusToHit;
        m_fBonusToDamage = -m_fBonusToDamage;
        m_fACBonus = -m_fACBonus;
    }
}

JResult CItem::SpawnItem( JVector vSpawnPoint )
{
    bool bItemSpawned = false;
    JLog( LOG_LEVEL_INFO, false, "Trying to spawn item type: %s...", m_id->m_szName );

    if( vSpawnPoint.IsWithinWorld() )
    {
        return SpawnAt( vSpawnPoint );
    }

    JVector vTryPos;
    JIVector *vOpen;
    while( !bItemSpawned )
    {
        JLog( LOG_LEVEL_INFO, false, "." );
        vOpen = g_pGame->GetDungeon()->AnyOpenTile();
        vTryPos.Init( VEC_EXPAND( *vOpen ) );

        // JLog( LOG_LEVEL_NOISE, false, "Trying to spawn item type: %d at <%.2f %.2f>...\n",
        // m_md->m_dwType, vTryPos.x, vTryPos.y ); g_pGame->GetMsgs()->Printf( "Trying to spawn item
        // type: %d at <%.2f
        // %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );

        if( SpawnAt( vTryPos ) == JSUCCESS )
        {
            bItemSpawned = true;
        }
    }
    return JSUCCESS;
}

JResult CItem::SpawnAt( JVector vSpawnPoint )
{
    if( g_pGame && g_pGame->GetDungeon()->CanPlaceItemAt( vSpawnPoint ) == DUNG_COLL_NO_COLLISION )
    {
        m_vPos = vSpawnPoint;
        g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurItem = this;

        JLog( LOG_LEVEL_INFO, false, "Success! Spawned at <%.2f %.2f>\n", VEC_EXPAND( m_vPos ) );
        // g_pGame->GetMsgs()->Printf( "Success!\n" );

        return JSUCCESS;
    }
    return JBOGUSKEY;
}

bool CItem::Update( float fCurTime )
{
    if( ( m_id->m_dwFlags & ITEM_COLOR_MULTI ) == ITEM_COLOR_MULTI )
    {
        m_fColorChangeInterval += fCurTime;
    }

    if( !g_pGame->GetPlayer()->m_bIsDisturbed )
    {
        JIVector vItem( VEC_EXPAND( m_vPos ) );
        JIVector vPlayer( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
        if( Util::Nearby( vItem, 1 ).Contains( vPlayer ) )
        {
            g_pGame->GetDungeon()->DisturbPlayer();
        }
    }
    return true;
}

// draw routines
void CItem::SetColor()
{
    if( m_fColorChangeInterval < COLOR_CHANGE_TIMEOUT )
        return;

    if( ( m_id->m_dwFlags & ITEM_COLOR_MULTI ) == ITEM_COLOR_MULTI )
    {
        int which_color = Util::GetRandom( 0, m_id->m_Colors->length() - 1 );
        m_Color.SetColor( *( m_id->m_Colors->GetNthLink( which_color )->m_lpData ) );
    }
    m_fColorChangeInterval = 0.0f;
}

unsigned char ItemIDs[ITEM_IDX_MAX + 1] = "|)[](]]\"=~{}{}&?!-_?$~//\\/|/|]!";
const int EquipTypes[ITEM_IDX_MAX + 1] = {
    EQUIP_IDX_MAIN_HAND, EQUIP_IDX_OFF_HAND,  EQUIP_IDX_ARMOR,     EQUIP_IDX_HELMET,
    EQUIP_IDX_CLOAK,     EQUIP_IDX_GLOVES,    EQUIP_IDX_BOOTS,     EQUIP_IDX_AMULET,
    EQUIP_IDX_RING,      EQUIP_IDX_TORCH,     EQUIP_IDX_MAIN_HAND, EQUIP_IDX_AMMO,
    EQUIP_IDX_MAIN_HAND, EQUIP_IDX_AMMO,      EQUIP_IDX_INVALID,   EQUIP_IDX_INVALID,
    EQUIP_IDX_INVALID,   EQUIP_IDX_INVALID,   EQUIP_IDX_INVALID,   EQUIP_IDX_INVALID,
    EQUIP_IDX_INVALID,   EQUIP_IDX_INVALID,   EQUIP_IDX_MAIN_HAND, EQUIP_IDX_MAIN_HAND,
    EQUIP_IDX_MAIN_HAND, EQUIP_IDX_MAIN_HAND, EQUIP_IDX_MAIN_HAND, EQUIP_IDX_MAIN_HAND,
    EQUIP_IDX_MAIN_HAND, EQUIP_IDX_BELT,      EQUIP_IDX_INVALID };

int CItem::EquipType()
{
    int item_type = m_id->m_dwIndex;
    if( item_type <= ITEM_IDX_INVALID || item_type >= ITEM_IDX_MAX )
        return EQUIP_IDX_INVALID;
    return EquipTypes[item_type];
}

void CItemDef::FormatProperties( char *szOut, int maxLen, uint32 knownProps, uint32 itemFlags,
                                 uint32 charges, float fACBonus, float fBonusToHit,
                                 float fBonusToDamage )
{
    int pos = 0;
    if( knownProps & KNOWN_BONUSES )
    {
        switch( m_dwIndex )
        {
        case ITEM_IDX_SWORD:
        case ITEM_IDX_DAGGER:
        case ITEM_IDX_MACE:
        case ITEM_IDX_SPEAR:
        case ITEM_IDX_AXE:
        case ITEM_IDX_POLEARM:
        case ITEM_IDX_2H_SWORD:
            if( fBonusToHit != 0.0f || fBonusToDamage != 0.0f )
                pos += snprintf( szOut + pos, maxLen - pos, " (%+.0f, %+.0f)", fBonusToHit,
                                 fBonusToDamage );
            break;
        case ITEM_IDX_ARMOR:
        case ITEM_IDX_SHIELD:
        case ITEM_IDX_HELMET:
        case ITEM_IDX_CLOAK:
        case ITEM_IDX_GLOVES:
        case ITEM_IDX_BOOTS:
            if( fACBonus != 0.0f )
                pos += snprintf( szOut + pos, maxLen - pos, " [%+.0f]", fACBonus );
            break;
        case ITEM_IDX_RING:
        case ITEM_IDX_AMULET:
            if( fBonusToHit != 0.0f || fBonusToDamage != 0.0f )
                pos += snprintf( szOut + pos, maxLen - pos, " (%+.0f, %+.0f)", fBonusToHit,
                                 fBonusToDamage );
            else if( fACBonus != 0.0f )
                pos += snprintf( szOut + pos, maxLen - pos, " [%+.0f]", fACBonus );
            break;

        default:
            break;
        }
    }
    if( ( knownProps & KNOWN_CHARGES ) &&
        ( m_dwIndex == ITEM_IDX_WAND || m_dwIndex == ITEM_IDX_STAFF ) )
    {
        pos += snprintf( szOut + pos, maxLen - pos, " (%d charges)", charges );
    }
    if( ( knownProps & KNOWN_CURSED ) && ( itemFlags & ITEM_FLAG_CURSED ) )
    {
        pos += snprintf( szOut + pos, maxLen - pos, " {cursed}" );
    }
}

const char *CItem::GetName()
{
    static char szDisplay[128];
    const char *baseName;
    if( IsIdentified() )
    {
        baseName = m_id->m_szName;
    }
    else
    {
        baseName = m_id->m_szUnidentifiedName;
        if( m_id->m_bTried )
        {
            snprintf( szDisplay, sizeof( szDisplay ), "%s {tried}", baseName );
            return szDisplay;
        }
        return baseName;
    }

    snprintf( szDisplay, sizeof( szDisplay ), "%s", baseName );
    int baseLen = strlen( szDisplay );
    m_id->FormatProperties( szDisplay + baseLen, sizeof( szDisplay ) - baseLen, m_dwKnownProps,
                            m_dwFlags, m_dwCharges, m_fACBonus, m_fBonusToHit, m_fBonusToDamage );
    return szDisplay;
}

const char *CItem::GetPlural()
{
    static char szDisplay[128];
    const char *baseName;
    if( IsIdentified() )
    {
        baseName = m_id->m_szPlural;
    }
    else
    {
        baseName = m_id->m_szUnidentifiedPlural;
        if( m_id->m_bTried )
        {
            snprintf( szDisplay, sizeof( szDisplay ), "%s {tried}", baseName );
            return szDisplay;
        }
        return baseName;
    }

    snprintf( szDisplay, sizeof( szDisplay ), "%s", baseName );
    int baseLen = strlen( szDisplay );
    m_id->FormatProperties( szDisplay + baseLen, sizeof( szDisplay ) - baseLen, m_dwKnownProps,
                            m_dwFlags, m_dwCharges, m_fACBonus, m_fBonusToHit, m_fBonusToDamage );
    return szDisplay;
}

void CItem::Draw()
{
    // Don't draw if something else is there.
    if( g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurMonster != NULL ||
        g_pGame->GetPlayer()->m_vPos == m_vPos )
    {
        return;
    }

    char item_char = ItemIDs[m_id->m_dwIndex];
    JVector DUNG_ASPECT;

    SetColor();

    // PreDraw();
    g_pGame->GetDungeon()->m_TileSet->SetTileColor( m_Color );
    g_pGame->GetDungeon()->m_TileSet->DrawChar( item_char, m_vPos, vSize );
    // PostDraw();
}

void CItem::PreDraw() { g_pGame->GetDungeon()->PreDraw(); }

void CItem::PostDraw() { g_pGame->GetDungeon()->PostDraw(); }
