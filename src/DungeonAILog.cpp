// DungeonAILog.cpp
//
// AI logging implementation for CDungeon.
// Separated to keep Dungeon.cpp focused on game logic.

#include "Dungeon.h"
#include "AILog.h"
#include "AIRender.h"
#include "Player.h"

extern unsigned char TileIDs[];

void CDungeon::DumpToAILog()
{
    if( !AILog_IsActive() )
    {
        return;
    }

    CPlayer *player = g_pGame->GetPlayer();
    if( !player )
    {
        return;
    }

    JIVector playerPos( (int)player->m_vPos.x, (int)player->m_vPos.y );

    // Calculate visible bounds (21x21 centered on player), clamped to dungeon
    JRect viewBounds(
        playerPos.x - VIEW_RADIUS < 0 ? 0 : playerPos.x - VIEW_RADIUS,
        playerPos.y - VIEW_RADIUS < 0 ? 0 : playerPos.y - VIEW_RADIUS,
        playerPos.x + VIEW_RADIUS >= DUNG_WIDTH ? DUNG_WIDTH - 1 : playerPos.x + VIEW_RADIUS,
        playerPos.y + VIEW_RADIUS >= DUNG_HEIGHT ? DUNG_HEIGHT - 1 : playerPos.y + VIEW_RADIUS
    );

    int viewWidth = viewBounds.Width() + 1;
    int viewHeight = viewBounds.Height() + 1;

    // Build visible area map
    char map[VIEW_SIZE][VIEW_SIZE + 1];

    for( int vy = 0; vy < viewHeight; vy++ )
    {
        int worldY = viewBounds.top + vy;
        for( int vx = 0; vx < viewWidth; vx++ )
        {
            int worldX = viewBounds.left + vx;
            CDungeonTile *tile = m_Tiles + ( worldY * DUNG_WIDTH ) + worldX;
            if( tile && tile->m_dtd )
            {
                // Only show tiles that have been seen (prevents AI from "cheating")
                if( !( tile->m_dwFlags & DUNG_FLAG_SEEN ) )
                {
                    map[vy][vx] = ' ';
                }
                else
                {
                    int tileType = tile->m_dtd->m_dwType;
                    if( tileType >= 0 && tileType < DUNG_IDX_MAX )
                    {
                        map[vy][vx] = TileIDs[tileType];
                    }
                    else
                    {
                        map[vy][vx] = ' ';
                    }
                }
            }
            else
            {
                map[vy][vx] = ' ';
            }
        }
        map[vy][viewWidth] = '\0';
    }

    // Overlay items in visible area
    if( m_llItems )
    {
        CLink<CItem> *pCur = m_llItems->GetHead();
        while( pCur )
        {
            CItem *item = pCur->m_lpData;
            if( item )
            {
                JIVector itemPos( (int)item->m_vPos.x, (int)item->m_vPos.y );
                if( viewBounds.Contains( itemPos ) )
                {
                    map[itemPos.y - viewBounds.top][itemPos.x - viewBounds.left] = item->GetChar();
                }
            }
            pCur = m_llItems->GetNext( pCur );
        }
    }

    // Overlay monsters in visible area
    if( m_llMonsters )
    {
        CLink<CMonster> *pCur = m_llMonsters->GetHead();
        while( pCur )
        {
            CMonster *mon = pCur->m_lpData;
            if( mon )
            {
                JIVector monPos( (int)mon->GetPos().x, (int)mon->GetPos().y );
                if( viewBounds.Contains( monPos ) )
                {
                    map[monPos.y - viewBounds.top][monPos.x - viewBounds.left] = mon->GetChar();
                }
            }
            pCur = m_llMonsters->GetNext( pCur );
        }
    }

    // Overlay player
    if( viewBounds.Contains( playerPos ) )
    {
        map[playerPos.y - viewBounds.top][playerPos.x - viewBounds.left] = '@';
    }

    // Output DUNGEON event in text format
    AILog_Text( "DUNGEON turn:%d level:%d depth:%dft",
                g_pGame->GetITime(), depth, depth * 50 );
    AILog_Text( "  VIEW %d,%d %dx%d",
                viewBounds.left, viewBounds.top, viewWidth, viewHeight );
    AILog_Text( "  PLAYER %d,%d hp:%d/%d",
                (int)player->m_vPos.x, (int)player->m_vPos.y,
                (int)player->GetHP(), (int)player->GetMaxHP() );

    // Output map with RLE-encoded rows, skipping all-wall rows
    AILog_Text( "  MAP" );
    for( int vy = 0; vy < viewHeight; vy++ )
    {
        if( AIRender_IsAllWalls( map[vy], viewWidth ) )
        {
            continue;
        }

        char rleBuffer[256];
        AIRender_RLEEncodeRow( map[vy], viewWidth, rleBuffer );
        AILog_Text( "    %d: %s", vy, rleBuffer );
    }
    AILog_Text( "  ENDMAP" );

    // Room/hallway counts
    if( m_dmCurLevel )
    {
        AILog_Text( "  ROOMS %d", m_dmCurLevel->HowManyRooms() );
        AILog_Text( "  HALLS %d", m_dmCurLevel->HowManyHallways() );
    }

    // Monster list
    if( m_llMonsters )
    {
        CLink<CMonster> *pCur = m_llMonsters->GetHead();
        while( pCur )
        {
            CMonster *mon = pCur->m_lpData;
            if( mon )
            {
                AILog_Text( "  MON %s %c %d,%d hp:%d",
                            AILog_Name( mon->GetName() ), mon->GetChar(),
                            (int)mon->GetPos().x, (int)mon->GetPos().y,
                            (int)mon->m_fCurHP );
            }
            pCur = m_llMonsters->GetNext( pCur );
        }
    }

    // Item list
    if( m_llItems )
    {
        CLink<CItem> *pCur = m_llItems->GetHead();
        while( pCur )
        {
            CItem *item = pCur->m_lpData;
            if( item )
            {
                AILog_Text( "  ITEM %s %c %d,%d",
                            AILog_Name( item->GetName() ), item->GetChar(),
                            (int)item->m_vPos.x, (int)item->m_vPos.y );
            }
            pCur = m_llItems->GetNext( pCur );
        }
    }

    AILog_Text( "ENDDUNGEON" );
    AILog_BlankLine();
}
