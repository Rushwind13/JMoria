#include "TestContext.hpp"
using cucumber::ScenarioScope;

GIVEN( "^I create an item that is cursed and unknown$" )
{
    ScenarioScope<TestCtx> context;
    // create a simple item instance
    context->Item = new CItem();
    context->Item->m_dwFlags = ITEM_FLAG_CURSED;
    context->Item->m_dwKnownIntrinsics = 0;
}

GIVEN( "^I create an item that is cursed and known$" )
{
    ScenarioScope<TestCtx> context;
    context->Item = new CItem();
    context->Item->m_dwFlags = ITEM_FLAG_CURSED;
    context->Item->SetKnownIntrinsic( ITEM_FLAG_CURSED );
}

GIVEN( "^I create an item that is uncursed but known$" )
{
    ScenarioScope<TestCtx> context;
    context->Item = new CItem();
    context->Item->m_dwFlags = 0; // uncursed
    context->Item->m_dwKnownIntrinsics = ITEM_FLAG_CURSED; // we know the cursed-status (and it's not cursed)
}

GIVEN( "^I create an item that is uncursed and unknown$" )
{
    ScenarioScope<TestCtx> context;
    context->Item = new CItem();
    context->Item->m_dwFlags = 0; // uncursed
    context->Item->m_dwKnownIntrinsics = 0; // unknown
}

THEN( "^the item should not be known to be cursed$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( context->Item->IsKnownIntrinsic( ITEM_FLAG_CURSED ) );
}

THEN( "^the item should be known to be cursed$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( context->Item->IsKnownIntrinsic( ITEM_FLAG_CURSED ) );
}

THEN( "^the item should be known to be uncursed$" )
{
    ScenarioScope<TestCtx> context;
    // known flag should be set even if underlying flags are 0
    EXPECT_TRUE( context->Item->IsKnownIntrinsic( ITEM_FLAG_CURSED ) );
}
