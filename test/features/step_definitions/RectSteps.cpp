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
WHEN("^I call SetWidth ([0-9]+),true$")
{
    REGEX_PARAM(int, width);
    ScenarioScope<TestCtx> context;
    context->area.SetWidth(width,true);
}
WHEN("^I call SetWidth ([0-9]+),false$")
{
    REGEX_PARAM(int, width);
    ScenarioScope<TestCtx> context;
    context->area.SetWidth(width,false);
}

/*#######
##
## THEN
##
#######*/

THEN("^The rect corners are now ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+)$")
{
    REGEX_PARAM(float,l);
    REGEX_PARAM(float,t);
    REGEX_PARAM(float,r);
    REGEX_PARAM(float,b);
    JRect expected(l,t,r,b);
    ScenarioScope<TestCtx> context;

    EXPECT_EQ(expected.left, context->area.left);
    EXPECT_EQ(expected.top, context->area.top);
    EXPECT_EQ(expected.right, context->area.right);
    EXPECT_EQ(expected.bottom, context->area.bottom);
}

THEN("^The rect width is ([0-9]+)$") {
    REGEX_PARAM(int, expected);
    ScenarioScope<TestCtx> context;
    int actual = context->area.Width();

    EXPECT_EQ(expected, actual);
}

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