#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^I have a Player$" )
{
    ScenarioScope<TestCtx> context;
    // get a player
    if( g_pGame )
    {
        g_pGame->Term();
    }
    g_pGame = NULL;
    g_pGame = new CGame;
    context->result = g_pGame->Init( "../../JMoria/" );
}
GIVEN( "^I spawn a ([A-Za-z ]+):([0-9]+)$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, index );
    strcpy( context->szBuffer, item.c_str() );
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    context->index = index;

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( context->index );
    context->result = CItem::CreateItem( pid, context->vec_b );
}

GIVEN( "^the player has a ([A-Za-z ]+):([0-9]+) in inventory$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, item_id );
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    int inv_index =
        g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex )->m_lpData->m_id->m_dwIndex;
    int item_index = pid->m_dwIndex;

    EXPECT_EQ( inv_index, item_index );
}

GIVEN( "^the ([A-Za-z ]+):([0-9]+) (is|is not) cursed$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, item_id );
    REGEX_PARAM( std::string, choice );
    int chance = ( choice == "is" ) ? 100 : 0;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    pItem->SetCursed( chance );

    int actual =
        g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurItem->m_dwFlags & ITEM_FLAG_CURSED;
    int expected = ( choice == "is" ) ? ITEM_FLAG_CURSED : 0;

    EXPECT_EQ( actual, expected );
}

GIVEN( "^the player has a ([A-Za-z ]+):([0-9]+) in equipment$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, item_id );
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( pid->m_dwIndex )->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    char *have = actual->GetName();

    EXPECT_EQ( strcmp( want, have ), 0 );
}

GIVEN( "^the player equips the item ([0-9]+)$" )
{
    REGEX_PARAM( int, item_id );
    ScenarioScope<TestCtx> context;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    context->result_bool = g_pGame->GetPlayer()->Wield(
        g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex ) );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^the player takes off the item ([0-9]+)$" )
{
    REGEX_PARAM( int, item_id );
    ScenarioScope<TestCtx> context;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetLink( pid->m_dwIndex );
    context->result_bool = g_pGame->GetPlayer()->Remove( pLink );
}

/*#######
##
## THEN
##
#######*/

THEN( "^The ([A-Za-z ]+):([0-9]+) is in (inventory|equipment)$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, item_id );
    REGEX_PARAM( std::string, list );
    ScenarioScope<TestCtx> context;
    JLinkList<CItem> *pList = ( list == "inventory" ) ? g_pGame->GetPlayer()->m_llInventory
                                                      : g_pGame->GetPlayer()->m_llEquipment;
    CItem *expected = NULL;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    CLink<CItem> *pLink = pList->GetLink( pid->m_dwIndex );
    CItem *actual = pLink->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    char *have = actual->GetName();

    int result = strcmp( want, have );
    if( result != 0 )
        JLog( LOG_LEVEL_WARN, true, "want %s have %s\n", want, have );

    EXPECT_EQ( result, 0 );
}

THEN( "^The ([A-Za-z ]+):([0-9]+) is not in (inventory|equipment)$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, item_id );
    REGEX_PARAM( std::string, list );
    ScenarioScope<TestCtx> context;
    JLinkList<CItem> *pList = ( list == "inventory" ) ? g_pGame->GetPlayer()->m_llInventory
                                                      : g_pGame->GetPlayer()->m_llEquipment;
    CItem *expected = NULL;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item_id );
    CLink<CItem> *pLink = pList->GetLink( pid->m_dwIndex );
    CItem *actual = NULL;

    if( pLink )
    {
        // found an item of the same type, make sure it's a different one
        actual = pLink->m_lpData;
        EXPECT_NE( actual->m_id->m_szName, item );
    }
    else
    {
        // nothing of this type; win
        EXPECT_EQ( expected, actual );
    }
}