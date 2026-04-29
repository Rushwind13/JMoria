// RenderASCII.h
//
// ncurses-based ASCII renderer for JMoria
//
#ifndef __RENDERASCII_H__
#define __RENDERASCII_H__

#include "Constants.h"
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
    ASCIILayoutRegion monsters;
    ASCIILayoutRegion use;
    ASCIILayoutRegion endgame;
    ASCIILayoutRegion monRecall;
    ASCIILayoutRegion itemRecall;
    ASCIILayoutRegion map;

    // Dynamically compute layout for any terminal size
    static ASCIILayout CreateForSize( int w, int h );

    // Threshold: inventory panel shown permanently when terminal is this wide
    static constexpr int INV_AUTO_WIDTH = 100;
    static constexpr int STATS_WIDTH = 25;
    static constexpr int INV_WIDTH = 25;
    static constexpr int MSG_HEIGHT = MSGS_ROWS;
    static constexpr int MONSTERS_HEIGHT = 10;
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

    bool DrawChar( const JFVector &vPos, JVector &vSize, char ch ) override;

    void PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate = false,
                         bool bInverse = false, JFVector *vTranslate = 0 ) override;
    void PostDrawObjects() override;

    void DrawTextBoundingBox( JRect rect, JColor color ) override;

    int GetScreenWidth() const override { return m_layout.termWidth; }
    int GetScreenHeight() const override { return m_layout.termHeight; }
    int GetTextInset() const override { return 1; }
    int GetMaxTextWidth() const override { return m_layout.termWidth * 6; }
    int GetMaxTextHeight() const override { return m_layout.termHeight * 8; }

    // Check if terminal was resized and recalculate layout
    bool CheckResize() override;

    bool ShouldAutoShowInventory() const override
    {
        return m_layout.termWidth >= ASCIILayout::INV_AUTO_WIDTH;
    }

    bool ShouldAutoShowEquipment() const override { return false; }
    bool ShouldAutoShowMonsters() const override { return false; }

    // Set DisplayText rects from ASCII layout (char coords → pixel space)
    void ConfigureDisplayRegions( CDisplayText *pMsgs, CDisplayText *pStats, CDisplayText *pInv,
                                  CDisplayText *pEquip, CDisplayText *pUse, CDisplayText *pEndGame,
                                  CDisplayText *pMonsters, CDisplayText *pMonRecall,
                                  CDisplayText *pItemRecall, CDisplayText *pMap ) override;

    // Translate ncurses input into renderer-agnostic events
    bool PollEvent( JInputEvent &event ) override;

    const ASCIILayout &GetLayout() const { return m_layout; }

private:
    bool m_bInitted;
    ASCIILayout m_layout;

    // Current drawing state
    JColor m_currentColor;
    JRect m_currentBounds; // Current PreDrawObjects bounds (pixel coords from OpenGL world)
    JFVector m_currentTranslate;
    bool m_bTranslating;

    // Color support
    enum ColorMode
    {
        COLOR_NONE, // no color support
        COLOR_8,    // 8 basic colors
        COLOR_16,   // 8 colors + A_BOLD for bright variants
        COLOR_256   // xterm 256-color palette
    };
    ColorMode m_colorMode;
    bool m_bHasColor;
    void InitColors();

    // Returns the ncurses color pair index. Sets outAttr to any extra
    // attributes needed (e.g. A_BOLD for bright colors in 16-color mode).
    int GetColorPair( JColor color, attr_t &outAttr );

    // Coordinate mapping: convert OpenGL pixel/world coords to terminal char coords
    int MapX( float worldX );
    int MapY( float worldY );

    // Map a tile index (font texture offset) back to an ASCII char
    char TileIndexToChar( int tileIndex );
};

#endif // __RENDERASCII_H__
