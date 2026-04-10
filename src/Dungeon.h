// Dungeon.h

#ifndef __DUNGEON_H__
#define __DUNGEON_H__

#include "DungeonMap.h"
#include "DungeonTile.h"
#include "Item.h"
#include "JLinkList.h"
#include "Monster.h"
#include "TileSet.h"

// class CMonster;
// class CItem;

// #define DUNG_FONT_COURIER
#ifdef DUNG_FONT_COURIER
#define DUNG_TILESET CTileset( szBasedir, "Resources/Courier.png", 32, 32 )
#define DUNG_ASPECT vSize( 1.0f, 1.0f )
#else
#define DUNG_TILESET CTileset( szBasedir, "Resources/SmallText6X8.png", 6, 8 )
#define DUNG_ASPECT vSize( 0.75f, 1.0f )
#endif

#define DUNG_ZOOM_MIN 4
#define DUNG_ZOOM_NORMAL 20
#define DUNG_ZOOM_MAX 100

class CDungeon
{
    // Member Variables
public:
    int m_dwHeight;
    int m_dwWidth;
    CDungeonTile *m_Tiles;
    int depth; // dungeon level / 50ft.
    CTileset *m_TileSet;
    JLinkList<CMonster> *m_llMonsters;
    JLinkList<CItem> *m_llItems;
    JLinkList<JIVector> *m_llOpenArea;

protected:
    CDungeonTileDef *m_dtdlist;
    JLinkList<CMonsterDef> *m_llMonsterDefs;
    JLinkList<CItemDef> *m_llItemDefs;
    JVector m_vLookPos;
    JVector m_vProjectilePos;
    JLinkList<JIVector> *m_llLOSLine;

private:
    Uint16 m_dwZoom;
    bool m_bDraw;

    // Member Functions
public:
    CDungeon()
        : m_Tiles( NULL ),
          m_dwWidth( 0 ),
          m_dwHeight( 0 ),
          depth( 0 ),
          // m_Tileset(NULL),
          m_dtdlist( NULL ),
          m_Rect( -DUNG_WIDTH, DUNG_HEIGHT, DUNG_WIDTH, -DUNG_HEIGHT ),
          m_vfTranslate( (int)( -DUNG_WIDTH * 0.5f ), (int)( -DUNG_HEIGHT * 0.5f ) ),
          m_dwZoom( DUNG_ZOOM_NORMAL ),
          m_vLookPos( (int)( -DUNG_WIDTH * 0.5f ), (int)( -DUNG_HEIGHT * 0.5f ) ),
          // m_dmTownLevel(NULL),
          m_llItems( NULL ),
          m_llMonsters( NULL ),
          m_llOpenArea( NULL ),
          m_llItemDefs( NULL ),
          m_llMonsterDefs( NULL ),
          m_llLOSLine( NULL ),
          m_dmCurLevel( NULL ) {};
    ~CDungeon() { Term(); }
    void DumpMap();
    void PreDraw();
    void Draw();
    void DrawDungeon();
    void DrawItems();
    void DrawMonsters();
    void PostDraw();
    void Init( const char *szBasedir );
    void Term();
    bool Tick( const int dwClock );
    bool Update( float fCurTime );
    JResult UpdateSeen();
    void LightRoom( CRoom *pRoom );
    bool CanSeeEachOther( JIVector vSource, JIVector vTarget, uint32 dwFlags = 0 );
    bool PlayerCanSee( JVector vCheck, uint32 dwFlags = 0 );
    void DisturbPlayer();

    void SetLookPosition( JVector vNewPos ) { m_vLookPos.Init( VEC_EXPAND( vNewPos ) ); }
    JVector GetLookPosition() { return m_vLookPos; }

    void SetProjectilePosition( JVector vNewPos )
    {
        m_vProjectilePos.Init( VEC_EXPAND( vNewPos ) );
    }
    JVector GetProjectilePosition() { return m_vProjectilePos; }

    void SetLOSLine( JLinkList<JIVector> *pLine )
    {
        ClearLOSLine();
        m_llLOSLine = pLine;
    }
    void ClearLOSLine()
    {
        if( m_llLOSLine )
        {
            m_llLOSLine->Terminate();
            delete m_llLOSLine;
            m_llLOSLine = NULL;
        }
    }
    bool IsOnLOSLine( JVector vPos );

    JResult OnChangeLevel( const int delta );

    void Zoom( Uint16 dwDelta )
    {
        m_dwZoom += dwDelta;
        if( m_dwZoom < DUNG_ZOOM_MIN )
        {
            m_dwZoom = DUNG_ZOOM_MIN;
        }
        else if( m_dwZoom > DUNG_ZOOM_MAX )
        {
            m_dwZoom = DUNG_ZOOM_MAX;
        }
    }

    CDungeonTile *GetITile( JIVector &vPos )
    {
        if( !vPos.IsInWorld() )
        {
            return NULL;
        }
        return ( m_Tiles + ( vPos.y * DUNG_WIDTH ) +
                 vPos.x ); // going to have to work in offsets, too, if the dungeon's bigger than
                           // the screen. --Jimbo
    };
    CDungeonTile *GetTile( const JVector &vPos )
    {
        JIVector checkPos( (int)vPos.x, (int)vPos.y );
        if( !checkPos.IsInWorld() )
        {
            return NULL;
        }
        // casting to int is *absolutely* the wrong way to do this. fix this once it inevitably
        // breaks.
        return ( m_Tiles + ( (int)vPos.y * DUNG_WIDTH ) +
                 (int)vPos.x ); // going to have to work in offsets, too, if the dungeon's bigger
                                // than the screen. --Jimbo
    };
    JIVector *AnyOpenTile()
    {
        return m_llOpenArea->GetNthLink( Util::GetRandom( 0, m_llOpenArea->length() - 1 ) )
            ->m_lpData;
    };
    int IsWalkableFor( JVector &vPos, bool isPlayer = false );
    int CanPlaceItemAt( JVector &vPos );
    int CanPlaceStairsAt( JVector &vPos );
    bool IsOpenable( JVector &vPos );
    bool IsTunnelable( JVector &vPos );
    bool IsCloseable( JVector &vPos );
    int IsStairs( JVector &vPos );
    CRoom *InRoom( JVector &vPos );
    CDungeonMap *GetCurLevel() { return m_dmCurLevel; }
    CMonsterDef *GetMonsterDef( const char *szMonsterName );
    CMonsterDef *GetMonsterDef( int which_monster );
    CItemDef *GetItemDef( const char *szItemName );
    CItemDef *GetItemDef( int which_item );
    bool SpawnMonster( int which_monster );
    void RemoveMonster( CMonster *pMon );
    JResult Modify( JVector &vPos );
    CItem *PickUp( JVector &vPickupPos );
    void Drop( CItem *pItem, JVector &vDropPos );
    void PopulateLevel( const int depth ); // Place scenery, items, and monsters
    void SetDrawFlag( bool bDraw ) { m_bDraw = bDraw; }

protected:
    JRect m_Rect;
    JFVector m_vfTranslate;
    CDungeonMap *m_dmCurLevel; // Data about the curennt level (and holds dungeon gen algorithm)
    float m_fOpenFloorArea;    // How much of the current level is open?
    void extracted();

    // CDungeonMap m_dmTownLevel;	// The Dungeon remembers where you live...

    JResult CreateNewLevel( const int delta );
    JResult CreateMap();
    JResult InitDungeonTiles();
    JResult PlaceScenery( const int depth );
    JResult PlaceStairs( const int desired, const int type );
    JResult PlaceItems( const int depth );
    JResult SpawnMonsters( const int depth );
    int ChooseMonsterForDepth( const int depth );
    int ChooseItemForDepth( const int depth );

    JResult TerminateLevel();
#ifdef UNIT_TEST
public:
#else
private:
#endif // UNIT_TEST
    bool IsOnScreen( JVector vPos );
    bool IsLit( JVector vPos );
};
#endif // __DUNGEON_H__
