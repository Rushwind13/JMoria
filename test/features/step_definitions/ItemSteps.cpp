#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"


GIVEN("^I have an ItemDef$")
{
    ScenarioScope<TestCtx> context;
    g_Constants.Init();

    CItemDef *pid;
    CDataFile dfItems;
    dfItems.Open("../../JMoria/Resources/Items.txt"); // step out of the test directory

    pid = new CItemDef;
    context->ItemDef = dfItems.ReadItem(*pid);
}

WHEN("^I CreateItem$")
{
    ScenarioScope<TestCtx> context;
    context->Success = CItem::CreateItem(context->ItemDef);
}

THEN("^The item should succeed in creation.$")
{
    ScenarioScope<TestCtx> context;
    JResult expected = JSUCCESS;
    JResult actual = context->Success;
    EXPECT_EQ(expected, actual);
}