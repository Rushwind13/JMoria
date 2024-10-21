#ifndef __TILESET_H__
#define __TILESET_H__

#include "JMDefs.h"

class CTileset
{
	// Member Functions
public:
	CTileset() {}
	CTileset( const char *szName, int cellWidth, int cellHeight ) { Load( szName, cellWidth, cellHeight ); }

	bool IsBMP( const char *szName ) { return false; }

	float TexelW() { return m_vTexels.x; }
	float TexelH() { return m_vTexels.y; }

	JResult Load( const char *szName, int cellwidth, int cellheight );

#ifdef RENDER_TILESET_POSTLOAD_NEEDED
	uint32 Texture() { return m_Texture; }
#endif // postload needed

	JIVector &GetTile( int dwIndex, JIVector &vTile )
	 {
		/*if( index >= TILE_IDX_NUMTILES )
		{
			return TILE_IDX_INVALID;
		}/**/

		vTile.x =  dwIndex % m_dwTilesPerRow;
		vTile.y = dwIndex / m_dwTilesPerRow;

		return( vTile );
	}

	void PreDrawTile();
	void PostDrawTile();
	void SetTileColor( JColor color );
	bool DrawTile( int dwIndex, const JFVector &vPos, JVector &vSize, bool bIsTextured );
protected:
private:


	// Member Variables
public:
protected:
#ifdef RENDER_TILESET_POSTLOAD_NEEDED
	uint32 m_Texture; // created with glBindTexture();glTexImage2D(); use with glBindTexture();
#endif // postload needed
	JFVector m_vTexels;
	int m_dwTilesPerRow; // = Texture Width / Cell Width
private:
};

#endif // __TILESET_H__