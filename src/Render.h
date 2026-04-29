// Render.h
//
// the SDL/OpenGL renderer
// Jimbo S. Harris 5/12/2002
#ifndef __RENDER_H__
#define __RENDER_H__

#ifdef DISPLAY_FRAMERATE
class CDisplayText;
#endif

// screen width, height, and bit depth
// (eventually should be either user-settable
// or moved somewhere more permanent)
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP 32

#include "JMDefs.h"
#include "RenderBase.h"
#ifdef RENDER_OPENGL
#include "SDL2/SDL.h"
#endif

class CRender : public IRenderBackend
{
public:
    CRender()
        : m_bHasBeenInitted( false ),
          m_dwScreenWidth( 0 ),
          m_dwScreenHeight( 0 ),
          m_dwScreenBPP( 0 ),
          m_dwWindowFlags( 0 ),
          m_hWindow( NULL ),
          m_dwTileMetricsTilesPerRow( 0 ),
          m_dwZoom( ZOOM_NORMAL ) {};
    virtual ~CRender() { Term(); }

    virtual JResult Init( int width, int height, int bpp );
    JResult InitSDL();
    JResult InitGL();

    JResult ResizeWindow( int width, int height );
    virtual void Term()
    {
        if( m_hWindow )
        {
            SDL_DestroyWindow( m_hWindow );
            m_hWindow = NULL;
        }
        SDL_Quit();
    };

    int GetWindowFlags() { return m_dwWindowFlags; }
    virtual void PreDraw();
    virtual void PostDraw();

    virtual void PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate = false,
                                 bool bInverse = false, JFVector *vTranslate = 0 );
    virtual void PostDrawObjects();

    virtual void DrawTextBoundingBox( JRect rect, JColor color );

    virtual void SwapBuffers();
    SDL_Window *GetWindow() { return m_hWindow; }
    void SetWindow( SDL_Window *window ) { m_hWindow = window; }

    virtual bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile,
                           JFVector &vTexels );
    virtual bool DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile );
    virtual void PreDrawTile();
    virtual void PostDrawTile();
    virtual void SetTileColor( JColor color );

    virtual bool DrawChar( const JFVector &vPos, JVector &vSize, char ch );
    virtual void SetTileMetrics( int tilesPerRow, JFVector vTexels );

    // Declare this if you need to do more than just "load" the texture.
    virtual JResult PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP,
                                     int dwImageWidth, int dwImageHeight, int dwCellWidth,
                                     int dwCellHeight );

    // IRenderBackend configuration
    int GetScreenWidth() const override { return m_dwScreenWidth; }
    int GetScreenHeight() const override { return m_dwScreenHeight; }

    // Translate SDL events into renderer-agnostic events
    bool PollEvent( JInputEvent &event ) override;

    // Zoom control (OpenGL viewport)
    Uint16 GetZoom() const override { return m_dwZoom; }
    void SetZoom( Uint16 zoom ) override { m_dwZoom = zoom; }
    void Zoom( Uint16 dwDelta ) override
    {
        m_dwZoom += dwDelta;
        if( m_dwZoom < ZOOM_MIN )
            m_dwZoom = ZOOM_MIN;
        else if( m_dwZoom > ZOOM_MAX )
            m_dwZoom = ZOOM_MAX;
    }

protected:
    int m_dwScreenWidth;
    int m_dwScreenHeight;
    int m_dwScreenBPP;

    int m_dwWindowFlags;

private:
    static constexpr Uint16 ZOOM_MIN = 4;
    static constexpr Uint16 ZOOM_NORMAL = 20;
    static constexpr Uint16 ZOOM_MAX = 100;

    bool m_bHasBeenInitted;
    SDL_Window *m_hWindow;
    Uint16 m_dwZoom;

    // Tileset metrics for DrawChar: stored by SetTileMetrics(), used to convert chars to quads
    int m_dwTileMetricsTilesPerRow;
    JFVector m_vTileMetricsTexels;

#ifdef DISPLAY_FRAMERATE
    int m_dwFrames;
    int m_dwT0;
    CDisplayText *m_fps;
#endif // DISPLAY_FRAMERATE
};
#endif // __RENDER_H__
