//
// DisplayText.cpp
//
// Definitions of classes needed to display text in an OpenGL window
//
// Create multiple instances to make multiple text areas
// OpenGL stack-safe
// Scrolling, Text color, background, bounding box, etc.

#include "DisplayText.h"
#include "EndGameState.h"
#include "Item.h"
#include "JMDefs.h"
#include "RenderBase.h"

class CGame;

#define FONT_DRAW_W 6
#define FONT_DRAW_H 8

// 12288 = ~12KB, enough for the 100x100 dungeon map on the death screen
// (worst case ~100 rows * ~102 chars/row ≈ 10200, plus header/margin)
#define TEXT_MAXCHARS 12288
// Constructor
CDisplayText::CDisplayText( const char *szBasedir, JRect in, uint8 inAlpha )
    : m_Rect( in ),
      m_dwWidth( in.Width() ),
      m_dwHeight( in.Height() ),
      m_dwUsedLines( m_dwHeight / FONT_DRAW_H ),
      m_dwFreeLines( 0 ),
      m_szText( NULL ),
      m_szDrawPtr( NULL ),
      m_vPos( m_Rect.Left(), m_Rect.Top() ),
      m_dwFlags( FLAG_TEXT_NONE ),
      m_dwMarginLeft( 0 ),
      m_dwMarginTop( 0 ),
      m_rcViewport( 0, 480, 640, 0 )
{
    m_szText = new char[TEXT_MAXCHARS];
    memset( m_szText, 0, sizeof( *m_szText ) );
    m_szDrawPtr = m_szText;

    m_TileSet = new CTileset( szBasedir, "Resources/SmallText6X8.png", 6, 8 );

    m_Color.SetColor( 0, 0, 0, 255 );
    m_BoundingBoxColor.SetColor( 128, 170, 192, inAlpha );
}

CDisplayText::~CDisplayText()
{
    if( m_szText )
    {
        delete[] m_szText;
        m_szText = NULL;
    }
    if( m_TileSet )
    {
        delete m_TileSet;
        m_TileSet = NULL;
    }
}

bool CDisplayText::Update( float fCurTime ) { return true; }

void CDisplayText::SetRect( JRect in )
{
    m_Rect = in;
    m_dwWidth = in.Width();
    m_dwHeight = in.Height();
    m_dwUsedLines = m_dwHeight / FONT_DRAW_H;
    m_vPos.Init( m_Rect.Left(), m_Rect.Top() );
}

void CDisplayText::SetContentMargin( int left, int top )
{
    m_dwMarginLeft = left;
    m_dwMarginTop = top;
}

// Setup functions
void CDisplayText::PreDraw()
{
    bool bInverse;

    if( m_dwFlags & FLAG_TEXT_INVERSE )
    {
        bInverse = true;
    }
    else
    {
        bInverse = false;
    }
#ifdef RENDER_TILESET_POSTLOAD_NEEDED
    g_pGame->GetRender()->PreDrawObjects( m_rcViewport, m_TileSet->Texture(), false, bInverse );
#else
    g_pGame->GetRender()->PreDrawObjects( m_rcViewport, 0, false, bInverse );
#endif
}

void CDisplayText::PostDraw() { g_pGame->GetRender()->PostDrawObjects(); }

// Unformatted Text Drawing functions
void CDisplayText::Draw()
{
    Paginate();
    PreDraw();
    DrawBoundingBox();

    // In ASCII mode, text must be inset so it doesn't collide with box-drawing chars.
    // Also clamp bottom to the renderer's actual screen height so text doesn't
    // overflow past the bottom border.
    int inset = g_pGame->GetRender()->GetTextInset();
    int insetX = inset * FONT_DRAW_W;
    int insetY = inset * FONT_DRAW_H;
    int maxH = g_pGame->GetRender()->GetMaxTextHeight();
    int drawBottom = m_Rect.Bottom() - insetY;
    if( drawBottom > maxH - insetY )
        drawBottom = maxH - insetY;
    DrawStr( m_Rect.Left() + insetX + m_dwMarginLeft, m_Rect.Top() + insetY + m_dwMarginTop, true,
             drawBottom, m_szDrawPtr );
    PostDraw();
}

void CDisplayText::DrawStr( int x, int y, const char *szString )
{
    PreDraw();
    DrawStr( x, y, false, 0, szString );
    PostDraw();
}

void CDisplayText::DrawStr( int x, int y, bool bBoundsCheck, int dwYMax, const char *szString )
{
    JLog( LOG_LEVEL_NOISE, true, "Trying to draw string: %s\n", szString );
    JVector vScreen( (float)x, (float)y );
    JVector vSize( (float)FONT_DRAW_W, (float)FONT_DRAW_H );
    const char *ptr = szString;

    m_TileSet->PreDrawTile();
    m_TileSet->SetTileColor( m_Color );

    // Do that string parse, baby
    while( *ptr != nul )
    {
        if( *ptr == ' ' )
        {
            vScreen.x += FONT_DRAW_W;
        }
        else if( *ptr > ' ' && *ptr <= '~' )
        {
            m_TileSet->DrawChar( *ptr, vScreen, vSize );

            vScreen.x += FONT_DRAW_W;
        }

        if( *ptr == '\n' )
        {
            vScreen.Init( (float)x, (float)( vScreen.y + FONT_DRAW_H ) );

            // Don't scroll off the bottom of the bounding box
            if( bBoundsCheck && vScreen.y > dwYMax )
            {
                break;
            }
        }

        ptr++;
    }

    m_TileSet->PostDrawTile();
}

void CDisplayText::DrawBoundingBox()
{

    if( m_dwFlags & FLAG_TEXT_BOUNDING_BOX )
    {
        g_pGame->GetRender()->DrawTextBoundingBox( m_Rect, m_BoundingBoxColor );
    }
    else
    {
        g_pGame->GetRender()->DrawTextBoundingBox( m_Rect, JColor( 51, 128, 102, 255 ) );
    }
}

void CDisplayText::Paginate()
{
    int dwAddLines = 0;
    int dwAddLinesMax;
    char *ptr;

    // Account for text inset reducing available lines, and clamp to
    // the renderer's actual screen height (e.g. 24 rows in ASCII mode
    // vs the 60-row pixel-rect for the endgame/intro panel).
    int inset = g_pGame->GetRender()->GetTextInset();
    int maxLines = g_pGame->GetRender()->GetMaxTextHeight() / FONT_DRAW_H;
    int usedLines = m_dwUsedLines;
    if( usedLines > maxLines )
        usedLines = maxLines;
    usedLines -= ( 2 * inset );
    if( usedLines < 1 )
        usedLines = 1;

    dwAddLinesMax = usedLines + m_dwFreeLines;

    ptr = strchr( m_szText, nul );
    while( ptr > m_szText )
    {
        if( *ptr == '\n' )
        {
            dwAddLines++;
        }
        if( dwAddLines == dwAddLinesMax )
        {
            // ran out of room; sorry.
            ptr++;
            break;
        }
        ptr--;
    }

    if( ptr == m_szText && dwAddLines > usedLines )
    {
        m_dwFreeLines = dwAddLines - usedLines + 1;
    }

    m_szDrawPtr = ptr;
}

// Formatted Text Drawing Functions
void CDisplayText::Printf( const char *fmt, ... )
{
    va_list vList;
    char szBuffer3[TEXT_MAXCHARS];

    // Get resultant output string
    va_start( vList, fmt );
    vsprintf( szBuffer3, fmt, vList );

    DrawFormattedStr( szBuffer3 );

    va_end( vList );
}

void CDisplayText::DrawFormattedStr( const char *szString )
{
    char szBuffer[TEXT_MAXCHARS];
    char szBuffer2[TEXT_MAXCHARS];
    char *ptr;
    char *ptr2;

    // Account for text inset so wrapping uses the interior width.
    // Also clamp to the renderer's max text width so lines that exceed
    // the terminal width get wrapped instead of silently clipped.
    int inset = g_pGame->GetRender()->GetTextInset();
    int insetX = inset * FONT_DRAW_W;
    int maxW = g_pGame->GetRender()->GetMaxTextWidth();
    int wrapLeft = m_Rect.Left() + insetX;
    int wrapRight = m_Rect.Right() - insetX;
    if( wrapRight > maxW - insetX )
        wrapRight = maxW - insetX;
    JIVector vPos( wrapLeft, m_Rect.Top() );

    Util::jstrcpy( szBuffer, szString );
    ptr = szBuffer;

    while( *ptr )
    {
        if( *ptr >= ' ' && *ptr <= '~' )
        {
            vPos.x += FONT_DRAW_W;
        }
        else if( *ptr == '\n' )
        {
            vPos.x = wrapLeft;
        }

        if( vPos.x > wrapRight )
        {
            ptr++;

            if( m_dwFlags & FLAG_TEXT_WRAP_WHITESPACE )
            {
                // Find the last space
                while( ptr > szBuffer && *ptr != ' ' )
                {
                    ptr--;
                }
            }

            Util::jstrcpy( szBuffer2, ptr );
            *ptr++ = '\n';
            *ptr = nul;

            // Kill the leading space (if necessary)
            if( m_dwFlags & FLAG_TEXT_WRAP_WHITESPACE && szBuffer2[0] == ' ' )
            {
                ptr2 = &szBuffer2[1];
            }
            else
            {
                ptr2 = szBuffer2;
            }

            // Guard against buffer overflow: each wrap adds a '\n' byte,
            // so the total can exceed TEXT_MAXCHARS. Truncate if needed.
            int used = (int)( ptr - szBuffer );
            int remain = Util::jstrlen( ptr2 );
            if( used + remain < TEXT_MAXCHARS )
            {
                Util::jstrcat( ptr, ptr2 );
            }
            else
            {
                // Truncate: copy as much as fits
                int avail = TEXT_MAXCHARS - used - 1;
                if( avail > 0 )
                {
                    memcpy( ptr, ptr2, avail );
                    ptr[avail] = nul;
                }
                else
                {
                    *ptr = nul;
                }
            }
            vPos.x = wrapLeft;
        }
        else
        {
            ptr++;
        }
    }

    // Make sure it's going to fit in the bounding box by removing
    // strings at the top of the bounding box
    ptr = m_szText;
    while( Util::jstrlen( ptr ) + Util::jstrlen( szBuffer ) > TEXT_MAXCHARS )
    {
        ptr = strchr( ptr, '\n' );
        if( !ptr )
        {
            // Can't free enough room — truncate szBuffer to fit
            int avail = TEXT_MAXCHARS - Util::jstrlen( m_szText ) - 1;
            if( avail < 0 )
                avail = 0;
            szBuffer[avail] = nul;
            ptr = m_szText;
            break;
        }

        ptr++;
    }

    // If there are strings that need to be removed, do that now.
    if( ptr != m_szText )
    {
        Util::jstrcpy( szBuffer2, ptr );
        Util::jstrcpy( m_szText, szBuffer2 );
    }

    // Now that we have the space, concatenate the new string.
    Util::jstrcat( m_szText, szBuffer );
}

#define SHOW_EMPTY
void CDisplayText::DisplayFixedList( JLinkList<CItem> *pList, const CDisplayMeta *pMeta,
                                     const uint8 dwIndex )
{
    // TODO: Use dwIndex to filter the
    CLink<CItem> *pLink = pList->GetHead();
    Clear();
    Printf( pMeta->header );
    char cListId = 'a';

    while( cListId <= pMeta->limit )
    {
        if( pLink != NULL && pLink->m_dwIndex == cListId - 'a' )
        {
            Printf( "%c - %s\n", cListId, pLink->m_lpData->GetName() );
            pLink = pList->GetNext( pLink );
        }
#ifdef SHOW_EMPTY
        else
        {

            Printf( "%c - (None)\n", cListId );
        }
#endif // SHOW_EMPTY
        cListId++;
    }
}

void CDisplayText::DisplayList( JLinkList<CItem> *pList, const CDisplayMeta *pMeta,
                                const uint8 dwIndex )
{
    // TODO: Use dwIndex to filter the
    CLink<CItem> *pLink = pList->GetHead();
    CItem *pItem;
    Clear();
    Printf( pMeta->header );
    char cListId = 'a';
    while( pLink != NULL )
    {
        pItem = pLink->m_lpData;
        if( pItem->IsStackable() && pItem->m_dwCount > 1 )
        {
            Printf( "%c - %d %s\n", cListId, pItem->m_dwCount, pItem->GetPlural() );
        }
        else
        {
            Printf( "%c - %s\n", cListId, pItem->GetName() );
        }

        if( cListId < pMeta->limit )
        {
            cListId++;
        }
        else
        {
            Printf( pMeta->footer );
            break;
        }
        pLink = pList->GetNext( pLink );
    }
}
void CDisplayText::DisplayList( JLinkList<CScore> *pList, const CDisplayMeta *pMeta,
                                const uint8 dwIndex )
{
    // TODO: Use dwIndex to filter the
    CLink<CScore> *pLink = pList->GetHead();
    CScore *pItem;
    Clear();
    Printf( pMeta->header );
    char cListId = 1;
    while( pLink != NULL )
    {
        pItem = pLink->m_lpData;
        Printf( "%d - %s\n", cListId, pItem->GetName() );

        if( cListId < pMeta->limit )
        {
            cListId++;
        }
        else
        {
            Printf( pMeta->footer );
            break;
        }
        pLink = pList->GetNext( pLink );
    }
}
