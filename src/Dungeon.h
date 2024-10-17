// Dungeon.h

#ifndef __DUNGEON_H__
#define __DUNGEON_H__

#include "JLinkList.h"
#include "DungeonTile.h"
#include "Monster.h"
#include "Item.h"
#include "TileSet.h"
#include "DungeonMap.h"

//class CMonster;
//class CItem;

//#define DUNG_FONT_COURIER
#ifdef DUNG_FONT_COURIER
#define DUNG_TILESET CTileset("Resources/Courier.png", 32, 32 )
#define DUNG_ASPECT vSize(1.0f,1.0f)
#else
#define DUNG_TILESET CTileset("Resources/SmallText6X8.png", 6,8 )
#define DUNG_ASPECT vSize(0.75f,1.0f)
#endif

#define DUNG_ZOOM_MIN		4
#define DUNG_ZOOM_NORMAL	20
#define DUNG_ZOOM_MAX		100

class CDungeon
{
	// Member Variables
public:
	int m_dwHeight;
	int m_dwWidth;
	CDungeonTile	*m_Tiles;
	int depth; // dungeon level / 50ft.
	CTileset *m_TileSet;
	JLinkList<CMonster> *m_llMonsters;
	JLinkList<CItem> *m_llItems;
protected:
    CDungeonTileDef *m_dtdlist;
    JLinkList <CMonsterDef> *m_llMonsterDefs;
    JLinkList <CItemDef> *m_llItemDefs;
private:
	Uint16 m_dwZoom;
    bool m_bDraw;

	// Member Functions
public:
	CDungeon():
	  m_Tiles(NULL),
	  m_dwWidth(0),
	  m_dwHeight(0),
	  depth(0),
	  //m_Tileset(NULL),
	  m_dtdlist(NULL),
	  m_Rect( -DUNG_WIDTH, DUNG_HEIGHT, DUNG_WIDTH, -DUNG_HEIGHT ),
	  m_vfTranslate( (int)(-DUNG_WIDTH*0.5f), (int)(-DUNG_HEIGHT*0.5f) ),
	  m_dwZoom(DUNG_ZOOM_NORMAL),
	  //m_dmTownLevel(NULL),
	  m_dmCurLevel(NULL)
	  {};
	~CDungeon() { Term(); };
	void PreDraw()	;
	void Draw()	;
    void DrawDungeon();
    void DrawItems();
    void DrawMonsters();
	void PostDraw()	;
	void Init();
	void Term();
    bool Tick( const int dwClock );
	bool Update( float fCurTime );
    JResult OnChangeLevel(const int delta);

	void Zoom( Uint16 dwDelta )
	{
		m_dwZoom += dwDelta;
		if( m_dwZoom < DUNG_ZOOM_MIN )
		{
			m_dwZoom = DUNG_ZOOM_MIN;
		}
		else if ( m_dwZoom > DUNG_ZOOM_MAX )
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
		return (m_Tiles + (vPos.y * DUNG_WIDTH) + vPos.x ); // going to have to work in offsets, too, if the dungeon's bigger than the screen. --Jimbo
	};
	CDungeonTile *GetTile( const JVector &vPos )
	{
		JIVector checkPos((int)vPos.x, (int)vPos.y);
		if( !checkPos.IsInWorld() )
		{
			return NULL;
		}
		// casting to int is *absolutely* the wrong way to do this. fix this once it inevitably breaks.
		return (m_Tiles + ((int)vPos.y * DUNG_WIDTH) + (int)vPos.x ); // going to have to work in offsets, too, if the dungeon's bigger than the screen. --Jimbo
	};
	int IsWalkableFor( JVector &vPos, bool isPlayer=false );
    int CanPlaceItemAt( JVector &vPos );
    int CanPlaceStairsAt( JVector &vPos );
	bool IsOpenable( JVector &vPos );
	bool IsTunnelable( JVector &vPos );
	bool IsCloseable( JVector &vPos );
	int  IsStairs( JVector &vPos );
	void RemoveMonster( CMonster *pMon );
	JResult Modify( JVector &vPos );
	CItem *PickUp( JVector &vPickupPos );
	void Drop( CItem *pItem, JVector &vDropPos );
protected:
	JRect m_Rect;
	JFVector m_vfTranslate;
	CDungeonMap *m_dmCurLevel;		// Data about the curennt level (and holds dungeon gen algorithm)
	float m_fOpenFloorArea; // How much of the current level is open?
	void extracted();

	//CDungeonMap m_dmTownLevel;	// The Dungeon remembers where you live...

	JResult CreateNewLevel(const int delta);
	JResult CreateMap();
    JResult InitDungeonTiles();
    JResult PlaceScenery(const int depth);
    JResult PlaceStairs(const int desired, const int type);
	JResult PlaceItems(const int depth);
	JResult SpawnMonsters(const int depth);
	int ChooseMonsterForDepth(const int depth);
	int ChooseItemForDepth(const int depth);

	JResult TerminateLevel();
private:

	bool IsOnScreen(JVector vPos);
};
#endif // __DUNGEON_H__
