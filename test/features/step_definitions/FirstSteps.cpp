#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"
AFTER_ALL() { JLog( LOG_LEVEL_ERROR, false, "-------------------- (After all scenarios)\n" ); }
AFTER()
{
    g_pGame = NULL;
    JLog( LOG_LEVEL_ERROR, false, "-------------------- (After each scenario)\n" );
}
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