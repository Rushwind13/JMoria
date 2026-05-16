// ElementalImmunitySteps.cpp — BDD step definitions for monster_elemental_immunity.feature
#include "TestContext.hpp"
using cucumber::ScenarioScope;

// Maps element name to an elemental touch effect name from Effects.txt.
static const char *ElementToEffectName( const std::string &element )
{
    if( element == "fire" )
        return "Fire Touch";
    if( element == "cold" )
        return "Cold Touch";
    if( element == "lightning" )
        return "Lightning Touch";
    if( element == "acid" )
        return "Acid Touch";
    return NULL;
}

// ── GIVEN ────────────────────────────────────────────────────────────────────

GIVEN( "^I record the spawned monster's HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    ASSERT_NE( pMon, nullptr );
    context->result_float = pMon->m_fCurHP;
}

GIVEN( "^the spawned monster is damaged to half HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    ASSERT_NE( pMon, nullptr );
    pMon->m_fCurHP = pMon->m_fHP * 0.5f;
}

// ── WHEN ─────────────────────────────────────────────────────────────────────

WHEN( "^I fire a (fire|cold|lightning|acid) elemental bolt at the spawned monster$" )
{
    REGEX_PARAM( std::string, element );
    ScenarioScope<TestCtx> context;

    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    ASSERT_NE( pMon, nullptr );

    const char *szEffectName = ElementToEffectName( element );
    ASSERT_NE( szEffectName, nullptr );

    CEffectDef *pED = g_pGame->GetDungeon()->GetEffectDef( szEffectName );
    ASSERT_NE( pED, nullptr ) << "Effect not found: " << szEffectName;

    CEffect effect;
    effect.m_ed = pED;
    effect.m_dwEffect = pED->m_dwEffect;
    effect.m_dwFlags = pED->m_dwFlags;
    effect.m_dwFlags2 = pED->m_dwFlags2;
    effect.m_dwModifier = pED->m_dwModifier;
    effect.SetAmount( pED->m_szAmount );

    g_pGame->GetPlayer()->SetRangedHitPosition( pMon->GetPos() );
    g_pGame->GetPlayer()->DoElementalHit( &effect );
}

// ── THEN ─────────────────────────────────────────────────────────────────────

THEN( "^the spawned monster's HP equals the recorded HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    ASSERT_NE( pMon, nullptr );
    EXPECT_FLOAT_EQ( pMon->m_fCurHP, context->result_float );
}

THEN( "^the spawned monster's HP is greater than the recorded HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    ASSERT_NE( pMon, nullptr );
    EXPECT_GT( pMon->m_fCurHP, context->result_float );
}

THEN( "^the spawned monster's HP is less than the recorded HP$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    // Monster destroyed by the hit counts as HP 0 — still less than recorded
    if( pMon == nullptr )
        return;
    EXPECT_LT( pMon->m_fCurHP, context->result_float );
}
