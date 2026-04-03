#include "TileSet.h"

#ifdef RENDER_TILESET_POSTLOAD_NEEDED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#endif

#include "RenderBase.h"

JResult CTileset::Load( const char *szName, int dwCellWidth, int dwCellHeight )
{
#ifdef RENDER_TILESET_POSTLOAD_NEEDED
    SDL_Surface *TextureImage;
    JResult retval = JSUCCESS;

    bool bIsBMP = false;

    if( IsBMP( szName ) )
    {
        TextureImage = SDL_LoadBMP( szName );
        bIsBMP = true;
    }
    else
    {
        JLog( LOG_LEVEL_DEBUG, true, "loading %s\n", szName );
        TextureImage = IMG_Load( szName );
    }

    // break out if you couldn't load this image
    if( !TextureImage )
    {
        return JERROR();
    }

    // Turn this image into an OpenGL texture
    if( g_pGame )
    {
        retval = g_pGame->GetRender()->PostLoadTexture( m_Texture, TextureImage->pixels, 4, bIsBMP,
                                                        TextureImage->w, TextureImage->h,
                                                        dwCellWidth, dwCellHeight );
    }
    else
    {
        JLog( LOG_LEVEL_WARN, true,
              "g_pGame not initialized yet, when loading %s -- m_Texture not created.\n", szName );
    }

    m_dwTilesPerRow = TextureImage->w / dwCellWidth;
    m_vTexels.x = 1.0f / (float)m_dwTilesPerRow;
    m_vTexels.y = (float)dwCellHeight / (float)TextureImage->h;

    // destroy the temporary surface
    if( TextureImage )
    {
        SDL_FreeSurface( TextureImage );
    }
    return retval;
#else
    // ASCII-only: no image to load.
    // Compute grid metrics from the known sprite sheet layout.
    // SmallText6X8.png is 570x8 (95 chars), Courier.png is 960x128 (30 per row).
    // In ASCII mode we only need m_dwTilesPerRow and m_vTexels for DrawChar→DrawTile conversion,
    // but since the ASCII renderer bypasses tile coords entirely, these are best-effort defaults.
    m_dwTilesPerRow = 95;
    m_vTexels.x = 1.0f / (float)m_dwTilesPerRow;
    m_vTexels.y = 1.0f;
    return JSUCCESS;
#endif
}

bool CTileset::DrawTile( int dwIndex, const JFVector &vPos, JVector &vSize, bool bIsTextured )
{
    /*if( !IsInWorld( vPos ) )
    {
            return false;
    }/**/

    JIVector vTile;
    GetTile( dwIndex, vTile );

    // if( bIsTextured )
    {
        g_pGame->GetRender()->DrawTile( vPos, vSize, vTile, m_vTexels );
    }
    // else
    //{
    // 	g_pGame->GetRender()->DrawTile( vPos, vSize, vTile );
    //}
    return true;
}

bool CTileset::DrawChar( char ch, const JFVector &vPos, JVector &vSize )
{
    g_pGame->GetRender()->DrawChar( vPos, vSize, ch );
    return true;
}

void CTileset::PreDrawTile()
{
    g_pGame->GetRender()->SetTileMetrics( m_dwTilesPerRow, m_vTexels );
    g_pGame->GetRender()->PreDrawTile();
}

void CTileset::PostDrawTile() { g_pGame->GetRender()->PostDrawTile(); }

void CTileset::SetTileColor( JColor color ) { g_pGame->GetRender()->SetTileColor( color ); }
