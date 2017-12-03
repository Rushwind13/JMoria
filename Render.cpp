// Render.cpp
// implementation of the SDL/OpenGL Render
// Jimbo S. Harris 5/12/2002

#define DISPLAY_FRAMERATE

#include "Render.h"
#include "OpenGL/gl.h"
#include "SDL2/SDL.h"
//#include "Game.h"

#ifdef DISPLAY_FRAMERATE
#include "DisplayText.h"
#endif

JResult CRender::Init( int width, int height, int bpp )
{
	JResult retval;

	// Set our member variables
	m_dwScreenWidth = width;
	m_dwScreenHeight = height;
	m_dwScreenBPP = bpp;

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
	m_fps = new CDisplayText( JRect(540,460,  640,480) );
	m_fps->SetFlags(FLAG_TEXT_INVERSE);
#endif

	// game on.
	return JSUCCESS;
}

JResult CRender::InitSDL()
{
    m_dwWindowFlags  = SDL_WINDOW_OPENGL; // Enable OpenGL in SDL
    m_dwWindowFlags |= SDL_WINDOW_RESIZABLE; // Enable window resizing
    
    // Sets up OpenGL double buffering
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
    // Allocate window and context.
    m_hWindow = SDL_CreateWindow("JMoria", 0, 0, m_dwScreenWidth, m_dwScreenHeight, m_dwWindowFlags);
    if (m_hWindow != NULL) {
        SDL_GLContext glcontext = SDL_GL_CreateContext(m_hWindow);
        if (glcontext == NULL)
        {
            fprintf( stderr, "Couldn't allocate OpenGL context: %s\n", SDL_GetError());
            SDL_DestroyWindow(m_hWindow);
            return JERROR();
        }
    } else {
        fprintf( stderr, "Couldn't allocate a window: %s\n", SDL_GetError());
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	//glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA) ;
	//glBlendFunc(GL_ZERO, GL_SRC_ALPHA) ;
	//glBlendFunc(GL_ONE, GL_DST_ALPHA) ;
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	//glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );

    // The Type Of Depth Test To Do
    glDepthFunc( GL_LEQUAL );

    return( JSUCCESS );
}

JResult CRender::ResizeWindow( int width, int height )
{
    // Setup our viewport.
    glViewport( 0, 0, ( GLint )m_dwScreenWidth, ( GLint )m_dwScreenHeight );
	
    // change to the projection matrix and set
    // our viewing volume.
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
	
    // Set our perspective
	glOrtho( -DUNG_WIDTH, DUNG_WIDTH, DUNG_HEIGHT, -DUNG_HEIGHT, -1, 1 );
	
	// Make sure we're changing the model view and not the projection
    glMatrixMode( GL_MODELVIEW );
	
    // Reset The View
    glLoadIdentity( );
	
    return( JSUCCESS );
}

void CRender::PreDraw()
{
    // Clear The Screen And The Depth Buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void CRender::PostDraw()
{	
#ifdef DISPLAY_FRAMERATE
	m_fps->Draw();
    // Gather our frames per second
    m_dwFrames++;
    {
		GLint t = SDL_GetTicks();
		if (t - m_dwT0 >= 5000)
        {
#ifdef _DEBUG
			GLfloat seconds = (t - m_dwT0) / 1000.0f;
			GLfloat fps = m_dwFrames / seconds;
			m_fps->Printf("%g FPS\n", fps);
#endif // _DEBUG
			m_dwT0 = t;
			m_dwFrames = 0;
		}
    }
#endif // DISPLAY_FRAMERATE
    SwapBuffers();
}

void CRender::SwapBuffers()
{
    // pageflip
    SDL_GL_SwapWindow(m_hWindow);
}

void CRender::PreDrawTile()
{
	glBegin( GL_QUADS );
}

void CRender::PostDrawTile()
{
	glEnd();
}

void CRender::SetTileColor( JColor color )
{
	glColor4ub( COLOR_EXPAND(color) );
}

void CRender::DrawTextBoundingBox( JRect rect )
{
	glColor4f( 1, 1, 1, 0.1f );
	glDisable(GL_TEXTURE_2D);
	glRecti(RECT_EXPAND(rect));
	glEnable(GL_TEXTURE_2D);
}

// call me between glBegin() and glEnd().
// vSize is size of tile in viewport units
// vTexels is size of tile in Texels
// vPos is position in the world in viewport units
// vTile is pos on the texture in texel coordinate units
bool CRender::DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile, JFVector &vTexels)
{
	JFVector vPos1(vPos.x + vSize.x, vPos.y + vSize.y);

	float s = vTile.x * vTexels.x;
	float t = vTile.y * vTexels.y;
	float s1 = (vTile.x+1) * vTexels.x;
	float t1 = (vTile.y+1) * vTexels.y;
	
	

	glTexCoord2f( s,  t  );	glVertex2f( vPos.x,	vPos.y	);	
	glTexCoord2f( s1, t  );	glVertex2f( vPos1.x,vPos.y	);
	glTexCoord2f( s1, t1 );	glVertex2f( vPos1.x,vPos1.y	);
	glTexCoord2f( s,  t1 );	glVertex2f( vPos.x,	vPos1.y	);

	return true;
}

bool CRender::DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile)
{
	JFVector vPos1(vPos.x + vSize.x, vPos.y + vSize.y);

	glDisable(GL_TEXTURE_2D);
	
	glVertex2f( vPos.x,	vPos.y	);	
	glVertex2f( vPos1.x,vPos.y	);
	glVertex2f( vPos1.x,vPos1.y	);
	glVertex2f( vPos.x,	vPos1.y	);

	glEnable(GL_TEXTURE_2D);
	return true;
}

void CRender::PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate, bool bInverse, JFVector *vTranslate )
{
	// Push the neccessary Matrices on the stack
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		glOrtho( VIEWRECT_EXPAND( rcBounds ), -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();

	if( bTranslate )
	{
		glTranslatef( VEC_EXPAND(*vTranslate), 0.0f );
	}

	// Push the neccessary Attributes on the stack
	glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);

	glBindTexture(GL_TEXTURE_2D, Texture);
	if( !glIsTexture( Texture ) )
	{
		printf("Hey! That's not a texture.\n");
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	glEnable(GL_TEXTURE_2D);

	// Always Draw in Front
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	if( bInverse )
	{
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void CRender::PostDrawObjects()
{
	// Return to previous Matrix and Attribute states. Easy cleanup!
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}


#ifdef RENDER_TILESET_POSTLOAD_NEEDED
JResult CRender::PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP, int dwImageWidth, int dwImageHeight, int dwCellWidth, int dwCellHeight )
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
	glTexImage2D( GL_TEXTURE_2D, 0, dwColorsPerPixel, dwImageWidth,
		dwImageHeight, 0, dwColorDef,
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

/* SDL 1 / Windows initializer
 JResult CRender::InitSDL()
 {
 // this holds some info about our display
 const SDL_VideoInfo *videoInfo;
 
 
 // initialize SDL
 if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
 fprintf( stderr, "Video initialization failed: %s\n",
 SDL_GetError( ) );
 return JERROR();
	}
 
 // Fetch the video info
 videoInfo = SDL_GetVideoInfo( );
 
 if ( !videoInfo )
	{
 fprintf( stderr, "Video query failed: %s\n",
 SDL_GetError( ) );
 return JERROR();
	}
 
 // the flags to pass to SDL_SetVideoMode
 m_dwVideoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
 m_dwVideoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
 m_dwVideoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
 m_dwVideoFlags |= SDL_RESIZABLE;       // Enable window resizing
 
 // This checks to see if surfaces can be stored in memory
 if ( videoInfo->hw_available )
	{
 m_dwVideoFlags |= SDL_HWSURFACE;
	}
 else
	{
 m_dwVideoFlags |= SDL_SWSURFACE;
	}
 
 // This checks if hardware blits can be done
 if ( videoInfo->blit_hw )
	{
 m_dwVideoFlags |= SDL_HWACCEL;
	}
 
 // Sets up OpenGL double buffering
 SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
 
 // get a SDL surface
 m_hSurface = SDL_SetVideoMode( m_dwScreenWidth, m_dwScreenHeight, m_dwScreenBPP,
 m_dwVideoFlags );
 
 // Verify there is a surface
 if ( !m_hSurface )
	{
 fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
 return JERROR();
	}
 
	return JSUCCESS;
 }
/**/