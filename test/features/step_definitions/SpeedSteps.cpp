#include "TestContext.hpp"
using cucumber::ScenarioScope;

/*#######
##
## THEN
##
#######*/

THEN( "^the player speed is (\\d+\\.\\d+)$" )
{
    REGEX_PARAM( float, expected );
    float actual = g_pGame->GetPlayer()->GetSpeed();
    EXPECT_FLOAT_EQ( actual, expected );
}

THEN( "^the player speed is greater than (\\d+\\.\\d+)$" )
{
    REGEX_PARAM( float, threshold );
    float actual = g_pGame->GetPlayer()->GetSpeed();
    EXPECT_GT( actual, threshold );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^the player uses the ([A-Za-z ]+)$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex );
    EXPECT_NE( pLink, (CLink<CItem> *)NULL );
    if( pLink )
    {
        context->result = g_pGame->GetPlayer()->Quaff( pLink );
        EXPECT_EQ( context->result, JSUCCESS );
    }
}
