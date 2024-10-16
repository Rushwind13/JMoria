#include "TileSet.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include "Render.h"

JResult CTileset::Load( const char *szName, int dwCellWidth, int dwCellHeight )
{
	SDL_Surface *TextureImage;
	JResult retval = JSUCCESS;

	bool bIsBMP = false;

	if( IsBMP( szName ) )
	{
		TextureImage = SDL_LoadBMP(szName);
		bIsBMP = true;
	}
	else
	{
		TextureImage = IMG_Load( szName );
	}

	// break out if you couldn't load this image
	if( !TextureImage )
	{
		return JERROR();
	}

#ifdef RENDER_TILESET_POSTLOAD_NEEDED
	// Turn this image into an OpenGL texture
	if( g_pGame )
	{
		retval = g_pGame->GetRender()->PostLoadTexture( m_Texture, TextureImage->pixels, 4, bIsBMP,
				   TextureImage->w, TextureImage->h, dwCellWidth, dwCellHeight );
	}
#endif

	m_dwTilesPerRow = TextureImage->w / dwCellWidth; 
	m_vTexels.x	= 1.0f / (float)m_dwTilesPerRow;
	m_vTexels.y	= (float)dwCellHeight / (float)TextureImage->h;

#ifdef RENDER_TILESET_POSTLOAD_NEEDED	// destroy the temporary surface
	if( TextureImage )
	{
		SDL_FreeSurface(TextureImage);
	}
#endif
	
	return retval;
}


bool CTileset::DrawTile( int dwIndex, const JFVector &vPos, JVector &vSize, bool bIsTextured )
{	
	/*if( !IsInWorld( vPos ) )
	{
		return false;
	}/**/	
	
	JIVector vTile;
	GetTile( dwIndex, vTile );
	
	//if( bIsTextured )
	{
		g_pGame->GetRender()->DrawTile( vPos, vSize, vTile, m_vTexels );
	}
	//else
	//{
	//	g_pGame->GetRender()->DrawTile( vPos, vSize, vTile );
	//}
	
	
	return true;
}

void CTileset::PreDrawTile()
{
	g_pGame->GetRender()->PreDrawTile();
}

void CTileset::PostDrawTile()
{
	g_pGame->GetRender()->PostDrawTile();
}

void CTileset::SetTileColor( JColor color )
{
	g_pGame->GetRender()->SetTileColor(color);
}
