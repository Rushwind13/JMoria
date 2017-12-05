// 
// DisplayText.h
// 
// Declarations of classes needed to display text in an OpenGL window
//
// Create multiple instances to make multiple text areas
// OpenGL stack-safe
// Scrolling, Text color, background, bounding box, etc.
#ifndef __DISPLAYTEXT_H__
#define __DISPLAYTEXT_H__

#include "JMDefs.h"
#ifdef __WIN32__
#include "windef.h"
#endif //__WIN32__
#include "TileSet.h"

#define FLAG_TEXT_NONE				0x0
#define FLAG_TEXT_BOUNDING_BOX		0x1
#define FLAG_TEXT_INVERSE			0x2
#define FLAG_TEXT_IGNORE_WHITESPACE	0x4
#define FLAG_TEXT_WRAP_WHITESPACE	0x8

class CDisplayText
{
	// Member functions
public:
	CDisplayText( JRect in );

	bool Update( float fCurTime );
	void Draw();
	void DrawStr( int x, int y, char *szString );
	void Printf( const char *fmt, ... );

	void Clear() { m_szText[0] = nul; };
	void SetColor( JColor in ) { m_Color.SetColor( in ); };
	void SetFlags( int dwFlags ) { m_dwFlags = dwFlags; };

	void PageUp()		{ m_dwFreeLines += m_dwUsedLines; };
	void PageDown()		{ m_dwFreeLines -= m_dwUsedLines; if( m_dwFreeLines < 0 ) m_dwFreeLines = 0; };
	void PageReset()	{ m_dwFreeLines = 0; };


protected:
	void PreDraw();
	void PostDraw();

	void Paginate();
	void DrawStr( int x, int y, bool bBoundsCheck, int dwYMax, char *szString );
	void DrawBoundingBox();
	void DrawFormattedStr( const char *str );
private:

	// Member variables
public:
protected:
private:
	// Bounding box
	JRect m_Rect;
	JRect m_rcViewport;
	int m_dwWidth;
	int m_dwHeight;

	// Text
	char *m_szText;
	char *m_szDrawPtr;

	// Auxiliary
	int	m_dwUsedLines;
	int m_dwFreeLines;
	JIVector m_vPos;
	JColor	m_Color;
	int m_dwFlags;

	// The font
	CTileset *m_TileSet;
};

#endif // __DISPLAYTEXT_H__