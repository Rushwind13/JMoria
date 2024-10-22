#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^It is Wednesday$" )
{
    ScenarioScope<TestCtx> context;
    context->vec = JVector( 0, 0 );
}

/*#######
##
## WHEN
##
#######*/

/*#######
##
## THEN
##
#######*/

THEN( "^Wearing Pink$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->vec, JVector( 0, 0 ) );
}