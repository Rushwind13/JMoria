#ifndef __DUNGEONMAP_H__
#define __DUNGEONMAP_H__
#include "jmdefs.h"
#include "jlinklist.h"

// CDungeonMapTile:
// the info needed for map generation
// and display of the map, one per tile
// on the current level.
class CDungeonMapTile
{
	
	// Member variables
	
public:
	CDungeonMapTile():
	  m_uiType(DUNG_IDX_INVALID),
	  m_Rect(),
	  m_dwFlags(0)
	  {};
	~CDungeonMapTile() {};

	Uint8	GetType() const { return m_uiType; }; 
	void	SetType(Uint8 type) { m_uiType = type; };
	
	int		GetFlags() const { return m_dwFlags; }; 
	void	SetFlags(int type) { m_dwFlags = type; }; 
	
protected:
private:
	Uint8	m_uiType;	// what kind of tile is this? (floor, wall, door, etc)
	JRect	m_Rect;		// What are this tiles coordinates in world space? (ltrb)
	int		m_dwFlags;	// 32 bits to use as you please. Please reference here what's using them (or the .h)
	
	// Member functions
	
public:
	void SetLink( CLink <CDungeonMapTile> *link ) { m_pllLink = link; };
protected:
private:
	CLink <CDungeonMapTile> *m_pllLink; // My link into the Used Space list (or another list)	
};

// CDungeonMap:
// holder class for dungeon generation
// algorithm.
class CDungeonMap
{
	// Member variables
public:
	CDungeonMap():
	  m_dmtTiles(NULL)
	{};
	~CDungeonMap() {};
	
protected:
private:
	CDungeonMapTile	*m_dmtTiles;	// 1 DungeonMapTile per tile, has "graphical" info (size, location, type, ...)
	JLinkList<CDungeonMapTile> *m_llUsedSpace;
	
	// Member functions
public:
	void CreateDungeon(const int depth);
	Uint8 GetdtdIndex( JIVector vPos )
	{ 
		if( GetTile(vPos) )
		{
			return GetTile(vPos)->GetType();
		}
		return (Uint8)-1;
	};

	int GetFlags( JIVector vPos )
	{ 
		if( GetTile(vPos) )
		{
			return GetTile(vPos)->GetFlags();
		}
		return -1;
	};
protected:
	int CheckArea( const JRect *rcCheck, const int direction, bool bIsHallway );
	void FillArea( const Uint8 type, const JRect *rcFill, const int direction, JIVector *vOrigin, bool bIsHallway );
	
	void MakeRoom( const JIVector *vPos, const int direction, const int recurdepth );
	void MakeHall( const JIVector *vPos, const int direction, const int recurdepth );

	CDungeonMapTile	*GetTile(JIVector vPos)
	{
		if(	m_dmtTiles == NULL 
			||	vPos.x < 0 || vPos.y < 0
			|| vPos.y > DUNG_HEIGHT || vPos.x > DUNG_WIDTH )
		{
			return NULL;
		}

		return &m_dmtTiles[vPos.y * DUNG_WIDTH + vPos.x];
	}
private:
	
};
#endif // __DUNGEONMAP_H__
