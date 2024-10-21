#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I have a brain$")
{
  ScenarioScope<TestCtx> context;
  context->brain = new CAIBrain;
  context->brain->SetState(BRAINSTATE_SEEK);

}

GIVEN("^I set the move type to MON_AI_SEEKPLAYER$")
{
  ScenarioScope<TestCtx> context;
  context->brain->m_dwMoveType = MON_AI_SEEKPLAYER;
}

/*#######
##
## WHEN
##
#######*/
WHEN("^I set the brain state to ([0-9]+)$")
{
  REGEX_PARAM(int, desired);
  ScenarioScope<TestCtx> context;
  context->brain->SetState((eBrainState)desired);
}

WHEN("^I call UpdateSeek$")
{
  ScenarioScope<TestCtx> context;
  bool result = context->brain->UpdateSeek(1.0f);
  context->result_bool = result;
}

WHEN("^I call WalkSeek$")
{
  ScenarioScope<TestCtx> context;
  JVector dest(0,0);
  bool result = context->brain->WalkSeek(1.0f, dest, DUNG_COLL_NO_COLLISION);
  context->result_bool = result;
}

/*#######
##
## THEN
##
#######*/

THEN("^GetState returns ([0-9]+)$")
{
  REGEX_PARAM(int, expected);
  ScenarioScope<TestCtx> context;
  eBrainState actual = context->brain->GetState();

  EXPECT_EQ(expected, actual);
}

THEN("^UpdateSeek was successful$")
{
  ScenarioScope<TestCtx> context;
  bool actual = context->result_bool;

  EXPECT_EQ(actual, true);
}

THEN("^WalkSeek was successful$")
{
  ScenarioScope<TestCtx> context;
  bool actual = context->result_bool;

  EXPECT_EQ(actual, true);
}