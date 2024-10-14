#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN("^I load the player tileset$")
{
  ScenarioScope<TestCtx> context;
  context->tileset = new CTileset();
}

/*#######
##
## WHEN
##
#######*/

WHEN("^I call Load$")
{
  ScenarioScope<TestCtx> context;
  const char *szName="../../JMoria/Resources/Courier.png";
  const JIVector aspect(32,32);

  context->result = context->tileset->Load(szName, 32, 32);
}

/*#######
##
## THEN
##
#######*/

THEN("^I get a successful result$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ(context->result, JSUCCESS);
}

THEN("^The tileset is loaded$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ(context->tileset->IsBMP("anything"),false);
}