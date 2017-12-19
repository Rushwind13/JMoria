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
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     32

#include "JMDefs.h"

class CRender
{
public:
	CRender()
		: m_bHasBeenInitted(false),
		m_dwScreenWidth(0),
		m_dwScreenHeight(0),
		m_dwScreenBPP(0),
		m_dwWindowFlags(0),
		m_hWindow(NULL)
	{};
	virtual ~CRender(){ Term(); };
	
	virtual JResult Init( int width, int height, int bpp );
	JResult InitSDL();
	JResult InitGL();
	
	JResult ResizeWindow( int width, int height );
	virtual void Term()
	{
		if( m_hWindow )
		{
			SDL_DestroyWindow(m_hWindow);
			m_hWindow = NULL;
		}
		SDL_Quit();
	};
	
	int GetWindowFlags() { return m_dwWindowFlags; };
	virtual void PreDraw();
	virtual void PostDraw();
	
	virtual void	PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate=false, bool bInverse=false, JFVector *vTranslate=0 );
	virtual void	PostDrawObjects();

	virtual void	DrawTextBoundingBox( JRect rect, uint8 alpha=25 );
	
	virtual void SwapBuffers();
	SDL_Window *GetWindow() { return m_hWindow; };
	void SetWindow(SDL_Window *window) { m_hWindow = window; };
	
	virtual bool	DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile, JFVector &vTexels);
	virtual bool	DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile);
	virtual void	PreDrawTile();
	virtual void	PostDrawTile();
	virtual void	SetTileColor( JColor color );	
	
	// Declare this if you need to do more than just "load" the texture.
	virtual JResult	PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP, int dwImageWidth, int dwImageHeight, int dwCellWidth, int dwCellHeight );

protected:
	int	m_dwScreenWidth;
	int m_dwScreenHeight;
	int m_dwScreenBPP;
	
	int m_dwWindowFlags;
	
private:
	bool m_bHasBeenInitted;
    SDL_Window *m_hWindow;
	
#ifdef DISPLAY_FRAMERATE
	int	m_dwFrames;
	int m_dwT0;
	CDisplayText *m_fps;
#endif // DISPLAY_FRAMERATE
	
};
#endif // __RENDER_H__
