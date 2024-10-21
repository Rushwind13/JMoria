#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>

using cucumber::ScenarioScope;
#include "TestContext.hpp"


GIVEN("^A map with a single level$")
{
    ScenarioScope<TestCtx> context;
    g_Constants.Init();
    context->map.CreateDungeon(1);
}
GIVEN("^The monster configuration file$")
{
    ScenarioScope<TestCtx> context;
    context->dfMonsters.Open("../../JMoria/Resources/Monsters.txt");
}
WHEN("^I read a monster from the config file$")
{
    ScenarioScope<TestCtx> context;
    CMonsterDef *mid;
    mid = new CMonsterDef;
    context->monsterDef = context->dfMonsters.ReadMonster(*mid);
}
WHEN("^I create a monster$")
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon;
    pMon = new CMonster;
    context->monster = pMon;
    //TODO: had to make this public to test it
    pMon->Init(context->monsterDef);
}
THEN("^I can see the monster name$")
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ(std::string("Giant Frog"), context->monster->GetName());
}