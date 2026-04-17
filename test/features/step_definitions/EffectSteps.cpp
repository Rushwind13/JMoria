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
