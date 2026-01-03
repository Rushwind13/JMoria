#include "TestContext.hpp"
#include "AIRender.h"

using cucumber::ScenarioScope;

// Test context extension for AIRender tests
static char g_szTestRow[256];
static int g_nTestRowLen;
static char g_szEncodedResult[256];
static int g_nEncodedLen;
static bool g_bIsAllWallsResult;

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I have a row \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, row );
    strncpy( g_szTestRow, row.c_str(), sizeof( g_szTestRow ) - 1 );
    g_szTestRow[sizeof( g_szTestRow ) - 1] = '\0';
    g_nTestRowLen = strlen( g_szTestRow );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^I call RLEEncodeRow$" )
{
    g_nEncodedLen = AIRender_RLEEncodeRow( g_szTestRow, g_nTestRowLen, g_szEncodedResult );
    g_szEncodedResult[g_nEncodedLen] = '\0';
}

WHEN( "^I call IsAllWalls$" )
{
    g_bIsAllWallsResult = AIRender_IsAllWalls( g_szTestRow, g_nTestRowLen );
}

/*#######
##
## THEN
##
#######*/

THEN( "^The encoded result is \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, expected );
    EXPECT_STREQ( expected.c_str(), g_szEncodedResult );
}

THEN( "^IsAllWalls returns true$" )
{
    EXPECT_TRUE( g_bIsAllWallsResult );
}

THEN( "^IsAllWalls returns false$" )
{
    EXPECT_FALSE( g_bIsAllWallsResult );
}
