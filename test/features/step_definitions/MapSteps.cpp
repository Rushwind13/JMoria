#include "JLinkList.h"
#include "TestContext.hpp"
using cucumber::ScenarioScope;

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I have a Map$" )
{
    ScenarioScope<TestCtx> context;
    if( context->pMap != nullptr )
    {
        delete context->pMap;
        context->pMap = nullptr;
    }
    context->pMap = new JMap<JString>;
}

GIVEN( "^I insert entry <([0-9.-]+),([a-z]+)> into the Map$" )
{
    REGEX_PARAM( int, key );
    REGEX_PARAM( std::string, dumbvalue );
    ScenarioScope<TestCtx> context;
    // Store the value directly - just allocate and store it
    JString *value = new JString( dumbvalue.c_str() );
    context->pMap->Add( value, key );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I Get entry ([0-9.-]+) from the Map$" )
{
    REGEX_PARAM( int, key );
    ScenarioScope<TestCtx> context;
    context->index = key;
    context->pLink = context->pMap->Get( key );
    Util::jstrcpy( context->szBuffer, context->pLink->m_lpData->GetValue() );
}
/*#######
##
## THEN
##
#######*/
THEN( "^Entry ([0-9.-]+) is ([a-z]+)$" )
{
    REGEX_PARAM( int, key );
    REGEX_PARAM( std::string, dumbvalue );
    char value[1024];
    strcpy( value, dumbvalue.c_str() );
    ScenarioScope<TestCtx> context;
    EXPECT_EQ( context->index, key );
    EXPECT_EQ( Util::jstrcmp( context->szBuffer, value ), 0 );
}

THEN( "^the (prev|next) index is <([0-9]+)>$" )
{
    REGEX_PARAM( std::string, leaf );
    REGEX_PARAM( int, key );
    ScenarioScope<TestCtx> context;
    bool isPrev = ( Util::jstrcmp( leaf.c_str(), "prev" ) == 0 );
    int actual = isPrev ? context->pLink->prev->m_dwIndex : context->pLink->next->m_dwIndex;
    EXPECT_EQ( actual, key );
}