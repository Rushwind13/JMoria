#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I have a JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) in the data$")
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

/*#######
##
## THEN
##
#######*/

THEN("^The rect is in world$") {
    bool expected = true;
    ScenarioScope<TestCtx> context;
    bool actual = context->area.IsInWorld();

    EXPECT_EQ(expected, actual);
}

THEN("^The rect is not in world$") {
    bool expected = false;
    ScenarioScope<TestCtx> context;
    bool actual = context->area.IsInWorld();

    EXPECT_EQ(expected, actual);
}