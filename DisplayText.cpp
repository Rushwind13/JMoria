//
// DisplayText.cpp
//
// Definitions of classes needed to display text in an OpenGL window
//
// Create multiple instances to make multiple text areas
// OpenGL stack-safe
// Scrolling, Text color, background, bounding box, etc.

#include "JMDefs.h"
#include "DisplayText.h"
#include "Render.h"
#include "Item.h"
#include "EndGameState.h"

class CGame;

#define FONT_DRAW_W	8
#define FONT_DRAW_H	10

#define TEXT_MAXCHARS 2048
// Constructor
CDisplayText::CDisplayText( JRect in, uint8 inAlpha )
:	m_Rect(in),
m_dwWidth(in.Width()),
m_dwHeight( in.Height() ),
m_dwUsedLines( m_dwHeight / FONT_DRAW_H ),
m_dwFreeLines( 0 ),
m_szText( NULL ),
m_szDrawPtr( NULL ),
m_vPos( m_Rect.Left(), m_Rect.Top() ),
m_dwFlags( FLAG_TEXT_NONE ),
m_cBoundingBoxAlpha(inAlpha),
m_rcViewport( 0, 480, 640, 0 )
{
	m_szText = new char[TEXT_MAXCHARS];
	memset( m_szText, 0, sizeof( *m_szText ) );
	m_szDrawPtr = m_szText;

	m_TileSet = new CTileset("Resources/Courier.png", 32, 32);

	m_Color.SetColor( 0, 0, 0, 255 );
}

bool CDisplayText::Update( float fCurTime )
{
	return true;
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
	g_pGame->GetRender()->PreDrawObjects( m_rcViewport, m_TileSet->Texture(), false, bInverse);
}

void CDisplayText::PostDraw()
{
	g_pGame->GetRender()->PostDrawObjects();
}

// Unformatted Text Drawing functions
void CDisplayText::Draw()
{
	Paginate();
	PreDraw();
		if( m_dwFlags & FLAG_TEXT_BOUNDING_BOX)
		{
			DrawBoundingBox();
		}
		DrawStr( m_Rect.Left(), m_Rect.Top(), true, m_Rect.Bottom(), m_szDrawPtr );
	PostDraw();
}

void CDisplayText::DrawStr( int x, int y, char *szString )
{
	PreDraw();
		DrawStr( x, y, false, 0, szString );
	PostDraw();
}

void CDisplayText::DrawStr(int x, int y, bool bBoundsCheck, int dwYMax, char *szString )
{
	//printf("Trying to draw string: %s\n", szString);
	JVector vScreen( (float)x, (float)y );
	JVector vSize( (float)FONT_DRAW_W, (float)FONT_DRAW_H );
	char *ptr = szString;
	int index;

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
			index = *ptr - ' ' - 1;

			m_TileSet->DrawTile( index, vScreen, vSize, true );
			
			vScreen.x += FONT_DRAW_W;
		}

		if( *ptr == '\n' )
		{
			vScreen.Init( (float)x, (float)(vScreen.y + FONT_DRAW_H) );

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
	g_pGame->GetRender()->DrawTextBoundingBox( m_Rect, m_cBoundingBoxAlpha );
}

void CDisplayText::Paginate()
{
	int dwAddLines = 0;
	int dwAddLinesMax;
	char *ptr;

	dwAddLinesMax = m_dwUsedLines + m_dwFreeLines;

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

	if( ptr == m_szText && dwAddLines > m_dwUsedLines )
	{
		m_dwFreeLines = dwAddLines - m_dwUsedLines + 1;
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

	DrawFormattedStr(szBuffer3);

	va_end(vList);
}

void CDisplayText::DrawFormattedStr(const char *szString)
{
	char szBuffer[TEXT_MAXCHARS];
	char szBuffer2[TEXT_MAXCHARS];
	char *ptr;
	char *ptr2;
	JIVector vPos(m_Rect.Left(), m_Rect.Top());

	strcpy( szBuffer, szString );
	ptr = szBuffer;

	while( *ptr )
	{
		if( *ptr >= ' ' && *ptr <= '~' )
		{
			vPos.x += FONT_DRAW_W;
		}
		else if( *ptr == '\n' )
		{
			vPos.x = m_Rect.Left();
		}

		if( vPos.x > m_Rect.Right() )
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

			strcpy( szBuffer2, ptr );
			*ptr++	= '\n';
			*ptr	= nul;

			// Kill the leading space (if necessary)
			if( m_dwFlags & FLAG_TEXT_WRAP_WHITESPACE && szBuffer2[0] == ' ' )
			{
				ptr2 = &szBuffer2[1];
			}
			else
			{
				ptr2 = szBuffer2;
			}

			strcat( ptr, ptr2 );
			vPos.x = m_Rect.Left();
		}
		else
		{
			ptr++;
		}
	}

	// Make sure it's going to fit in the bounding box by removing
	// strings at the top of the bounding box
	ptr = m_szText;
	while( strlen( ptr ) + strlen( szBuffer ) > TEXT_MAXCHARS )
	{
		ptr = strchr( ptr, '\n' );
		if( !ptr )
		{
			assert( !"CDisplayText::DrawFormattedString: string too long!" );
		}

		ptr++;
	}

	// If there are strings that need to be removed, do that now.
	if( ptr != m_szText )
	{
		strcpy( szBuffer2, ptr );
		strcpy( m_szText, szBuffer2 );
	}

	// Now that we have the space, concatenate the new string.
	strcat( m_szText, szBuffer );
}

void CDisplayText::DisplayList( JLinkList<CItem> *pList, const CDisplayMeta *pMeta, const uint8 dwIndex )
{
    // TODO: Use dwIndex to filter the
    CLink<CItem> *pLink = pList->GetHead();
    CItem *pItem;
    Clear();
    Printf(pMeta->header);
    char cListId = 'a';
    while(pLink != NULL)
    {
        pItem = pLink->m_lpData;
        Printf("%c - %s\n", cListId, pItem->GetName());
        
        if( cListId < pMeta->limit )
        {
            cListId++;
        }
        else
        {
            printf(pMeta->footer);
            break;
        }
        pLink = pList->GetNext(pLink);
    }
    
}
void CDisplayText::DisplayList( JLinkList<CScore> *pList, const CDisplayMeta *pMeta, const uint8 dwIndex )
{
    // TODO: Use dwIndex to filter the
    CLink<CScore> *pLink = pList->GetHead();
    CScore *pItem;
    Clear();
    Printf(pMeta->header);
    char cListId = 1;
    while(pLink != NULL)
    {
        pItem = pLink->m_lpData;
        Printf("%d - %s\n", cListId, pItem->GetName());
        
        if( cListId < pMeta->limit )
        {
            cListId++;
        }
        else
        {
            printf(pMeta->footer);
            break;
        }
        pLink = pList->GetNext(pLink);
    }
    
}
