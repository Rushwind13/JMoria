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

JResult CItem::CreateItem( CItemDef *pid )
{
//    int desired = Util::Roll(pid->m_szAppear);
//    for( int count=0; count < desired; count++ )
    {
        CItem *pItem;
        pItem = new CItem;
        
        // Initialize the Item from the ItemDef
        pItem->Init(pid);
        
        // Put the item in the world
        pItem->SpawnItem();
        
        // Now that the item is set up, add it to the global list of items
        pItem->m_pllLink = g_pGame->GetDungeon()->m_llItems->Add(pItem);
    }
    
    return JSUCCESS;
}

void CItem::Init( CItemDef *pid )
{
    m_id = pid;
    m_Color.SetColor(m_id->m_Color);
    if( Util::GetRandom(0,1) )
    {
        m_dwFlags |= ITEM_FLAG_CURSED;
        m_Color.SetColor(255,0,0,255);
    }
}

JResult CItem::SpawnItem()
{
    bool bItemSpawned = false;
    printf("Trying to spawn item type: %s...", m_id->m_szName);
    JVector vTryPos;
    while( !bItemSpawned )
    {
        vTryPos.Init( (float)(Util::GetRandom(0, DUNG_WIDTH-1)), (float)(Util::GetRandom(0, DUNG_HEIGHT-1)) );
        
        //printf("Trying to spawn item type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
        //g_pGame->GetMsgs()->Printf( "Trying to spawn item type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
        
        if( g_pGame->GetDungeon()->CanPlaceItemAt(vTryPos) == DUNG_COLL_NO_COLLISION )
        {
            m_vPos = vTryPos;
            g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurItem = this;
            bItemSpawned = true;
            printf( "Success!\n" );
            //g_pGame->GetMsgs()->Printf( "Success!\n" );
        }
    }
    
    return JSUCCESS;
}

// draw routines
void CItem::SetColor()
{
    if( m_fColorChangeInterval < COLOR_CHANGE_TIMEOUT ) return;
    
    if( (m_id->m_dwFlags & ITEM_COLOR_MULTI) == ITEM_COLOR_MULTI )
    {
        int which_color = Util::GetRandom(0,m_id->m_Colors->length()-1);
        m_Color.SetColor(*(m_id->m_Colors->GetLink(which_color)->m_lpData));
    }
    m_fColorChangeInterval = 0.0f;
}


unsigned char ItemIDs[ITEM_IDX_MAX+1] = "|)[](]]\"=~{}{}&?!-_?$~";
void CItem::Draw()
{
    // Don't draw if something else is there.
    if( g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurMonster != NULL ||
       g_pGame->GetPlayer()->m_vPos == m_vPos )
    {
        return;
    }
    Uint8 item_tile = ItemIDs[m_id->m_dwIndex] - ' ' - 1;
    JVector vSize(1,1);
    
    SetColor();
    
    //PreDraw();
    g_pGame->GetDungeon()->m_TileSet->SetTileColor( m_Color );
    g_pGame->GetDungeon()->m_TileSet->DrawTile( item_tile, m_vPos, vSize, false );
    //PostDraw();
}

void CItem::PreDraw()
{
    g_pGame->GetDungeon()->PreDraw();
}

void CItem::PostDraw()
{
    g_pGame->GetDungeon()->PostDraw();
}
