#include "DisplayText.h"
#include "Game.h"
#include "TestContext.hpp"
#include <cstring>
using cucumber::ScenarioScope;

/*#######
##
## WHEN
##
#######*/

WHEN( "^I toggle the monsters pane$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame->ToggleMonsters();
}

WHEN( "^I update visible monsters$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame->GetPlayer()->DisplayVisibleMonsters();
}

/*#######
##
## THEN
##
#######*/

THEN( "^the monsters pane is showing$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( g_pGame->IsShowingMonsters() );
}

THEN( "^the monsters pane is not showing$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( g_pGame->IsShowingMonsters() );
}

THEN( "^the monsters pane contains \"([^\"]+)\"$" )
{
    REGEX_PARAM( std::string, text );
    ScenarioScope<TestCtx> context;
    const char *paneText = g_pGame->GetMonsters()->m_szText;
    EXPECT_NE( strstr( paneText, text.c_str() ), nullptr )
        << "Expected to find \"" << text << "\" in monsters pane:\n"
        << paneText;
}

THEN( "^the monsters pane lists \"([^\"]+)\" before \"([^\"]+)\"$" )
{
    REGEX_PARAM( std::string, first );
    REGEX_PARAM( std::string, second );
    ScenarioScope<TestCtx> context;
    const char *paneText = g_pGame->GetMonsters()->m_szText;
    const char *pFirst = strstr( paneText, first.c_str() );
    const char *pSecond = strstr( paneText, second.c_str() );
    ASSERT_NE( pFirst, nullptr ) << "\"" << first << "\" not found in monsters pane:\n" << paneText;
    ASSERT_NE( pSecond, nullptr ) << "\"" << second << "\" not found in monsters pane:\n"
                                  << paneText;
    EXPECT_LT( pFirst, pSecond ) << "Expected \"" << first << "\" before \"" << second
                                 << "\" in monsters pane:\n"
                                 << paneText;
}
