// Dungeon.cpp
//
// implementation of the Dungeon

#include "Dungeon.h"

// TODO: this is for the collision defines; should move those someplace more useful --Jimbo
#include "CmdState.h"
#include "DisplayText.h"
#include "Effect.h"
#include "FileParse.h"
#include "Player.h"
#include "RenderBase.h"

extern unsigned char ItemIDs[];

unsigned char TileIDs[DUNG_IDX_MAX + 1] = ".#+''<<>>:#@";
int ModifiedTileTypes[DUNG_IDX_MAX + 1] = {
    DUNG_IDX_INVALID,   // 0  FLOOR: can't modify
    DUNG_IDX_INVALID,   // 1  WALL: can't modify
    DUNG_IDX_OPEN_DOOR, // 2  DOOR: opens
    DUNG_IDX_DOOR,      // 3  OPEN_DOOR: closes
    DUNG_IDX_INVALID,   // 4  BROKEN_DOOR: can't modify
    DUNG_IDX_INVALID,   // 5  UPSTAIRS: can't modify
    DUNG_IDX_INVALID,   // 6  LONG_UPSTAIRS: can't modify
    DUNG_IDX_INVALID,   // 7  DOWNSTAIRS: can't modify
    DUNG_IDX_INVALID,   // 8  LONG_DOWNSTAIRS: can't modify
    DUNG_IDX_FLOOR,     // 9  RUBBLE: tunnels to floor
    DUNG_IDX_DOOR,      // 10 SECRET_DOOR: reveals as closed door
    DUNG_IDX_INVALID,   // 11 PLAYER: can't modify
};
// extern Uint8 dungeontiles[DUNG_HEIGHT][DUNG_WIDTH];

// Setup - the one-time-run stuff to set up the Dungeon
//
// create all the dungeon tile types,
// read the monsters config data

void CDungeon::Init( const char *szBasedir )
{
    int i;
    // Initialize all the Dungeon stuff, baby.
    m_dtdlist = new CDungeonTileDef[DUNG_IDX_MAX];

    // Create the dungeon tile defs (at the moment, there are 7 or so entries in this array)
    for( i = DUNG_IDX_FLOOR; i < DUNG_IDX_MAX; i++ )
    {
        m_dtdlist[i].m_dwType = i;
        m_dtdlist[i].m_dwModifiedType = ModifiedTileTypes[i];
        m_dtdlist[i].m_chTile = TileIDs[i];
        switch( m_dtdlist[i].m_dwType )
        {
        case DUNG_IDX_FLOOR:
            m_dtdlist[i].m_Color.SetColor( 192, 192, 192, 255 );
            break;
        case DUNG_IDX_WALL:
        case DUNG_IDX_SECRET_DOOR:
            m_dtdlist[i].m_Color.SetColor( 64, 64, 64, 255 );
            break;
        case DUNG_IDX_DOOR:
            m_dtdlist[i].m_Color.SetColor( 64, 32, 128, 255 );
            break;
        case DUNG_IDX_BROKEN_DOOR:
            m_dtdlist[i].m_Color.SetColor( 100, 60, 40, 255 );
            break;
        case DUNG_IDX_OPEN_DOOR:
            m_dtdlist[i].m_Color.SetColor( 192, 192, 192, 255 );
            break;
        case DUNG_IDX_UPSTAIRS:
        case DUNG_IDX_LONG_UPSTAIRS:
            m_dtdlist[i].m_Color.SetColor( 175, 175, 175, 255 );
            break;
        case DUNG_IDX_DOWNSTAIRS:
        case DUNG_IDX_LONG_DOWNSTAIRS:
            m_dtdlist[i].m_Color.SetColor( 195, 195, 195, 255 );
            break;
        case DUNG_IDX_RUBBLE:
            m_dtdlist[i].m_Color.SetColor( 200, 100, 64, 255 );
            break;
        case DUNG_IDX_PLAYER:
            m_dtdlist[i].m_Color.SetColor( 255, 255, 255, 255 );
            break;
        default:
            m_dtdlist[i].m_Color.SetColor( 0, 0, 0, 255 );
            break;
        }
    }

    // Load the effect catalog from config (before monsters and items, so both can reference by
    // name)
    m_llEffectDefs = new JLinkList<CEffectDef>;

    CEffectDef *ped;
    CDataFile dfEffects;
    char szEffectFilename[256];
    sprintf( szEffectFilename, "%s%s", szBasedir, "Resources/Effects.txt" );
    if( !dfEffects.Open( szEffectFilename ) )
    {
        JLog( LOG_LEVEL_ERROR, true, "FATAL: Cannot open Effects.txt at: %s\n", szEffectFilename );
        exit( 1 );
    }

    ped = new CEffectDef;
    while( dfEffects.ReadEffect( *ped ) )
    {
        m_llEffectDefs->Add( ped );
        ped = new CEffectDef;
    }
    delete ped;

    // Load the monster list from config
    // TODO: Make this a method on CMonsterDef.
    m_llMonsterDefs = new JLinkList<CMonsterDef>;

    CMonsterDef *pmd;
    CDataFile dfMonsters;
    char szMonsterFile[256];
    sprintf( szMonsterFile, "%s%s", szBasedir, "Resources/Monsters.txt" );
    if( !dfMonsters.Open( szMonsterFile ) )
    {
        JLog( LOG_LEVEL_ERROR, true, "FATAL: Cannot open Monsters.txt at: %s\n", szMonsterFile );
        exit( 1 );
    }
    dfMonsters.SetDungeon( this );

    pmd = new CMonsterDef;
    while( dfMonsters.ReadMonster( *pmd ) )
    {
        m_llMonsterDefs->Add( pmd );
        pmd = new CMonsterDef;
    }

    delete pmd;

    // Load the item list from config
    // TODO: Make this a method on CItemDef.
    m_llItemDefs = new JLinkList<CItemDef>;

    CItemDef *pid;
    CDataFile dfItems;
    char szItemFilename[256];
    sprintf( szItemFilename, "%s%s", szBasedir, "Resources/Items.txt" );
    if( !dfItems.Open( szItemFilename ) )
    {
        JLog( LOG_LEVEL_ERROR, true, "FATAL: Cannot open Items.txt at: %s\n", szItemFilename );
        exit( 1 );
    }
    dfItems.SetDungeon( this );

    pid = new CItemDef;
    while( dfItems.ReadItem( *pid ) )
    {
        m_llItemDefs->Add( pid );
        pid = new CItemDef;
    }

    delete pid;

    // Load the graphics
    // Just one tile set at the moment.
    // m_TileSet = new CTileset("Resources/Courier.png", 32, 32 );
    m_TileSet = new DUNG_TILESET;

    m_bDraw = true;

#ifndef CLOCKSTEP
    OnChangeLevel( DUNG_CFG_START_LEVEL );
#endif
}

JResult CDungeon::TerminateLevel()
{
    m_bDraw = false;
    if( m_Tiles )
    {
        delete[] m_Tiles;
        m_Tiles = NULL;
    }
    if( m_dmCurLevel )
    {
        delete m_dmCurLevel;
        m_dmCurLevel = NULL;
    }
    if( m_llMonsters )
    {
        m_llMonsters->Terminate();
        delete m_llMonsters;
        m_llMonsters = NULL;
    }
    if( m_llItems )
    {
        m_llItems->Terminate();
        delete m_llItems;
        m_llItems = NULL;
    }
    if( m_llOpenArea )
    {
        m_llOpenArea->Terminate();
        delete m_llOpenArea;
        m_llOpenArea = NULL;
    }

    return JSUCCESS;
}

JResult CDungeon::CreateNewLevel( const int delta )
{
    depth += delta;
    if( depth < 0 )
        depth = 0;
    if( depth > DUNG_MAXDEPTH )
        depth = DUNG_MAXDEPTH;

    CreateMap();

#ifndef CLOCKSTEP
    // In normal mode, place scenery/items/monsters immediately after dungeon creation
    PopulateLevel( depth );
#else
    // In CLOCKSTEP mode, these will be placed after dungeon generation completes
    JLog( LOG_LEVEL_INFO, false,
          "CLOCKSTEP: Scenery/items/monsters will be placed after generation.\n" );
#endif

    char *mapStr = DumpMap();
    JLog( LOG_LEVEL_DEBUG, false, "[MAP] Dungeon map (%d rooms, %d halls):\n",
          m_dmCurLevel->HowManyRooms(), m_dmCurLevel->HowManyHallways() );
    JLog( LOG_LEVEL_DEBUG, false, "%s", mapStr );
    delete[] mapStr;

    m_bDraw = true;
    return JSUCCESS;
}

void CDungeon::PopulateLevel( const int depth )
{
    PlaceScenery( depth );
    if( depth > 0 )
    {
        PlaceItems( depth );
        SpawnMonsters( depth );
    }
    else
    {
        // Town: empty item and monster lists
        m_llItems = new JLinkList<CItem>;
        m_llMonsters = new JLinkList<CMonster>;
    }

    // Instance IDs from the previous level are no longer valid
    if( g_pGame->RecallMonster() )
        g_pGame->RecallMonster()->ResetLevelSightings();

    // Spawn the player last — they arrive on a fully populated level
    g_pGame->GetPlayer()->m_bHasSpawned = false;
    g_pGame->GetPlayer()->SpawnPlayer();
    UpdateSeen();
    if( depth <= 0 )
    {
        JLog( LOG_LEVEL_INFO, false, "You are in town.\n" );
        g_pGame->GetMsgs()->Printf( "You are in town.\n" );
    }
    else
    {
        JLog( LOG_LEVEL_INFO, false, "You pass through a one-way door, to arrive on level %d.\n",
              depth );
        g_pGame->GetMsgs()->Printf( "You pass through a one-way door, to arrive on level %d.\n",
                                    depth );
    }
}

JResult CDungeon::CreateMap()
{
    m_Tiles = new CDungeonTile[DUNG_HEIGHT * DUNG_WIDTH];
    // Create the randomized dungeon
    m_dmCurLevel = new CDungeonMap;
    m_dmCurLevel->CreateDungeon( depth );
#ifndef CLOCKSTEP
    // In normal mode, complete the dungeon immediately, retrying if too small
    int generation_attempt = 0;
    while( generation_attempt < DUNG_MAX_GENERATION_RETRIES )
    {
        while( m_dmCurLevel->ProcessStep() )
            ;

        // Town level always has exactly 1 room — skip room count check
        if( depth <= 0 )
            break;

        int rooms = m_dmCurLevel->HowManyRooms();
        if( rooms >= DUNG_MIN_ROOMS_REQUIRED )
            break;

        generation_attempt++;
        JLog( LOG_LEVEL_WARN, true,
              "[DUNGEN] Generation produced only %d rooms (need %d), retrying (%d/%d)\n", rooms,
              DUNG_MIN_ROOMS_REQUIRED, generation_attempt, DUNG_MAX_GENERATION_RETRIES );

        // Reset dungeon tiles so InitDungeonTiles doesn't see stale data
        delete m_dmCurLevel;
        m_dmCurLevel = new CDungeonMap;
        m_dmCurLevel->CreateDungeon( depth );
    }

    // Post-generation cleanup: prune dead-end hallway tiles
    int pruned = m_dmCurLevel->PruneDeadEndHallways();

    // Retrieve diagnostics with finalized timing from dungeon generation
    const DungeonGenDiagnostics &diag = m_dmCurLevel->GetDiagnostics();

    JLog( LOG_LEVEL_INFO, true, "Rooms in current level: %d\n", m_dmCurLevel->HowManyRooms() );
    JLog( LOG_LEVEL_INFO, true, "Hallways in current level: %d\n",
          m_dmCurLevel->HowManyHallways() );
    m_dmCurLevel->LogRoomCoordinates();
    if( generation_attempt > 0 )
        JLog( LOG_LEVEL_INFO, true, "[DUNGEN] Generation required %d retries\n",
              generation_attempt );
    JLog( LOG_LEVEL_INFO, true, "Generation time: %.2f ms (%.3f seconds)\n", diag.total_time_ms,
          diag.total_time_ms / 1000.0 );
    JLog( LOG_LEVEL_INFO, true, "[DUNGEN] Generation complete in %.2f ms\n", diag.total_time_ms );
    JLog( LOG_LEVEL_INFO, true, "[DUNGEN]   Steps: %d created, %d rooms, %d halls, %d skipped\n",
          diag.steps_created, diag.rooms_created, diag.hallways_created, diag.steps_skipped );
    JLog( LOG_LEVEL_INFO, true,
          "[DUNGEN]   Fill operations: %d, Conflicts: %d, Repeated failures: %d\n",
          diag.fill_operations, diag.conflicts_detected, diag.repeated_failures );
    if( diag.total_time_ms > 0.0 )
    {
        double steps_per_sec = ( diag.steps_created * 1000.0 ) / diag.total_time_ms;
        JLog( LOG_LEVEL_INFO, true, "[DUNGEN]   Performance: %.1f steps/second\n", steps_per_sec );
    }
#else
    // In CLOCKSTEP mode, dungeon will be generated step-by-step via Tick() calls
    JLog( LOG_LEVEL_INFO, true, "CLOCKSTEP: Dungeon generation ready. Press SPACE to step.\n" );
#endif

    InitDungeonTiles();

    return JSUCCESS;
}

char *CDungeon::DumpMap()
{
    extern unsigned char TileIDs[];
    extern unsigned char MonIDs[];

    // Build the full map into a temporary buffer
    char map[DUNG_HEIGHT][DUNG_WIDTH + 1];
    int minX = DUNG_WIDTH, maxX = 0, minY = DUNG_HEIGHT, maxY = 0;

    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        for( int x = 0; x < DUNG_WIDTH; x++ )
        {
            JIVector v( x, y );
            CDungeonTile *pTile = GetITile( v );
            int type = pTile && pTile->m_dtd ? pTile->m_dtd->m_dwType : DUNG_IDX_WALL;

            // Overlay monsters and items on the base map
            if( pTile && pTile->m_pCurMonster )
                map[y][x] = (char)MonIDs[pTile->m_pCurMonster->m_md->m_dwIndex];
            else if( pTile && pTile->m_pCurItem )
                map[y][x] = (char)ItemIDs[pTile->m_pCurItem->m_id->m_dwIndex];
            else if( type == DUNG_IDX_WALL )
            {
                bool show = false;
                for( int dy = -1; dy <= 1 && !show; dy++ )
                {
                    for( int dx = -1; dx <= 1 && !show; dx++ )
                    {
                        if( dx == 0 && dy == 0 )
                            continue;
                        JIVector vN( x + dx, y + dy );
                        CDungeonTile *pN = GetITile( vN );
                        if( pN && pN->m_dtd && pN->m_dtd->m_dwType != DUNG_IDX_WALL )
                            show = true;
                    }
                }
                map[y][x] = show ? '#' : ' ';
            }
            else
                map[y][x] = ( type >= 0 && type < DUNG_IDX_MAX ) ? (char)TileIDs[type] : '?';

            if( map[y][x] != ' ' )
            {
                if( x < minX )
                    minX = x;
                if( x > maxX )
                    maxX = x;
                if( y < minY )
                    minY = y;
                if( y > maxY )
                    maxY = y;
            }
        }
        map[y][DUNG_WIDTH] = '\0';
    }

    // Add a 1-tile margin
    if( minX > 0 )
        minX--;
    if( minY > 0 )
        minY--;
    if( maxX < DUNG_WIDTH - 1 )
        maxX++;
    if( maxY < DUNG_HEIGHT - 1 )
        maxY++;

    // Build output string: each trimmed row + newline
    // Worst case: (maxX-minX+2) chars per row * (maxY-minY+1) rows + null
    int rowLen = maxX - minX + 2; // content + newline
    int numRows = maxY - minY + 1;
    char *result = new char[rowLen * numRows + 1];
    char *ptr = result;

    for( int y = minY; y <= maxY; y++ )
    {
        int end = maxX;
        while( end > minX && map[y][end] == ' ' )
            end--;

        int len = end - minX + 1;
        memcpy( ptr, &map[y][minX], len );
        ptr += len;
        *ptr++ = '\n';
    }
    *ptr = '\0';

    return result;
}

void CDungeon::RevealMap( JRect rc )
{
    rc.ClampToWorld();

    for( int y = rc.top; y <= rc.bottom; y++ )
    {
        for( int x = rc.left; x <= rc.right; x++ )
        {
            JIVector v( x, y );
            CDungeonTile *pTile = GetITile( v );
            if( !pTile || !pTile->m_dtd )
                continue;
            if( pTile->m_dtd->m_dwType != DUNG_IDX_WALL )
            {
                pTile->SetFlags( DUNG_FLAG_SEEN );
            }
            else
            {
                // Reveal walls adjacent to non-wall tiles (same logic as DumpMap)
                for( int dy = -1; dy <= 1; dy++ )
                {
                    for( int dx = -1; dx <= 1; dx++ )
                    {
                        if( dx == 0 && dy == 0 )
                            continue;
                        JIVector vN( x + dx, y + dy );
                        CDungeonTile *pN = GetITile( vN );
                        if( pN && pN->m_dtd && pN->m_dtd->m_dwType != DUNG_IDX_WALL )
                        {
                            pTile->SetFlags( DUNG_FLAG_SEEN );
                            goto next_tile;
                        }
                    }
                }
            }
        next_tile:;
        }
    }
}

JResult CDungeon::InitDungeonTiles()
{
    JIVector vDungeon;
    JIVector *vOpen = new JIVector( -1, -1 );
    Uint8 dung_tile_type = DUNG_IDX_INVALID;
    int open_area = 0;

    if( m_llOpenArea == NULL )
    {
        m_llOpenArea = new JLinkList<JIVector>;
    }
    // Create the dungeon array (at the moment, there are 16*16 entries in this list)
    // Set position, Tile type, and Flags for entire dungeon
    for( vDungeon.y = 0; vDungeon.y < DUNG_HEIGHT; vDungeon.y++ )
    {
        for( vDungeon.x = 0; vDungeon.x < DUNG_WIDTH; vDungeon.x++ )
        {
            GetITile( vDungeon )->m_vPos.x = (float)vDungeon.x;
            GetITile( vDungeon )->m_vPos.y = (float)vDungeon.y;
            dung_tile_type = m_dmCurLevel->GetdtdIndex( vDungeon );
            if( dung_tile_type == DUNG_IDX_FLOOR )
            {
                vOpen = new JIVector( VEC_EXPAND( vDungeon ) );
                m_llOpenArea->Add( vOpen );
                open_area++;
            }
            GetITile( vDungeon )->m_dtd = &m_dtdlist[dung_tile_type];
            GetITile( vDungeon )->m_dwFlags = m_dmCurLevel->GetFlags( vDungeon );
        }
    }

    // delete vOpen;

    m_fOpenFloorArea = (float)open_area;
    return JSUCCESS;
}

JResult CDungeon::PlaceScenery( const int depth )
{
    // Town level: only downstairs
    if( depth <= 0 )
    {
        PlaceStairs( 1, DUNG_IDX_DOWNSTAIRS );
        PlaceStairs( 1, DUNG_IDX_LONG_DOWNSTAIRS );
        return JSUCCESS;
    }

    int upstairs = Util::GetRandom( 1, 5 );
    int long_upstairs = Util::GetRandom( 0, 2 );
    int downstairs = ( depth < DUNG_MAXDEPTH ) ? Util::GetRandom( 1, 5 ) : 0;
    int long_downstairs = ( depth < DUNG_MAXDEPTH ) ? Util::GetRandom( 0, 2 ) : 0;

    PlaceStairs( upstairs, DUNG_IDX_UPSTAIRS );
    PlaceStairs( long_upstairs, DUNG_IDX_LONG_UPSTAIRS );
    PlaceStairs( downstairs, DUNG_IDX_DOWNSTAIRS );
    PlaceStairs( long_downstairs, DUNG_IDX_LONG_DOWNSTAIRS );

    // Doors (open, closed, locked, secret, broken)
    // Traps
    // Rubble (very rare)
    // Gold Veins (only in walls, can run a couple deep)

    return JSUCCESS;
}

static const char *StairName( int type )
{
    switch( type )
    {
    case DUNG_IDX_UPSTAIRS:
        return "upstairs";
    case DUNG_IDX_LONG_UPSTAIRS:
        return "long upstairs";
    case DUNG_IDX_DOWNSTAIRS:
        return "downstairs";
    case DUNG_IDX_LONG_DOWNSTAIRS:
        return "long downstairs";
    default:
        return "???";
    }
}

JResult CDungeon::PlaceStairs( const int desired, const int type )
{
    int count = 0;
    bool bStairsSpawned = false;
    JIVector *vOpen;
    while( count < desired )
    {
        bStairsSpawned = false;
        JLog( LOG_LEVEL_INFO, false, "Trying to spawn %s...", StairName( type ) );
        JVector vTryPos;
        while( !bStairsSpawned )
        {
            JLog( LOG_LEVEL_INFO, false, "." );
            vOpen = g_pGame->GetDungeon()->AnyOpenTile();
            vTryPos.Init( VEC_EXPAND( *vOpen ) );

            if( CanPlaceStairsAt( vTryPos ) == DUNG_COLL_NO_COLLISION )
            {
                GetTile( vTryPos )->m_dtd = &m_dtdlist[type];
                bStairsSpawned = true;
                JLog( LOG_LEVEL_INFO, false, "Success! Spawned at <%.2f %.2f>\n",
                      VEC_EXPAND( vTryPos ) );
            }
        }
        count++;
    }
    return JSUCCESS;
}

JResult CDungeon::PlaceItems( const int depth )
{
    m_llItems = new JLinkList<CItem>;

    int desired_items = int( m_fOpenFloorArea * DUNG_CFG_ITEMS_PER_LEVEL );
    JLog( LOG_LEVEL_INFO, false, "\nPossible spawn points: %0.2f  desired items: %d\n",
          m_fOpenFloorArea, desired_items );

    while( desired_items > 0 )
    {
        int which_item = ChooseItemForDepth( depth );
        CItemDef *chosen_item = GetItemDef( which_item );
        if( chosen_item == NULL )
            continue;
        JLog( LOG_LEVEL_NOISE, true, "Choosing item %d, called %s", which_item,
              chosen_item->m_szName );

        CItem::CreateItem( chosen_item );

        desired_items--;
    }

    return JSUCCESS;
}

JResult CDungeon::SpawnMonsters( const int depth )
{
    m_llMonsters = new JLinkList<CMonster>;

    int desired_monsters = int( m_fOpenFloorArea * DUNG_CFG_MONSTERS_PER_LEVEL );
    JLog( LOG_LEVEL_INFO, false, "\nPossible spawn points: %0.2f  desired monsters: %d\n",
          m_fOpenFloorArea, desired_monsters );

    while( desired_monsters > 0 )
    {

        // Spawn a monster into dungeon
        // TODO: make this into a method on CMonster.
#define RANDOM_MONSTER
#ifdef RANDOM_MONSTER
        int which_monster = ChooseMonsterForDepth( depth );
#else
        int which_monster = m_llMonsterDefs->length() - 1;
        // which_monster = 0;
#endif // RANDOM_MONSTER
        if( SpawnMonster( which_monster ) )
        {
            desired_monsters--;
        }
    }

    return JSUCCESS;
}

CMonsterDef *CDungeon::GetMonsterDef( const char *szMonsterName )
{
    CLink<CMonsterDef> *pLink = m_llMonsterDefs->GetHead();
    CMonsterDef *pid;
    if( pLink == NULL )
        return NULL;
    while( pLink != NULL )
    {
        pid = pLink->m_lpData;
        if( Util::jstrcmp( pid->m_szName, szMonsterName ) == 0 )
        {
            return pid;
        }
        pLink = pLink->next;
    }
    return NULL;
}

CMonsterDef *CDungeon::GetMonsterDef( int which_monster )
{
    if( which_monster <= MON_IDX_INVALID || which_monster >= m_llMonsterDefs->length() )
    {
        JLog( LOG_LEVEL_WARN, true, "got an invalid monster: %d\n", which_monster );
        return NULL;
    }
    return m_llMonsterDefs->GetLink( which_monster )->m_lpData;
}

bool CDungeon::SpawnMonster( int which_monster )
{
    CMonsterDef *chosen_monster = GetMonsterDef( which_monster );
    if( chosen_monster == NULL )
    {
        return false;
    }
    JLog( LOG_LEVEL_NOISE, true, "Choosing monster %d, called %s...", which_monster,
          chosen_monster->m_szName );

    CMonster::CreateMonster( chosen_monster );
    return true;
}

CItemDef *CDungeon::GetItemDef( const char *szItemName )
{
    CLink<CItemDef> *pLink = m_llItemDefs->GetHead();
    CItemDef *pid;
    if( pLink == NULL )
        return NULL;
    while( pLink != NULL )
    {
        pid = pLink->m_lpData;
        if( Util::jstrcmp( pid->m_szName, szItemName ) == 0 )
        {
            return pid;
        }
        pLink = pLink->next;
    }
    return NULL;
}

CItemDef *CDungeon::GetItemDef( int which_item )
{
    if( which_item <= ITEM_IDX_INVALID || which_item >= m_llItemDefs->length() )
    {
        JLog( LOG_LEVEL_WARN, true, "got an invalid item: %d\n", which_item );
        return NULL;
    }
    return m_llItemDefs->GetLink( which_item )->m_lpData;
}

CEffectDef *CDungeon::GetEffectDef( const char *szEffectName )
{
    CLink<CEffectDef> *pLink = m_llEffectDefs->GetHead();
    CEffectDef *ped;
    if( pLink == NULL )
        return NULL;
    while( pLink != NULL )
    {
        ped = pLink->m_lpData;
        if( Util::jstrcmp( ped->m_szName, szEffectName ) == 0 )
        {
            return ped;
        }
        pLink = pLink->next;
    }
    return NULL;
}

int CDungeon::ChooseItemForDepth( const int depth )
{
    // Build Gaussian weights: w = exp(-0.5 * ((depth - peak) / sigma)^2)
    // Level field is the peak depth; LevelSigma controls the spread (default 10).
    int n = m_llItemDefs->length();
    float total = 0.0f;

    CLink<CItemDef> *pLink = m_llItemDefs->GetHead();
    while( pLink != NULL )
    {
        CItemDef *id = pLink->m_lpData;
        float delta = (float)( depth - id->m_dwLevel );
        float sigma = id->m_fLevelSigma;
        id->m_fSpawnWeight = Util::windowed_bell( delta, sigma );
        total += id->m_fSpawnWeight;
        pLink = pLink->next;
    }

    if( total <= 0.0f )
    {
        JLog( LOG_LEVEL_WARN, true, "No items available at depth %d\n", depth );
        return ITEM_IDX_INVALID;
    }

    float r = Util::GetRandom( 0.0f, total );
    float cumulative = 0.0f;
    pLink = m_llItemDefs->GetHead();
    int idx = 0;
    while( pLink != NULL )
    {
        cumulative += pLink->m_lpData->m_fSpawnWeight;
        if( r <= cumulative )
            return idx;
        pLink = pLink->next;
        idx++;
    }
    return n - 1; // fallback
}

int CDungeon::ChooseMonsterForDepth( const int depth, const float sigma )
{
    // Build Gaussian weights: w = exp(-0.5 * ((depth - peak) / effective_sigma)^2)
    // The sigma parameter overrides the per-monster LevelSigma when non-zero.
    int n = m_llMonsterDefs->length();
    float total = 0.0f;

    CLink<CMonsterDef> *pLink = m_llMonsterDefs->GetHead();
    while( pLink != NULL )
    {
        CMonsterDef *md = pLink->m_lpData;
        float effective_sigma = ( sigma > 0.0f ) ? sigma : md->m_fLevelSigma;
        float delta = (float)( depth - md->m_dwLevel );
        md->m_fSpawnWeight = Util::windowed_bell( delta, effective_sigma );
        total += md->m_fSpawnWeight;
        pLink = pLink->next;
    }

    if( total <= 0.0f )
    {
        JLog( LOG_LEVEL_WARN, true, "No monsters available at depth %d\n", depth );
        return MON_IDX_INVALID;
    }

    float r = Util::GetRandom( 0.0f, total );
    float cumulative = 0.0f;
    pLink = m_llMonsterDefs->GetHead();
    int idx = 0;
    while( pLink != NULL )
    {
        cumulative += pLink->m_lpData->m_fSpawnWeight;
        if( r <= cumulative )
            return idx;
        pLink = pLink->next;
        idx++;
    }
    return n - 1; // fallback
}

JResult CDungeon::OnChangeLevel( const int delta )
{
    JLog( LOG_LEVEL_INFO, false, "Changing level...\n" );
    TerminateLevel();
    CreateNewLevel( delta );
    JLog( LOG_LEVEL_INFO, false, "done.\n" );
    return JSUCCESS;
}

int counter = 0;
bool CDungeon::Tick( const int dwClock )
{
    /*if( dwClock % 2 == 1 )
    {
        // don't draw the dungeon this update
        m_bDraw = false;
    }
    else
    {
        m_bDraw = true;
    }/**/

    bool bWorking = m_dmCurLevel->ProcessStep();
#ifndef CLOCKSTEP
    // Auto-advance to next level when generation completes (disabled in CLOCKSTEP mode)
    if( bWorking == false )
    {
        counter++;
        if( counter == 10 )
        {
            counter = 0;
            OnChangeLevel( 1 );
        }
    }
#endif

    InitDungeonTiles();
    return bWorking;
}

bool CDungeon::Update( float fCurTime )
{
    if( m_llItems )
    {
        CLink<CItem> *pLink = m_llItems->GetHead();
        CItem *pItem;

        while( pLink != NULL )
        {
            pItem = pLink->m_lpData;
            pItem->Update( fCurTime );
            pLink = m_llItems->GetNext( pLink );
        }
    }

    UpdateSeen();
    UpdateVisibility();
    return true;
}

JResult CDungeon::UpdateSeen()
{
    // In CLOCKSTEP mode, player may not be spawned yet
    if( !g_pGame->GetPlayer() || !g_pGame->GetPlayer()->m_bHasSpawned )
        return JSUCCESS;

    JIVector vPlayer( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    JRect rcSeen = Util::Nearby( vPlayer, 1 );

    JLog( LOG_LEVEL_NOISE, false, "player: <%d %d> seen: <%d %d %d %d>\n", VEC_EXPAND( vPlayer ),
          RECT_EXPAND( rcSeen ) );

    JIVector vTile;
    CRoom *pRoom;
    for( vTile.y = rcSeen.top; vTile.y <= rcSeen.bottom; vTile.y++ )
    {
        for( vTile.x = rcSeen.left; vTile.x <= rcSeen.right; vTile.x++ )
        {
            GetITile( vTile )->SetFlags( DUNG_FLAG_SEEN );
            pRoom = m_dmCurLevel->InRoom( vTile );
            if( pRoom != NULL &&
                ( g_pGame->GetPlayer()->IsWizard() ||
                  ( pRoom->HasFlags( DUNG_FLAG_LIT ) && !pRoom->HasFlags( DUNG_FLAG_SEEN ) ) ) )
            {
                LightRoom( pRoom );
            }
        }
    }

    return JSUCCESS;
}

void CDungeon::LightRoom( CRoom *pRoom )
{
    JVector vCurPos;
    JRect rcRoom = pRoom->GetEdges();
    for( vCurPos.y = rcRoom.top; vCurPos.y <= rcRoom.bottom; vCurPos.y++ )
    {
        for( vCurPos.x = rcRoom.left; vCurPos.x <= rcRoom.right; vCurPos.x++ )
        {
            GetTile( vCurPos )->SetFlags( DUNG_FLAG_LIT | DUNG_FLAG_SEEN );
        }
    }
    pRoom->SetFlags( DUNG_FLAG_SEEN );
}

void CDungeon::LightPosition( const JVector &vPos )
{
    // If the beam crosses a room tile, light the whole room.
    CRoom *pRoom = InRoom( const_cast<JVector &>( vPos ) );
    if( pRoom )
    {
        pRoom->SetFlags( DUNG_FLAG_LIT );
        LightRoom( pRoom );
        return;
    }
    // Otherwise (hallway / open area): light the tile itself and all 8 neighbours
    // so that adjacent walls become visible.
    JVector vNeighbour;
    for( int dy = -1; dy <= 1; dy++ )
    {
        for( int dx = -1; dx <= 1; dx++ )
        {
            vNeighbour.x = vPos.x + dx;
            vNeighbour.y = vPos.y + dy;
            CDungeonTile *pTile = GetTile( vNeighbour );
            if( pTile )
                pTile->SetFlags( DUNG_FLAG_LIT | DUNG_FLAG_SEEN );
        }
    }
}

bool SightCollisionTest( JVector &vTest );

void CDungeon::UpdateVisibility()
{
    if( !g_pGame->GetPlayer() || !g_pGame->GetPlayer()->m_bHasSpawned )
        return;

    // Clear all VISIBLE flags
    JVector vTile;
    for( vTile.x = 0; vTile.x < DUNG_WIDTH; vTile.x++ )
    {
        for( vTile.y = 0; vTile.y < DUNG_HEIGHT; vTile.y++ )
        {
            GetTile( vTile )->UnsetFlags( DUNG_FLAG_VISIBLE );
        }
    }

    JIVector vPlayer( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );

    // Player's own tile is always visible
    GetITile( vPlayer )->SetFlags( DUNG_FLAG_VISIBLE );

    // If player is in a lit room, entire room is visible
    JVector vPlayerF( g_pGame->GetPlayer()->m_vPos );
    CRoom *pPlayerRoom = InRoom( vPlayerF );
    if( pPlayerRoom && pPlayerRoom->HasFlags( DUNG_FLAG_LIT ) )
    {
        JRect rcRoom = pPlayerRoom->GetEdges();
        for( vTile.y = rcRoom.top; vTile.y <= rcRoom.bottom; vTile.y++ )
        {
            for( vTile.x = rcRoom.left; vTile.x <= rcRoom.right; vTile.x++ )
            {
                GetTile( vTile )->SetFlags( DUNG_FLAG_VISIBLE );
            }
        }
    }

    // Check tiles within max sight distance using LOS
    int maxDist = SIGHT_DISTANCE_LIT;
    JRect rcCheck = Util::Nearby( vPlayer, maxDist );
    JIVector viCheck;
    for( viCheck.y = rcCheck.top; viCheck.y <= rcCheck.bottom; viCheck.y++ )
    {
        for( viCheck.x = rcCheck.left; viCheck.x <= rcCheck.right; viCheck.x++ )
        {
            CDungeonTile *pTile = GetITile( viCheck );
            if( !pTile || ( pTile->m_dwFlags & DUNG_FLAG_VISIBLE ) )
                continue;

            int dx = Util::abs( viCheck.x - vPlayer.x );
            int dy = Util::abs( viCheck.y - vPlayer.y );
            int chebyshev = MAX( dx, dy );

            // Tiles beyond base sight range need to be in a lit room, or be
            // individually lit (e.g. by a Wand of Light beam), to be visible
            if( chebyshev > SIGHT_DISTANCE_PLAYER )
            {
                JVector vCheckF( viCheck.x, viCheck.y );
                CRoom *pTargetRoom = InRoom( vCheckF );
                bool inLitRoom = pTargetRoom && pTargetRoom->HasFlags( DUNG_FLAG_LIT );
                bool isLitTile = ( pTile->m_dwFlags & DUNG_FLAG_LIT ) != 0;
                if( !inLitRoom && !isLitTile )
                    continue;
            }

            // Manhattan distance + 1: the Bresenham staircase needs dx x-steps
            // and dy y-steps so the safety cap must be at least dx+dy+1.
            int target_distance = dx + dy + 1;

            if( Util::Bresenham( vPlayer, viCheck, target_distance, SightCollisionTest ) )
            {
                pTile->SetFlags( DUNG_FLAG_VISIBLE );
            }
        }
    }
}

bool CollisionTest( JVector &vTest )
{
    return g_pGame->GetDungeon()->IsWalkableFor( vTest ) == DUNG_COLL_NO_COLLISION;
}

// Sight line collision test - only open doors are transparent to visibility
// Closed doors, secret doors, walls, and rubble block line-of-sight
bool SightCollisionTest( JVector &vTest )
{
    if( !vTest.IsWithinWorld() )
        return false;

    CDungeonTile *curTile = g_pGame->GetDungeon()->GetTile( vTest );
    if( curTile == NULL )
        return false;

    int type = curTile->m_dtd->m_dwType;

    // Explicitly block sight-blocking obstacles
    // Walls and rubble always block sight
    if( type == DUNG_IDX_WALL || type == DUNG_IDX_RUBBLE )
        return false; // Blocked

    // Closed/secret doors block sight - check CURRENT type, not modified type
    if( type == DUNG_IDX_DOOR || type == DUNG_IDX_SECRET_DOOR )
        return false; // Closed door blocks sight

    // Open doors allow sight through
    if( type == DUNG_IDX_OPEN_DOOR )
        return true; // Open door allows sight through

    // Everything else allows sight (floors, stairs, etc.)
    return true; // Allow sight
}

bool CDungeon::CanSeeEachOther( JIVector vSource, JIVector vTarget, uint32 dwFlags )
{
    // can see things in the same room, if the room is LIT
    CRoom *prTarget = m_dmCurLevel->InRoom( vTarget );
    CRoom *prSource = m_dmCurLevel->InRoom( vSource );
    if( prTarget && prTarget->HasFlags( DUNG_FLAG_SEEN ) && prTarget == prSource )
        return true;

    // check for "see through walls" effects

    // check for ESP and not EMPTY_MIND
    uint32 esp = EFFECT_FLAG_ESP;
    uint32 empty_mind = MON_FLAG_EMPTY_MIND;
    if( dwFlags >= esp )
        JLog( LOG_LEVEL_NOISE, true, "performing esp check\n" );
    if( ( ( dwFlags & esp ) == esp ) && ( ( dwFlags & empty_mind ) == 0 ) )
    {
        JLog( LOG_LEVEL_DEBUG, true, "detected a thinking mind at <%d %d>\n",
              VEC_EXPAND( vTarget ) );
        if( Util::Nearby( vSource, SIGHT_DISTANCE_ESP ).Contains( vTarget ) )
            return true;
    }

    // check for both infravision and warm body
    uint32 heat_sense = ( EFFECT_FLAG_INFRA | MON_FLAG_WARM );
    if( dwFlags >= heat_sense )
        JLog( LOG_LEVEL_NOISE, true, "performing infra check\n" );
    if( ( dwFlags & heat_sense ) == heat_sense )
    {
        JLog( LOG_LEVEL_DEBUG, true, "sensed a heat source at <%d %d>\n", VEC_EXPAND( vTarget ) );
        if( Util::Nearby( vSource, SIGHT_DISTANCE_INFRA ).Contains( vTarget ) )
            return true;
    }

    // check for "in visible range" before doing the
    // more expensive line-of-sight test
    // Extended sight distance only applies within the same room.
    // Different rooms must pass line-of-sight check even if both are lit.
    int sight_distance = SIGHT_DISTANCE_PLAYER;
    if( prSource && prTarget && prSource == prTarget && prSource->HasFlags( DUNG_FLAG_LIT ) )
        sight_distance = SIGHT_DISTANCE_LIT;

    if( !Util::Nearby( vSource, sight_distance ).Contains( vTarget ) )
        return false;

    // Use exact distance to target so the Bresenham line stops AT the target
    // rather than continuing past it into potential walls
    int dx = Util::abs( vTarget.x - vSource.x );
    int dy = Util::abs( vTarget.y - vSource.y );
    // Manhattan distance + 1: the Bresenham staircase needs dx x-steps and
    // dy y-steps so the safety cap must be at least dx+dy+1.
    int target_distance = dx + dy + 1;

    // No "see through walls" effects are active
    // Check for obstacles along the line between
    // the player and the position
    // Use SightCollisionTest to allow vision through doors
    //
    return Util::Bresenham( vSource, vTarget, target_distance, SightCollisionTest );
}

bool CDungeon::PlayerCanSee( JVector vCheck, uint32 dwFlags )
{
    // In CLOCKSTEP mode, player may not exist yet
    if( !g_pGame->GetPlayer() || !g_pGame->GetPlayer()->m_bHasSpawned )
        return true; // Show everything when no player

    if( g_pGame->GetPlayer()->IsWizard() )
        return true;

    JVector playerPos = g_pGame->GetPlayer()->m_vPos;
    JIVector viCheck( VEC_EXPAND( vCheck ) );
    JIVector viPlayer( VEC_EXPAND( playerPos ) );

    // The player's own tile is always visible
    if( vCheck == playerPos )
        return true;

    // All other tiles require line-of-sight checks
    dwFlags |= g_pGame->GetPlayer()->GetIntrinsic( EFFECT_FLAG_ESP | EFFECT_FLAG_INFRA );

    return CanSeeEachOther( viPlayer, viCheck, dwFlags );
}

bool CDungeon::IsOnScreen( JVector vPos )
{
    // 12.7.2017 - the below code is totally broken and
    // was causing monsters/items not to display if they were "too far"
    // from the player. MIN/MAX offsets are set to totally bogus values
    // which is the actual bug, but revisit this if you are either
    // 1) trying to do sight-distance, or
    // 2) trying to speed things up by not drawing off-screen stuff,
    //    once big dungeons are happening.
    //    if( g_pGame->GetPlayer() == NULL )
    //    {
    //        return false;
    //    }
    //
    //    JVector vPlayer = g_pGame->GetPlayer()->m_vPos;
    //    if( vPos.x < vPlayer.x - SCREEN_MIN_XOFF ||
    //        vPos.x > vPlayer.x + SCREEN_MAX_XOFF ||
    //        vPos.y < vPlayer.y - SCREEN_MIN_YOFF ||
    //        vPos.y > vPlayer.y + SCREEN_MAX_YOFF )
    //    {
    //        return false;
    //    }
    return true;
}

void CDungeon::Draw()
{
    if( !m_bDraw )
    {
        return;
    }

    PreDraw();

    // Select Our Texture
    DrawDungeon();

    DrawItems();

    DrawMonsters();

    PostDraw();
}

bool CDungeon::IsLit( JVector vPos )
{
    // In CLOCKSTEP mode, player may not exist yet - consider everything lit
    if( !g_pGame->GetPlayer() || !g_pGame->GetPlayer()->m_bHasSpawned )
        return true;

    if( !g_pGame->GetPlayer()->LightSource() )
        return false;
    JIVector vPlayer( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    JIVector vTarget( VEC_EXPAND( vPos ) );
    return Util::WithinRadius( vPlayer, vTarget, g_pGame->GetPlayer()->LightRadius() );
}

void CDungeon::DrawDungeon()
{
    // Brute force method; optimize this later
    JVector vScreen;
#ifdef DUNG_FONT_COURIER
    JVector vSize( 1.0f, 1.0f );
#else
    JVector vSize( 0.75f, 1.0f );
#endif

    // In CLOCKSTEP mode, player may not be spawned yet
    JVector vDefaultPos( DUNG_WIDTH / 2, DUNG_HEIGHT / 2 );
    JVector vPlayerPos = ( g_pGame->GetPlayer() && g_pGame->GetPlayer()->m_bHasSpawned )
                             ? g_pGame->GetPlayer()->m_vPos
                             : vDefaultPos;

    JVector vLook = ( g_pGame->GetGameStateIndex() == STATE_LOOK ) ? m_vLookPos : vPlayerPos;
    JVector vProjectile =
        ( g_pGame->GetGameStateIndex() == STATE_RANGED ) ? m_vProjectilePos : vPlayerPos;
    JColor color;

    for( vScreen.x = 0; vScreen.x < DUNG_WIDTH; vScreen.x++ )
    {
        for( vScreen.y = 0; vScreen.y < DUNG_HEIGHT; vScreen.y++ )
        {
            CDungeonTile *curTile = GetTile( vScreen );

            if( g_pGame->GetGameStateIndex() == STATE_LOOK && vScreen == vLook )
            {
                if( curTile == NULL )
                    continue; // safety: don't crash on out-of-bounds look position
            }

            // In CLOCKSTEP mode, show all tiles regardless of visibility
            // In normal gameplay, only show tiles that have been seen
            else if( g_pGame->GetGameStateIndex() != STATE_CLOCKSTEP &&
                     ( curTile == NULL || ( ( curTile->m_dwFlags & DUNG_FLAG_SEEN ) == 0 ) ) )
            {
                continue;
            }

            bool isVisible = ( curTile->m_dwFlags & DUNG_FLAG_VISIBLE ) != 0;

            // Determine tile color based on game state
            bool bRangedBeamTile = false;
            if( g_pGame->GetGameStateIndex() == STATE_RANGED )
            {
                // Set default color for projectiles (white)
                color = JColor( 255, 255, 255, 255 );

                if( m_llProjectileTrajectory )
                {
                    // Check if vScreen is on the trajectory path
                    int pathIndex = 0;
                    CLink<JIVector> *plPos = m_llProjectileTrajectory->GetHead();
                    while( plPos && pathIndex <= m_dwProjectileColorIndex )
                    {
                        JIVector curPos = *( plPos->m_lpData );
                        if( (int)vScreen.x == curPos.x && (int)vScreen.y == curPos.y )
                        {
                            // On trajectory - use effect colors if available
                            if( m_pProjectileEffect )
                            {
                                bRangedBeamTile =
                                    m_pProjectileEffect->m_cBeamChar != ItemIDs[ITEM_IDX_ARROW];
                                // Get color from effect definition, cycling through colors
                                if( m_pProjectileEffect->m_llColors &&
                                    m_pProjectileEffect->m_llColors->length() > 0 )
                                {
                                    int colorIndex =
                                        pathIndex % m_pProjectileEffect->m_llColors->length();
                                    CLink<JColor> *plColor =
                                        m_pProjectileEffect->m_llColors->GetNthLink( colorIndex );
                                    if( plColor )
                                    {
                                        color = *( plColor->m_lpData );
                                    }
                                }
                            }
                            break;
                        }
                        plPos = plPos->next;
                        pathIndex++;
                    }
                }
            }

            if( !bRangedBeamTile )
            {
                // Normal tile coloring logic (for non-beam tiles or when no multicolor effect)
                if( g_pGame->GetGameStateIndex() == STATE_LOOK && vScreen == vLook )
                {
                    color = JColor( 100, 0, 100, 255 );
                }
                else if( g_pGame->GetGameStateIndex() == STATE_RANGED && vScreen == vProjectile )
                {
                    color = JColor( 255, 255, 85, 255 );
                    bRangedBeamTile = true; // Draw beam character at current projectile position
                }
                else if( IsOnLOSLine( vScreen ) )
                {
                    color = JColor( 85, 255, 255, 255 );
                }
                else if( g_pGame->GetGameStateIndex() == STATE_CLOCKSTEP ||
                         g_pGame->GetPlayer()->IsWizard() )
                {
                    color = curTile->m_dtd->m_Color;
                }
                else if( !isVisible )
                {
                    // Fog of War: seen but not currently visible — dim grey
                    color = JColor( 60, 60, 80, 255 );
                }
                else if( IsLit( vScreen ) )
                {
                    color = JColor( 200, 200, 0, 255 );
                }
                else
                {
                    color = curTile->m_dtd->m_Color;
                }
            }
            m_TileSet->SetTileColor( color );
            char chDraw = ( bRangedBeamTile && m_pProjectileEffect )
                              ? m_pProjectileEffect->m_cBeamChar
                              : curTile->m_dtd->m_chTile;
            m_TileSet->DrawChar( chDraw, vScreen, vSize );
        }
    }
}

void CDungeon::DisturbPlayer() { g_pGame->GetPlayer()->m_bIsDisturbed = true; }

bool CDungeon::IsOnLOSLine( JVector vPos )
{
    if( !m_llLOSLine )
        return false;
    CLink<JIVector> *pLink = m_llLOSLine->GetHead();
    while( pLink )
    {
        if( pLink->m_lpData && pLink->m_lpData->x == (int)vPos.x &&
            pLink->m_lpData->y == (int)vPos.y )
            return true;
        pLink = pLink->next;
    }
    return false;
}

void CDungeon::DrawItems()
{
    if( !m_llItems )
        return;

    CLink<CItem> *pLink = m_llItems->GetHead();
    CItem *pItem;

    while( pLink != NULL )
    {
        pItem = pLink->m_lpData;
        if( pItem && IsOnScreen( pItem->m_vPos ) &&
            PlayerCanSee( pItem->m_vPos, MON_FLAG_EMPTY_MIND ) )
        {
            pItem->Draw();
        }
        pLink = m_llItems->GetNext( pLink );
    }
}

void CDungeon::DrawMonsters()
{
    if( !m_llMonsters )
        return;
    CLink<CMonster> *pLink = m_llMonsters->GetHead();
    CMonster *pMon;

    while( pLink != NULL )
    {
        pMon = pLink->m_lpData;
        uint32 dwFlags = pMon->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND );
        if( pMon && IsOnScreen( pMon->GetPos() ) &&
            ( PlayerCanSee( pMon->GetPos(), dwFlags ) || pMon->m_bDetected ) )
        {
            pMon->Draw();
        }
        pLink = m_llMonsters->GetNext( pLink );
    }
}

void CDungeon::PreDraw()
{
    if( g_pGame->GetPlayer() != NULL )
    {
#ifdef CLOCKSTEP
        // Wide zoom during generation to see full dungeon; normal zoom during gameplay
        if( g_pGame->GetGameStateIndex() == STATE_CLOCKSTEP )
            g_pGame->GetRender()->SetZoom( DUNG_WIDTH / 2 );
        else
            g_pGame->GetRender()->SetZoom( 20 );
#endif
        int xinitval = g_pGame->GetRender()->GetZoom();
        // int xinitval = 16;
        int yinitval = xinitval;

#define ORIGIN_PLAYER
#ifdef ORIGIN_PLAYER
#ifdef CLOCKSTEP
        // In CLOCKSTEP state, center on the entire dungeon.
        // After transitioning to gameplay, center on the player.
        int xorigin, yorigin;
        if( g_pGame->GetGameStateIndex() == STATE_CLOCKSTEP )
        {
            xorigin = 0;
            yorigin = 0;
        }
        else
        {
            xorigin = (int)g_pGame->GetPlayer()->m_vPos.x - DUNG_WIDTH / 2;
            yorigin = (int)g_pGame->GetPlayer()->m_vPos.y - DUNG_HEIGHT / 2;
        }
#else
        int xorigin = (int)g_pGame->GetPlayer()->m_vPos.x - DUNG_WIDTH / 2;
        int yorigin = (int)g_pGame->GetPlayer()->m_vPos.y - DUNG_HEIGHT / 2;
#endif
#else
        int xorigin = 0; // + is left (?!)
        int yorigin = 0; // + is up
#endif
        m_Rect.Init( xorigin - xinitval, yorigin + yinitval, xorigin + xinitval,
                     yorigin - yinitval );
    }
#ifdef RENDER_TILESET_POSTLOAD_NEEDED
    g_pGame->GetRender()->PreDrawObjects( m_Rect, m_TileSet->Texture(), true, false,
                                          &m_vfTranslate );
#else
    g_pGame->GetRender()->PreDrawObjects( m_Rect, 0, true, false, &m_vfTranslate );
#endif

    // Do any external setup that needs doing.
    m_TileSet->PreDrawTile();
}

void CDungeon::PostDraw()
{
    // Do any external teardown that needs doing.
    m_TileSet->PostDrawTile();

    g_pGame->GetRender()->PostDrawObjects();
}

void CDungeon::Term()
{
    if( m_Tiles )
    {
        delete[] m_Tiles;
        m_Tiles = NULL;
    }
    if( m_TileSet )
    {
        delete m_TileSet;
        m_TileSet = NULL;
    }

    if( m_dtdlist )
    {
        delete[] m_dtdlist;
        m_dtdlist = NULL;
    }

    if( m_llMonsterDefs )
    {
        m_llMonsterDefs->Terminate();
        delete m_llMonsterDefs;
        m_llMonsterDefs = NULL;
    }

    if( m_llItemDefs )
    {
        m_llItemDefs->Terminate();
        delete m_llItemDefs;
        m_llItemDefs = NULL;
    }

    if( m_llEffectDefs )
    {
        m_llEffectDefs->Terminate();
        delete m_llEffectDefs;
        m_llEffectDefs = NULL;
    }

    ClearLOSLine();
}

void CDungeon::RemoveMonster( CMonster *pMon )
{
    CLink<CMonster> *pLink;
    pLink = pMon->m_pllLink;

    // Clear player's target if it points to this monster (prevents dangling pointer)
    if( g_pGame->GetPlayer()->GetTarget() == pMon )
    {
        g_pGame->GetPlayer()->SetTarget( NULL );
    }

    // Invalidate visible monsters cache (it holds non-owning CMonster* pointers)
    g_pGame->GetPlayer()->ClearVisibleMonsters();

    GetTile( pMon->GetPos() )->m_pCurMonster = NULL;
    m_llMonsters->Remove( pLink );
}

int CDungeon::IsWalkableFor( JVector &vPos, bool isPlayer )
{
    if( !vPos.IsWithinWorld() )
        return false;
    // Check for someone else standing there first (handles things that can walk thru walls)
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL )
    {
        JLog( LOG_LEVEL_ERROR, true, "Hey! That's a bad tile.\n" );
        return false;
    }
    if( curTile->m_pCurMonster != NULL )
    {
        // Monsters can collide with other monsters
        JLog( LOG_LEVEL_DEBUG, true, "Monster attacking other monsters is not implemented yet.\n" );
        return DUNG_COLL_MONSTER;
    }
    else if( !isPlayer && g_pGame->GetPlayer() && g_pGame->GetPlayer()->m_bHasSpawned &&
             g_pGame->GetPlayer()->m_vPos == vPos )
    {
        // Monsters colliding with the player can be hazardous to your health.
        return DUNG_COLL_PLAYER;
    }
    else if( isPlayer && curTile->m_pCurItem != NULL )
    {
        return DUNG_COLL_ITEM;
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    int type = curTile->m_dtd->m_dwType;
    switch( type )
    {
    case DUNG_IDX_WALL:
    case DUNG_IDX_DOOR:
    case DUNG_IDX_SECRET_DOOR:
    case DUNG_IDX_RUBBLE:
        return type;
        break;
    case DUNG_IDX_UPSTAIRS:
    case DUNG_IDX_LONG_UPSTAIRS:
    case DUNG_IDX_DOWNSTAIRS:
    case DUNG_IDX_LONG_DOWNSTAIRS:
        if( g_pGame->GetPlayer()->m_bHasSpawned )
        {
            return DUNG_COLL_NO_COLLISION;
        }
        JLog( LOG_LEVEL_WARN, true, "Player can't spawn on stairs\n" );
        return type;
        break;
    default:
        return DUNG_COLL_NO_COLLISION;
        break;
    }
}

int CDungeon::CanPlaceStairsAt( JVector &vPos )
{
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL )
    {
        JLog( LOG_LEVEL_ERROR, true, "Hey! That's a bad tile.\n" );
        return false;
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    int type = curTile->m_dtd->m_dwType;
    switch( type )
    {
    case DUNG_IDX_WALL:
    case DUNG_IDX_DOOR:
    case DUNG_IDX_OPEN_DOOR:
    case DUNG_IDX_RUBBLE:
    case DUNG_IDX_UPSTAIRS:
    case DUNG_IDX_LONG_UPSTAIRS:
    case DUNG_IDX_DOWNSTAIRS:
    case DUNG_IDX_LONG_DOWNSTAIRS:
        return type;
        break;
    default:
        return DUNG_COLL_NO_COLLISION;
        break;
    }
}

int CDungeon::CanPlaceItemAt( JVector &vPos )
{
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Hey! That's a bad tile.\n" );
        return false;
    }
    if( curTile->m_pCurItem != NULL )
    {
        // Items can collide with other Items
        return DUNG_COLL_ITEM;
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    int type = curTile->m_dtd->m_dwType;
    switch( type )
    {
    case DUNG_IDX_WALL:
    case DUNG_IDX_DOOR:
    case DUNG_IDX_OPEN_DOOR:
    case DUNG_IDX_RUBBLE:
    case DUNG_IDX_UPSTAIRS:
    case DUNG_IDX_LONG_UPSTAIRS:
    case DUNG_IDX_DOWNSTAIRS:
    case DUNG_IDX_LONG_DOWNSTAIRS:
        return type;
        break;
    default:
        return DUNG_COLL_NO_COLLISION;
        break;
    }
}

bool CDungeon::IsOpenable( JVector &vPos )
{
    // trivial check; is this a modifiable tile at all?
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
    {
        return false;
    }
    // Check for someone else standing there first (handles things that can walk thru walls)

    if( curTile->m_pCurItem )
    {
        return ( curTile->m_pCurItem->IsOpenable() );
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    if( curTile->m_dtd->m_dwType == DUNG_IDX_SECRET_DOOR )
    {
        if( Util::GetRandom( 1, 100 ) <= CHANCE_FIND_SECRET_BUMP )
        {
            g_pGame->GetMsgs()->Printf( "You have found a secret door!\n" );
            g_pGame->GetDungeon()->Modify( curTile->m_vPos );
            return true;
        }
        return false;
    }
    if( curTile->m_dtd->m_dwType == DUNG_IDX_DOOR )
        return !curTile->HasFlags( DUNG_FLAG_LOCKED );
    return false;
}

bool CDungeon::IsTunnelable( JVector &vPos )
{
    // trivial check; is this a modifiable tile at all?
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
    {
        return false;
    }
    // Check for someone else standing there first (handles things that can walk thru walls)

    if( curTile->m_pCurItem )
    {
        return ( curTile->m_pCurItem->IsTunnelable() );
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    return ( curTile->m_dtd->m_dwType == DUNG_IDX_RUBBLE );
}

bool CDungeon::IsCloseable( JVector &vPos )
{
    // trivial check; is this a modifiable tile at all?
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL || curTile->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
    {
        return false;
    }
    // Check for someone else standing there first (handles things that can walk thru walls)

    if( curTile->m_pCurItem )
    {
        return ( curTile->m_pCurItem->IsCloseable() );
    }

    // If you get here, the square was unoccupied. Now check for running into inanimates...
    return ( curTile->m_dtd->m_dwType == DUNG_IDX_OPEN_DOOR );
}

int CDungeon::IsStairs( JVector &vPos )
{
    // trivial check; is this a modifiable tile at all?
    CDungeonTile *curTile = GetTile( vPos );
    if( curTile == NULL )
    {
        return DUNG_IDX_INVALID;
    }

    if( curTile->m_dtd->m_dwType == DUNG_IDX_UPSTAIRS ||
        curTile->m_dtd->m_dwType == DUNG_IDX_LONG_UPSTAIRS ||
        curTile->m_dtd->m_dwType == DUNG_IDX_DOWNSTAIRS ||
        curTile->m_dtd->m_dwType == DUNG_IDX_LONG_DOWNSTAIRS )
    {
        return curTile->m_dtd->m_dwType;
    }

    // If you get here, the square was not a staircase
    return DUNG_IDX_INVALID;
}

CRoom *CDungeon::InRoom( JVector &vPos )
{
    JIVector viPos( VEC_EXPAND( vPos ) );
    return m_dmCurLevel->InRoom( viPos );
}

JResult CDungeon::Modify( JVector &vPos )
{
    if( GetTile( vPos )->m_dtd->m_dwModifiedType == DUNG_IDX_INVALID )
    {
        // hey! you can't modify that tile! How did you get here?!
        JLog( LOG_LEVEL_ERROR, true, "Modify error: Can't modify type %d at <%f %f>\n",
              GetTile( vPos )->m_dtd->m_dwType, VEC_EXPAND( vPos ) );
        return JERROR();
    }

    GetTile( vPos )->m_dtd = &m_dtdlist[GetTile( vPos )->m_dtd->m_dwModifiedType];

    return JSUCCESS;
}

CItem *CDungeon::PickUp( JVector &vPickupPos )
{
    CItem *pItem = GetTile( vPickupPos )->m_pCurItem;
    if( pItem )
        m_llItems->Remove( pItem->m_pllLink, false );
    return pItem;
}

void CDungeon::Drop( CItem *pItem, JVector &vDropPos )
{
    JLog( LOG_LEVEL_WARN, true, "CDungeon::Drop called - pItem=%p, type=%s, count=%d at <%f %f>\n",
          pItem, pItem ? pItem->GetName() : "NULL", pItem ? pItem->m_dwCount : 0,
          VEC_EXPAND( vDropPos ) );

    JVector vFinalPos = vDropPos;

    // Check if we can stack with an existing item of the same type
    CDungeonTile *pTargetTile = GetTile( vDropPos );
    if( pTargetTile && pTargetTile->m_pCurItem != NULL )
    {
        CItem *pExisting = pTargetTile->m_pCurItem;
        // If same item type and stackable, increment count and discard the new item
        if( pExisting->m_id->m_dwIndex == pItem->m_id->m_dwIndex && pItem->IsStackable() )
        {
            pExisting->m_dwCount += pItem->m_dwCount;
            JLog( LOG_LEVEL_DEBUG, true, "Stacked item; new count: %d\n", pExisting->m_dwCount );
            delete pItem; // Discard the new item; existing one now has all counts
            JLog( LOG_LEVEL_DEBUG, true, ">>Drop: Stacking, returning early\n" );
            return;
        }

        // Different item type or non-stackable; find an adjacent empty tile
        bool bFoundSpot = false;
        JLog( LOG_LEVEL_DEBUG, true,
              ">>Drop: Different item or non-stackable, searching adjacent tiles\n" );

        // Use Util::Nearby to get all adjacent tiles
        JRect rcNearby = Util::Nearby( JIVector( (int)vDropPos.x, (int)vDropPos.y ), 1 );

        for( int x = rcNearby.Left(); x <= rcNearby.Right() && !bFoundSpot; x++ )
        {
            for( int y = rcNearby.Top(); y <= rcNearby.Bottom() && !bFoundSpot; y++ )
            {
                // Skip the original position
                if( x == (int)vDropPos.x && y == (int)vDropPos.y )
                    continue;

                JVector vTry( (float)x, (float)y );

                // Check if this position is valid and empty
                CDungeonTile *pTile = GetTile( vTry );
                if( pTile != NULL && pTile->m_pCurItem == NULL &&
                    IsWalkableFor( vTry, false ) == DUNG_COLL_NO_COLLISION )
                {
                    vFinalPos = vTry;
                    bFoundSpot = true;
                    JLog( LOG_LEVEL_DEBUG, true, ">>Drop: Found adjacent spot at <%f %f>\n",
                          vFinalPos.x, vFinalPos.y );
                }
            }
        }

        if( !bFoundSpot )
        {
            // No adjacent spot found; cannot drop here - item disappears
            JLog( LOG_LEVEL_DEBUG, true, ">>Drop: No adjacent spot found, item disappears\n" );
            g_pGame->GetMsgs()->Printf( "The %s disappears.\n", pItem->GetName() );
            delete pItem;
            return;
        }
    }

    JLog( LOG_LEVEL_DEBUG, true, ">>Drop: Adding to m_llItems at <%f %f>\n", vFinalPos.x,
          vFinalPos.y );
    GetTile( vFinalPos )->m_pCurItem = pItem;
    pItem->m_vPos = vFinalPos;
    pItem->m_pllLink = m_llItems->Add( pItem, pItem->m_id->m_dwIndex, pItem->GetInstanceId() );
}

bool CDungeon::LockDoor( JVector pos )
{
    CDungeonTile *pTile = GetTile( pos );
    if( !pTile || !pTile->m_dtd || pTile->m_dtd->m_dwType != DUNG_IDX_DOOR )
        return false;
    pTile->SetFlags( DUNG_FLAG_LOCKED );
    return true;
}

bool CDungeon::UnlockDoor( JVector pos )
{
    CDungeonTile *pTile = GetTile( pos );
    if( !pTile || !pTile->m_dtd || pTile->m_dtd->m_dwType != DUNG_IDX_DOOR )
        return false;
    pTile->UnsetFlags( DUNG_FLAG_LOCKED );
    return true;
}

void CDungeon::Aggravate( JVector vOrigin ) { CEffect::Fire( "Aggravate Monsters", vOrigin ); }

JResult CDungeon::LightArea( JVector vPos )
{
    CRoom *pRoom = InRoom( vPos );
    if( pRoom )
    {
        pRoom->SetFlags( DUNG_FLAG_LIT );
        LightRoom( pRoom );

        g_pGame->GetMsgs()->Printf( "The room is flooded with light!\n" );
        return JSUCCESS;
    }
    g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
    return JBOGUSKEY;
}
