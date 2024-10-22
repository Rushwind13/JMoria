#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

GIVEN( "^A map with a single level$" )
{
    ScenarioScope<TestCtx> context;
    g_Constants.Init();
    context->map.CreateDungeon( 1 );
}
GIVEN( "^The monster configuration file$" )
{
    ScenarioScope<TestCtx> context;
    context->dfMonsters.Open( "../../JMoria/Resources/Monsters.txt" );
}
WHEN( "^I read a monster from the config file$" )
{
    ScenarioScope<TestCtx> context;
    CMonsterDef *mid;
    mid = new CMonsterDef;
    context->monsterDef = context->dfMonsters.ReadMonster( *mid );
}
WHEN( "^I create a monster$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon;
    pMon = new CMonster;
    context->monster = pMon;
    // TODO: had to make this public to test it
    pMon->Init( context->monsterDef );
}
THEN( "^I can see the monster name$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( std::string( "Giant Frog" ), context->monster->GetName() );
}
GIVEN( "^I read all the monsters from the config file$" )
{
    ScenarioScope<TestCtx> context;
    CMonsterDef *pmd;
    pmd = new CMonsterDef;
    while( context->dfMonsters.ReadMonster( *pmd ) )
    {
        context->m_llMonsterDefs->Add( pmd );
        pmd = new CMonsterDef;
    }
}
WHEN( "^I create a monster for each configuration$" )
{
    ScenarioScope<TestCtx> context;
    CLink<CMonsterDef> *pLink = context->m_llMonsterDefs->GetHead();
    while( pLink != NULL )
    {
        CMonster *pMon;
        pMon = new CMonster;
        pMon->Init( pLink->m_lpData );
        context->m_llMonsters->Add( pMon );
        pLink = context->m_llMonsterDefs->GetNext( pLink );
    }
}
THEN( "^I can see all the monster names$" )
{
    ScenarioScope<TestCtx> context;
    CLink<CMonster> *pLink = context->m_llMonsters->GetHead();
    while( pLink != NULL )
    {
        printf( "Monster is %s\n", pLink->m_lpData->GetName() );
        pLink = context->m_llMonsters->GetNext( pLink );
    }
}