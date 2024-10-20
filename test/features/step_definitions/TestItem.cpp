#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestItemContext.hpp"


GIVEN("^I have a ItemDef$")
{
    // ScenarioScope<TestItemCtx> context;
    // // Load the item list from config
    // // TODO: Make this a method on CItemDef.

    // m_llItemDefs = new JLinkList<CItemDef>;

    // CItemDef *pid;
    // CDataFile dfItems;
    // dfItems.Open("../Resources/Items.txt"); //step out of the test directory

    // pid = new CItemDef;
    // while( dfItems.ReadItem(*pid) )
    // {
    //     m_llItemDefs->Add(pid);
    //     pid = new CItemDef;
    // }
    // *ItemDef = pid; // the last item, sure
    // delete pid;

}
// WHEN("^I call GetHallRect for east from ([0-9.-]+),([0-9.-]+)$")
// {
//     REGEX_PARAM(int,x);
//     REGEX_PARAM(int,y);
//     JRect rcHall(x,y,x,y);
//     ScenarioScope<TestItemCtx> context;
//     context->map.GetHallRect( rcHall, DIR_EAST );
// }
// THEN("^The JRect ([0-9.-]+),([0-9.-]+),([0-9.-]+),([0-9.-]+) is now filled with ([0-9]+)$")
// {
//     REGEX_PARAM(float,l);
//     REGEX_PARAM(float,t);
//     REGEX_PARAM(float,r);
//     REGEX_PARAM(float,b);
//     REGEX_PARAM(int,type);
//     ScenarioScope<TestItemCtx> context;

//     int x,y;
//     for(y=t; y<=b; y++)
//     {
//         for(x=l; x<=r; x++)
//         {
//             JIVector vCheck(x,y);
//             int expected = context->map.GetdtdIndex(vCheck);
//             // printf("<%d %d>: %d/%d ", VEC_EXPAND(vCheck), expected, type);
//             EXPECT_EQ(expected, type);
//         }
//         // printf("\n");
//     }
// }