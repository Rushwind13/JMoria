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

    // Verify flag-based color lookup returns correct colors for fire
    // Expected: RGB(255,0,0) red, RGB(255,128,0) orange (spec color), RGB(255,200,0) yellow-orange
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color is red
    Uint8 r, g, b, a;
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 0 );
    EXPECT_EQ( b, 0 );

    // Check second color is orange (task spec color)
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 128 );
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

    // Verify flag-based color lookup returns correct colors for cold
    // Expected: RGB(100,200,255) (spec color), RGB(150,220,255), RGB(200,240,255)
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color (task spec color)
    Uint8 r, g, b, a;
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 100 );
    EXPECT_EQ( g, 200 );
    EXPECT_EQ( b, 255 );

    // Check second color is lighter blue
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

    // Verify flag-based color lookup returns correct colors for acid
    // Expected: RGB(0,200,0) (spec color), RGB(100,220,100), RGB(150,255,150)
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );

    // Check first color (task spec color)
    Uint8 r, g, b, a;
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 0 );
    EXPECT_EQ( g, 200 );
    EXPECT_EQ( b, 0 );

    // Check second color is light green
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

    // Verify flag-based color lookup returns correct colors for electricity
    // Expected: RGB(255,255,100) (spec color), RGB(255,255,200), RGB(255,255,255) white
    JColor color0 = pDungeon->GetBeamColorForEffect( pEffect, 0 );
    JColor color1 = pDungeon->GetBeamColorForEffect( pEffect, 1 );
    JColor color2 = pDungeon->GetBeamColorForEffect( pEffect, 2 );

    // Check first color (task spec color)
    Uint8 r, g, b, a;
    color0.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 100 );

    // Check second color is lighter yellow
    color1.GetColor( r, g, b, a );
    EXPECT_EQ( r, 255 );
    EXPECT_EQ( g, 255 );
    EXPECT_EQ( b, 200 );

    // Check third color is white
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

    // Verify the beam character comes from flag-based lookup (R1 requirement)
    // GetBeamCharForEffect maps flags to characters at render time
    char actualChar = pDungeon->GetBeamCharForEffect( pEffect );
    EXPECT_EQ( actualChar, expectedChar );
}
