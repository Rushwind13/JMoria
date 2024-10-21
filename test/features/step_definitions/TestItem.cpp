#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestItemContext.hpp"


GIVEN("^I have an ItemDef$")
{
    ScenarioScope<TestItemCtx> context;
    // Load the item list from config
    // TODO: Make this a method on CItemDef.

    g_Constants.Init();
    context->m_llItemDefs = new JLinkList<CItemDef>;

    CItemDef *pid;
    CDataFile dfItems;
    dfItems.Open("../../JMoria/Resources/Items.txt"); //step out of the test directory

    pid = new CItemDef;
    context->ItemDef = dfItems.ReadItem(*pid);
}

WHEN("^I CreateItem$")
{
    ScenarioScope<TestItemCtx> context;
    context->Success = CItem::CreateItem(context->ItemDef);
}

THEN("^The item should succeed in creation.$")
{
    ScenarioScope<TestItemCtx> context;
    JResult expected = JSUCCESS;
    JResult actual = context->Success;
    EXPECT_EQ(expected, actual);
}