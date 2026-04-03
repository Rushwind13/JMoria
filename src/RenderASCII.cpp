// RenderASCII.cpp
//
// ncurses-based ASCII renderer for JMoria
//

#include "RenderASCII.h"
#include <cstring>

// ---- ASCIILayout factory methods ----

// 80x24 layout:
// Row 0-1:   Messages (80 wide, 2 tall)
// Row 2-21:  Stats(12) | Dungeon(50) | Inventory(18)
// Row 2-11:  (inventory top half)
// Row 12-21: (equipment bottom half)
// Row 22-23: Status line
ASCIILayout ASCIILayout::Create80x24()
{
    ASCIILayout l;
    l.termWidth = 80;
    l.termHeight = 24;

    l.messages  = { 0, 0, 80, 5 };
    l.dungeon   = { 0, 5, 80, 24 };
    // Fly-out panels: drawn over the dungeon when toggled
    l.stats     = { 0, 5, 25, 24 };
    l.inventory = { 55, 5, 80, 15 };
    l.equipment = { 55, 15, 80, 24 };
    l.use       = { 20, 5, 60, 24 };
    l.endgame   = { 0, 0, 80, 24 };

    return l;
}

// 125x40 layout: more room for everything
ASCIILayout ASCIILayout::Create125x40()
{
    ASCIILayout l;
    l.termWidth = 125;
    l.termHeight = 40;

    l.messages  = { 0, 0, 125, 3 };
    l.stats     = { 0, 3, 20, 38 };
    l.dungeon   = { 20, 3, 95, 38 };
    l.inventory = { 95, 3, 125, 20 };
    l.equipment = { 95, 20, 125, 38 };
    l.use       = { 30, 5, 90, 38 };
    l.endgame   = { 0, 0, 125, 40 };

    return l;
}

// ---- CRenderASCII ----

CRenderASCII::CRenderASCII()
    : m_bInitted( false ),
      m_bHasColor( false ),
      m_bTranslating( false )
{
    m_currentColor.SetColor( 255, 255, 255, 255 );
    memset( &m_currentBounds, 0, sizeof( m_currentBounds ) );
    m_currentTranslate.Init( 0.0f, 0.0f );
}

JResult CRenderASCII::Init( int width, int height, int bpp )
{
    // Pick layout based on requested size
    if( width >= 125 && height >= 40 )
        m_layout = ASCIILayout::Create125x40();
    else
        m_layout = ASCIILayout::Create80x24();

    // Initialize ncurses
    initscr();
    raw();
    keypad( stdscr, TRUE );
    noecho();
    curs_set( 0 ); // hide cursor
    nodelay( stdscr, TRUE ); // non-blocking getch
    timeout( 0 );

    InitColors();

    m_bInitted = true;
    return JSUCCESS;
}

void CRenderASCII::Term()
{
    if( m_bInitted )
    {
        endwin();
        m_bInitted = false;
    }
}

void CRenderASCII::InitColors()
{
    if( !has_colors() )
    {
        m_bHasColor = false;
        return;
    }

    start_color();
    m_bHasColor = true;

    // Define color pairs: pair_id = foreground color index + 1
    // Pair 0 is reserved by ncurses (white on black)
    init_pair( 1, COLOR_WHITE, COLOR_BLACK );
    init_pair( 2, COLOR_RED, COLOR_BLACK );
    init_pair( 3, COLOR_GREEN, COLOR_BLACK );
    init_pair( 4, COLOR_YELLOW, COLOR_BLACK );
    init_pair( 5, COLOR_BLUE, COLOR_BLACK );
    init_pair( 6, COLOR_MAGENTA, COLOR_BLACK );
    init_pair( 7, COLOR_CYAN, COLOR_BLACK );
    init_pair( 8, COLOR_BLACK, COLOR_BLACK ); // dark/invisible
}

int CRenderASCII::GetColorPair( JColor color )
{
    if( !m_bHasColor )
        return 0;

    Uint8 r, g, b, a;
    color.GetColor( r, g, b, a );

    // Map RGBA to nearest ncurses color pair
    // Very dark = pair 8 (black on black)
    if( r < 40 && g < 40 && b < 40 )
        return 8;

    // Find the dominant channel
    if( r > g && r > b )
    {
        if( g > 128 ) return 4; // yellow (red + green)
        if( b > 128 ) return 6; // magenta (red + blue)
        return 2;               // red
    }
    if( g > r && g > b )
    {
        if( b > 128 ) return 7; // cyan (green + blue)
        return 3;               // green
    }
    if( b > r && b > g )
    {
        if( r > 128 ) return 6; // magenta
        return 5;               // blue
    }

    // Gray/white — anything not already caught as "very dark" above
    // should be visible.  Dark grays (e.g. walls at 64,64,64) need to
    // map to white since the terminal background is black.
    return 1; // white
}

char CRenderASCII::TileIndexToChar( int tileIndex )
{
    // The font tileset maps chars starting from ' '+1 = '!'
    // tileIndex 0 = '!', 1 = '"', etc.
    char ch = (char)( tileIndex + ' ' + 1 );
    if( ch < ' ' || ch > '~' )
        ch = '?';
    return ch;
}

// ---- Frame management ----

void CRenderASCII::PreDraw()
{
    erase(); // clear the virtual screen
}

void CRenderASCII::PostDraw()
{
    SwapBuffers();
}

void CRenderASCII::SwapBuffers()
{
    refresh(); // push virtual screen to terminal
}

// ---- Object/region drawing ----

// PreDrawObjects sets up the coordinate mapping context.
// In OpenGL, this pushes a projection matrix. For ASCII, we just
// record the bounds so MapX/MapY can convert positions.
void CRenderASCII::PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate,
                                   bool bInverse, JFVector *vTranslate )
{
    m_currentBounds = rcBounds;
    m_bTranslating = bTranslate;
    if( vTranslate )
        m_currentTranslate = *vTranslate;
    else
        m_currentTranslate.Init( 0.0f, 0.0f );
}

void CRenderASCII::PostDrawObjects()
{
    // Nothing to tear down
}

// ---- Coordinate mapping ----

// Determine which layout region we're drawing into based on the current bounds.
// Dungeon uses world-space bounds (e.g. left=-20..right=+20 centered on player).
// DisplayText uses pixel-space bounds (e.g. 0,480,640,0 viewport).
//
// We detect the context by checking if bounds look like pixel coords (large values)
// vs world coords (small values around dungeon size).

int CRenderASCII::MapX( float worldX )
{
    // Apply translation if active
    float x = worldX;
    if( m_bTranslating )
        x += m_currentTranslate.x;

    // Detect context: DisplayText uses viewport (0, 480, 640, 0)
    // where left < right and top > bottom (OpenGL screen coords).
    // Dungeon uses world coords where values are in dungeon tile range.
    bool isPixelSpace = ( m_currentBounds.left >= 0 && m_currentBounds.right > 500 );

    if( isPixelSpace )
    {
        // Pixel space (DisplayText): map 0..640 to terminal char X
        return (int)( x / 6.0f );
    }
    else
    {
        // World space (Dungeon): 1:1 tile-to-char mapping centered in viewport
        float boundsLeft = (float)m_currentBounds.left;
        float boundsRight = (float)m_currentBounds.right;
        float boundsCenter = ( boundsLeft + boundsRight ) / 2.0f;
        int viewCenter = m_layout.dungeon.left + m_layout.dungeon.Width() / 2;
        return viewCenter + (int)( x - boundsCenter + 0.5f );
    }
}

int CRenderASCII::MapY( float worldY )
{
    float y = worldY;
    if( m_bTranslating )
        y += m_currentTranslate.y;

    bool isPixelSpace = ( m_currentBounds.left >= 0 && m_currentBounds.right > 500 );

    if( isPixelSpace )
    {
        // Pixel space: map 0..480 to terminal row
        return (int)( y / 8.0f );
    }
    else
    {
        // World space (Dungeon): 1:1 tile-to-char mapping centered in viewport.
        // JRect: Init(xo-zoom, yo+zoom, xo+zoom, yo-zoom)
        //   top=yo+zoom, bottom=yo-zoom → center = (top+bottom)/2 = yo
        // OpenGL: glOrtho(VIEWRECT_EXPAND) feeds top as "bottom" param,
        //   so higher world Y → bottom of screen → higher terminal row.
        float boundsTop = (float)m_currentBounds.top;
        float boundsBottom = (float)m_currentBounds.bottom;
        float boundsCenter = ( boundsTop + boundsBottom ) / 2.0f;
        int viewCenter = m_layout.dungeon.top + m_layout.dungeon.Height() / 2;
        return viewCenter + (int)( y - boundsCenter + 0.5f );
    }
}

// ---- Tile drawing ----

void CRenderASCII::PreDrawTile()
{
    // Nothing to batch-start in ncurses
}

void CRenderASCII::PostDrawTile()
{
    // Nothing to batch-end in ncurses
}

void CRenderASCII::SetTileColor( JColor color )
{
    // In OpenGL, black text sits on a translucent colored quad and is readable.
    // In the terminal the background is black, so remap black text to white.
    Uint8 r, g, b, a;
    color.GetColor( r, g, b, a );
    if( r < 40 && g < 40 && b < 40 )
        m_currentColor.SetColor( 255, 255, 255, a );
    else
        m_currentColor = color;
}

bool CRenderASCII::DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile,
                             JFVector &vTexels )
{
    // vTile is the position on the font texture in tile units.
    // Convert tile index back to ASCII char.
    // The tileset stores tiles in a grid; the index into the ASCII range is:
    //   index = vTile.y * tilesPerRow + vTile.x
    // For the 6x8 font on a typical texture, tilesPerRow varies,
    // but the original code does: index = charValue - ' ' - 1
    // and GetTile does: vTile.x = index % tilesPerRow, vTile.y = index / tilesPerRow
    // We can recover the linear index and convert back.
    // tilesPerRow = textureWidth / cellWidth. For SmallText6X8.png (96px wide, 6px cells) = 16
    int tilesPerRow = (int)( 1.0f / vTexels.x + 0.5f );
    int tileIndex = vTile.y * tilesPerRow + vTile.x;

    char ch = TileIndexToChar( tileIndex );

    int screenX = MapX( vPos.x );
    int screenY = MapY( vPos.y );

    // Clip to the dungeon region when drawing world-space tiles
    bool isPixelSpace = ( m_currentBounds.left >= 0 && m_currentBounds.right > 500 );
    if( !isPixelSpace )
    {
        if( screenX < m_layout.dungeon.left || screenX >= m_layout.dungeon.right ||
            screenY < m_layout.dungeon.top  || screenY >= m_layout.dungeon.bottom )
            return false;
    }
    else
    {
        if( screenX < 0 || screenX >= m_layout.termWidth ||
            screenY < 0 || screenY >= m_layout.termHeight )
            return false;
    }

    int pair = GetColorPair( m_currentColor );
    if( m_bHasColor )
        attron( COLOR_PAIR( pair ) );

    mvaddch( screenY, screenX, ch );

    if( m_bHasColor )
        attroff( COLOR_PAIR( pair ) );

    return true;
}

bool CRenderASCII::DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile )
{
    // Untextured tile — draw a solid block
    int screenX = MapX( vPos.x );
    int screenY = MapY( vPos.y );

    bool isPixelSpace = ( m_currentBounds.left >= 0 && m_currentBounds.right > 500 );
    if( !isPixelSpace )
    {
        if( screenX < m_layout.dungeon.left || screenX >= m_layout.dungeon.right ||
            screenY < m_layout.dungeon.top  || screenY >= m_layout.dungeon.bottom )
            return false;
    }
    else
    {
        if( screenX < 0 || screenX >= m_layout.termWidth ||
            screenY < 0 || screenY >= m_layout.termHeight )
            return false;
    }

    int pair = GetColorPair( m_currentColor );
    if( m_bHasColor )
        attron( COLOR_PAIR( pair ) );

    mvaddch( screenY, screenX, '#' );

    if( m_bHasColor )
        attroff( COLOR_PAIR( pair ) );

    return true;
}

// ---- UI drawing ----

void CRenderASCII::DrawTextBoundingBox( JRect rect, JColor color )
{
    // Convert pixel rect to terminal char coords
    int left = rect.left / 6;
    int top = rect.top / 8;
    int right = ( rect.right / 6 ) - 1;
    int bottom = ( rect.bottom / 8 ) - 1;

    // Clamp to screen
    if( left < 0 ) left = 0;
    if( top < 0 ) top = 0;
    if( right >= m_layout.termWidth ) right = m_layout.termWidth - 1;
    if( bottom >= m_layout.termHeight ) bottom = m_layout.termHeight - 1;
    if( right <= left || bottom <= top ) return;

    int pair = GetColorPair( color );
    if( m_bHasColor )
        attron( COLOR_PAIR( pair ) );

    // Draw box using ACS characters
    mvaddch( top, left, ACS_ULCORNER );
    mvaddch( top, right, ACS_URCORNER );
    mvaddch( bottom, left, ACS_LLCORNER );
    mvaddch( bottom, right, ACS_LRCORNER );

    for( int x = left + 1; x < right; x++ )
    {
        mvaddch( top, x, ACS_HLINE );
        mvaddch( bottom, x, ACS_HLINE );
    }
    for( int y = top + 1; y < bottom; y++ )
    {
        mvaddch( y, left, ACS_VLINE );
        mvaddch( y, right, ACS_VLINE );
    }

    if( m_bHasColor )
        attroff( COLOR_PAIR( pair ) );
}
