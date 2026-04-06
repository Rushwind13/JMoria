// RenderASCII.cpp
//
// ncurses-based ASCII renderer for JMoria
//
#include "JMDefs.h"
#ifdef RENDER_ASCII

#include "RenderASCII.h"
#include <cmath>
#include <cstring>

// ---- ASCIILayout factory ----

// Dynamically compute layout for any terminal size.
// Stats panel always on the left, dungeon fills the middle,
// inventory/equipment on the right only when terminal is wide enough.
ASCIILayout ASCIILayout::CreateForSize( int w, int h )
{
    ASCIILayout l;
    l.termWidth = w;
    l.termHeight = h;

    int msgH = MSG_HEIGHT;
    if( h < 12 )
        msgH = 2; // very small terminal
    int bodyTop = msgH;
    int bodyBottom = h;

    l.messages = { 0, 0, w, msgH };
    l.endgame = { 0, 0, w, h };

    // Stats panel (always on left)
    int statsRight = STATS_WIDTH;
    if( statsRight > w / 3 )
        statsRight = w / 3; // don't take more than 1/3
    l.stats = { 0, bodyTop, statsRight, bodyBottom };

    // Inventory/equipment panel (right side, only when wide enough)
    int invLeft = w;
    if( w >= INV_AUTO_WIDTH )
    {
        invLeft = w - INV_WIDTH;
        int invMid = bodyTop + ( bodyBottom - bodyTop ) / 2;
        l.inventory = { invLeft, bodyTop, w, invMid };
        l.equipment = { invLeft, invMid, w, bodyBottom };
    }
    else
    {
        // Fly-out positions (drawn over dungeon when toggled)
        int flyRight = w;
        int flyLeft = w - INV_WIDTH;
        if( flyLeft < w / 2 )
            flyLeft = w / 2;
        int flyMid = bodyTop + ( bodyBottom - bodyTop ) / 2;
        l.inventory = { flyLeft, bodyTop, flyRight, flyMid };
        l.equipment = { flyLeft, flyMid, flyRight, bodyBottom };
    }

    // Dungeon fills the space between stats and inventory
    l.dungeon = { statsRight, bodyTop, invLeft, bodyBottom };

    // Use panel (center overlay)
    int useLeft = w / 4;
    int useRight = 3 * w / 4;
    l.use = { useLeft, bodyTop, useRight, bodyBottom };

    return l;
}

// ---- CRenderASCII ----

CRenderASCII::CRenderASCII() : m_bInitted( false ), m_bHasColor( false ), m_bTranslating( false )
{
    m_currentColor.SetColor( 255, 255, 255, 255 );
    memset( &m_currentBounds, 0, sizeof( m_currentBounds ) );
    m_currentTranslate.Init( 0.0f, 0.0f );
}

JResult CRenderASCII::Init( int width, int height, int bpp )
{
    // Initialize ncurses
    initscr();
    raw();
    keypad( stdscr, TRUE );
    noecho();
    curs_set( 0 );           // hide cursor
    nodelay( stdscr, TRUE ); // non-blocking getch
    timeout( 0 );

    // Use actual terminal size (ignore requested width/height)
    int termW, termH;
    getmaxyx( stdscr, termH, termW );
    m_layout = ASCIILayout::CreateForSize( termW, termH );

    InitColors();

    m_bInitted = true;
    return JSUCCESS;
}

bool CRenderASCII::CheckResize()
{
    int termW, termH;
    getmaxyx( stdscr, termH, termW );
    if( termW == m_layout.termWidth && termH == m_layout.termHeight )
        return false;

    m_layout = ASCIILayout::CreateForSize( termW, termH );
    return true;
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
        if( g > 128 )
            return 4; // yellow (red + green)
        if( b > 128 )
            return 6; // magenta (red + blue)
        return 2;     // red
    }
    if( g > r && g > b )
    {
        if( b > 128 )
            return 7; // cyan (green + blue)
        return 3;     // green
    }
    if( b > r && b > g )
    {
        if( r > 128 )
            return 6; // magenta
        return 5;     // blue
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

void CRenderASCII::PostDraw() { SwapBuffers(); }

void CRenderASCII::SwapBuffers()
{
    refresh(); // push virtual screen to terminal
}

// ---- Object/region drawing ----

// PreDrawObjects sets up the coordinate mapping context.
// In OpenGL, this pushes a projection matrix. For ASCII, we just
// record the bounds so MapX/MapY can convert positions.
void CRenderASCII::PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate, bool bInverse,
                                   JFVector *vTranslate )
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
        // World space (Dungeon): 1:1 tile-to-char mapping.
        // Convert world float coordinates to integer screen positions.
        // Use direct cast (truncation) rather than any rounding to avoid
        // items being placed outside viewport bounds due to rounding artifacts.
        int tileX = (int)x;
        // Stable center calculation: use floor division for consistent rounding
        // This prevents discontinuities when player crosses integer boundaries
        int boundSum = m_currentBounds.left + m_currentBounds.right;
        int centerX = boundSum / 2;
        if( boundSum < 0 && ( boundSum & 1 ) )
            centerX--; // Floor rounding for negative odd sums

        int viewCenter = m_layout.dungeon.left + m_layout.dungeon.Width() / 2;
        return viewCenter + ( tileX - centerX );
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
        // World space (Dungeon): 1:1 tile-to-char mapping.
        // Higher world Y → higher terminal row (matching OpenGL convention).
        // Use direct cast (truncation) rather than any rounding to avoid
        // items being placed outside viewport bounds due to rounding artifacts.
        int tileY = (int)y;
        // Stable center calculation: use floor division for consistent rounding
        // This prevents discontinuities when player crosses integer boundaries
        int boundSum = m_currentBounds.top + m_currentBounds.bottom;
        int centerY = boundSum / 2;
        if( boundSum < 0 && ( boundSum & 1 ) )
            centerY--; // Floor rounding for negative odd sums

        int viewCenter = m_layout.dungeon.top + m_layout.dungeon.Height() / 2;
        return viewCenter + ( tileY - centerY );
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
    // Legacy path: recover the character from tile grid coordinates.
    // New code should call DrawChar() directly instead.
    int tilesPerRow = (int)( 1.0f / vTexels.x + 0.5f );
    int tileIndex = vTile.y * tilesPerRow + vTile.x;
    char ch = TileIndexToChar( tileIndex );

    return DrawChar( vPos, vSize, ch );
}

bool CRenderASCII::DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile )
{
    // Untextured tile — draw a solid block
    return DrawChar( vPos, vSize, '#' );
}

bool CRenderASCII::DrawChar( const JFVector &vPos, JVector &vSize, char ch )
{
    int screenX = MapX( vPos.x );
    int screenY = MapY( vPos.y );

    bool isPixelSpace = ( m_currentBounds.left >= 0 && m_currentBounds.right > 500 );
    if( !isPixelSpace )
    {
        if( screenX < m_layout.dungeon.left - 3 || screenX >= m_layout.dungeon.right + 3 ||
            screenY < m_layout.dungeon.top - 3 || screenY >= m_layout.dungeon.bottom + 3 )
            return false;
    }
    else
    {
        if( screenX < 0 || screenX >= m_layout.termWidth || screenY < 0 ||
            screenY >= m_layout.termHeight )
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

// ---- UI drawing ----

void CRenderASCII::DrawTextBoundingBox( JRect rect, JColor color )
{
    // Convert pixel rect to terminal char coords
    int left = rect.left / 6;
    int top = rect.top / 8;
    int right = ( rect.right / 6 ) - 1;
    int bottom = ( rect.bottom / 8 ) - 1;

    // Clamp to screen
    if( left < 0 )
        left = 0;
    if( top < 0 )
        top = 0;
    if( right >= m_layout.termWidth )
        right = m_layout.termWidth - 1;
    if( bottom >= m_layout.termHeight )
        bottom = m_layout.termHeight - 1;
    if( right <= left || bottom <= top )
        return;

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
        // Fill interior with spaces so earlier draws (dungeon) don't bleed through
        for( int x = left + 1; x < right; x++ )
            mvaddch( y, x, ' ' );
        mvaddch( y, right, ACS_VLINE );
    }

    if( m_bHasColor )
        attroff( COLOR_PAIR( pair ) );
}
#endif // RENDER_ASCII
