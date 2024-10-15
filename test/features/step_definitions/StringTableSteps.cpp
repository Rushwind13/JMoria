#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I initialize my Constants$")
{
  ScenarioScope<TestCtx> context;
  context->constants.Init();
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

THEN("^I get a StringTable with <([A-Z_]+)> in it with index ([0-9]+)$")
{
    REGEX_PARAM(std::string, entry);
    REGEX_PARAM(int, index);
    ScenarioScope<TestCtx> context;
    int actual = context->constants.LookupString(entry.c_str());
    EXPECT_EQ(actual, index);
}