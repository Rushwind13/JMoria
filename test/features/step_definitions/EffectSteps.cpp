#include "TestContext.hpp"
using cucumber::ScenarioScope;

/*#######
##
## GIVEN
##
#######*/

// (uses "I initialize my Constants" from StringTableSteps.cpp)

/*#######
##
## WHEN
##
#######*/

WHEN( "^I call LookupEffectFlag with <([A-Z_0-9]+)>$" )
{
    REGEX_PARAM( std::string, flagName );
    ScenarioScope<TestCtx> context;
    context->effect_flags = 0;
    context->effect_flags2 = 0;
    context->constants.LookupEffectFlag( flagName.c_str(), context->effect_flags,
                                         context->effect_flags2 );
}

WHEN( "^I call EffectFlagToString with word 1 set to (\\d+) and word 2 set to (\\d+)$" )
{
    REGEX_PARAM( int, w1 );
    REGEX_PARAM( int, w2 );
    ScenarioScope<TestCtx> context;
    context->effect_flags = (uint32)w1;
    context->effect_flags2 = (uint32)w2;
    const char *name =
        context->constants.EffectFlagToString( context->effect_flags, context->effect_flags2 );
    Util::jstrcpy( context->szBuffer, name );
}

/*#######
##
## THEN
##
#######*/

THEN( "^I can look up <([A-Z_0-9]+)> and get value (\\d+)$" )
{
    REGEX_PARAM( std::string, entry );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    int actual = context->constants.LookupString( entry.c_str() );
    EXPECT_EQ( actual, expected );
}

THEN( "^flags word 1 is (\\d+)$" )
{
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( (int)context->effect_flags, expected );
}

THEN( "^flags word 2 is (\\d+)$" )
{
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( (int)context->effect_flags2, expected );
}

THEN( "^the effect flag name is <([^>]+)>$" )
{
    REGEX_PARAM( std::string, expected );
    ScenarioScope<TestCtx> context;
    EXPECT_STREQ( context->szBuffer, expected.c_str() );
}

GIVEN( "^I record the dungeon depth$" )
{
    ScenarioScope<TestCtx> context;
    context->hi = g_pGame->GetDungeon()->depth;
}

GIVEN( "^the player descends to depth (\\d+)$" )
{
    REGEX_PARAM( int, target );
    ScenarioScope<TestCtx> context;
    int delta = target - g_pGame->GetDungeon()->depth;
    g_pGame->GetDungeon()->OnChangeLevel( delta );
}

THEN( "^the dungeon depth is the same$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( g_pGame->GetDungeon()->depth, context->hi );
}

THEN( "^the dungeon depth is (\\d+)$" )
{
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( g_pGame->GetDungeon()->depth, expected );
}

THEN( "^the dungeon depth is greater than (\\d+)$" )
{
    REGEX_PARAM( int, threshold );
    ScenarioScope<TestCtx> context;
    EXPECT_GT( g_pGame->GetDungeon()->depth, threshold );
}

GIVEN( "^I record the monster count$" )
{
    ScenarioScope<TestCtx> context;
    context->lo = g_pGame->GetDungeon()->m_llMonsters->length();
}

THEN( "^the monster count increased$" )
{
    ScenarioScope<TestCtx> context;
    int before = context->lo;
    int after = g_pGame->GetDungeon()->m_llMonsters->length();
    EXPECT_GT( after, before );
}

THEN( "^the player has an active fear resistance$" )
{
    // Intrinsic flag must be set AND tracked as a resist (not a debuff)
    EXPECT_NE( g_pGame->GetPlayer()->GetIntrinsic( EFFECT_FLAG_AFRAID ), 0 );
    EXPECT_TRUE( g_pGame->GetPlayer()->HasActiveResistFor( EFFECT_FLAG_AFRAID ) );
}

THEN( "^the player does not have the afraid debuff$" )
{
    // If the AFRAID flag is set it must be as a resist, not a debuff
    if( g_pGame->GetPlayer()->GetIntrinsic( EFFECT_FLAG_AFRAID ) != 0 )
        EXPECT_TRUE( g_pGame->GetPlayer()->HasActiveResistFor( EFFECT_FLAG_AFRAID ) );
}

THEN( "^the player has an active poison resistance$" )
{
    // Intrinsic flag must be set AND tracked as a resist (not a debuff)
    EXPECT_NE( g_pGame->GetPlayer()->GetIntrinsic( EFFECT_FLAG_POISON ), 0 );
    EXPECT_TRUE( g_pGame->GetPlayer()->HasActiveResistFor( EFFECT_FLAG_POISON ) );
}

THEN( "^the player does not have the poisoned debuff$" )
{
    // If the POISON flag is set it must be as a resist, not a debuff
    if( g_pGame->GetPlayer()->GetIntrinsic( EFFECT_FLAG_POISON ) != 0 )
        EXPECT_TRUE( g_pGame->GetPlayer()->HasActiveResistFor( EFFECT_FLAG_POISON ) );
}
