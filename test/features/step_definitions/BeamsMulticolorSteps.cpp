#include "TestContext.hpp"
using cucumber::ScenarioScope;

#include "Constants.h"
#include "Dungeon.h"
#include "Effect.h"
#include "Game.h"

/*#######
##
## THEN - Multicolor Beam Verification
##
#######*/

THEN( "^the beam effect has (EFFECT_FLAG_[A-Z_]+)$" )
{
    REGEX_PARAM( std::string, flagName );
    ScenarioScope<TestCtx> context;

    // Get the projectile effect from the dungeon
    CEffectDef *pEffect = g_pGame->GetDungeon()->GetProjectileEffect();

    // If effect is not set yet, it might still be NULL - that's a failure
    if( pEffect == nullptr )
    {
        FAIL() << "Projectile effect is NULL - beam was not set during zap";
        return;
    }

    // Cache it for later steps
    context->cachedProjectileEffect = pEffect;

    // Look up the expected flag value
    uint32 expectedFlags = 0;
    uint32 expectedFlags2 = 0;
    context->constants.LookupEffectFlag( flagName.c_str(), expectedFlags, expectedFlags2 );

    // Verify the effect has the expected flag
    EXPECT_TRUE( ( pEffect->m_dwFlags & expectedFlags ) != 0 );
}

THEN( "^the beam renders with red/orange gradient colors$" )
{
    ScenarioScope<TestCtx> context;
    CEffectDef *pEffect = context->cachedProjectileEffect;
    ASSERT_NE( pEffect, nullptr );

    // Verify the effect has FIRE flag
    EXPECT_TRUE( ( pEffect->m_dwFlags & EFFECT_FLAG_FIRE ) != 0 );

    // Verify the dungeon uses the effect for rendering
    CDungeon *pDungeon = g_pGame->GetDungeon();
    ASSERT_NE( pDungeon, nullptr );
    CEffectDef *pRenderEffect = pDungeon->GetProjectileEffect();
    EXPECT_EQ( pRenderEffect, pEffect );

    // Verify trajectory is set (rendering code path requires this)
    JLinkList<JIVector> *pTrajectory = pDungeon->GetProjectileTrajectory();
    ASSERT_NE( pTrajectory, nullptr );
    ASSERT_GT( pTrajectory->length(), 0 );

    // Verify game is in RANGED state (required for beam rendering at Dungeon.cpp:1234)
    EXPECT_EQ( g_pGame->GetGameStateIndex(), STATE_RANGED );

    // Reset test instrumentation before rendering
    pDungeon->m_beamWasRendered = false;

    // Trigger actual rendering to verify DrawDungeon() code path
    // This ensures SetTileColor and DrawChar are called with beam parameters
    // Rendering code at Dungeon.cpp:1234-1262 only executes if:
    // 1. State == STATE_RANGED (verified above)
    // 2. m_llProjectileTrajectory != NULL (verified above)
    // 3. m_pProjectileEffect != NULL (verified above)
    // All conditions met → lines 1255 and 1302 will call our helpers
    pDungeon->DrawDungeon();

    // F1 fix: Verify beam was ACTUALLY rendered (not just helpers exist)
    // If DrawDungeon() skipped calling helpers, this would be false
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the ACTUAL color that was rendered (F1 fix)
    // This is the exact value passed to SetTileColor() at line 1304
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    // F2 fix: Fire should start with orange (255,128,0), not red
    EXPECT_EQ( r, 255 ) << "Fire beam red component wrong";
    EXPECT_EQ( g, 128 ) << "Fire beam green component wrong (should be 128 for orange)";
    EXPECT_EQ( b, 0 ) << "Fire beam blue component wrong";

    // Also verify helper returns correct colors for reference
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color is orange (F2 fix - spec color RGB(255,128,0))
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 128 );
    EXPECT_EQ( b, 0 );

    // Check second color is orange-red gradient
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 64 );
    EXPECT_EQ( b, 0 );
}

THEN( "^the beam renders with cyan/blue/white gradient colors$" )
{
    ScenarioScope<TestCtx> context;
    CEffectDef *pEffect = context->cachedProjectileEffect;
    ASSERT_NE( pEffect, nullptr );

    // Verify the effect has COLD flag
    EXPECT_TRUE( ( pEffect->m_dwFlags & EFFECT_FLAG_COLD ) != 0 );

    // Verify the dungeon uses the effect for rendering
    CDungeon *pDungeon = g_pGame->GetDungeon();
    ASSERT_NE( pDungeon, nullptr );
    CEffectDef *pRenderEffect = pDungeon->GetProjectileEffect();
    EXPECT_EQ( pRenderEffect, pEffect );

    // Verify trajectory is set (rendering code path requires this)
    JLinkList<JIVector> *pTrajectory = pDungeon->GetProjectileTrajectory();
    ASSERT_NE( pTrajectory, nullptr );
    ASSERT_GT( pTrajectory->length(), 0 );

    // Verify game is in RANGED state (required for beam rendering at Dungeon.cpp:1234)
    EXPECT_EQ( g_pGame->GetGameStateIndex(), STATE_RANGED );

    // Reset test instrumentation before rendering
    pDungeon->m_beamWasRendered = false;

    // Trigger actual rendering to verify DrawDungeon() code path
    // Rendering code at Dungeon.cpp:1234-1262 only executes if all conditions met
    pDungeon->DrawDungeon();

    // F1 fix: Verify beam was ACTUALLY rendered (not just helpers exist)
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the ACTUAL color that was rendered (F1 fix)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 100 ) << "Cold beam red component wrong";
    EXPECT_EQ( g, 200 ) << "Cold beam green component wrong";
    EXPECT_EQ( b, 255 ) << "Cold beam blue component wrong";

    // Also verify helper returns correct colors for cold
    // Expected: RGB(100,200,255) (spec color), RGB(150,220,255), RGB(200,240,255)
    // These are the EXACT values DrawDungeon() passes to SetTileColor() at line 1255
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color (task spec color - what DrawDungeon renders at pathIndex=0)
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 100 );
    EXPECT_EQ( g, 200 );
    EXPECT_EQ( b, 255 );

    // Check second color is lighter blue (what DrawDungeon renders at pathIndex=1)
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 150 );
    EXPECT_EQ( g, 220 );
    EXPECT_EQ( b, 255 );
}

THEN( "^the beam renders with green gradient colors$" )
{
    ScenarioScope<TestCtx> context;
    CEffectDef *pEffect = context->cachedProjectileEffect;
    ASSERT_NE( pEffect, nullptr );

    // Verify the effect has ACID flag
    EXPECT_TRUE( ( pEffect->m_dwFlags & EFFECT_FLAG_ACID ) != 0 );

    // Verify the dungeon uses the effect for rendering
    CDungeon *pDungeon = g_pGame->GetDungeon();
    ASSERT_NE( pDungeon, nullptr );
    CEffectDef *pRenderEffect = pDungeon->GetProjectileEffect();
    EXPECT_EQ( pRenderEffect, pEffect );

    // Verify trajectory is set (rendering code path requires this)
    JLinkList<JIVector> *pTrajectory = pDungeon->GetProjectileTrajectory();
    ASSERT_NE( pTrajectory, nullptr );
    ASSERT_GT( pTrajectory->length(), 0 );

    // Verify game is in RANGED state (required for beam rendering at Dungeon.cpp:1234)
    EXPECT_EQ( g_pGame->GetGameStateIndex(), STATE_RANGED );

    // Reset test instrumentation before rendering
    pDungeon->m_beamWasRendered = false;

    // Trigger actual rendering to verify DrawDungeon() code path
    // Rendering code at Dungeon.cpp:1234-1262 only executes if all conditions met
    pDungeon->DrawDungeon();

    // F1 fix: Verify beam was ACTUALLY rendered (not just helpers exist)
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the ACTUAL color that was rendered (F1 fix)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 0 ) << "Acid beam red component wrong";
    EXPECT_EQ( g, 200 ) << "Acid beam green component wrong";
    EXPECT_EQ( b, 0 ) << "Acid beam blue component wrong";

    // Also verify helper returns correct colors for acid
    // Expected: RGB(0,200,0) (spec color), RGB(100,220,100), RGB(150,255,150)
    // These are the EXACT values DrawDungeon() passes to SetTileColor() at line 1255
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color (task spec color - what DrawDungeon renders at pathIndex=0)
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 0 );
    EXPECT_EQ( g, 200 );
    EXPECT_EQ( b, 0 );

    // Check second color is light green (what DrawDungeon renders at pathIndex=1)
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 100 );
    EXPECT_EQ( g, 220 );
    EXPECT_EQ( b, 100 );
}

THEN( "^the beam renders with yellow/white gradient colors$" )
{
    ScenarioScope<TestCtx> context;
    CEffectDef *pEffect = context->cachedProjectileEffect;
    ASSERT_NE( pEffect, nullptr );

    // Verify the effect has ELECTRICITY flag
    EXPECT_TRUE( ( pEffect->m_dwFlags & EFFECT_FLAG_ELECTRICITY ) != 0 );

    // Verify the dungeon uses the effect for rendering
    CDungeon *pDungeon = g_pGame->GetDungeon();
    ASSERT_NE( pDungeon, nullptr );
    CEffectDef *pRenderEffect = pDungeon->GetProjectileEffect();
    EXPECT_EQ( pRenderEffect, pEffect );

    // Verify trajectory is set (rendering code path requires this)
    JLinkList<JIVector> *pTrajectory = pDungeon->GetProjectileTrajectory();
    ASSERT_NE( pTrajectory, nullptr );
    ASSERT_GT( pTrajectory->length(), 0 );

    // Verify game is in RANGED state (required for beam rendering at Dungeon.cpp:1234)
    EXPECT_EQ( g_pGame->GetGameStateIndex(), STATE_RANGED );

    // Reset test instrumentation before rendering
    pDungeon->m_beamWasRendered = false;

    // Trigger actual rendering to verify DrawDungeon() code path
    // Rendering code at Dungeon.cpp:1234-1262 only executes if all conditions met
    pDungeon->DrawDungeon();

    // F1 fix: Verify beam was ACTUALLY rendered (not just helpers exist)
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the ACTUAL color that was rendered (F1 fix)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 ) << "Electric beam red component wrong";
    EXPECT_EQ( g, 255 ) << "Electric beam green component wrong";
    EXPECT_EQ( b, 100 ) << "Electric beam blue component wrong";

    // Also verify helper returns correct colors for electricity
    // Expected: RGB(255,255,100) (spec color), RGB(255,255,200), RGB(255,255,255) white
    // These are the EXACT values DrawDungeon() passes to SetTileColor() at line 1255
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );
    JColor color2 = pDungeon->GetBeamColorForEffect( pEffect, 2 );

    // Check first color (task spec color - what DrawDungeon renders at pathIndex=0)
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 100 );

    // Check second color is lighter yellow (what DrawDungeon renders at pathIndex=1)
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 200 );

    // Check third color is white (what DrawDungeon renders at pathIndex=2)
    color2.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 255 );
}

THEN( "^the beam renders with character '(.)'$" )
{
    REGEX_PARAM( char, expectedChar );
    ScenarioScope<TestCtx> context;

    // Try to get the effect from cache first, otherwise get it from the dungeon
    CEffectDef *pEffect = context->cachedProjectileEffect;
    if( pEffect == nullptr )
    {
        pEffect = g_pGame->GetDungeon()->GetProjectileEffect();
    }

    // Verify we have an effect
    ASSERT_NE( pEffect, nullptr );

    // Verify the dungeon has the effect set for rendering
    CDungeon *pDungeon = g_pGame->GetDungeon();
    ASSERT_NE( pDungeon, nullptr );
    CEffectDef *pRenderEffect = pDungeon->GetProjectileEffect();
    EXPECT_EQ( pRenderEffect, pEffect );

    // Verify trajectory is set (rendering code path requires this)
    JLinkList<JIVector> *pTrajectory = pDungeon->GetProjectileTrajectory();
    ASSERT_NE( pTrajectory, nullptr );
    ASSERT_GT( pTrajectory->length(), 0 );

    // Verify game is in RANGED state (required for beam rendering at Dungeon.cpp:1234)
    EXPECT_EQ( g_pGame->GetGameStateIndex(), STATE_RANGED );

    // Reset test instrumentation before rendering
    pDungeon->m_beamWasRendered = false;
    pDungeon->m_lastBeamCharRendered = '\0';

    // Trigger actual rendering to verify DrawDungeon() code path
    // This ensures DrawChar is called with the beam character at line 1308
    // Rendering code at Dungeon.cpp:1234-1262 only executes if all conditions met
    pDungeon->DrawDungeon();

    // F1 fix: Verify beam was ACTUALLY rendered (not just helpers exist)
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // F1 fix: Verify the ACTUAL character that was rendered
    // This is the exact value passed to DrawChar() at line 1308
    EXPECT_EQ( pDungeon->m_lastBeamCharRendered, expectedChar )
        << "Rendered beam character does not match expected";

    // Also verify the beam character comes from flag-based lookup (R1 requirement)
    // GetBeamCharForEffect maps flags to characters at render time
    // This is the value DrawDungeon() gets from the helper at line 1306
    char actualChar = pDungeon->GetBeamCharForEffect( pEffect );
    EXPECT_EQ( actualChar, expectedChar );
}
