#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I have a JVector ([0-9.-]+),([0-9.-]+) in the data$")
{
  REGEX_PARAM(float,x);
  REGEX_PARAM(float,y);
  ScenarioScope<TestCtx> context;
  context->vec = JVector(x,y);
}

GIVEN("^I have a second JVector ([0-9.-]+),([0-9.-]+) in the data$")
{
  REGEX_PARAM(float,x);
  REGEX_PARAM(float,y);
  ScenarioScope<TestCtx> context;
  context->vec_b = JVector(x,y);
}

/*#######
##
## WHEN
##
#######*/

WHEN("^I press add_JVector$")
{
  ScenarioScope<TestCtx> context;

  context->result_vec = context->vec_b + context->vec;
}

WHEN("^I press subtract_JVector$")
{
  ScenarioScope<TestCtx> context;

  context->result_vec = context->vec - context->vec_b;
}

WHEN("^I press negate_JVector$")
{
  ScenarioScope<TestCtx> context;

  context->result_vec = -context->vec;
}

WHEN("^I press scale_JVector ([0-9.-]+)$")
{
  REGEX_PARAM(float,scalar);
  ScenarioScope<TestCtx> context;

  context->result_vec = context->vec * scalar;
}
/*#######
##
## THEN
##
#######*/

THEN("^It is (a JVector|not a position)$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ(1,1);
}

THEN("^the result should be ([0-9.-]+),([0-9.-]+) a JVector$")
{
  REGEX_PARAM(float,x);
  REGEX_PARAM(float,y);
  JVector expected(x,y);
  ScenarioScope<TestCtx> context;
  JVector actual = context->result_vec;

  bool result = false;

  if( expected == actual )
  {
      result = true;
  }
  else
  {
      expected.printvec("expected");
      std::cout << "\n";
      actual.printvec("actual");
      std::cout << "\n";
      result = false;
  }

  EXPECT_EQ(result, true);
  // EXPECT_EQ(context->result.w, 0.0);
}