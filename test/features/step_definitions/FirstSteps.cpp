#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^It is Wednesday$")
{
  ScenarioScope<TestCtx> context;
  context->vec = JVector(0,0);
}

GIVEN("^I have a JVector ([0-9.-]+),([0-9.-]+) in the house$")
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

WHEN("^I press add_vector$")
{
  ScenarioScope<TestCtx> context;

  context->result_vec = context->vec_b + context->vec;
}


/*#######
##
## THEN
##
#######*/

THEN("^Wearing Pink$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ(context->vec,JVector(0,0));
}

THEN("^It is (a vector|not a position)$")
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
      VEC_EXPAND(expected);
      // std::cout << std::endl;
      VEC_EXPAND(actual);
      // std::cout << std::endl;
      result = false;
  }

  EXPECT_EQ(result, true);
  // EXPECT_EQ(context->result.w, 0.0);
}