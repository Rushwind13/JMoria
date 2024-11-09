#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

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
    context->map.FillArea( DUNG_IDX_FLOOR, &( rcBlocking ) );
}

GIVEN( "^I have a JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) to fill$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    ScenarioScope<TestCtx> context;
    context->area = JRect( l, t, r, b );
}

GIVEN("^I have a room create step$")
{
    ScenarioScope<TestCtx> context;
    JIVector vRoom(50,50);
    context->pStep = context->map.MakeRoomStep(vRoom,DIR_NONE,0);

    JLog(LOG_LEVEL_ERROR, false, "room: <%d %d %d %d>\n", RECT_EXPAND(context->pStep->m_rcArea));
}

GIVEN("^I have a E hallway create step$")
{
    ScenarioScope<TestCtx> context;
    JIVector vHallway = context->map.GetWallOrigin(context->pStep, DIR_EAST);
    context->pStep = context->map.MakeHallStep(vHallway,DIR_EAST,1);
    context->area.Init(context->pStep->m_rcArea);
    JLog(LOG_LEVEL_ERROR, false, "E hallway: <%d %d %d %d>\n", RECT_EXPAND(context->area));
}

/*#######
##
## WHEN
##
#######*/
WHEN("^I create a S hallway create step$")
{
    ScenarioScope<TestCtx> context;
    context->vec_i = context->map.GetHallOrigin(context->pStep, DUNG_CREATE_STEP_MAKE_HALLWAY);
    context->pStep = context->map.MakeHallStep(context->vec_i,DIR_SOUTH,2);

    JLog(LOG_LEVEL_ERROR, false, "S origin: <%d %d>\n", VEC_EXPAND(context->vec_i));
}

WHEN( "^I call GetHallRect for east from ([0-9.-]+),([0-9.-]+)$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    JRect rcHall( x, y, x, y );
    ScenarioScope<TestCtx> context;
    context->map.GetHallRect( rcHall, DIR_EAST );
}
WHEN( "^I call GetHallRect for west from ([0-9.-]+),([0-9.-]+)$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call GetHallRect for north from ([0-9.-]+),([0-9.-]+)$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call GetHallRect for south from ([0-9.-]+),([0-9.-]+)$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call FillArea for a room$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call FillArea for a room N$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call FillArea for a room S$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call FillArea for a room E$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}
WHEN( "^I call FillArea for a room W$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}

WHEN( "^I call FillArea for a hallway north$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}

WHEN( "^I call FillArea for a hallway south$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}

WHEN( "^I call FillArea for a hallway west$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}

WHEN( "^I call FillArea for a hallway east$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ) );
}

/*#######
##
## THEN
##
#######*/

THEN("^the S hallway meets the E hallway$")
{
    ScenarioScope<TestCtx> context;
    JRect s(context->vec_i,0,0);
    JRect e(context->area);

    // New Hallway should start +1 in the direction of the last hallway
    JIVector vEast(e.right+1, e.top);

    EXPECT_EQ(context->vec_i.x, vEast.x);
    EXPECT_EQ(context->vec_i.y, vEast.y);
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

THEN( "^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is now lit$" )
{
    REGEX_PARAM( float, l );
    REGEX_PARAM( float, t );
    REGEX_PARAM( float, r );
    REGEX_PARAM( float, b );
    ScenarioScope<TestCtx> context;

    int x, y;
    for( y = t; y <= b; y++ )
    {
        for( x = l; x <= r; x++ )
        {
            JIVector vCheck( x, y );
            int expected = context->map.GetFlags( vCheck );
            JLog( LOG_LEVEL_NOISE, true, "<%d %d>: %d/%d ", VEC_EXPAND( vCheck ), expected,
                  DUNG_FLAG_LIT );
            EXPECT_EQ( expected, DUNG_FLAG_LIT );
        }
        JLog( LOG_LEVEL_NOISE, true, "\n" );
    }
}