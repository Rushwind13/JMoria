#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^the player has a (zero|nonzero) lightsource$" )
{
    REGEX_PARAM( std::string, choice );
    ScenarioScope<TestCtx> context;
    context->result_float = g_pGame->GetPlayer()->LightSource();
    if( choice == "zero" )
    {
        EXPECT_EQ( context->result_float, 0.0f );
    }
    else
    {
        EXPECT_GT( context->result_float, 0.0f );
    }
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
THEN( "^the area near the player (is|is not) lit$" )
{
    REGEX_PARAM( std::string, choice );
    ScenarioScope<TestCtx> context;
    context->result_float = g_pGame->GetPlayer()->LightSource();
    if( choice == "is" )
    {
        EXPECT_GT( context->result_float, 0.0f );
    }
    else
    {
        EXPECT_EQ( context->result_float, 0.0f );
    }
    bool actual = g_pGame->GetDungeon()->IsLit( g_pGame->GetPlayer()->m_vPos );
    bool expected = ( choice == "is" );
    EXPECT_EQ( actual, expected );
}