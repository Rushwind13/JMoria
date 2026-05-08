#include "TestContext.hpp"
using cucumber::ScenarioScope;

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
    context->result = g_pGame->Init( "../../JMoria/", RenderMode::None );
    EXPECT_EQ( context->result, JSUCCESS );
}

GIVEN( "^I spawn a ([A-Za-z ]+)$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    Util::jstrcpy( context->szBuffer, item.c_str() );
    context->vec_b = g_pGame->GetPlayer()->m_vPos;

    JLog( LOG_LEVEL_INFO, true, "<%.2f %.2f>\n", VEC_EXPAND( context->vec_b ) );
    g_pGame->GetPlayer()->PickUp( context->vec_b );

    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );

    int compare = Util::jstrcmp( context->szBuffer, pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );
    context->result = CItem::CreateItem( pid, context->vec_b );

    EXPECT_EQ( context->result, JSUCCESS );

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    if( !pItem )
    {
        JLog( LOG_LEVEL_WARN, true, "Item spawn failed\n" );
        EXPECT_EQ( false, true );
    }
    else
    {
        JLog( LOG_LEVEL_INFO, true, "Item spawned: %s\n", pItem->GetName() );
    }
}

GIVEN( "^the player has a ([A-Za-z ]+) in inventory$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    JLog( LOG_LEVEL_INFO, true, "Item is %s\n", pid->m_szName );
    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    JLog( LOG_LEVEL_INFO, true, "Compare is %d\n", compare );
    EXPECT_EQ( compare, 0 );

    g_pGame->GetPlayer()->PickUp( context->vec_b );
    int inv_index =
        g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex )->m_lpData->m_id->m_dwIndex;
    int item_index = pid->m_dwIndex;

    JLog( LOG_LEVEL_INFO, true, "Inventory index is %d and item index is %d\n", inv_index,
          item_index );
    EXPECT_EQ( inv_index, item_index );
}

GIVEN( "^the ([A-Za-z ]+) (is|is not) cursed$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, choice );
    bool cursed = ( choice == "is" ) ? true : false;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );

    JLog( LOG_LEVEL_INFO, true, "<%.2f %.2f>\n", VEC_EXPAND( context->vec_b ) );

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    if( pItem )
    {
        JLog( LOG_LEVEL_INFO, true, "item %d\n", pItem->m_dwFlags );
        pItem->SetCursed( cursed );
    }
    else
    {
        JLog( LOG_LEVEL_ERROR, true, "item not found\n" );
    }

    int actual =
        g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurItem->m_dwFlags & ITEM_FLAG_CURSED;
    int expected = ( choice == "is" ) ? ITEM_FLAG_CURSED : 0;

    EXPECT_EQ( actual, expected );
}

GIVEN( "^the ([A-Za-z ]+) (is|is not) identified$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, choice );
    bool cursed = ( choice == "is" ) ? true : false;

    JLog( LOG_LEVEL_DEBUG, true, "Checking identified\n" );
}

GIVEN( "^the player has a ([A-Za-z ]+) in equipment at ([-0-9]+)$" )
{
    ScenarioScope<TestCtx> context;
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, equip_id );
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );
    CItem *expected = NULL;
    CItem *actual = g_pGame->GetPlayer()->m_llEquipment->GetLink( equip_id )->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    const char *have = actual->GetName();

    EXPECT_EQ( Util::jstrcmp( want, have ), 0 );
}

GIVEN( "^the player equips the ([A-Za-z ]+)$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    context->result = g_pGame->GetPlayer()->Wield(
        g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex ) );
    // EXPECT_EQ( context->result, JSUCCESS );
}

/*#######
##
## WHEN
##
#######*/

WHEN( "^the player takes off the ([A-Za-z ]+) at ([-0-9]+)$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, equip_id );
    ScenarioScope<TestCtx> context;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetLink( equip_id );
    context->result = g_pGame->GetPlayer()->RemoveEquipment( pLink );
}

WHEN( "^the player reads the scroll in inventory at ([-0-9]+)$" )
{
    REGEX_PARAM( int, inv_id );
    ScenarioScope<TestCtx> context;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( inv_id );
    JLog( LOG_LEVEL_DEBUG, true, "Found item to read is %s\n", pLink->m_lpData->GetName() );
    context->pendingSourceItemDef = pLink->m_lpData->m_id;
    context->pendingSourceItemFlags = pLink->m_lpData->m_dwFlags;
    g_pGame->GetPlayer()->Read( pLink );
}

WHEN( "^I display equipment$" ) { JLog( LOG_LEVEL_DEBUG, true, "Display the equipment\n" ); }

/* Programmatic API steps */
WHEN( "^I programmatically wield the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    ASSERT_NE( pItem, (CItem *)NULL );
    uint32 dwInst = pItem->GetInstanceId();
    /* pick up the spawned item into inventory so programmatic APIs can find it */
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetInstanceId( dwInst );
    context->result = g_pGame->GetPlayer()->Wield( pLink );
    context->result_int = (int)dwInst;
    EXPECT_EQ( context->result, JSUCCESS );
}

WHEN( "^I programmatically quaff the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    ASSERT_NE( pItem, (CItem *)NULL );
    uint32 dwInst = pItem->GetInstanceId();
    /* pick up the spawned item into inventory so programmatic APIs can find it */
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetInstanceId( dwInst );
    context->result = g_pGame->GetPlayer()->Quaff( pLink );
    context->result_int = (int)dwInst;
    EXPECT_EQ( context->result, JSUCCESS );
}

/* non-asserting / attempt variants for negative tests */
WHEN( "^I programmatically attempt to wield the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    ASSERT_NE( pItem, (CItem *)NULL );
    uint32 dwInst = pItem->GetInstanceId();
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetInstanceId( dwInst );
    context->result = g_pGame->GetPlayer()->Wield( pLink );
    context->result_int = (int)dwInst;
}

WHEN( "^I programmatically attempt to remove the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    uint32 dwInst = (uint32)context->result_int;
    /* attempt to remove and store boolean result for assertion in THEN */
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetInstanceId( dwInst );
    context->result_bool = g_pGame->GetPlayer()->RemoveEquipment( pLink );
}

/*#######
##
## THEN
##
#######*/

THEN( "^The ([A-Za-z ]+) is in (inventory|equipment) at ([-0-9]+)$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, list );
    REGEX_PARAM( int, equip_id );
    ScenarioScope<TestCtx> context;
    JLinkList<CItem> *pList = ( list == "inventory" ) ? g_pGame->GetPlayer()->m_llInventory
                                                      : g_pGame->GetPlayer()->m_llEquipment;
    CItem *expected = NULL;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );
    int item_type = pid->m_dwIndex;
    int equip_slot = EQUIP_IDX_MAIN_HAND + equip_id;
    int index = ( list == "inventory" ) ? item_type : equip_slot;
    CLink<CItem> *pLink = pList->GetLink( index );
    CItem *actual = pLink->m_lpData;
    EXPECT_NE( expected, actual );

    const char *want = item.c_str();
    const char *have = actual->GetName();

    int result = Util::jstrcmp( want, have );
    if( result != 0 )
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", want, have );

    EXPECT_EQ( result, 0 );
}

THEN( "^The ([A-Za-z ]+) is not in (inventory|equipment) at ([-0-9]+)$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( std::string, list );
    REGEX_PARAM( int, equip_id );
    ScenarioScope<TestCtx> context;
    JLinkList<CItem> *pList = ( list == "inventory" ) ? g_pGame->GetPlayer()->m_llInventory
                                                      : g_pGame->GetPlayer()->m_llEquipment;
    CItem *expected = NULL;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    int compare = Util::jstrcmp( item.c_str(), pid->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_WARN, true, "want %s have %s\n", item.c_str(), pid->m_szName );
    }
    EXPECT_EQ( compare, 0 );
    int index = ( list == "inventory" ) ? pid->m_dwIndex : equip_id;
    CLink<CItem> *pLink =
        ( list == "inventory" ) ? pList->GetLink( index ) : pList->GetLink( index );
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

THEN( "^the equipped ([A-Za-z ]+) at ([-0-9]+) (is|is not) cursed$" )
{
    REGEX_PARAM( std::string, item );
    REGEX_PARAM( int, equip_id );
    REGEX_PARAM( std::string, choice );

    ScenarioScope<TestCtx> context;
    JLinkList<CItem> *pList = g_pGame->GetPlayer()->m_llEquipment;

    CLink<CItem> *pLink = pList->GetLink( equip_id );
    int actual = pLink->m_lpData->m_dwFlags & ITEM_FLAG_CURSED;
    int expected = ( choice == "is" ) ? ITEM_FLAG_CURSED : 0;

    JLog( LOG_LEVEL_DEBUG, true, "Cursed state is %d\n", actual );
    EXPECT_EQ( actual, expected );
}

THEN( "^the spawned item is equipped at ([-0-9]+)$" )
{
    REGEX_PARAM( int, equip_id );
    ScenarioScope<TestCtx> context;
    int wanted_slot = EQUIP_IDX_MAIN_HAND + equip_id;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetLink( wanted_slot );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL );
    CItem *actual = pLink->m_lpData;
    ASSERT_NE( actual, (CItem *)NULL );
    EXPECT_EQ( actual->GetInstanceId(), (uint32)context->result_int );
}

THEN( "^the spawned item is removed from inventory$" )
{
    ScenarioScope<TestCtx> context;
    uint32 dwInst = (uint32)context->result_int;
    bool found = false;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    while( pLink )
    {
        if( pLink->m_lpData && pLink->m_lpData->GetInstanceId() == dwInst )
        {
            found = true;
            break;
        }
        pLink = g_pGame->GetPlayer()->m_llInventory->GetNext( pLink );
    }
    EXPECT_FALSE( found );
}

THEN( "^the ([A-Za-z ]+) is not labeled as cursed$" )
{
    JLog( LOG_LEVEL_DEBUG, true, "Showing label on equipment\n" );
}

WHEN( "^I programmatically remove the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    uint32 dwInst = (uint32)context->result_int;
    /* Expect the item to be currently equipped */
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetInstanceId( dwInst );
    bool ok = g_pGame->GetPlayer()->RemoveEquipment( pLink );
    EXPECT_TRUE( ok );
}

WHEN( "^I programmatically drop the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    ASSERT_NE( pItem, (CItem *)NULL );
    uint32 dwInst = pItem->GetInstanceId();
    /* pick up the spawned item into inventory so programmatic APIs can find it */
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    context->result_int = (int)dwInst;
    JLog( LOG_LEVEL_INFO, true, "[DROP STEP] before DropItem: dwInst=%u context->result_int=%d\n",
          dwInst, context->result_int );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetInstanceId( dwInst );
    bool ok = g_pGame->GetPlayer()->Drop( pLink->m_lpData );
    EXPECT_TRUE( ok );
    /* ensure tile at spawn location now contains the item */
    CDungeonTile *pGround = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pGround->m_pCurItem, (CItem *)NULL );
    /* store the actual instance id from the ground tile to the scenario context
        (some runs didn't preserve the previously-stored id reliably), then
        assert equality. */
    uint32 dwGroundInst = pGround->m_pCurItem->GetInstanceId();
    context->result_int = (int)dwGroundInst;
    JLog( LOG_LEVEL_INFO, true,
          "[DROP STEP] after DropItem: groundInst=%u context->result_int=%d\n", dwGroundInst,
          context->result_int );
    EXPECT_EQ( dwGroundInst, dwInst );
}

WHEN( "^I programmatically read the spawned item$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    CItem *pItem = pTile->m_pCurItem;
    ASSERT_NE( pItem, (CItem *)NULL );
    uint32 dwInst = pItem->GetInstanceId();
    /* pick up the spawned item into inventory so programmatic APIs can find it */
    g_pGame->GetPlayer()->PickUp( context->vec_b );
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetInstanceId( dwInst );
    context->result = g_pGame->GetPlayer()->Read( pLink );
    context->result_int = (int)dwInst;
    EXPECT_EQ( context->result, JSUCCESS );
}

THEN( "^the spawned item is back in inventory$" )
{
    ScenarioScope<TestCtx> context;
    uint32 dwInst = (uint32)context->result_int;
    bool found = false;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetHead();
    while( pLink )
    {
        if( pLink->m_lpData && pLink->m_lpData->GetInstanceId() == dwInst )
        {
            found = true;
            break;
        }
        pLink = g_pGame->GetPlayer()->m_llInventory->GetNext( pLink );
    }
    EXPECT_TRUE( found );
}

THEN( "^the spawned item is on the ground at spawn location$" )
{
    ScenarioScope<TestCtx> context;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pTile->m_pCurItem, (CItem *)NULL );
    JLog( LOG_LEVEL_INFO, true, "[THEN GROUND] pTile inst=%u context->result_int=%d\n",
          pTile->m_pCurItem->GetInstanceId(), context->result_int );
    EXPECT_EQ( pTile->m_pCurItem->GetInstanceId(), (uint32)context->result_int );
}

THEN( "^the programmatic remove failed$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( context->result_bool );
}

THEN( "^the programmatic wield failed$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_NE( context->result, JSUCCESS );
}

GIVEN( "^A ([A-Za-z ]+) is in the primary weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;

    // Spawn the item
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, (CItemDef *)NULL ) << "Item def not found: " << item;

    CItem::CreateItem( pid, context->vec_b );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pTile->m_pCurItem, (CItem *)NULL ) << "Item not spawned";

    // Pick up the item
    g_pGame->GetPlayer()->PickUp( context->vec_b );

    // Equip it to primary weapon slot
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL ) << "Item not in inventory";
    JResult res = g_pGame->GetPlayer()->Wield( pLink );
    EXPECT_EQ( res, JSUCCESS ) << "Failed to wield " << item;

    // Verify it's in primary slot
    CItem *pEquipped =
        g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Primary weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

GIVEN( "^A ([A-Za-z ]+) is in the secondary weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;

    // Spawn the item
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, (CItemDef *)NULL ) << "Item def not found: " << item;

    CItem::CreateItem( pid, context->vec_b );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pTile->m_pCurItem, (CItem *)NULL ) << "Item not spawned";

    // Pick up the item
    g_pGame->GetPlayer()->PickUp( context->vec_b );

    // Equip it to secondary main slot (EQUIP_IDX_2ND_MAIN)
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL ) << "Item not in inventory";

    // Move item from inventory to secondary weapon slot directly
    CItem *pItem = pLink->m_lpData;
    g_pGame->GetPlayer()->m_llInventory->Remove( pLink, false );
    pItem->m_pllLink = g_pGame->GetPlayer()->m_llEquipment->Add( pItem, EQUIP_IDX_2ND_MAIN,
                                                                 pItem->GetInstanceId() );

    // Verify it's in secondary main slot
    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_2ND_MAIN )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Secondary weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

GIVEN( "^A ([A-Za-z ]+) is in the offhand weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;

    // Spawn the item
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, (CItemDef *)NULL ) << "Item def not found: " << item;

    CItem::CreateItem( pid, context->vec_b );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pTile->m_pCurItem, (CItem *)NULL ) << "Item not spawned";

    // Pick up the item
    g_pGame->GetPlayer()->PickUp( context->vec_b );

    // Move item from inventory to offhand slot directly
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL ) << "Item not in inventory";

    CItem *pItem = pLink->m_lpData;
    g_pGame->GetPlayer()->m_llInventory->Remove( pLink, false );
    pItem->m_pllLink = g_pGame->GetPlayer()->m_llEquipment->Add( pItem, EQUIP_IDX_OFF_HAND,
                                                                 pItem->GetInstanceId() );

    // Verify it's in offhand slot
    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_OFF_HAND )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Offhand weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

GIVEN( "^A ([A-Za-z ]+) is in the secondary offhand weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );
    ScenarioScope<TestCtx> context;

    // Spawn the item
    context->vec_b = g_pGame->GetPlayer()->m_vPos;
    CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( item.c_str() );
    ASSERT_NE( pid, (CItemDef *)NULL ) << "Item def not found: " << item;

    CItem::CreateItem( pid, context->vec_b );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    ASSERT_NE( pTile->m_pCurItem, (CItem *)NULL ) << "Item not spawned";

    // Pick up the item
    g_pGame->GetPlayer()->PickUp( context->vec_b );

    // Move item from inventory to secondary offhand slot directly
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llInventory->GetLink( pid->m_dwIndex );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL ) << "Item not in inventory";

    CItem *pItem = pLink->m_lpData;
    g_pGame->GetPlayer()->m_llInventory->Remove( pLink, false );
    pItem->m_pllLink = g_pGame->GetPlayer()->m_llEquipment->Add( pItem, EQUIP_IDX_2ND_OFF,
                                                                 pItem->GetInstanceId() );

    // Verify it's in secondary offhand slot
    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_2ND_OFF )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Secondary offhand weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

WHEN( "^the player swaps weapons$" ) { g_pGame->GetPlayer()->XchangeWeapons(); }

THEN( "^A ([A-Za-z ]+) should be in the primary weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );

    CItem *pEquipped =
        g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Primary weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

THEN( "^A ([A-Za-z ]+) should be in the offhand weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );

    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_OFF_HAND )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Offhand weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

THEN( "^A ([A-Za-z ]+) should be in the secondary weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );

    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_2ND_MAIN )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Secondary weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

THEN( "^A ([A-Za-z ]+) should be in the secondary offhand weapon equipment slot$" )
{
    REGEX_PARAM( std::string, item );

    CItem *pEquipped = g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_2ND_OFF )->m_lpData;
    EXPECT_NE( pEquipped, (CItem *)NULL ) << "Secondary offhand weapon slot is empty";
    EXPECT_EQ( Util::jstrcmp( pEquipped->GetName(), item.c_str() ), 0 )
        << "Expected " << item << " but got " << pEquipped->GetName();
}

WHEN( "^the player applies the pending effect to equipment at ([-0-9]+)$" )
{
    REGEX_PARAM( int, slot );
    ScenarioScope<TestCtx> context;
    CLink<CItem> *pLink = g_pGame->GetPlayer()->m_llEquipment->GetLink( slot );
    ASSERT_NE( pLink, (CLink<CItem> *)NULL );
    CEffect *pEffect = CPlayer::FindNeedsChoiceEffect( context->pendingSourceItemDef );
    g_pGame->GetPlayer()->ApplyChosenItem( pLink, pEffect, context->pendingSourceItemFlags );
}