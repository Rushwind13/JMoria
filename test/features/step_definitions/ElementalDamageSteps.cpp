#include "TestContext.hpp"
using cucumber::ScenarioScope;

GIVEN( "^I have (\\d+) ([A-Za-z ]+) in my inventory$" )
{
    REGEX_PARAM( int, count );
    REGEX_PARAM( std::string, itemName );
    ScenarioScope<TestCtx> context;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( itemName.c_str() );
    int compare = Util::jstrcmp( itemName.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", itemName.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );

    JVector vPos = g_pGame->GetPlayer()->m_vPos;
    context->vec_b = vPos;

    for( int i = 0; i < count; i++ )
    {
        CItem::CreateItem( pid, vPos );
        g_pGame->GetPlayer()->PickUp( vPos );
    }
}

WHEN( "^the player takes (\\d+) (fire|cold|acid|lightning|physical) damage hits$" )
{
    REGEX_PARAM( int, times );
    REGEX_PARAM( std::string, element );
    ScenarioScope<TestCtx> context;

    uint32 dwElement = 0;
    if( element == "fire" )
        dwElement = EFFECT_FLAG_FIRE;
    else if( element == "cold" )
        dwElement = EFFECT_FLAG_COLD;
    else if( element == "acid" )
        dwElement = EFFECT_FLAG_ACID;
    else if( element == "lightning" )
        dwElement = EFFECT_FLAG_ELECTRICITY;
    // "physical" leaves dwElement = 0

    for( int i = 0; i < times; i++ )
    {
        g_pGame->GetPlayer()->TakeDamage( 0.0f, "test_monster", dwElement );
    }
}

THEN( "^the player's ([A-Za-z ]+) count is less than (\\d+)$" )
{
    REGEX_PARAM( std::string, itemName );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( itemName.c_str() );

    int total = 0;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    while( pLink != NULL )
    {
        if( pLink->m_lpData->m_id->m_dwIndex == pid->m_dwIndex )
            total += pLink->m_lpData->m_dwCount;
        pLink = pLink->next;
    }

    JLog( LOG_LEVEL_INFO, true, "Item count for %s: %d (expected < %d)\n", itemName.c_str(),
          total, expected );
    EXPECT_LT( total, expected );
}

THEN( "^the player's ([A-Za-z ]+) count is (\\d+)$" )
{
    REGEX_PARAM( std::string, itemName );
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( itemName.c_str() );

    int total = 0;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    while( pLink != NULL )
    {
        if( pLink->m_lpData->m_id->m_dwIndex == pid->m_dwIndex )
            total += pLink->m_lpData->m_dwCount;
        pLink = pLink->next;
    }

    JLog( LOG_LEVEL_INFO, true, "Item count for %s: %d (expected %d)\n", itemName.c_str(), total,
          expected );
    EXPECT_EQ( total, expected );
}
