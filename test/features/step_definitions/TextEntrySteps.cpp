#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I create a TextEntry of <([A-Za-z]+)>,([0-9.-]+)$" )
{
    REGEX_PARAM( std::string, key );
    REGEX_PARAM( int, value );
    ScenarioScope<TestCtx> context;
    context->pair.Init( key.c_str(), value );
}

/*#######
##
## WHEN
##
#######*/

/*#######
##
## THEN
##
#######*/

THEN( "^I get a TextEntry with <([A-Za-z]+)>,([0-9.-]+)$" )
{
    REGEX_PARAM( std::string, key );
    REGEX_PARAM( int, value );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( strcmp( context->pair.m_szString, key.c_str() ), 0 );
    EXPECT_EQ( context->pair.m_dwValue, value );
}