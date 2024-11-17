#ifndef __DUNGEONMAP_H__
#define __DUNGEONMAP_H__
#include "JLinkList.h"
#include "JMDefs.h"

#define DUNG_CREATE_STEP_INVALID -1
#define DUNG_CREATE_STEP_MAKE_ROOM 0
#define DUNG_CREATE_STEP_MAKE_HALLWAY 1
#define DUNG_CREATE_STEP_MAX 2

#define MAX_RECURDEPTH 10

#define DUNG_HALL_MINLENGTH 2
#define DUNG_HALL_MAXLENGTH 8
class CDungeonCreationStep
{
public:
    CDungeonCreationStep()
        : m_dwIndex( DUNG_CREATE_STEP_INVALID ),
          m_dwDirection( DIR_NONE ),
          m_dwRecurDepth( MAX_RECURDEPTH )
    {
        m_vPos.Init();
        m_rcArea.Init( 0, 0, 0, 0 );
        m_pdwVisited = new bool[4];
        memset( m_pdwVisited, false, 4 );
    };
    ~CDungeonCreationStep() {}

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
    CDungeonMapTile() : m_uiType( DUNG_IDX_INVALID ), m_Rect(), m_dwFlags( 0 ) {};
    ~CDungeonMapTile() {}

    Uint8 GetType() const { return m_uiType; }
    void SetType( Uint8 type ) { m_uiType = type; }

    uint32 GetFlags() const { return m_dwFlags; }
    uint32 HasFlags( const uint32 type ) { return m_dwFlags & type; }
    void SetFlags( const uint32 type ) { m_dwFlags |= type; }
    void UnsetFlags( const uint32 type ) { m_dwFlags &= ~type; }

protected:
private:
    Uint8 m_uiType;   // what kind of tile is this? (floor, wall, door, etc)
    JRect m_Rect;     // What are this tiles coordinates in world space? (ltrb)
    uint32 m_dwFlags; // 32 bits to use as you please. Please reference here what's using them (or
                      // the .h)

    // Member functions

public:
    void SetLink( CLink<CDungeonMapTile> *link ) { m_pllLink = link; }

protected:
private:
    CLink<CDungeonMapTile> *m_pllLink; // My link into the Used Space list (or another list)
};

class CRoom
{
    // Methods
public:
    CRoom( const JRect rcIn ) : m_dwFlags( 0 ), m_rcRoom( rcIn ) { Init(); };
    CRoom() : m_dwFlags( 0 ), m_rcRoom( 0, 0, 0, 0 ), m_rcEdges( 0, 0, 0, 0 ) {};
    ~CRoom() {};

    JResult Init()
    {
        m_rcEdges = Util::Edges( m_rcRoom );
        return JSUCCESS;
    };
    JRect GetArea() { return m_rcRoom; };
    JRect GetEdges() { return m_rcEdges; };
    uint32 GetFlags() { return m_dwFlags; };
    uint32 HasFlags( const uint32 dwIn ) { return m_dwFlags & dwIn; };
    void SetFlags( const uint32 dwIn ) { m_dwFlags |= dwIn; };
    void UnsetFlags( const uint32 dwIn ) { m_dwFlags &= ~dwIn; };

protected:
    JRect m_rcRoom;
    JRect m_rcEdges;
    uint32 m_dwFlags;
    // does the room know it has monsters?
    // does the room know it has items?
    // does the room know it is a treasure room?
    // does the room know if it has stairs?
    // can a room have a max number of staris/items/monsters/traps/doors/...
private:
    // Member variables
public:
protected:
private:
};

// CDungeonMap:
// holder class for dungeon generation
// algorithm.
class CDungeonMap
{
    // Member variables
public:
    CDungeonMap()
        : m_dmtTiles( NULL ),
          m_stkDungeonMapCreation( NULL ),
          m_llRooms( NULL ),
          m_llHallways( NULL ),
          m_dwDepth( 0 )
    {
        m_stkDungeonMapCreation = new JStack<CDungeonCreationStep>;
    };
    ~CDungeonMap() { Term(); };

protected:
    void Term()
    {
        if( m_dmtTiles )
        {
            delete[] m_dmtTiles;
            m_dmtTiles = NULL;
        }
        if( m_llRooms )
        {
            m_llRooms->Terminate();
            delete m_llRooms;
            m_llRooms = NULL;
        }
        if( m_llHallways )
        {
            m_llHallways->Terminate();
            delete m_llHallways;
            m_llHallways = NULL;
        }
    };

private:
    CDungeonMapTile
        *m_dmtTiles; // 1 DungeonMapTile per tile, has "graphical" info (size, location, type, ...)
    // JLinkList<CDungeonMapTile> *m_llUsedSpace;
    JStack<CDungeonCreationStep> *m_stkDungeonMapCreation;
    JLinkList<CRoom> *m_llRooms;
    JLinkList<CRoom> *m_llHallways;
    uint32 m_dwDepth;

    // Member functions
public:
    void CreateDungeon( const int depth );
    void InitDungeonCreate( JIVector &vOrigin );
    bool CreateOneStep();
    int Opposite( int direction );
    CDungeonCreationStep *MakeRoomStep( const JIVector &vPos, const int direction,
                                        const int recurdepth );
    CDungeonCreationStep *MakeHallStep( const JIVector &vPos, const int direction,
                                        const int recurdepth );
    void GetRoomRect( JRect &rcRoom, const int direction );
    void GetHallRect( JRect &rcHall, const int direction );
    JIVector &GetWallOrigin( CDungeonCreationStep *pStep, const int direction );
    JIVector &GetHallOrigin( CDungeonCreationStep *pStep,
                             int step_type = DUNG_CREATE_STEP_MAKE_ROOM );

    Uint8 GetdtdIndex( JIVector vPos )
    {
        if( GetTile( vPos ) )
        {
            return GetTile( vPos )->GetType();
        }
        return (Uint8)-1;
    };

    int GetFlags( JIVector vPos )
    {
        if( GetTile( vPos ) )
        {
            return GetTile( vPos )->GetFlags();
        }
        return -1;
    };

    CRoom *InRoom( JIVector vCheck )
    {
        if( !m_llRooms )
            return NULL;
        CLink<CRoom> *pLink = m_llRooms->GetHead();

        if( pLink == NULL )
        {
            return NULL;
        }

        while( pLink != NULL )
        {
            if( pLink->m_lpData->GetArea().Contains( vCheck ) )
            {
                JLog( LOG_LEVEL_DEBUG, true, "<%d %d> Inside room: <%d %d %d %d>\n",
                      VEC_EXPAND( vCheck ), RECT_EXPAND( pLink->m_lpData->GetArea() ) );
                return pLink->m_lpData;
            }
            pLink = m_llRooms->GetNext( pLink );
        }
        return NULL;
    }

    int HowManyRooms()
    {
        if( m_llRooms )
            return m_llRooms->length();
        else
            return 0;
    };

    int HowManyHallways()
    {
        if( m_llHallways )
            return m_llHallways->length();
        else
            return 0;
    };

#ifdef UNIT_TEST
public:
#else
protected:
#endif
    int CheckArea( const JRect *rcCheck, const int direction, bool bIsHallway );
    bool CheckArea( CDungeonCreationStep *pStep );
    bool CheckInterior( const JRect rcCheck );
    bool CheckBorder( const JRect rcCheck, int direction );

    int LitChance();
    JResult LightArea( CRoom *pRoom );
    JResult FillDungeonArea( Uint8 type, JRect rcFill, bool bBoundsCheck = true );
    void FillArea( const Uint8 type, CRoom *pRoom );
    void FillArea( const CDungeonCreationStep *pStep );
    void AddDoor( JIVector vHall, int direction );
    bool IsDoor( const int type );

    void MakeRoom( const JIVector *vPos, const int direction, const int recurdepth );
    void MakeHall( const JIVector *vPos, const int direction, const int recurdepth );

    CDungeonMapTile *GetTile( JIVector vPos )
    {
        if( m_dmtTiles == NULL || !vPos.IsInWorld() )
        {
            return NULL;
        }

        return &m_dmtTiles[vPos.y * DUNG_WIDTH + vPos.x];
    }

private:
};
#endif // __DUNGEONMAP_H__
