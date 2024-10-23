#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I have a game$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame = new CGame;
}

/*#######
##
## WHEN
##
#######*/
WHEN("^I initialize the game$")
{
    ScenarioScope<TestCtx> context;
    context->result = g_pGame->Init("../../JMoria/");
}

/*#######
##
## THEN
##
#######*/

THEN( "^the game initalized successfully$" )
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}