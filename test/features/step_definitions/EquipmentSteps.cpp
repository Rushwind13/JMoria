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
    g_pGame = NULL;
    g_pGame = new CGame;
    context->result = g_pGame->Init( "../../JMoria/" );
}
GIVEN( "^I spawn a dagger$" )
{
    ScenarioScope<TestCtx> context;
    context->vec_b = g_pGame->GetPlayer()->m_vPos;

    int item = 11;
    context->ItemDef = g_pGame->GetDungeon()->GetItemDef( item );
    context->result = CItem::CreateItem( context->ItemDef, context->vec_b );
}

GIVEN( "^the player has a dagger in inventory$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    int inv_index = g_pGame->GetPlayer()->m_llInventory->GetHead()->m_dwIndex;
    int item_index = context->ItemDef->m_dwIndex;

    EXPECT_EQ( inv_index, item_index );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^the player equips the item$" )
{
    ScenarioScope<TestCtx> context;
    context->result_bool =
        g_pGame->GetPlayer()->Wield( g_pGame->GetPlayer()->m_llInventory->GetHead() );
}

/*#######
##
## THEN
##
#######*/

// THEN( "^The ([a-z]+) ([is|is not]) in ([inventory|equipment])$" )
// {
//     REGEX_PARAM(std::string, object);
//     REGEX_PARAM(std::string, item_state);
//     REGEX_PARAM(std::string, item_location);
//     ScenarioScope<TestCtx> context;

//     bool state = item_state == "is";

//     JLinkList<CItem> *which_list = item_location == "inventory" ?
//     g_pGame->GetPlayer()->m_llInventory : g_pGame->GetPlayer()->m_llEquipment;

//     // which_list->GetLink()

// }

THEN( "^The dagger is in inventory$" )
{
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llInventory->GetLink( 0 )->m_lpData;
    EXPECT_NE( expected, actual );
}

THEN( "^The dagger is not in inventory$" )
{
    ScenarioScope<TestCtx> context;
    CItem *expected = NULL;
    CLink<CItem> *pLink =
        g_pGame->GetPlayer()->m_llInventory->GetLink( context->ItemDef->m_dwIndex );
    CItem *actual = NULL;

    if( pLink )
        actual = pLink->m_lpData;
    EXPECT_EQ( expected, actual );
}

THEN( "^The dagger is not in equipment$" )
{
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( 0 )->m_lpData;
    EXPECT_EQ( expected, actual );
}

THEN( "^The dagger is in equipment$" )
{
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( 0 )->m_lpData;
    EXPECT_NE( expected, actual );
}