#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I load the player tileset$" )
{
    ScenarioScope<TestCtx> context;
    context->tileset = new CTileset();
    memset( context->szTileset, 0, 1024 );
    strcpy( context->szTileset, "../../JMoria/Resources/Courier.png" );
    context->aspect.Init( 32, 32 );
}

GIVEN( "^I load the messages tileset$" )
{
    ScenarioScope<TestCtx> context;
    context->tileset = new CTileset();
    memset( context->szTileset, 0, 1024 );
    strcpy( context->szTileset, "../../JMoria/Resources/SmallText6X8.png" );
    context->aspect.Init( 6, 8 );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^I call Load$" )
{
    ScenarioScope<TestCtx> context;

    context->result = context->tileset->Load( context->szTileset, VEC_EXPAND( context->aspect ) );
}

WHEN( "^I ask for tile ([0-9.-]+)$" )
{
    REGEX_PARAM( int, index );
    ScenarioScope<TestCtx> context;
    context->index = index;
    context->vTile = context->tileset->GetTile( context->index, context->vTile );
}

/*#######
##
## THEN
##
#######*/

THEN( "^I get a successful result$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->result, JSUCCESS );
}

THEN( "^The player tileset is loaded$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->tileset->IsBMP( "anything" ), false );
    EXPECT_EQ( context->tileset->TexelW(), 0.0625f );
}

THEN( "^The messages tileset is loaded$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->tileset->IsBMP( "anything" ), false );
    EXPECT_EQ( context->tileset->TexelW(), 0.0625f );
}

THEN( "^I get tile ([0-9.-]+),([0-9.-]+) back$" )
{
    REGEX_PARAM( int, x );
    REGEX_PARAM( int, y );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->vTile.x, x );
    EXPECT_EQ( context->vTile.y, y );
}