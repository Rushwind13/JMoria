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

    // Build JSON output with RLE-encoded visible map
    char buffer[20000];
    int offset = 0;

    offset += sprintf( buffer + offset, "{\"type\":\"dungeon\"" );
    offset += sprintf( buffer + offset, ",\"turn\":%d", g_pGame->GetITime() );
    offset += sprintf( buffer + offset, ",\"level\":%d", depth );
    offset += sprintf( buffer + offset, ",\"depth_ft\":%d", depth * 50 );
    offset += sprintf( buffer + offset, ",\"view\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
                       viewBounds.left, viewBounds.top, viewWidth, viewHeight );

    // Add map as array of RLE-encoded strings, skipping all-wall rows
    offset += sprintf( buffer + offset, ",\"map\":[" );
    bool firstRow = true;
    for( int vy = 0; vy < viewHeight; vy++ )
    {
        // Skip all-wall rows
        if( AIRender_IsAllWalls( map[vy], viewWidth ) )
        {
            continue;
        }

        if( !firstRow )
        {
            offset += sprintf( buffer + offset, "," );
        }
        firstRow = false;

        // Output row index and RLE-encoded content
        offset += sprintf( buffer + offset, "[%d,\"", vy );

        char rleBuffer[256];
        int rleLen = AIRender_RLEEncodeRow( map[vy], viewWidth, rleBuffer );
        memcpy( buffer + offset, rleBuffer, rleLen );
        offset += rleLen;

        offset += sprintf( buffer + offset, "\"]" );
    }
    offset += sprintf( buffer + offset, "]" );

    // Add room/hallway counts if available
    if( m_dmCurLevel )
    {
        offset += sprintf( buffer + offset, ",\"rooms\":%d", m_dmCurLevel->HowManyRooms() );
        offset += sprintf( buffer + offset, ",\"hallways\":%d", m_dmCurLevel->HowManyHallways() );
    }

    // Add player info
    if( player )
    {
        offset += sprintf( buffer + offset, ",\"player\":{\"x\":%d,\"y\":%d,\"hp\":%d,\"max_hp\":%d}",
                           (int)player->m_vPos.x, (int)player->m_vPos.y,
                           (int)player->GetHP(), (int)player->GetMaxHP() );
    }

    // Add monster list
    offset += sprintf( buffer + offset, ",\"monsters\":[" );
    bool firstMon = true;
    if( m_llMonsters )
    {
        CLink<CMonster> *pCur = m_llMonsters->GetHead();
        while( pCur )
        {
            CMonster *mon = pCur->m_lpData;
            if( mon )
            {
                if( !firstMon )
                {
                    offset += sprintf( buffer + offset, "," );
                }
                firstMon = false;
                offset += sprintf( buffer + offset,
                                   "{\"name\":\"%s\",\"char\":\"%c\",\"x\":%d,\"y\":%d,\"hp\":%d}",
                                   mon->GetName(), mon->GetChar(),
                                   (int)mon->GetPos().x, (int)mon->GetPos().y,
                                   (int)mon->m_fCurHP );
            }
            pCur = m_llMonsters->GetNext( pCur );
        }
    }
    offset += sprintf( buffer + offset, "]" );

    // Add item list
    offset += sprintf( buffer + offset, ",\"items\":[" );
    bool firstItem = true;
    if( m_llItems )
    {
        CLink<CItem> *pCur = m_llItems->GetHead();
        while( pCur )
        {
            CItem *item = pCur->m_lpData;
            if( item )
            {
                if( !firstItem )
                {
                    offset += sprintf( buffer + offset, "," );
                }
                firstItem = false;
                offset += sprintf( buffer + offset,
                                   "{\"name\":\"%s\",\"char\":\"%c\",\"x\":%d,\"y\":%d}",
                                   item->GetName(), item->GetChar(),
                                   (int)item->m_vPos.x, (int)item->m_vPos.y );
            }
            pCur = m_llItems->GetNext( pCur );
        }
    }
    offset += sprintf( buffer + offset, "]}" );

    AILog_Write( buffer );
}
