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
    context->map.FillArea( DUNG_IDX_FLOOR, &( rcBlocking ), DIR_NONE, false );
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

GIVEN("^I have an origin (([0-9]+),([0-9]+))$")
{
    REGEX_PARAM( float, x );
    REGEX_PARAM( float, y );
    ScenarioScope<TestCtx> context;
    context->origin = JIVector(x,y);
}

GIVEN("^I have a DungeonCreationStep$")
{
    ScenarioScope<TestCtx> context;
    CDungeonCreationStep *step = context->map.MakeRoomStep(context->origin, DIR_NONE, 0);
    context->dungeonCreationStep = step;
}

GIVEN("^I have a direction north$")
{
    ScenarioScope<TestCtx> context;
    context->direction = DIR_NORTH;
}

/*#######
##
## WHEN
##
#######*/
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
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NONE, false );
}
WHEN( "^I call GetHallRect for north from ([0-9.-]+),([0-9.-]+)$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NONE, false );
}
WHEN( "^I call GetHallRect for south from ([0-9.-]+),([0-9.-]+)$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NONE, false );
}
WHEN( "^I call FillArea for a room$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NONE, false );
}
WHEN( "^I call FillArea for a room N$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NORTH, false );
}
WHEN( "^I call FillArea for a room S$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_SOUTH, false );
}
WHEN( "^I call FillArea for a room E$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_EAST, false );
}
WHEN( "^I call FillArea for a room W$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_WEST, false );
}

WHEN( "^I call FillArea for a hallway north$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_NORTH, true );
}

WHEN( "^I call FillArea for a hallway south$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_SOUTH, true );
}

WHEN( "^I call FillArea for a hallway west$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_WEST, true );
}

WHEN( "^I call FillArea for a hallway east$" )
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &( context->area ), DIR_EAST, true );
}

WHEN( "^I call CheckArea$" )
{
    ScenarioScope<TestCtx> context;
    bool areaChecked = context->map.CheckArea( context->dungeonCreationStep );
    context->areaChecked = areaChecked;
}

WHEN("^I call get room rect$")
{
    ScenarioScope<TestCtx> context;
    JIVector vPos;
    vPos.Init(3, 3);
    JRect rcRoom;
    rcRoom.Init( vPos, 0, 0 );
    JIVector vSize;
    vSize.Init(10,10);
    context->map.GetRoomRect(context->direction, rcRoom, vSize);
    context->room = rcRoom;
}


/*#######
##
## THEN
##
#######*/

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
            // printf("<%d %d>: %d/%d ", VEC_EXPAND(vCheck), expected, type);
            EXPECT_EQ( expected, type );
        }
        // printf("\n");
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
            // printf("<%d %d>: %d/%d ", VEC_EXPAND(vCheck), expected, DUNG_FLAG_LIT);
            EXPECT_EQ( expected, DUNG_FLAG_LIT );
        }
        // printf("\n");
    }
}

THEN("^I'll have a CDungeonCreationStep$")
{
    ScenarioScope<TestCtx> context;
    // printf("Step is %d\n", context->dungeonCreationStep->m_dwIndex);
    EXPECT_EQ( context->dungeonCreationStep->m_dwIndex, DUNG_CREATE_STEP_MAKE_ROOM );
}

THEN("^I'll have a valid area$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->areaChecked, 1 );
}

THEN("^I'll have a valid N room rect$")
{
    ScenarioScope<TestCtx> context;
    JRect expected = context->room;
    printf("JRect is <%d, %d, %d, %d>\n", RECT_EXPAND(expected) );
    EXPECT_EQ( expected.Width(), 6 );
    EXPECT_EQ( expected.Height(), 2 );
}