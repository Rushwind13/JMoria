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

    // Verify beam was rendered
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the rendered color comes from the Fire palette (frame 0 = first trajectory tile)
    // Colors.txt Fire[0] = (255,0,0)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 ) << "Fire beam red component wrong";
    EXPECT_EQ( g, 0 ) << "Fire beam green component wrong";
    EXPECT_EQ( b, 0 ) << "Fire beam blue component wrong";

    // Verify GetColor() returns the correct palette frames
    // Colors.txt Fire: (255,0,0) ; (255,165,0) ; (255,255,0)
    JColor color0 = pEffect->GetColor( 0 );
    JColor color1 = pEffect->GetColor( 1 );

    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 0 );
    EXPECT_EQ( b, 0 );

    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 165 );
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

    // Verify beam was rendered
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the rendered color comes from the Cold palette (frame 0 = first trajectory tile)
    // Colors.txt Cold[0] = (0,255,255)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 0 ) << "Cold beam red component wrong";
    EXPECT_EQ( g, 255 ) << "Cold beam green component wrong";
    EXPECT_EQ( b, 255 ) << "Cold beam blue component wrong";

    // Verify GetColor() returns the correct palette frames
    // Colors.txt Cold: (0,255,255) ; (135,206,250) ; (240,248,255)
    JColor color0 = pEffect->GetColor( 0 );
    JColor color1 = pEffect->GetColor( 1 );

    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 0 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 255 );

    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 135 );
    EXPECT_EQ( g, 206 );
    EXPECT_EQ( b, 250 );
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

    // Verify beam was rendered
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the rendered color comes from the Acid palette (frame 0 = first trajectory tile)
    // Colors.txt Acid[0] = (50,205,50)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 50 ) << "Acid beam red component wrong";
    EXPECT_EQ( g, 205 ) << "Acid beam green component wrong";
    EXPECT_EQ( b, 50 ) << "Acid beam blue component wrong";

    // Verify GetColor() returns the correct palette frames
    // Colors.txt Acid: (50,205,50) ; (144,238,144) ; (173,255,47)
    JColor color0 = pEffect->GetColor( 0 );
    JColor color1 = pEffect->GetColor( 1 );

    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 50 );
    EXPECT_EQ( g, 205 );
    EXPECT_EQ( b, 50 );

    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 144 );
    EXPECT_EQ( g, 238 );
    EXPECT_EQ( b, 144 );
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

    // Verify beam was rendered
    EXPECT_TRUE( pDungeon->m_beamWasRendered )
        << "Beam rendering path was not executed in DrawDungeon()";

    // Verify the rendered color comes from the Lightning palette (frame 0 = first trajectory tile)
    // Colors.txt Lightning[0] = (255,255,0)
    Uint8 r, g, b, a;
    pDungeon->m_lastBeamColorRendered.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 ) << "Electric beam red component wrong";
    EXPECT_EQ( g, 255 ) << "Electric beam green component wrong";
    EXPECT_EQ( b, 0 ) << "Electric beam blue component wrong";

    // Verify GetColor() returns the correct palette frames
    // Colors.txt Lightning: (255,255,0) ; (255,255,255) ; (255,215,0)
    JColor color0 = pEffect->GetColor( 0 );
    JColor color1 = pEffect->GetColor( 1 );
    JColor color2 = pEffect->GetColor( 2 );

    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 0 );

    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 255 );

    color2.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 215 );
    EXPECT_EQ( b, 0 );
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

    // Verify the beam character comes from the effect definition
    char actualChar = pEffect->GetBeamChar();
    EXPECT_EQ( actualChar, expectedChar );
}
