// RenderBase.h
//
// Abstract renderer interface for JMoria
// Allows both OpenGL and ASCII (ncurses) rendering backends
//
#ifndef __RENDERBASE_H__
#define __RENDERBASE_H__

#include "JMDefs.h"

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
	virtual void	PreDrawTile() = 0;
	virtual void	PostDrawTile() = 0;
	virtual void	SetTileColor( JColor color ) = 0;
	virtual bool	DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile, JFVector &vTexels) = 0;
	virtual bool	DrawTile(const JFVector &vPos, JVector &vSize, JIVector &vTile) = 0;

	// Object/text drawing (screen space)
	virtual void	PreDrawObjects( JRect rcBounds, uint32 Texture, bool bTranslate=false, bool bInverse=false, JFVector *vTranslate=0 ) = 0;
	virtual void	PostDrawObjects() = 0;

	// UI drawing
	virtual void	DrawTextBoundingBox( JRect rect, JColor color ) = 0;

	// Texture loading (optional; backends that don't use textures can no-op)
	virtual JResult	PostLoadTexture( uint32 &texture, void *data, int dwColorsPerPixel, bool bIsBMP, int dwImageWidth, int dwImageHeight, int dwCellWidth, int dwCellHeight ) { return 0; };

	// Configuration
	virtual int GetScreenWidth() const = 0;
	virtual int GetScreenHeight() const = 0;

	// Text inset: number of character cells to inset text from bounding box edges.
	// ASCII mode returns 1 because box-drawing chars occupy cells; OpenGL returns 0
	// because the bounding box is a translucent quad behind the text.
	virtual int GetTextInset() const { return 0; }
};

#endif // __RENDERBASE_H__
