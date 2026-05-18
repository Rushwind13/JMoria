#include "AIMgr.h"
#include "ModState.h"
#include "TestContext.hpp"
using cucumber::ScenarioScope;

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

// Returns the door position: 2 steps east of the player's spawn position.
static JVector DoorPosEast( TestCtx *ctx ) { return JVector( ctx->vec.x + 2, ctx->vec.y ); }

// Returns the monster spawn position: 3 steps east of the player.
static JVector MonsterPosEast( TestCtx *ctx ) { return JVector( ctx->vec.x + 3, ctx->vec.y ); }

// -----------------------------------------------------------------------
// GIVEN
// -----------------------------------------------------------------------

GIVEN( "^I place tiles for a door corridor east of the player$" )
{
    ScenarioScope<TestCtx> context;
    JVector vBase = context->vec;

    // Lay down a clear 4-tile corridor east of the player, then punch in the
    // door at offset +2.  Clear any stale occupants so spawn checks pass.
    for( int dx = 1; dx <= 4; dx++ )
    {
        JVector v( vBase.x + dx, vBase.y );
        CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( v );
        EXPECT_NE( pTile, nullptr );
        int type = ( dx == 2 ) ? DUNG_IDX_DOOR : DUNG_IDX_FLOOR;
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( type );
        pTile->m_dwFlags = 0; // clear any stale lock / seen flags
        pTile->m_pCurMonster = nullptr;
    }
}

GIVEN( "^I lock the door east of the player$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    bool ok = g_pGame->GetDungeon()->LockDoor( vDoor );
    EXPECT_TRUE( ok );
}

GIVEN( "^I place an? ([-A-Za-z ]+) east of the door$" )
{
    REGEX_PARAM( std::string, monName );
    ScenarioScope<TestCtx> context;

    JVector vMon = MonsterPosEast( context.get() );
    JIVector vI( VEC_EXPAND( vMon ) );

    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( monName.c_str() );
    EXPECT_NE( pmd, nullptr );

    JResult r = CMonster::CreateMonster( pmd, vI );
    EXPECT_EQ( r, JSUCCESS );

    // Record where we put the monster so later steps can find it.
    context->vec_b = vMon;
}

GIVEN( "^the player has a strength bonus of (-?[0-9]+)$" )
{
    REGEX_PARAM( int, bonus );
    ScenarioScope<TestCtx> context;
    g_pGame->GetPlayer()->m_fDamageModifier = static_cast<float>( bonus );
}

GIVEN( "^the player has an Iron Spike in inventory$" )
{
    ScenarioScope<TestCtx> context;
    CItemDef *pDef = g_pGame->GetDungeon()->GetItemDef( ITEM_IDX_SPIKE );
    EXPECT_NE( pDef, nullptr );
    CItem *pItem = new CItem();
    pItem->Init( pDef );
    pItem->m_pllLink = g_pGame->GetPlayer()->m_llInventory->Add( pItem, pItem->m_id->m_dwIndex,
                                                                 pItem->GetInstanceId() );
}

// -----------------------------------------------------------------------
// WHEN
// -----------------------------------------------------------------------

WHEN( "^I drive the spawned monster toward the player$" )
{
    ScenarioScope<TestCtx> context;

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( context->vec_b );
    EXPECT_NE( pTile, nullptr );
    CMonster *pMon = pTile->m_pCurMonster;
    EXPECT_NE( pMon, nullptr );

    // Wake the monster (it spawns sleeping) and override state for the test.
    pMon->m_dwActiveEffects &= ~EFFECT_FLAG_SLEEP;
    pMon->m_pBrain->SetState( BRAINSTATE_GOTODEST );
    // Drive west (-1,0): monster is east of door, player is west of door.
    pMon->m_pBrain->m_vVel.Init( -1, 0 );
    pMon->m_pBrain->m_fSpeed = 1.0f;
    pMon->m_pBrain->m_fStateTicks = 0.0f;

    pMon->m_pBrain->UpdateGoToDest( 1.0f );
}

WHEN( "^the player bashes the door east$" )
{
    ScenarioScope<TestCtx> context;

    CModState mod;
    JVector vDoor = DoorPosEast( context.get() );
    mod.m_vNewPos = vDoor;
    context->result_bool = mod.DoBash();
}

WHEN( "^the player spikes the door east$" )
{
    ScenarioScope<TestCtx> context;

    CModState mod;
    mod.m_vNewPos = DoorPosEast( context.get() );
    context->result_bool = mod.DoSpike();
    EXPECT_TRUE( context->result_bool );
}

WHEN( "^the player unspikes the door east$" )
{
    ScenarioScope<TestCtx> context;

    CModState mod;
    mod.m_vNewPos = DoorPosEast( context.get() );
    context->result_bool = mod.DoUnspike();
    EXPECT_TRUE( context->result_bool );
}

// -----------------------------------------------------------------------
// THEN
// -----------------------------------------------------------------------

THEN( "^the door east of the player is open$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vDoor );
    EXPECT_NE( pTile, nullptr );
    EXPECT_EQ( pTile->m_dtd->m_dwType, DUNG_IDX_OPEN_DOOR );
}

THEN( "^the door east of the player is broken$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vDoor );
    EXPECT_NE( pTile, nullptr );
    EXPECT_EQ( pTile->m_dtd->m_dwType, DUNG_IDX_BROKEN_DOOR );
}

THEN( "^the door east of the player is closed$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vDoor );
    EXPECT_NE( pTile, nullptr );
    EXPECT_EQ( pTile->m_dtd->m_dwType, DUNG_IDX_DOOR );
}

THEN( "^the door east of the player is locked$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vDoor );
    EXPECT_NE( pTile, nullptr );
    EXPECT_EQ( pTile->m_dtd->m_dwType, DUNG_IDX_DOOR );
    EXPECT_TRUE( pTile->HasFlags( DUNG_FLAG_LOCKED ) );
}

THEN( "^the door east of the player is not locked$" )
{
    ScenarioScope<TestCtx> context;
    JVector vDoor = DoorPosEast( context.get() );
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vDoor );
    EXPECT_NE( pTile, nullptr );
    EXPECT_EQ( pTile->m_dtd->m_dwType, DUNG_IDX_DOOR );
    EXPECT_FALSE( pTile->HasFlags( DUNG_FLAG_LOCKED ) );
}
