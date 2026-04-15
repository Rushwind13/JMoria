// RenderASCII.cpp
//
// ncurses-based ASCII renderer for JMoria
//
#include "JMDefs.h"
#ifdef RENDER_ASCII

#include "DisplayText.h"
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

CRenderASCII::CRenderASCII()
    : m_bInitted( false ),
      m_bHasColor( false ),
      m_colorMode( COLOR_NONE ),
      m_bTranslating( false )
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

void CRenderASCII::ConfigureDisplayRegions( CDisplayText *pMsgs, CDisplayText *pStats,
                                            CDisplayText *pInv, CDisplayText *pEquip,
                                            CDisplayText *pUse, CDisplayText *pEndGame )
{
    auto toPixelRect = []( const ASCIILayoutRegion &r )
    { return JRect( r.left * 6, r.top * 8, r.right * 6, r.bottom * 8 ); };

    pMsgs->SetRect( toPixelRect( m_layout.messages ) );
    pStats->SetRect( toPixelRect( m_layout.stats ) );
    pInv->SetRect( toPixelRect( m_layout.inventory ) );
    pEquip->SetRect( toPixelRect( m_layout.equipment ) );
    pUse->SetRect( toPixelRect( m_layout.use ) );
    pEndGame->SetRect( toPixelRect( m_layout.endgame ) );
    pEndGame->SetContentMargin( 0, 0 );
}

bool CRenderASCII::PollEvent( JInputEvent &event )
{
    int ch = getch();
    if( ch == ERR )
        return false; // no input available

    // Terminal resize: consume the event, CheckResize handles the actual resize
    if( ch == KEY_RESIZE )
        return false;

    event.type = JInputEvent::KEY;
    memset( &event.keysym, 0, sizeof( event.keysym ) );

    // Map ncurses keys to JMoria keysyms
    // For ASCII printable characters, JKeycode values match ASCII codes
    if( ch >= 'a' && ch <= 'z' )
    {
        event.keysym.sym = (JKeycode)ch;
        event.keysym.mod = JMOD_NONE;
    }
    else if( ch >= 'A' && ch <= 'Z' )
    {
        // Uppercase: map to lowercase sym + shift modifier
        event.keysym.sym = (JKeycode)( ch - 'A' + 'a' );
        event.keysym.mod = JMOD_SHIFT;
    }
    else if( ch >= 1 && ch <= 26 && ch != '\n' && ch != '\r' )
    {
        // Ctrl+letter: ch 1 = Ctrl+A, ch 3 = Ctrl+C, etc.
        // Exclude \n (10) and \r (13) so they reach the Enter case below.
        event.keysym.sym = (JKeycode)( 'a' + ch - 1 );
        event.keysym.mod = JMOD_CTRL;
    }
    else if( ch >= '0' && ch <= '9' )
    {
        event.keysym.sym = (JKeycode)ch;
        event.keysym.mod = JMOD_NONE;
    }
    else
    {
        // Map special keys
        switch( ch )
        {
        case '\n':
        case '\r':
        case KEY_ENTER:
            event.keysym.sym = JKEY_RETURN;
            break;
        case 27: // Escape
            event.keysym.sym = JKEY_ESCAPE;
            break;
        case ' ':
            event.keysym.sym = JKEY_SPACE;
            break;
        case '.':
            event.keysym.sym = JKEY_PERIOD;
            break;
        case '>': // Shift+.
            event.keysym.sym = JKEY_PERIOD;
            event.keysym.mod = JMOD_SHIFT;
            break;
        case ',':
            event.keysym.sym = JKEY_COMMA;
            break;
        case '<': // Shift+,
            event.keysym.sym = JKEY_COMMA;
            event.keysym.mod = JMOD_SHIFT;
            break;
        case ';':
            event.keysym.sym = JKEY_SEMICOLON;
            break;
        case '*': // Shift+8: target command
            event.keysym.sym = JKEY_8;
            event.keysym.mod = JMOD_SHIFT;
            break;
        case KEY_BACKSPACE:
        case 127: // DEL on some terminals
            event.keysym.sym = JKEY_BACKSPACE;
            break;
        case KEY_DC: // ncurses Delete key
            event.keysym.sym = JKEY_DELETE;
            break;
        case KEY_F( 1 ):
            event.keysym.sym = JKEY_F1;
            break;
        default:
            // Unknown key, ignore
            return false;
        }
    }

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
        m_colorMode = COLOR_NONE;
        return;
    }

    start_color();
    m_bHasColor = true;

    if( COLORS >= 256 )
    {
        // 256-color mode: pair i maps xterm color i as foreground on black.
        // Pair 0 is reserved by ncurses, so pair N = color N (skip pair 0).
        m_colorMode = COLOR_256;
        for( int i = 1; i < 256; i++ )
            init_pair( i, i, COLOR_BLACK );
    }
    else
    {
        // 16-color mode (or 8-color mode with A_BOLD for bright variants).
        // Pairs 1-8: normal colors on black.
        m_colorMode = COLOR_16;
        init_pair( 1, COLOR_BLACK, COLOR_BLACK );
        init_pair( 2, COLOR_RED, COLOR_BLACK );
        init_pair( 3, COLOR_GREEN, COLOR_BLACK );
        init_pair( 4, COLOR_YELLOW, COLOR_BLACK );
        init_pair( 5, COLOR_BLUE, COLOR_BLACK );
        init_pair( 6, COLOR_MAGENTA, COLOR_BLACK );
        init_pair( 7, COLOR_CYAN, COLOR_BLACK );
        init_pair( 8, COLOR_WHITE, COLOR_BLACK );
    }
}

// ---- 256-color helpers ----

// The xterm-256 palette:
//   0-7     = standard colors (black, red, green, yellow, blue, magenta, cyan, white)
//   8-15    = bright colors
//   16-231  = 6x6x6 color cube: index = 16 + 36*r + 6*g + b  (r,g,b in 0..5)
//   232-255 = grayscale ramp (dark to light, 24 shades)

static int Nearest256( Uint8 r, Uint8 g, Uint8 b )
{
    // Check grayscale first: if r≈g≈b, use the grayscale ramp for better precision.
    int maxC = r;
    if( g > maxC )
        maxC = g;
    if( b > maxC )
        maxC = b;
    int minC = r;
    if( g < minC )
        minC = g;
    if( b < minC )
        minC = b;

    if( maxC - minC < 20 )
    {
        // Grayscale: ramp is indices 232-255, mapping to luminances 8,18,28,...238
        int avg = ( r + g + b ) / 3;
        if( avg < 4 )
            return 16; // black (from color cube)
        if( avg > 243 )
            return 231;                     // white (from color cube)
        int grayIdx = ( avg - 8 + 5 ) / 10; // round to nearest step of 10
        if( grayIdx < 0 )
            grayIdx = 0;
        if( grayIdx > 23 )
            grayIdx = 23;
        return 232 + grayIdx;
    }

    // Map to 6x6x6 color cube
    // Cube levels: 0, 95, 135, 175, 215, 255  (indices 0-5)
    static const int cubeLevels[6] = { 0, 95, 135, 175, 215, 255 };
    auto nearest6 = []( int v ) -> int
    {
        int best = 0;
        int bestDist = abs( v - cubeLevels[0] );
        for( int i = 1; i < 6; i++ )
        {
            int d = abs( v - cubeLevels[i] );
            if( d < bestDist )
            {
                bestDist = d;
                best = i;
            }
        }
        return best;
    };

    int ri = nearest6( r );
    int gi = nearest6( g );
    int bi = nearest6( b );
    return 16 + 36 * ri + 6 * gi + bi;
}

// ---- 16-color helpers ----

// The 16-color palette maps to 8 ncurses color pairs (1-8) plus A_BOLD
// for bright variants.  We define a table of reference RGB values for
// the 16 standard terminal colors and find the closest match.

struct TermColor16
{
    int pair;  // ncurses color pair (1-8)
    bool bold; // use A_BOLD?
    Uint8 r, g, b;
};

// clang-format off
static const TermColor16 kPalette16[16] = {
    // Normal colors (pairs 1-8, no bold)
    { 1, false,   0,   0,   0 },   // 0  black
    { 2, false, 170,   0,   0 },   // 1  red
    { 3, false,   0, 170,   0 },   // 2  green
    { 4, false, 170, 170,   0 },   // 3  yellow/brown
    { 5, false,   0,   0, 170 },   // 4  blue
    { 6, false, 170,   0, 170 },   // 5  magenta
    { 7, false,   0, 170, 170 },   // 6  cyan
    { 8, false, 170, 170, 170 },   // 7  white (light gray)
    // Bright colors (same pairs, with A_BOLD)
    { 1, true,   85,  85,  85 },   // 8  bright black (dark gray)
    { 2, true,  255,  85,  85 },   // 9  bright red
    { 3, true,   85, 255,  85 },   // 10 bright green
    { 4, true,  255, 255,  85 },   // 11 bright yellow
    { 5, true,   85,  85, 255 },   // 12 bright blue
    { 6, true,  255,  85, 255 },   // 13 bright magenta
    { 7, true,   85, 255, 255 },   // 14 bright cyan
    { 8, true,  255, 255, 255 },   // 15 bright white
};
// clang-format on

static int Nearest16( Uint8 r, Uint8 g, Uint8 b, bool &outBold )
{
    int bestIdx = 0;
    int bestDist = 999999;
    for( int i = 0; i < 16; i++ )
    {
        int dr = (int)r - kPalette16[i].r;
        int dg = (int)g - kPalette16[i].g;
        int db = (int)b - kPalette16[i].b;
        int dist = dr * dr + dg * dg + db * db;
        if( dist < bestDist )
        {
            bestDist = dist;
            bestIdx = i;
        }
    }
    outBold = kPalette16[bestIdx].bold;
    return kPalette16[bestIdx].pair;
}

int CRenderASCII::GetColorPair( JColor color, attr_t &outAttr )
{
    outAttr = 0;
    if( !m_bHasColor )
        return 0;

    Uint8 r, g, b, a;
    color.GetColor( r, g, b, a );

    if( m_colorMode == COLOR_256 )
    {
        int idx = Nearest256( r, g, b );
        if( idx < 1 )
            idx = 1; // pair 0 is reserved
        return idx;
    }

    // 16-color mode: find nearest and set bold if needed
    bool bold = false;
    int pair = Nearest16( r, g, b, bold );
    if( bold )
        outAttr = A_BOLD;
    return pair;
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

    attr_t outAttr = 0;
    int pair = GetColorPair( m_currentColor, outAttr );
    if( m_bHasColor )
        attron( COLOR_PAIR( pair ) | outAttr );

    mvaddch( screenY, screenX, ch );

    if( m_bHasColor )
        attroff( COLOR_PAIR( pair ) | outAttr );

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

    attr_t boxAttr = 0;
    int pair = GetColorPair( color, boxAttr );
    if( m_bHasColor )
        attron( COLOR_PAIR( pair ) | boxAttr );

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
        attroff( COLOR_PAIR( pair ) | boxAttr );
}
#endif // RENDER_ASCII
