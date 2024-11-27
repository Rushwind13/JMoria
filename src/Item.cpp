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

JResult CItem::CreateItem( CItemDef *pid, JVector vSpawnPoint, bool bNear )
{
    //    int desired = Util::Roll(pid->m_szAppear);
    //    for( int count=0; count < desired; count++ )
    {
        CItem *pItem;
        pItem = new CItem;

        // Initialize the Item from the ItemDef
        pItem->Init( pid );

        // Apply cursed flag
        pItem->SetCursed( 5 );

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
    m_Color.SetColor( m_id->m_Color );
    m_fRemainingDuration = Util::GetRandom( 0.0f, m_id->m_fDuration );
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

char *CItem::GetName()
{
    if( false ) // IsIdentified() ) // TODO: MIKE: ID goes here
    {
        return m_id->m_szName;
    }
    else
    {
        return m_id->m_szUnidentifiedName;
    }
}

char *CItem::GetPlural()
{
    if( false ) // IsIdentified() )// TODO: MIKE: ID goes here
    {
        return m_id->m_szPlural;
    }
    else
    {
        return m_id->m_szUnidentifiedPlural;
    }
}

void CItem::Draw()
{
    // Don't draw if something else is there.
    if( g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurMonster != NULL ||
        g_pGame->GetPlayer()->m_vPos == m_vPos )
    {
        return;
    }

    Uint8 item_tile = ItemIDs[m_id->m_dwIndex] - ' ' - 1;
    JVector DUNG_ASPECT;

    SetColor();

    // PreDraw();
    g_pGame->GetDungeon()->m_TileSet->SetTileColor( m_Color );
    g_pGame->GetDungeon()->m_TileSet->DrawTile( item_tile, m_vPos, vSize, false );
    // PostDraw();
}

void CItem::PreDraw() { g_pGame->GetDungeon()->PreDraw(); }

void CItem::PostDraw() { g_pGame->GetDungeon()->PostDraw(); }
