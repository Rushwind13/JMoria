#ifndef __DUNGEONMAP_H__
#define __DUNGEONMAP_H__
#include "JMDefs.h"
#include "JLinkList.h"

#define DUNG_CREATE_STEP_INVALID -1
#define DUNG_CREATE_STEP_MAKE_ROOM 0
#define DUNG_CREATE_STEP_MAKE_HALLWAY 1
#define DUNG_CREATE_STEP_MAX 2

#define MAX_RECURDEPTH 10

#define DIR_NONE    -1
#define DIR_NORTH     0
#define DIR_SOUTH     1
#define DIR_WEST     2
#define DIR_EAST     3

#define DUNG_HALL_MINLENGTH    2
#define DUNG_HALL_MAXLENGTH    8
class CDungeonCreationStep
{
public:
    CDungeonCreationStep():
    m_dwIndex(DUNG_CREATE_STEP_INVALID),
    m_dwDirection(DIR_NONE),
    m_dwRecurDepth(MAX_RECURDEPTH)
    {
        m_vPos.Init();
        m_rcArea.Init(0,0,0,0);
        m_pdwVisited = new bool[4];
        memset(m_pdwVisited, false, 4);
    };
    ~CDungeonCreationStep(){};
    
    int m_dwIndex;
    int m_dwDirection;
    int m_dwRecurDepth;
    JIVector m_vPos;
    JRect m_rcArea;
    bool *m_pdwVisited;
};
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
	  m_dmtTiles(NULL),
      m_llUsedSpace(NULL),
      m_stkDungeonMapCreation(NULL)
	{
        m_stkDungeonMapCreation = new JStack<CDungeonCreationStep>;
    };
	~CDungeonMap()
    {
        Term();
    };
	
protected:
    void Term()
    {
        if( m_dmtTiles )
        {
            delete [] m_dmtTiles;
            m_dmtTiles = NULL;
        }
    };
private:
	CDungeonMapTile	*m_dmtTiles;	// 1 DungeonMapTile per tile, has "graphical" info (size, location, type, ...)
	JLinkList<CDungeonMapTile> *m_llUsedSpace;
    JStack<CDungeonCreationStep> *m_stkDungeonMapCreation;
	
	// Member functions
public:
	void CreateDungeon(const int depth);
    void InitDungeonCreate( JIVector &vOrigin );
    bool CreateOneStep();
    int Opposite( int direction );
    CDungeonCreationStep *MakeRoomStep( const JIVector &vPos, const int direction, const int recurdepth );
    CDungeonCreationStep *MakeHallStep( const JIVector &vPos, const int direction, const int recurdepth );
    void GetRoomRect(JRect &rcRoom, const int direction);
    void GetHallRect(JRect &rcHall, const int direction);
    JIVector &GetWallOrigin(CDungeonCreationStep *pStep, const int direction);
    JIVector &GetHallOrigin(CDungeonCreationStep *pStep);
    
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
public:
	int CheckArea( const JRect *rcCheck, const int direction, bool bIsHallway );
    bool CheckArea( CDungeonCreationStep *pStep );
    bool CheckInterior( const JRect rcCheck );
    bool CheckBorder( const JRect rcCheck, int direction );

	void FillArea( const Uint8 type, JRect *rcFill, const int direction, bool bIsHallway );
    void FillArea( const CDungeonCreationStep *pStep );
	void AddDoor(JIVector vHall, int direction );
	
	void MakeRoom( const JIVector *vPos, const int direction, const int recurdepth );
	void MakeHall( const JIVector *vPos, const int direction, const int recurdepth );

	CDungeonMapTile	*GetTile(JIVector vPos)
	{
		if(	m_dmtTiles == NULL || !vPos.IsInWorld() )
		{
			return NULL;
		}

		return &m_dmtTiles[vPos.y * DUNG_WIDTH + vPos.x];
	}
private:
};
#endif // __DUNGEONMAP_H__
