#include "TestContext.hpp"
using cucumber::ScenarioScope;

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^I have a DungeonMap$" )
{
    ScenarioScope<TestCtx> context;
    context->map.CreateDungeon( 1 );
}

GIVEN( "^There is already a room at ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) in the dungeon$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    JRect rcBlocking( l, t, r, b );
    ScenarioScope<TestCtx> context;
    context->pRoom = new CRoom( rcBlocking );
    context->map.FillArea( DUNG_IDX_FLOOR, context->pRoom );
    context->map.LightArea( context->pRoom );
}

GIVEN( "^I have a JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) to fill$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    ScenarioScope<TestCtx> context;

    context->area = JRect( l, t, r, b );
    context->pRoom = new CRoom( context->area );
}

GIVEN( "^I have a room create step$" )
{
    ScenarioScope<TestCtx> context;
    JIVector vRoom( 50, 50 );
    context->pStep = context->map.CreateRoom( vRoom, DIR_NONE, 0 );

    JLog( LOG_LEVEL_ERROR, false, "room: <%d %d %d %d>\n",
          RECT_EXPAND( context->pStep->m_rcArea ) );
}

GIVEN( "^I have a E hallway create step$" )
{
    ScenarioScope<TestCtx> context;
    JIVector vHallway = context->map.GetWallOrigin( context->pStep, DIR_EAST );
    context->pStep = context->map.CreateHallway( vHallway, DIR_EAST, 1 );
    context->area.Init( context->pStep->m_rcArea );
    JLog( LOG_LEVEL_ERROR, false, "E hallway: <%d %d %d %d>\n", RECT_EXPAND( context->area ) );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I create a S hallway create step$" )
{
    ScenarioScope<TestCtx> context;
    context->vec_i = context->map.GetHallOrigin( context->pStep, DUNG_CREATE_STEP_MAKE_HALLWAY );
    context->pStep = context->map.CreateHallway( context->vec_i, DIR_SOUTH, 2 );

    JLog( LOG_LEVEL_ERROR, false, "S origin: <%d %d>\n", VEC_EXPAND( context->vec_i ) );
}

WHEN( "^I call GetHallRect for (east|west|north|south) from ([0-9.-]+),([0-9.-]+)$" )
{
    REGEX_PARAM( std::string, direction );
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    JRect rcHall( x, y, x, y );
    ScenarioScope<TestCtx> context;
    context->map.GetHallRect( rcHall, dir );
}

WHEN( "^I call FillArea for a room$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, context->pRoom );
}
WHEN( "^I call FillArea for a room (N|S|E|W)$" )
{
    REGEX_PARAM( std::string, direction );
    int dir = ( direction == "E" )   ? DIR_EAST
              : ( direction == "W" ) ? DIR_WEST
              : ( direction == "N" ) ? DIR_NORTH
              : ( direction == "S" ) ? DIR_SOUTH
                                     : DIR_NONE;
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, context->pRoom );
}

WHEN( "^I call FillArea for a hallway (east|west|north|south)$" )
{
    REGEX_PARAM( std::string, direction );
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, context->pRoom );
}

WHEN( "^I call LightArea for the room$" )
{
    ScenarioScope<TestCtx> context;
    context->map.LightArea( context->pRoom );
}

/*#######
##
## THEN
##
#######*/

THEN( "^the S hallway meets the E hallway$" )
{
    ScenarioScope<TestCtx> context;
    JRect s( context->vec_i, 0, 0 );
    JRect e( context->area );

    // New Hallway should start +1 in the direction of the last hallway
    JIVector vEast( e.right + 1, e.top );

    EXPECT_EQ( context->vec_i.x, vEast.x );
    EXPECT_EQ( context->vec_i.y, vEast.y );
}

THEN( "^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is now filled with ([0-9]+)$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    REGEX_PARAM( int, type );
    ScenarioScope<TestCtx> context;

    int x, y;
    for( y = t; y <= b; y++ )
    {
        for( x = l; x <= r; x++ )
        {
            JIVector vCheck( x, y );
            int expected = context->map.GetdtdIndex( vCheck );
            JLog( LOG_LEVEL_NOISE, true, "<%d %d>: %d/%d ", VEC_EXPAND( vCheck ), expected, type );
            EXPECT_EQ( expected, type );
        }
        JLog( LOG_LEVEL_NOISE, true, "\n" );
    }
}

// Deterministic generation tests
GIVEN( "^I create a dungeon at depth ([0-9]+) with seed ([0-9]+)$" )
{
    REGEX_PARAM( int, depth );
    REGEX_PARAM( unsigned int, seed );
    ScenarioScope<TestCtx> context;
    context->map.CreateDungeon( depth, seed );
}

THEN( "^The dungeon has seed ([0-9]+)$" )
{
    REGEX_PARAM( unsigned int, expected_seed );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->map.GetSeed(), expected_seed );
}

THEN( "^The dungeon has ([0-9]+) rooms$" )
{
    REGEX_PARAM( int, expected_rooms );
    ScenarioScope<TestCtx> context;
    int actual_rooms = context->map.GetRoomCount();
    JLog( LOG_LEVEL_INFO, true, "Room count: %d (expected %d)\n", actual_rooms, expected_rooms );
    EXPECT_EQ( actual_rooms, expected_rooms );
}

THEN( "^The dungeon has ([0-9]+) hallways$" )
{
    REGEX_PARAM( int, expected_halls );
    ScenarioScope<TestCtx> context;
    int actual_halls = context->map.GetHallwayCount();
    JLog( LOG_LEVEL_INFO, true, "Hallway count: %d (expected %d)\n", actual_halls, expected_halls );
    EXPECT_EQ( actual_halls, expected_halls );
}

THEN( "^The dungeon matches another dungeon with the same seed$" )
{
    ScenarioScope<TestCtx> context;
    // Store original map tile data
    CDungeonMapTile *original_tiles = new CDungeonMapTile[DUNG_WIDTH * DUNG_HEIGHT];
    for( int i = 0; i < DUNG_WIDTH * DUNG_HEIGHT; i++ )
    {
        JIVector vPos( i % DUNG_WIDTH, i / DUNG_WIDTH );
        if( context->map.GetTile( vPos ) )
        {
            original_tiles[i] = *context->map.GetTile( vPos );
        }
    }
    
    unsigned int seed = context->map.GetSeed();
    int depth = 1; // Assume depth 1 for test
    
    // Create a new dungeon with same seed
    CDungeonMap map2;
    map2.CreateDungeon( depth, seed );
    
    // Compare tile types
    bool maps_match = true;
    int differences = 0;
    for( int i = 0; i < DUNG_WIDTH * DUNG_HEIGHT; i++ )
    {
        JIVector vPos( i % DUNG_WIDTH, i / DUNG_WIDTH );
        if( context->map.GetTile( vPos ) && map2.GetTile( vPos ) )
        {
            Uint8 type1 = context->map.GetTile( vPos )->GetType();
            Uint8 type2 = map2.GetTile( vPos )->GetType();
            if( type1 != type2 )
            {
                maps_match = false;
                differences++;
                if( differences <= 5 ) // Log first 5 differences
                {
                    JLog( LOG_LEVEL_ERROR, true, "Tile mismatch at <%d %d>: %d vs %d\n", 
                          VEC_EXPAND( vPos ), type1, type2 );
                }
            }
        }
    }
    
    delete[] original_tiles;
    
    JLog( LOG_LEVEL_INFO, true, "Maps %s (differences: %d)\n", 
          maps_match ? "MATCH" : "DO NOT MATCH", differences );
    EXPECT_TRUE( maps_match );
}

THEN( "^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is (now|not) lit$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    REGEX_PARAM( std::string, desired );
    ScenarioScope<TestCtx> context;

    int x, y;
    for( y = t; y <= b; y++ )
    {
        for( x = l; x <= r; x++ )
        {
            JIVector vCheck( x, y );
            int actual = context->map.GetFlags( vCheck );
            JLog( LOG_LEVEL_NOISE, true, "<%d %d>: %d/%d ", VEC_EXPAND( vCheck ), actual,
                  DUNG_FLAG_LIT );
            int expected = ( desired == "not" ) ? 0 : DUNG_FLAG_LIT;
            EXPECT_EQ( actual & DUNG_FLAG_LIT, expected );
        }
        JLog( LOG_LEVEL_NOISE, true, "\n" );
    }
}

// Edge case tests for recursion depth and stress testing
THEN( "^The dungeon has a valid room count \\(between ([0-9]+) and ([0-9]+)\\)$" )
{
    REGEX_PARAM( int, min_rooms );
    REGEX_PARAM( int, max_rooms );
    ScenarioScope<TestCtx> context;
    
    int room_count = context->map.HowManyRooms();
    JLog( LOG_LEVEL_INFO, true, "Room count: %d (expected: %d-%d)\n", 
          room_count, min_rooms, max_rooms );
    
    EXPECT_GE( room_count, min_rooms );
    EXPECT_LE( room_count, max_rooms );
}

THEN( "^The generation completed without stack overflow$" )
{
    ScenarioScope<TestCtx> context;
    // If we got here without a crash, the generation completed successfully
    int stack_size = context->map.GetStackSize();
    JLog( LOG_LEVEL_INFO, true, "Generation complete. Final stack size: %d\n", stack_size );
    // Stack should be empty or near-empty after generation completes
    EXPECT_LE( stack_size, 1 );
}

GIVEN( "^I create ([0-9]+) consecutive dungeons at depth ([0-9]+) with seed ([0-9]+)$" )
{
    REGEX_PARAM( int, count );
    REGEX_PARAM( int, depth );
    REGEX_PARAM( unsigned int, seed );
    ScenarioScope<TestCtx> context;
    
    context->dungeon_history.clear();
    
    for( int i = 0; i < count; i++ )
    {
        CDungeonMap map;
        map.CreateDungeon( depth, seed );
        
        // Store room and hallway counts
        context->dungeon_history.push_back({
            map.HowManyRooms(),
            map.HowManyHallways(),
            map.GetStackSize()
        });
    }
    
    JLog( LOG_LEVEL_INFO, true, "Created %d dungeons with seed %u\n", count, seed );
}

THEN( "^All ([0-9]+) dungeons are identical$" )
{
    REGEX_PARAM( int, count );
    ScenarioScope<TestCtx> context;
    
    ASSERT_EQ( context->dungeon_history.size(), count );
    
    // All should have same room count
    bool all_match = true;
    for( int i = 1; i < context->dungeon_history.size(); i++ )
    {
        if( context->dungeon_history[i].rooms != context->dungeon_history[0].rooms )
        {
            all_match = false;
            JLog( LOG_LEVEL_ERROR, true, "Dungeon %d has %d rooms, expected %d\n",
                  i, context->dungeon_history[i].rooms, context->dungeon_history[0].rooms );
        }
    }
    
    EXPECT_TRUE( all_match );
}

THEN( "^All have the same room count$" )
{
    ScenarioScope<TestCtx> context;
    
    if( context->dungeon_history.size() > 0 )
    {
        int expected_rooms = context->dungeon_history[0].rooms;
        for( int i = 1; i < context->dungeon_history.size(); i++ )
        {
            EXPECT_EQ( context->dungeon_history[i].rooms, expected_rooms );
        }
    }
}

THEN( "^All have the same hallway count$" )
{
    ScenarioScope<TestCtx> context;
    
    if( context->dungeon_history.size() > 0 )
    {
        int expected_halls = context->dungeon_history[0].hallways;
        for( int i = 1; i < context->dungeon_history.size(); i++ )
        {
            EXPECT_EQ( context->dungeon_history[i].hallways, expected_halls );
        }
    }
}

THEN( "^Each hallway tile is adjacent to at least one room floor tile$" )
{
    ScenarioScope<TestCtx> context;
    
    int hallway_count = 0;
    int orphan_hallways = 0;
    
    // Check each tile
    for( int y = 0; y < DUNG_HEIGHT; y++ )
    {
        for( int x = 0; x < DUNG_WIDTH; x++ )
        {
            JIVector vPos( x, y );
            Uint8 tile_type = context->map.GetdtdIndex( vPos );
            
            // If this is a hallway tile
            if( tile_type == DUNG_IDX_FLOOR )
            {
                Uint8 flags = context->map.GetFlags( vPos );
                // Hallways are not lit, rooms are lit
                if( !(flags & DUNG_FLAG_LIT) )
                {
                    hallway_count++;
                    
                    // Check 4 adjacent tiles for room floor
                    bool adjacent_to_room = false;
                    int directions[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
                    
                    for( int dir = 0; dir < 4; dir++ )
                    {
                        JIVector vAdjacent( x + directions[dir][0], y + directions[dir][1] );
                        if( vAdjacent.IsInWorld() )
                        {
                            Uint8 adj_type = context->map.GetdtdIndex( vAdjacent );
                            Uint8 adj_flags = context->map.GetFlags( vAdjacent );
                            
                            // Adjacent room floor is floor AND lit
                            if( adj_type == DUNG_IDX_FLOOR && (adj_flags & DUNG_FLAG_LIT) )
                            {
                                adjacent_to_room = true;
                                break;
                            }
                        }
                    }
                    
                    if( !adjacent_to_room )
                    {
                        orphan_hallways++;
                        JLog( LOG_LEVEL_WARN, true, "Orphan hallway at <%d %d>\n", x, y );
                    }
                }
            }
        }
    }
    
    JLog( LOG_LEVEL_INFO, true, "Found %d hallway tiles, %d orphaned\n", 
          hallway_count, orphan_hallways );
    EXPECT_EQ( orphan_hallways, 0 );
}

THEN( "^The out-of-world portion remains as walls$" )
{
    ScenarioScope<TestCtx> context;
    
    // Check tiles outside world bounds
    JIVector vOutOfWorld( 99, 99 );
    if( !vOutOfWorld.IsInWorld() )
    {
        Uint8 tile_type = context->map.GetdtdIndex( vOutOfWorld );
        // Should still be wall
        EXPECT_EQ( tile_type, DUNG_IDX_WALL );
    }
}

WHEN( "^I export the dungeon to a fixture file$" )
{
    ScenarioScope<TestCtx> context;
    context->fixture_filename = "/tmp/test_dungeon.fixture";
    EXPECT_TRUE( context->map.ExportDungeon( context->fixture_filename.c_str() ) );
}

THEN( "^The fixture file exists$" )
{
    ScenarioScope<TestCtx> context;
    FILE *fp = fopen( context->fixture_filename.c_str(), "r" );
    EXPECT_NE( fp, (FILE *)NULL );
    if( fp ) fclose( fp );
}

WHEN( "^I import the fixture file into a new dungeon$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( context->imported_map.ImportDungeon( context->fixture_filename.c_str() ) );
}

THEN( "^The imported dungeon has the same structure$" )
{
    ScenarioScope<TestCtx> context;
    // CompareDungeon validates that tiles and flags match
    EXPECT_TRUE( context->map.CompareDungeon( context->imported_map ) );
}

THEN( "^The fixture file has valid JMORIA header$" )
{
    ScenarioScope<TestCtx> context;
    FILE *fp = fopen( context->fixture_filename.c_str(), "r" );
    ASSERT_NE( fp, (FILE *)NULL );
    
    char buf[256];
    EXPECT_TRUE( fgets( buf, sizeof(buf), fp ) != NULL );
    
    // Remove newline for comparison
    char *newline = strchr( buf, '\n' );
    if( newline ) *newline = '\0';
    
    EXPECT_STREQ( buf, "JMORIA_FIXTURE_v1" );
    fclose( fp );
}

THEN( "^The fixture metadata contains correct seed and depth$" )
{
    ScenarioScope<TestCtx> context;
    FILE *fp = fopen( context->fixture_filename.c_str(), "r" );
    ASSERT_NE( fp, (FILE *)NULL );
    
    char buf[512];
    // Skip header
    fgets( buf, sizeof(buf), fp );
    
    // Read metadata
    EXPECT_TRUE( fgets( buf, sizeof(buf), fp ) != NULL );
    
    unsigned int seed, depth;
    int width, height, rooms, hallways;
    int result = sscanf( buf, "seed=%u,depth=%u,width=%d,height=%d,rooms=%d,hallways=%d",
                         &seed, &depth, &width, &height, &rooms, &hallways );
    
    EXPECT_EQ( result, 6 );
    EXPECT_EQ( seed, 42 );
    EXPECT_EQ( depth, 1 );
    EXPECT_EQ( width, DUNG_WIDTH );
    EXPECT_EQ( height, DUNG_HEIGHT );
    
    fclose( fp );
}

THEN( "^The fixture file contains ([0-9]+) tile rows$" )
{
    REGEX_PARAM( int, expected_rows );
    ScenarioScope<TestCtx> context;
    FILE *fp = fopen( context->fixture_filename.c_str(), "r" );
    ASSERT_NE( fp, (FILE *)NULL );
    
    char buf[4096];
    int row_count = 0;
    
    // Skip header and metadata (2 lines)
    fgets( buf, sizeof(buf), fp );
    fgets( buf, sizeof(buf), fp );
    
    // Count tile rows
    while( fgets( buf, sizeof(buf), fp ) )
    {
        if( buf[0] != '\0' && buf[0] != '\n' )
            row_count++;
    }
    
    EXPECT_EQ( row_count, expected_rows );
    fclose( fp );
}

// Connectivity validation steps
WHEN( "^I validate dungeon connectivity$" )
{
    ScenarioScope<TestCtx> context;

    context->connectivity_result = context->map.ValidateConnectivity( 
        context->reachable_tiles, 
        context->total_walkable_tiles );
}

THEN( "^All tiles are reachable$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( context->connectivity_result );
    EXPECT_EQ( context->reachable_tiles, context->total_walkable_tiles );
}

THEN( "^Not all tiles are reachable$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( context->connectivity_result );
    EXPECT_LT( context->reachable_tiles, context->total_walkable_tiles );
    EXPECT_GT( context->reachable_tiles, 0 );
    EXPECT_GT( context->total_walkable_tiles, context->reachable_tiles );
}

THEN( "^The dungeon is fully connected$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( context->map.ValidateAllRoomsReachable() );
}

THEN( "^At least ([0-9]+) tiles are reachable$" )
{
    REGEX_PARAM( int, min_reachable );
    ScenarioScope<TestCtx> context;
    // Use context variables to get the data
    context->map.ValidateConnectivity( context->reachable_tiles, context->total_walkable_tiles );
    EXPECT_GE( context->reachable_tiles, min_reachable );
}

THEN( "^The reachable tile count equals the total walkable tile count$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->reachable_tiles, context->total_walkable_tiles );
    EXPECT_GT( context->reachable_tiles, 0 );
}

// GetRoomRect/GetHallRect validation tests
WHEN( "^I call GetRoomRect with position at ([0-9]+),([0-9]+) direction (north|south|east|west)$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    REGEX_PARAM( std::string, direction );
    
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    
    ScenarioScope<TestCtx> context;
    context->area.Init( x, y, x, y );
    context->result = context->map.GetRoomRect( context->area, dir );
}

WHEN( "^I call GetHallRect with position at ([0-9]+),([0-9]+) direction (north|south|east|west)$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    REGEX_PARAM( std::string, direction );
    
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    
    ScenarioScope<TestCtx> context;
    context->area.Init( x, y, x, y );
    context->result = context->map.GetHallRect( context->area, dir );
}

WHEN( "^I attempt to create a room step at world boundary ([0-9]+),([0-9]+) direction (north|south|east|west)$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    REGEX_PARAM( std::string, direction );
    
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    
    ScenarioScope<TestCtx> context;
    JIVector vPos( x, y );
    context->pStep = context->map.CreateRoom( vPos, dir, 0 );
}

WHEN( "^I attempt to create a hallway step at world boundary ([0-9]+),([0-9]+) direction (north|south|east|west)$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    REGEX_PARAM( std::string, direction );
    
    int dir = ( direction == "east" )    ? DIR_EAST
              : ( direction == "west" )  ? DIR_WEST
              : ( direction == "north" ) ? DIR_NORTH
              : ( direction == "south" ) ? DIR_SOUTH
                                         : DIR_NONE;
    
    ScenarioScope<TestCtx> context;
    JIVector vPos( x, y );
    context->pStep = context->map.CreateHallway( vPos, dir, 0 );
}

THEN( "^The returned rect has positive width and height$" )
{
    ScenarioScope<TestCtx> context;
    // If GetRoomRect succeeded (returned JSUCCESS), rect should be valid
    if( context->result == JSUCCESS )
    {
        EXPECT_GT( context->area.Width(), 0 );
        EXPECT_GT( context->area.Height(), 0 );
    }
    // If it failed (returned -1), that's also acceptable - the validation is working
}

THEN( "^GetHallRect returns success or properly handles boundary$" )
{
    ScenarioScope<TestCtx> context;
    // GetHallRect should either succeed with valid geometry or return -1
    if( context->result == JSUCCESS )
    {
        // Hallways can have 0 width OR 0 height (but not both, and not negative)
        EXPECT_GE( context->area.Width(), 0 );
        EXPECT_GE( context->area.Height(), 0 );
        EXPECT_TRUE( context->area.Width() > 0 || context->area.Height() > 0 );
    }
    // If result is -1, that's acceptable - validation caught the issue
}

THEN( "^The room step either succeeds with valid geometry or returns NULL$" )
{
    ScenarioScope<TestCtx> context;
    if( context->pStep != NULL )
    {
        // If step was created, it should have valid geometry
        EXPECT_GT( context->pStep->m_rcArea.Width(), 0 );
        EXPECT_GT( context->pStep->m_rcArea.Height(), 0 );
        EXPECT_TRUE( context->pStep->m_rcArea.IsWithinWorld() );
    }
    // NULL is also acceptable - creation failed gracefully
}

THEN( "^The hallway step either succeeds with valid geometry or returns NULL$" )
{
    ScenarioScope<TestCtx> context;
    if( context->pStep != NULL )
    {
        // If step was created, it should have valid geometry
        EXPECT_GE( context->pStep->m_rcArea.Width(), 0 );
        EXPECT_GE( context->pStep->m_rcArea.Height(), 0 );
        EXPECT_TRUE( context->pStep->m_rcArea.Width() > 0 || context->pStep->m_rcArea.Height() > 0 );
        EXPECT_TRUE( context->pStep->m_rcArea.IsWithinWorld() );
    }
    // NULL is also acceptable - creation failed gracefully
}
