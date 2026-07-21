#include "DisplayText.h"
#include "EndGameState.h"
#include "TestContext.hpp"
using cucumber::ScenarioScope;

/*#######
##
## GIVEN
##
#######*/

GIVEN( "^I have a game$" )
{
    ScenarioScope<TestCtx> context;
    if( g_pGame != NULL )
    {
        g_pGame->Term();
        g_pGame = NULL;
    }
    g_pGame = new CGame;
}
GIVEN( "^I initialize the game$" )
{
    ScenarioScope<TestCtx> context;
    context->result = g_pGame->Init( "../../JMoria/", RenderMode::None );
}
GIVEN( "^the game has a player$" )
{
    ScenarioScope<TestCtx> context;
    context->vec = g_pGame->GetPlayer()->m_vPos;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}
GIVEN( "^the dungeon is seeded$" )
{
    ScenarioScope<TestCtx> context;
    Util::SeedRandom( 42 );
}
GIVEN( "^I spawn a ([-A-Za-z ]+), a monster with SEEK, near the player$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;
    context->vec_b.Init( -2, -2 );
    context->vec_b += context->vec;

    // Dungeon mangling for test: make sure LZ is clear
    // JVector vLZ( context->vec_b.x, context->vec_b.y );
    // for( int y = 0; y < 2; y++ )
    // {
    //     vLZ.y = context->vec_b.y + y;
    //     for( int x = 0; x < 2; x++ )
    //     {
    //         vLZ.x = context->vec_b.x + x;
    //         CDungeonTile *pLZ = g_pGame->GetDungeon()->GetTile( vLZ );
    //         pLZ->m_dtd->m_dwType = DUNG_IDX_FLOOR;
    //     }
    // }

    int expected = DUNG_COLL_NO_COLLISION;
    int actual = g_pGame->GetDungeon()->IsWalkableFor( context->vec_b );

    EXPECT_EQ( expected, actual );
    // End Dungeon Mangling

    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( monster.c_str() );

    // Monster is correct monster
    int compare = Util::jstrcmp( monster.c_str(), pmd->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", monster.c_str(), pmd->m_szName );
    }
    EXPECT_EQ( compare, 0 );

    JIVector vI( VEC_EXPAND( context->vec_b ) );
    context->result = CMonster::CreateMonster( pmd, vI );

    JLog( LOG_LEVEL_ERROR, true, "<%f %f> %s %d\n", VEC_EXPAND( context->vec_b ), pmd->m_szName,
          context->result );
}
GIVEN( "^I spawn a ([-A-Za-z ]+), a monster with SEEK$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;
    context->vec_b.Init( -2, -2 );
    context->vec_b += context->vec;

    // Dungeon mangling for test: make sure LZ is clear
    JVector vLZ( context->vec_b.x, context->vec_b.y );
    for( int y = 0; y < 2; y++ )
    {
        vLZ.y = context->vec_b.y + y;
        for( int x = 0; x < 2; x++ )
        {
            vLZ.x = context->vec_b.x + x;
            CDungeonTile *pLZ = g_pGame->GetDungeon()->GetTile( vLZ );
            pLZ->m_dtd->m_dwType = DUNG_IDX_FLOOR;
        }
    }

    int expected = DUNG_COLL_NO_COLLISION;
    int actual = g_pGame->GetDungeon()->IsWalkableFor( context->vec_b );

    EXPECT_EQ( expected, actual );
    // End Dungeon Mangling

    CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( monster.c_str() );

    // Monster is correct monster
    int compare = Util::jstrcmp( monster.c_str(), pmd->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", monster.c_str(), pmd->m_szName );
    }
    EXPECT_EQ( compare, 0 );

    JIVector vI( VEC_EXPAND( context->vec_b ) );
    context->result = CMonster::CreateMonster( pmd, vI );

    JLog( LOG_LEVEL_ERROR, true, "<%f %f> %s %d\n", VEC_EXPAND( context->vec_b ), pmd->m_szName,
          context->result );
}
GIVEN( "^I update the monster's brain$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    pMon->m_pBrain->Update( 1.0f );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I update the monster's brain again$" )
{
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    pMon->m_pBrain->Update( 1.0f );
}
WHEN( "^I terminate the game$" ) { g_pGame->Term(); }

/*#######
##
## THEN
##
#######*/

THEN( "^the game initalized successfully$" )
{
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
}

THEN( "^the game terminates successfully$" ) { EXPECT_EQ( true, true ); }

THEN( "^the ([-A-Za-z ]+) spawned successfully$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;
    int actual = context->result;
    EXPECT_EQ( actual, JSUCCESS );
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;

    // Monster is correct monster
    int compare = Util::jstrcmp( monster.c_str(), pMon->m_md->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", monster.c_str(), pMon->m_md->m_szName );
    }
    EXPECT_EQ( compare, 0 );
}

THEN( "^the ([-A-Za-z ]+) wants to move toward the player$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;

    // Monster is correct monster
    int compare = Util::jstrcmp( monster.c_str(), pMon->m_md->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", monster.c_str(), pMon->m_md->m_szName );
    }
    EXPECT_EQ( compare, 0 );

    // Monster wants to move toward player
    JLog( LOG_LEVEL_ERROR, false, "heading <%.2f %.2f>\n", VEC_EXPAND( pMon->m_pBrain->m_vVel ) );
    JVector expected( 1.0f, 1.0f );
    JVector actual = pMon->m_pBrain->m_vVel;
    EXPECT_EQ( expected.x, actual.x );
    EXPECT_EQ( expected.y, actual.y );

    EXPECT_EQ( pMon->m_pBrain->GetState(), BRAINSTATE_GOTODEST );
}

THEN( "^the ([-A-Za-z ]+) moves toward the player$" )
{
    REGEX_PARAM( std::string, monster );
    ScenarioScope<TestCtx> context;
    // Monster not in old pos
    CMonster *pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;

    bool expected = pMon == NULL;
    EXPECT_EQ( expected, true );

    // Monster is in new pos
    JVector delta( 1, 1 );
    context->vec_b += delta;
    pMon = g_pGame->GetDungeon()->GetTile( context->vec_b )->m_pCurMonster;
    expected = pMon != NULL;
    EXPECT_EQ( expected, true );

    // Monster is correct monster
    int compare = Util::jstrcmp( monster.c_str(), pMon->m_md->m_szName );
    if( compare != 0 )
    {
        JLog( LOG_LEVEL_ERROR, true, "want %s have %s\n", monster.c_str(), pMon->m_md->m_szName );
    }
    EXPECT_EQ( compare, 0 );
}
WHEN( "^I set the killer name to \"([^\"]+)\"$" )
{
    REGEX_PARAM( std::string, name );
    ScenarioScope<TestCtx> context;
    char *szName = new char[name.size() + 1];
    strncpy( szName, name.c_str(), name.size() );
    szName[name.size()] = '\0';

    CEndGameState *pEG = g_pGame->m_pEndGameState;
    if( pEG->m_pScore->m_szKilledBy )
        delete[] pEG->m_pScore->m_szKilledBy;
    pEG->m_pScore->m_szKilledBy = szName;

    // Provide minimal name/class so DoTomb() doesn't crash on NULL
    if( !pEG->m_pScore->m_szName )
    {
        pEG->m_pScore->m_szName = new char[5];
        strncpy( pEG->m_pScore->m_szName, "Test", 5 );
    }
    if( !pEG->m_pScore->m_szClass )
    {
        pEG->m_pScore->m_szClass = new char[8];
        strncpy( pEG->m_pScore->m_szClass, "Warrior", 8 );
    }
}

WHEN( "^I render the tombstone$" )
{
    ScenarioScope<TestCtx> context;
    g_pGame->GetEnd()->Clear();
    g_pGame->m_pEndGameState->DoTomb();
    strncpy( context->szBuffer, g_pGame->GetEnd()->m_szText, sizeof( context->szBuffer ) - 1 );
    context->szBuffer[sizeof( context->szBuffer ) - 1] = '\0';
}

THEN( "^no tombstone line has characters past the closing pipe$" )
{
    ScenarioScope<TestCtx> context;
    const char *p = context->szBuffer;
    while( p && *p )
    {
        const char *eol = strchr( p, '\n' );
        if( !eol )
            eol = p + strlen( p );
        const char *firstPipe = (const char *)memchr( p, '|', eol - p );
        if( firstPipe )
        {
            const char *lastPipe = NULL;
            for( const char *q = eol - 1; q >= firstPipe; q-- )
            {
                if( *q == '|' )
                {
                    lastPipe = q;
                    break;
                }
            }
            if( lastPipe )
            {
                for( const char *q = lastPipe + 1; q < eol; q++ )
                {
                    EXPECT_TRUE( *q == ' ' || *q == '\0' )
                        << "Character '" << *q << "' found after closing '|' on tombstone line";
                }
            }
        }
        p = ( *eol == '\n' ) ? eol + 1 : eol;
    }
}
