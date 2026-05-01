// MonsterRecallSteps.cpp — BDD step definitions for monster_recall.feature
#include "DisplayText.h"
#include "Dungeon.h"
#include "MonsterRecall.h"
#include "TestContext.hpp"
#include <cstdlib>
#include <cstring>

static const char *s_szRecallSaveFile = "/tmp/jmoria_recall_test.txt";

// ─── Helper ──────────────────────────────────────────────────────────────────

static void InitRecallContext( TestCtx *context )
{
    if( context->recall )
    {
        delete context->recall;
        context->recall = nullptr;
    }
    if( context->recallDT )
    {
        delete context->recallDT;
        context->recallDT = nullptr;
    }
    context->recall = new CMonsterRecall();
    context->recall->Init( "" ); // empty basedir = no file loaded
    // create a display text region big enough to hold recall output
    context->recallDT = new CDisplayText( "../../JMoria/", JRect( 0.0f, 0.0f, 400.0f, 400.0f ) );
}

static void PrintRecallFor( TestCtx *context, const std::string &name )
{
    context->recallDT->Clear();
    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( name.c_str() );
    ASSERT_NE( nullptr, pmd ) << "MonsterDef not found: " << name;
    context->recall->PrintRecall( pmd, context->recallDT );
}

// ─── Background steps handled by GameSteps ───────────────────────────────────

// ─── Recall setup ────────────────────────────────────────────────────────────

GIVEN( "^a fresh monster recall system$" )
{
    ScenarioScope<TestCtx> context;
    InitRecallContext( context.get() );
}

// ─── Sighting / kill / attack / flag ─────────────────────────────────────────

WHEN( "^I record a sighting of \"([^\"]*)\" at depth (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, depthFeet );
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );
    context->recall->RecordSighting( name.c_str(), depthFeet );
}

WHEN( "^I record a kill of \"([^\"]*)\" with estimated HP (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( float, estimatedHP );
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );
    context->recall->RecordKill( name.c_str(), estimatedHP );
}

WHEN( "^I record an attack by \"([^\"]*)\" of type (\\d+) damage (\\d+) effect \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, type );
    REGEX_PARAM( float, damage );
    REGEX_PARAM( std::string, effect );
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );
    context->recall->RecordAttackObservation( name.c_str(), type, damage,
                                              effect.empty() ? "" : effect.c_str() );
}

WHEN( "^I record observation of \"([^\"]*)\" flag (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, flag );
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );
    context->recall->RecordObservation( name.c_str(), flag );
}

WHEN( "^I save and reload the recall data$" )
{
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );

    // Save to temp file
    context->recall->SaveTo( s_szRecallSaveFile );

    // Create fresh recall and load from temp file
    delete context->recall;
    context->recall = new CMonsterRecall();
    context->recall->InitFrom( s_szRecallSaveFile );
}

WHEN( "^I load the recall fixture \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, path );
    ScenarioScope<TestCtx> context;
    ASSERT_NE( nullptr, context->recall );
    context->recall->InitFrom( path.c_str() );
}

// ─── Recall content assertions ───────────────────────────────────────────────

THEN( "^recall for \"([^\"]*)\" shows \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( std::string, expected );
    ScenarioScope<TestCtx> context;
    PrintRecallFor( context.get(), name );
    EXPECT_NE( nullptr, strstr( context->recallDT->m_szText, expected.c_str() ) )
        << "Recall for '" << name << "' did not show '" << expected
        << "'\nActual: " << context->recallDT->m_szText;
}

THEN( "^recall for \"([^\"]*)\" does not show \"([^\"]*)\"$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( std::string, unexpected );
    ScenarioScope<TestCtx> context;
    PrintRecallFor( context.get(), name );
    EXPECT_EQ( nullptr, strstr( context->recallDT->m_szText, unexpected.c_str() ) )
        << "Recall for '" << name << "' should NOT show '" << unexpected
        << "'\nActual: " << context->recallDT->m_szText;
}

THEN( "^the kill count for \"([^\"]*)\" is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    EXPECT_EQ( expected, pEntry->dwKills );
}

THEN( "^the encounter count for \"([^\"]*)\" is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    EXPECT_EQ( expected, pEntry->dwEncounters );
}

THEN( "^the HP min for \"([^\"]*)\" is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    EXPECT_EQ( expected, pEntry->dwHPObsMin );
}

THEN( "^the HP max for \"([^\"]*)\" is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    EXPECT_EQ( expected, pEntry->dwHPObsMax );
}

THEN( "^the attack count for \"([^\"]*)\" is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    EXPECT_EQ( expected, pEntry->nAttacks );
}

THEN( "^the attack seen count for \"([^\"]*)\" type (\\d+) is (\\d+)$" )
{
    REGEX_PARAM( std::string, name );
    REGEX_PARAM( int, type );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    const CRecallEntry *pEntry = context->recall->GetEntry( name.c_str() );
    ASSERT_NE( nullptr, pEntry );
    for( int i = 0; i < pEntry->nAttacks; i++ )
    {
        if( pEntry->attacks[i].dwType == type )
        {
            EXPECT_EQ( expected, pEntry->attacks[i].dwTimesObserved );
            return;
        }
    }
    ADD_FAILURE() << "No attack of type " << type << " found for " << name;
}
