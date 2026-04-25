#include "TestContext.hpp"
using cucumber::ScenarioScope;

GIVEN( "^A map with a single level$" )
{
    ScenarioScope<TestCtx> context;
    g_Constants.Init();
    context->map.CreateDungeon( 1 );

    // Create dungeon and set up effect definitions
    context->dungeon = new CDungeon();
    context->dungeon->m_llEffectDefs = new JLinkList<CEffectDef>;

    // Load Effects.txt
    CDataFile dfEffects;
    dfEffects.Open( "../../JMoria/Resources/Effects.txt" );

    CEffectDef *ped = new CEffectDef;
    while( dfEffects.ReadEffect( *ped ) )
    {
        context->dungeon->m_llEffectDefs->Add( ped );
        ped = new CEffectDef;
    }
    delete ped;
}
GIVEN( "^The monster configuration file$" )
{
    ScenarioScope<TestCtx> context;
    context->dfMonsters.Open( "../../JMoria/Resources/Monsters.txt" );
    context->dfMonsters.SetDungeon( context->dungeon );
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
    EXPECT_EQ( std::string( "Giant Ant" ), context->monster->GetName() );
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
        JLog( LOG_LEVEL_DEBUG, true, "Monster is %s\n", pLink->m_lpData->GetName() );
        pLink = context->m_llMonsters->GetNext( pLink );
    }
}

WHEN( "^I find a breath weapon monster$" )
{
    ScenarioScope<TestCtx> context;
    CMonsterDef *pmd = new CMonsterDef;
    while( context->dfMonsters.ReadMonster( *pmd ) )
    {
        CLink<CAttack> *pLink = pmd->m_llAttacks->GetHead();
        while( pLink != NULL )
        {
            if( pLink->m_lpData->m_dwType & MON_FLAG_BREATHE )
            {
                context->monsterDef = pmd;
                CMonster *pMon = new CMonster;
                pMon->Init( pmd );
                context->monster = pMon;
                return;
            }
            pLink = pmd->m_llAttacks->GetNext( pLink );
        }
        delete pmd;
        pmd = new CMonsterDef;
    }
    delete pmd;
    FAIL() << "No breath weapon monster found in Monsters.txt";
}

THEN( "^its breath damage equals its current HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = context->monster;

    // Find and select the breath attack
    CLink<CAttack> *pLink = context->monsterDef->m_llAttacks->GetHead();
    while( pLink != NULL )
    {
        if( pLink->m_lpData->m_dwType & MON_FLAG_BREATHE )
        {
            pMon->m_pCurrentAttack = pLink->m_lpData;
            break;
        }
        pLink = context->monsterDef->m_llAttacks->GetNext( pLink );
    }
    ASSERT_NE( pMon->m_pCurrentAttack, nullptr ) << "Monster has no breath attack";

    float fExpected = pMon->m_fCurHP;
    float fDamage = pMon->Damage( 1.0f );
    EXPECT_FLOAT_EQ( fDamage, fExpected );
}

WHEN( "^I deal (\\d+) damage to the breath monster$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( int, damage );
    CMonster *pMon = context->monster;
    ASSERT_GT( pMon->m_fCurHP, (float)damage )
        << "Damage would kill the monster; choose a smaller value";
    pMon->TakeDamage( (float)damage );
}