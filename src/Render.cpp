// Render.cpp
// implementation of the SDL/OpenGL Render
// Jimbo S. Harris 5/12/2002
#include "JMDefs.h"
#ifdef RENDER_OPENGL

// #define DISPLAY_FRAMERATE
// #define _DEBUG

#include "Render.h"
#ifdef __APPLE__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif
#include "SDL2/SDL.h"
#include <cstdlib> // for getenv
#include <cstring> // for strcmp

#ifdef DISPLAY_FRAMERATE
#include "DisplayText.h"
#endif

JResult CRender::Init( int width, int height, int bpp )
{
    JResult retval;

    // Check if we're in headless mode (for testing)
    const char *videoDriver = getenv( "SDL_VIDEODRIVER" );
    bool isHeadless = ( videoDriver != NULL && strcmp( videoDriver, "dummy" ) == 0 );

    // Set our member variables
    m_dwScreenWidth = width;
    m_dwScreenHeight = height;
    m_dwScreenBPP = bpp;

    // In headless mode, skip SDL/OpenGL initialization
    if( isHeadless )
    {
        JLog(
            LOG_LEVEL_INFO, true,
            "Running in headless mode (SDL_VIDEODRIVER=dummy), skipping graphics initialization" );
        m_hWindow = NULL;
        return JSUCCESS;
    }

    // Startup SDL
    retval = InitSDL();
    if( retval != JSUCCESS )
    {
        return retval;
    }

    // Startup OpenGL
    retval = InitGL();
    if( retval != JSUCCESS )
    {
        return retval;
    }

    // init the window size
    retval = ResizeWindow( width, height );
    if( retval != JSUCCESS )
    {
        return retval;
    }

#ifdef DISPLAY_FRAMERATE
    m_dwFrames = 0;
    m_dwT0 = 0;
    m_fps = new CDisplayText( "../JMoria/", JRect( 500, 450, 640, 480 ) );
    m_fps->SetFlags( FLAG_TEXT_INVERSE );
#endif

    // game on.
    return JSUCCESS;
}

JResult CRender::InitSDL()
{
    m_dwWindowFlags = SDL_WINDOW_OPENGL;     // Enable OpenGL in SDL
    m_dwWindowFlags |= SDL_WINDOW_RESIZABLE; // Enable window resizing

    // Sets up OpenGL double buffering
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // Allocate window and context.
    char window_title[32];
    sprintf( window_title, g_Strings[STR_WINDOW_TITLE], VERSION );
    m_hWindow =
        SDL_CreateWindow( window_title, 0, 0, m_dwScreenWidth, m_dwScreenHeight, m_dwWindowFlags );
    if( m_hWindow != NULL )
    {
        SDL_GLContext glcontext = SDL_GL_CreateContext( m_hWindow );
        if( glcontext == NULL )
        {
            fprintf( stderr, "Couldn't allocate OpenGL context: %s\n", SDL_GetError() );
            SDL_DestroyWindow( m_hWindow );
            return JERROR();
        }
    }
    else
    {
        fprintf( stderr, "Couldn't allocate a window: %s\n", SDL_GetError() );
        return JERROR();
    }

    return JSUCCESS;
}

JResult CRender::InitGL()
{
    // Enable Texture Mapping
    glEnable( GL_TEXTURE_2D );

    // Enable smooth shading
    glShadeModel( GL_SMOOTH );

    // Set the background color
    glClearColor( 0.2f, 0.5f, 0.4f, 1.0f );

    // Depth buffer setup
    glClearDepth( 1.0f );

    // Enables Depth Testing
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );

    // parms are: "What color is the background?" and "What color is the picture?", respectively.
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    // glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA) ;
    // glBlendFunc(GL_ZERO, GL_SRC_ALPHA) ;
    // glBlendFunc(GL_ONE, GL_DST_ALPHA) ;
    // glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    // glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );

    // The Type Of Depth Test To Do
    glDepthFunc( GL_LEQUAL );

    return ( JSUCCESS );
}

JResult CRender::ResizeWindow( int width, int height )
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return JSUCCESS;

    // Setup our viewport.
    glViewport( 0, 0, (GLint)m_dwScreenWidth, (GLint)m_dwScreenHeight );

    // change to the projection matrix and set
    // our viewing volume.

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    // Set our perspective
    glOrtho( -DUNG_WIDTH, DUNG_WIDTH, DUNG_HEIGHT, -DUNG_HEIGHT, -1, 1 );

    // Make sure we're changing the model view and not the projection
    glMatrixMode( GL_MODELVIEW );

    // Reset The View
    glLoadIdentity();

    return ( JSUCCESS );
}

void CRender::PreDraw()
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return;

    // Clear The Screen And The Depth Buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void CRender::PostDraw()
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return;

#ifdef DISPLAY_FRAMERATE
    m_fps->Draw();
    // Gather our frames per second
    m_dwFrames++;
    {
        GLint t = SDL_GetTicks();
        if( t - m_dwT0 >= 5000 )
        {
            // #ifdef _DEBUG
            GLfloat seconds = ( t - m_dwT0 ) / 1000.0f;
            GLfloat fps = m_dwFrames / seconds;
            m_fps->Printf( g_Strings[STR_FPS], fps );
            // #endif // _DEBUG
            m_dwT0 = t;
            m_dwFrames = 0;
        }
    }
#endif // DISPLAY_FRAMERATE
    SwapBuffers();
}

void CRender::SwapBuffers()
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return;

    // pageflip
    SDL_GL_SwapWindow( m_hWindow );
}

bool CRender::PollEvent( JInputEvent &event )
{
    SDL_Event sdlEvent;

    while( SDL_PollEvent( &sdlEvent ) )
    {
        switch( sdlEvent.type )
        {
        case SDL_WINDOWEVENT:
            switch( sdlEvent.window.event )
            {
            case SDL_WINDOWEVENT_RESIZED:
            {
                JResult retval = ResizeWindow( sdlEvent.window.data1, sdlEvent.window.data2 );
                if( retval != JSUCCESS )
                {
                    event.type = JInputEvent::QUIT;
                    return true;
                }
                break;
            }
            }
            // Focus events handled internally; continue polling
            break;
        case SDL_KEYDOWN:
        {
            event.type = JInputEvent::KEY;
            event.keysym.sym = (JKeycode)sdlEvent.key.keysym.sym;
            event.keysym.mod = (JKeymod)sdlEvent.key.keysym.mod;
            return true;
        }
        case SDL_QUIT:
            event.type = JInputEvent::QUIT;
            return true;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if( sdlEvent.button.state == SDL_RELEASED )
                break;
            switch( sdlEvent.button.button )
            {
            case MOUSE_WHEEL_UP:
                Zoom( 3 );
                break;
            case MOUSE_WHEEL_DOWN:
                Zoom( -3 );
                break;
            }
            break;
        default:
            break;
        }
    }

    return false;
}

void CRender::PreDrawTile()
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return;

    glBegin( GL_QUADS );
}

void CRender::PostDrawTile()
{
    // Skip in headless mode
    if( m_hWindow == NULL )
        return;

    glEnd();
}

void CRender::SetTileColor( JColor color )
{
    if( m_hWindow != NULL )
        glColor4ub( COLOR_EXPAND( color ) );
}

void CRender::DrawTextBoundingBox( JRect rect, JColor color )
{
    if( m_hWindow == NULL )
        return;
    glColor4ub( COLOR_EXPAND( color ) );
    glDisable( GL_TEXTURE_2D );
    glRecti( RECT_EXPAND( rect ) );
    glEnable( GL_TEXTURE_2D );
}

// call me between glBegin() and glEnd().
// vSize is size of tile in viewport units
// vTexels is size of tile in Texels
// vPos is position in the world in viewport units
// vTile is pos on the texture in texel coordinate units
bool CRender::DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile, JFVector &vTexels )
{
    JFVector vPos1( vPos.x + vSize.x, vPos.y + vSize.y );

    float s = vTile.x * vTexels.x;
    float t = vTile.y * vTexels.y;
    float s1 = ( vTile.x + 1 ) * vTexels.x;
    float t1 = ( vTile.y + 1 ) * vTexels.y;

    glTexCoord2f( s, t );
    glVertex2f( vPos.x, vPos.y );
    glTexCoord2f( s1, t );
    glVertex2f( vPos1.x, vPos.y );
    glTexCoord2f( s1, t1 );
    glVertex2f( vPos1.x, vPos1.y );
    glTexCoord2f( s, t1 );
    glVertex2f( vPos.x, vPos1.y );

    return true;
}

bool CRender::DrawTile( const JFVector &vPos, JVector &vSize, JIVector &vTile )
{
    JFVector vPos1( vPos.x + vSize.x, vPos.y + vSize.y );

    glDisable( GL_TEXTURE_2D );

    glVertex2f( vPos.x, vPos.y );
    glVertex2f( vPos1.x, vPos.y );
    glVertex2f( vPos1.x, vPos1.y );
    glVertex2f( vPos.x, vPos1.y );

    glEnable( GL_TEXTURE_2D );
    return true;
}

void CRender::PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate, bool bInverse,
                              JFVector *vTranslate )
{
    if( m_hWindow == NULL )
        return;

    // Push the neccessary Matrices on the stack
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( VIEWRECT_EXPAND( rcBounds ), -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    if( bTranslate )
    {
        glTranslatef( VEC_EXPAND( *vTranslate ), 0.0f );
    }

    // Push the neccessary Attributes on the stack
    glPushAttrib( GL_TEXTURE_BIT | GL_ENABLE_BIT );

    glBindTexture( GL_TEXTURE_2D, Texture );
    if( !glIsTexture( Texture ) )
    {
        JLog( LOG_LEVEL_ERROR, true, "Hey! That's not a texture.\n" );
    }
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable( GL_TEXTURE_2D );

    // Always Draw in Front
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_BLEND );
    if( bInverse )
    {
        glBlendFunc( GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA );
    }
    else
    {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
}

void CRender::PostDrawObjects()
{
    if( m_hWindow == NULL )
        return;

    // Return to previous Matrix and Attribute states. Easy cleanup!
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glPopAttrib();
}

#ifdef RENDER_TILESET_POSTLOAD_NEEDED
JResult CRender::PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP,
                                  int dwImageWidth, int dwImageHeight, int dwCellWidth,
                                  int dwCellHeight )
{
    uint32 dwColorDef = GL_RGBA;
    if( bIsBMP )
    {
        dwColorDef = GL_BGRA_EXT;
    }
    glGenTextures( 1, &texture );
    if( texture == 0 )
    {
        return JERROR();
    }

    glBindTexture( GL_TEXTURE_2D, texture );

    if( !glIsTexture( texture ) )
    {
        return JERROR();
    }

    // Generate The Texture
    glTexImage2D( GL_TEXTURE_2D, 0, dwColorsPerPixel, dwImageWidth, dwImageHeight, 0, dwColorDef,
                  GL_UNSIGNED_BYTE, data );

#ifdef _DEBUG
    GLint width;
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
#endif // _DEBUG

    // Linear Filtering
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    return JSUCCESS;
}
#endif // postload needed

void CRender::SetTileMetrics( int tilesPerRow, JFVector vTexels )
{
    m_dwTileMetricsTilesPerRow = tilesPerRow;
    m_vTileMetricsTexels = vTexels;
}

bool CRender::DrawChar( const JFVector &vPos, JVector &vSize, char ch )
{
    // Convert printable ASCII character to tile grid coordinates
    // using stored tileset metrics, then draw as a textured quad.
    int dwIndex = ch - ' ' - 1;
    JIVector vTile;
    vTile.x = dwIndex % m_dwTileMetricsTilesPerRow;
    vTile.y = dwIndex / m_dwTileMetricsTilesPerRow;

    return DrawTile( vPos, vSize, vTile, m_vTileMetricsTexels );
}
#endif // RENDER_OPENGL
