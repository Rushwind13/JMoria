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

// Maximum consecutive failures before abandoning a creation step to prevent "tails out"
#define MAX_STEP_FAILURES 3
class CDungeonCreationStep
{
public:
    CDungeonCreationStep()
        : m_dwIndex( DUNG_CREATE_STEP_INVALID ),
          m_dwDirection( DIR_NONE ),
          m_dwRecurDepth( MAX_RECURDEPTH ),
          m_dwFailureCount( 0 )
    {
        m_vPos.Init();
        m_rcArea.Init( 0, 0, 0, 0 );
        m_pdwVisited = new bool[4];
        memset( m_pdwVisited, false, 4 );
    };
    ~CDungeonCreationStep()
    {
        if( m_pdwVisited )
        {
            delete[] m_pdwVisited;
            m_pdwVisited = NULL;
        }
    }

    int m_dwIndex;
    int m_dwDirection;
    int m_dwRecurDepth;
    int m_dwFailureCount;  // Track consecutive failed attempts
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

// Dungeon generation diagnostics structure
struct DungeonGenDiagnostics
{
    int steps_created;        // Total creation steps processed
    int rooms_created;        // Successful room steps
    int hallways_created;     // Successful hallway steps
    int steps_skipped;        // Steps that failed creation (recursion depth, conflicts)
    int fill_operations;      // Total FillArea calls
    int conflicts_detected;   // Conflicts found during CheckArea
    int repeated_failures;    // Steps abandoned due to repeated failures (tails out prevention)
    double start_time_ms;     // Generation start time in milliseconds
    double end_time_ms;       // Generation end time in milliseconds
    double total_time_ms;     // Total generation time in milliseconds
    
    DungeonGenDiagnostics() : steps_created(0), rooms_created(0), hallways_created(0), 
                              steps_skipped(0), fill_operations(0), conflicts_detected(0),
                              repeated_failures(0), start_time_ms(0.0), end_time_ms(0.0),
                              total_time_ms(0.0) {}
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
          m_dwDepth( 0 ),
          m_dwSeed( 0 )
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
    unsigned int m_dwSeed;
    DungeonGenDiagnostics m_diagnostics;

    // Member functions
public:
    void CreateDungeon( const int depth );
    void CreateDungeon( const int depth, const unsigned int seed );
    
    // Diagnostic accessors
    int GetStackSize() const { return m_stkDungeonMapCreation ? m_stkDungeonMapCreation->length() : 0; }
    int GetRoomCount() const { return m_llRooms ? m_llRooms->length() : 0; }
    int GetHallwayCount() const { return m_llHallways ? m_llHallways->length() : 0; }
    unsigned int GetSeed() const { return m_dwSeed; }
    const DungeonGenDiagnostics& GetDiagnostics() const { return m_diagnostics; }
    
    // Fixture management for regression testing
    bool ExportDungeon( const char *pszFilename ) const;
    bool ImportDungeon( const char *pszFilename );
    bool CompareDungeon( const CDungeonMap &other ) const;
    
    void InitDungeonCreate( JIVector &vOrigin );
    bool CreateOneStep();
    int Opposite( int direction );
    void GetAdjacentDirections( int primary_dir, int &adj1, int &adj2 ) const;
    CDungeonCreationStep *MakeRoomStep( const JIVector &vPos, const int direction,
                                        const int recurdepth );
    CDungeonCreationStep *MakeHallStep( const JIVector &vPos, const int direction,
                                        const int recurdepth );
    JResult GetRoomRect( JRect &rcRoom, const int direction );
    JResult GetHallRect( JRect &rcHall, const int direction );
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
    
    // Connectivity validation
    bool ValidateConnectivity( int &reachable_tiles, int &total_walkable_tiles ) const;
    bool ValidateAllRoomsReachable() const;

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
