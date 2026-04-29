// RenderBase.h
//
// Abstract renderer interface for JMoria
// Allows both OpenGL and ASCII (ncurses) rendering backends
//
#ifndef __RENDERBASE_H__
#define __RENDERBASE_H__

#include "JKeys.h"
#include "JMDefs.h"

// Renderer-agnostic input event returned by PollEvent().
// Each renderer translates its native input (SDL / ncurses) into this.
struct JInputEvent
{
    enum Type
    {
        NONE,
        KEY,
        QUIT
    } type;
    JKeysym keysym; // valid when type == KEY

    JInputEvent() : type( NONE )
    {
        keysym.sym = (JKeycode)0;
        keysym.mod = JMOD_NONE;
    }
};

class IRenderBackend
{
public:
    virtual ~IRenderBackend() {};

    // Initialization and teardown
    virtual JResult Init( int width, int height, int bpp ) = 0;
    virtual void Term() = 0;

    // Frame management
    virtual void PreDraw() = 0;
    virtual void PostDraw() = 0;
    virtual void SwapBuffers() = 0;

    // Tile drawing (world space)
    virtual void PreDrawTile() = 0;
    virtual void PostDrawTile() = 0;
    virtual void SetTileColor( JColor color ) = 0;
    virtual bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile,
                           JFVector &vTexels ) = 0;
    virtual bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile ) = 0;

    // Object/text drawing (screen space)
    virtual void PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate = false,
                                 bool bInverse = false, JFVector *vTranslate = 0 ) = 0;
    virtual void PostDrawObjects() = 0;

    // UI drawing
    virtual void DrawTextBoundingBox( JRect rect, JColor color ) = 0;

    // Character-based drawing (ASCII-first path)
    // Renderers implement this to draw a printable ASCII character at a position.
    // ASCII renderer: maps to terminal cell and mvaddch directly.
    // OpenGL renderer: converts character to tile grid coords and draws a textured quad.
    virtual bool DrawChar( const JFVector &vPos, JVector &vSize, char ch ) = 0;

    // Store tileset metrics so the OpenGL renderer can convert characters to texture coords.
    // ASCII renderer can ignore this. Called by CTileset::PreDrawTile().
    virtual void SetTileMetrics( int tilesPerRow, JFVector vTexels ) {}

    // Texture loading (optional; backends that don't use textures can no-op)
    virtual JResult PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP,
                                     int dwImageWidth, int dwImageHeight, int dwCellWidth,
                                     int dwCellHeight )
    {
        return 0;
    };

    // Configuration
    virtual int GetScreenWidth() const = 0;
    virtual int GetScreenHeight() const = 0;

    // Text inset: number of character cells to inset text from bounding box edges.
    // ASCII mode returns 1 because box-drawing chars occupy cells; OpenGL returns 0
    // because the bounding box is a translucent quad behind the text.
    virtual int GetTextInset() const { return 0; }

    // Maximum text display area in pixel-space units (FONT_DRAW_W=6, FONT_DRAW_H=8).
    // Used to clamp wrap/pagination when the pixel rect exceeds the actual screen.
    // OpenGL: returns large values (no extra clamping needed).
    // ASCII: returns termWidth*6 x termHeight*8.
    virtual int GetMaxTextWidth() const { return 9999; }
    virtual int GetMaxTextHeight() const { return 9999; }

    // Check if terminal was resized. Returns true if layout changed.
    // OpenGL: always false. ASCII: detects ncurses resize.
    virtual bool CheckResize() { return false; }

    // Whether the inventory panel should auto-show based on screen width.
    // OpenGL: always true (fixed layout). ASCII: true when terminal is wide enough.
    virtual bool ShouldAutoShowInventory() const { return true; }

    // Whether the equipment panel should auto-show.
    // OpenGL: always true (fixed layout). ASCII: false (toggled by user).
    virtual bool ShouldAutoShowEquipment() const { return true; }

    // Whether the monsters pane should auto-show.
    // OpenGL: false (user toggles with v). ASCII: false (toggled by user).
    virtual bool ShouldAutoShowMonsters() const { return false; }

    // Configure DisplayText region rects based on renderer's layout.
    // Called at init and on resize. Each renderer knows its own coordinate system.
    virtual void ConfigureDisplayRegions( class CDisplayText *pMsgs, class CDisplayText *pStats,
                                          class CDisplayText *pInv, class CDisplayText *pEquip,
                                          class CDisplayText *pUse, class CDisplayText *pEndGame,
                                          class CDisplayText *pMonsters,
                                          class CDisplayText *pMonRecall = nullptr,
                                          class CDisplayText *pItemRecall = nullptr,
                                          class CDisplayText *pMap = nullptr )
    {
    }

    // Poll for the next input event. Returns true if an event was retrieved.
    // Each renderer translates its native input into JInputEvent.
    // Called in a loop by Game::HandleEvents() — no renderer branching needed.
    virtual bool PollEvent( JInputEvent &event ) { return false; }

    // Zoom control (OpenGL viewport concept — ASCII is fixed scale)
    virtual Uint16 GetZoom() const { return 20; }
    virtual void SetZoom( Uint16 zoom ) {}
    virtual void Zoom( Uint16 dwDelta ) {}
};

#endif // __RENDERBASE_H__
