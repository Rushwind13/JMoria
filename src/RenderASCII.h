// RenderASCII.h
//
// ncurses-based ASCII renderer for JMoria
//
#ifndef __RENDERASCII_H__
#define __RENDERASCII_H__

#include "RenderBase.h"
#include <curses.h>

// ASCII renderer screen layouts
struct ASCIILayoutRegion
{
    int left;
    int top;
    int right;
    int bottom;

    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
};

struct ASCIILayout
{
    int termWidth;
    int termHeight;

    ASCIILayoutRegion dungeon;
    ASCIILayoutRegion messages;
    ASCIILayoutRegion stats;
    ASCIILayoutRegion inventory;
    ASCIILayoutRegion equipment;
    ASCIILayoutRegion use;
    ASCIILayoutRegion endgame;

    static ASCIILayout Create80x24();
    static ASCIILayout Create125x40();
};

class CRenderASCII : public IRenderBackend
{
public:
    CRenderASCII();
    virtual ~CRenderASCII() { Term(); }

    // IRenderBackend implementation
    JResult Init( int width, int height, int bpp ) override;
    void Term() override;

    void PreDraw() override;
    void PostDraw() override;
    void SwapBuffers() override;

    void PreDrawTile() override;
    void PostDrawTile() override;
    void SetTileColor( JColor color ) override;
    bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile,
                   JFVector &vTexels ) override;
    bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile ) override;

    void PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate = false,
                         bool bInverse = false, JFVector *vTranslate = 0 ) override;
    void PostDrawObjects() override;

    void DrawTextBoundingBox( JRect rect, JColor color ) override;

    int GetScreenWidth() const override { return m_layout.termWidth; }
    int GetScreenHeight() const override { return m_layout.termHeight; }
    int GetTextInset() const override { return 1; }
    int GetMaxTextWidth() const override { return m_layout.termWidth * 6; }
    int GetMaxTextHeight() const override { return m_layout.termHeight * 8; }

private:
    bool m_bInitted;
    ASCIILayout m_layout;

    // Current drawing state
    JColor m_currentColor;
    JRect m_currentBounds;     // Current PreDrawObjects bounds (pixel coords from OpenGL world)
    JFVector m_currentTranslate;
    bool m_bTranslating;

    // Color support
    bool m_bHasColor;
    void InitColors();
    int GetColorPair( JColor color );

    // Coordinate mapping: convert OpenGL pixel/world coords to terminal char coords
    int MapX( float worldX );
    int MapY( float worldY );

    // Map a tile index (font texture offset) back to an ASCII char
    char TileIndexToChar( int tileIndex );
};

#endif // __RENDERASCII_H__
