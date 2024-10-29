#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>

using cucumber::ScenarioScope;
#include "TestContext.hpp"
#include <StepMacros.hpp>

/*#######
##
## GIVEN
##
#######*/
GIVEN( "^I have a Player$" )
{
    ScenarioScope<TestCtx> context;
    //get a player
    g_pGame = NULL;
    g_pGame = new CGame;
    g_pGame->Init("../../JMoria");
}
GIVEN( "^I spawn a dagger$" )
{
    ScenarioScope<TestCtx> context;



    int item = 11;
    CItemDef *chosen_item = g_pGame->GetDungeon()->m_llItemDefs->GetLink( item )->m_lpData;


    context->result = CMonster::CreateMonster( pmd, context->vec_b );
}

GIVEN("^the player has a dagger in inventory$")
{

}

/*#######
##
## WHEN
##
#######*/
WHEN( "^the player equips the item$" )
{
    ScenarioScope<TestCtx> context;
}

/*#######
##
## THEN
##
#######*/

THEN( "^The ([a-z]+) ([is|is not]) in ([inventory|equipment])$" )
{
    REGEX_PARAM(std::string, object);
    REGEX_PARAM(std::string, item_state);
    REGEX_PARAM(std::string, item_location);
    ScenarioScope<TestCtx> context;

    bool state = item_state == "is";



    JLinkList<CItem> *which_list = item_location == "inventory" ? g_pGame->GetPlayer()->m_llInventory : g_pGame->GetPlayer()->m_llEquipment;

    // which_list->GetLink()

}

THEN( "^The ring is in inventory$" )
{
   
}

THEN( "^The ring is not in equipment$" )
{
   
}

THEN( "^The ring is in equipment$" )
{
   
}