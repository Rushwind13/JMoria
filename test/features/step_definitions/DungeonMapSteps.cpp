#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/
GIVEN("^I have a DungeonMap$")
{
    ScenarioScope<TestCtx> context;
    context->map.CreateDungeon(1);
}

GIVEN("^There is already a room at ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) in the dungeon$")
{
  REGEX_PARAM(float,l);
  REGEX_PARAM(float,t);
  REGEX_PARAM(float,r);
  REGEX_PARAM(float,b);
  JRect rcBlocking(l,t,r,b);
  ScenarioScope<TestCtx> context;
  context->map.FillArea( DUNG_IDX_FLOOR, &(rcBlocking), DIR_NONE, false );
}

GIVEN("^I have a JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) to fill$")
{
  REGEX_PARAM(float,l);
  REGEX_PARAM(float,t);
  REGEX_PARAM(float,r);
  REGEX_PARAM(float,b);
  ScenarioScope<TestCtx> context;
  context->area = JRect(l,t,r,b);
}

/*#######
##
## WHEN
##
#######*/
WHEN("^I call GetHallRect for east from ([0-9.-]+),([0-9.-]+)$")
{
    REGEX_PARAM(int,x);
    REGEX_PARAM(int,y);
    JRect rcHall(x,y,x,y);
    ScenarioScope<TestCtx> context;
    context->map.GetHallRect( rcHall, DIR_EAST );
}
WHEN("^I call GetHallRect for west from ([0-9.-]+),([0-9.-]+)$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NONE, false );
}
WHEN("^I call GetHallRect for north from ([0-9.-]+),([0-9.-]+)$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NONE, false );
}
WHEN("^I call GetHallRect for south from ([0-9.-]+),([0-9.-]+)$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NONE, false );
}
WHEN("^I call FillArea for a room$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NONE, false );
}
WHEN("^I call FillArea for a room N$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NORTH, false );
}
WHEN("^I call FillArea for a room S$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_SOUTH, false );
}
WHEN("^I call FillArea for a room E$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_EAST, false );
}
WHEN("^I call FillArea for a room W$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_WEST, false );
}

WHEN("^I call FillArea for a hallway north$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_NORTH, true );
}

WHEN("^I call FillArea for a hallway south$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_SOUTH, true );
}

WHEN("^I call FillArea for a hallway west$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_WEST, true );
}

WHEN("^I call FillArea for a hallway east$")
{
    ScenarioScope<TestCtx> context;
    context->map.FillArea( DUNG_IDX_FLOOR, &(context->area), DIR_EAST, true );
}

/*#######
##
## THEN
##
#######*/

THEN("^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is now filled with ([0-9]+)$")
{
    REGEX_PARAM(float,l);
    REGEX_PARAM(float,t);
    REGEX_PARAM(float,r);
    REGEX_PARAM(float,b);
    REGEX_PARAM(int,type);
    ScenarioScope<TestCtx> context;

    int x,y;
    for(y=t; y<=b; y++)
    {
        for(x=l; x<=r; x++)
        {
            JIVector vCheck(x,y);
            int expected = context->map.GetdtdIndex(vCheck);
            // printf("<%d %d>: %d/%d ", VEC_EXPAND(vCheck), expected, type);
            EXPECT_EQ(expected, type);
        }
        // printf("\n");
    }
}

THEN("^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is now lit$")
{
    REGEX_PARAM(float,l);
    REGEX_PARAM(float,t);
    REGEX_PARAM(float,r);
    REGEX_PARAM(float,b);
    ScenarioScope<TestCtx> context;

    int x,y;
    for(y=t; y<=b; y++)
    {
        for(x=l; x<=r; x++)
        {
            JIVector vCheck(x,y);
            int expected = context->map.GetFlags(vCheck);
            // printf("<%d %d>: %d/%d ", VEC_EXPAND(vCheck), expected, DUNG_FLAG_LIT);
            EXPECT_EQ(expected, DUNG_FLAG_LIT);
        }
        // printf("\n");
    }
}