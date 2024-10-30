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
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, index);
    strcpy(context->szBuffer, item.c_str());
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    context->index = index;

    context->ItemDef = g_pGame->GetDungeon()->GetItemDef( context->index );
    context->result = CItem::CreateItem( context->ItemDef, context->vec_b );
}

GIVEN( "^the player has a ([A-Za-z ]+):([0-9]+) in inventory$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, index);
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    int inv_index = g_pGame->GetPlayer()->m_llInventory->GetLink(index)->m_lpData->m_id->m_dwIndex;
    int item_index = context->ItemDef->m_dwIndex;

    EXPECT_EQ( inv_index, item_index );
}

GIVEN( "^The ([A-Za-z ]+):([0-9]+) is in equipment at ([0-9]+)$" )
{
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, item_id);
    REGEX_PARAM(int, index);
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( index )->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    char *have = actual->GetName();

    printf("want %s have %s\n", want, have);
    EXPECT_EQ(strcmp(want, have), 0);
}

GIVEN( "^the player equips the item$" )
{
    ScenarioScope<TestCtx> context;
    context->result_bool =
        g_pGame->GetPlayer()->Wield( g_pGame->GetPlayer()->m_llInventory->GetHead() );
}

/*#######
##
## WHEN
##
#######*/


WHEN( "^the player takes off the item$" )
{
    ScenarioScope<TestCtx> context;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetLink(0);
    context->result_bool = g_pGame->GetPlayer()->Remove( pLink );
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



THEN( "^But The ([A-Za-z ]+):([0-9]+) is in inventory at ([0-9]+)$" )
{
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, item_id);
    REGEX_PARAM(int, index);
    printf("got here: ");
    JLinkList<CItem> *pList = g_pGame->GetPlayer()->m_llInventory;
    printf("inv %d ", pList->length());
    CItem *expected = NULL;
    CLink<CItem> *pLink = pList->GetLink( index );
    printf("link %d\n", pLink->m_dwIndex);
    CItem *actual = pLink->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    char *have = actual->GetName();

    printf("want %s have %s\n", want, have);
    EXPECT_EQ(strcmp(want, have), 0);
}

THEN( "^The ([A-Za-z ]+):([0-9]+) is not in inventory$" )
{
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, index);
    ScenarioScope<TestCtx> context;
    CItem *expected = NULL;
    CLink<CItem> *pLink =
        g_pGame->GetPlayer()->m_llInventory->GetLink( index );
    CItem *actual = NULL;

    if( pLink )
        actual = pLink->m_lpData;
    EXPECT_EQ( expected, actual );
}

THEN( "^The ([A-Za-z ]+):([0-9]+) is not in equipment at ([0-9]+)$" )
{
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, item_id);
    REGEX_PARAM(int, index);
    int length = g_pGame->GetPlayer()->m_llEquipment->length();
    EXPECT_EQ(length, index);
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( index )->m_lpData;
    EXPECT_EQ( expected, actual );
}

THEN( "^Also The ([A-Za-z ]+):([0-9]+) is in equipment at ([0-9]+)$" )
{
    REGEX_PARAM(std::string, item);
    REGEX_PARAM(int, item_id);
    REGEX_PARAM(int, index);
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( index )->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    char *have = actual->GetName();

    printf("want %s have %s\n", want, have);
    EXPECT_EQ(strcmp(want, have), 0);
}