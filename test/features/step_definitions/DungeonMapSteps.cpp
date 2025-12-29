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
    context->pStep = context->map.MakeRoomStep( vRoom, DIR_NONE, 0 );

    JLog( LOG_LEVEL_ERROR, false, "room: <%d %d %d %d>\n",
          RECT_EXPAND( context->pStep->m_rcArea ) );
}

GIVEN( "^I have a E hallway create step$" )
{
    ScenarioScope<TestCtx> context;
    JIVector vHallway = context->map.GetWallOrigin( context->pStep, DIR_EAST );
    context->pStep = context->map.MakeHallStep( vHallway, DIR_EAST, 1 );
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
    context->pStep = context->map.MakeHallStep( context->vec_i, DIR_SOUTH, 2 );

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