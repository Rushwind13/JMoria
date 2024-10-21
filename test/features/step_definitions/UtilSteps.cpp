#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

#include "Util.h"
using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I ask for a ([0-9-]+),([0-9-]+) result$")
{
  REGEX_PARAM(int, lo);
  REGEX_PARAM(int, hi);
  ScenarioScope<TestCtx> context;
  context->lo = lo;
  context->hi = hi;
}

GIVEN("^I ask for a ([0-9.-]+),([0-9.-]+) float result$")
{
  REGEX_PARAM(float, lo);
  REGEX_PARAM(float, hi);
  ScenarioScope<TestCtx> context;
  context->lo_f = lo;
  context->hi_f = hi;
}

GIVEN("^I ask for a ([0-9-]+)d([0-9-]+) dice roll$")
{
  REGEX_PARAM(int, count);
  REGEX_PARAM(int, sides);
  ScenarioScope<TestCtx> context;
  context->lo = count;
  context->hi = sides;
}

/*#######
##
## WHEN
##
#######*/

WHEN("^I call GetRandom$")
{
  ScenarioScope<TestCtx> context;

  context->result_int = Util::GetRandom(context->lo, context->hi);
}

WHEN("^I call GetRandom as float$")
{
  ScenarioScope<TestCtx> context;

  context->result_float = Util::GetRandom(context->lo_f, context->hi_f);
}

WHEN("^I call Roll 3d6$")
{
  ScenarioScope<TestCtx> context;

  context->result_int = Util::Roll("3d6");
}

WHEN("^I call Roll$")
{
  ScenarioScope<TestCtx> context;

  context->result_int = Util::Roll(context->lo, context->hi);
}

/*#######
##
## THEN
##
#######*/

THEN("^I get either 0 or 1$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_GE(context->result_int, 0);
    EXPECT_LE(context->result_int, 1);
}

THEN("^The result is between ([0-9-]+) and ([0-9-]+)$")
{
    REGEX_PARAM(int, lo);
    REGEX_PARAM(int, hi);
    ScenarioScope<TestCtx> context;
    EXPECT_GE(context->result_int, lo);
    EXPECT_LE(context->result_int, hi);
}

THEN("^The float result is between ([0-9.-]+) and ([0-9.-]+)$")
{
    REGEX_PARAM(float, lo);
    REGEX_PARAM(float, hi);
    ScenarioScope<TestCtx> context;
    EXPECT_GE(context->result_float, lo);
    EXPECT_LE(context->result_float, hi);
}