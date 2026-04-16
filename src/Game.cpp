// Game.cpp
//
// implementation of the Game Shell

#include "Game.h"
#include "Dungeon.h"
#include "Player.h"
#include "TileSet.h"

#include "ClockStepState.h"
#include "CmdState.h"
#include "EndGameState.h"
#include "IntroState.h"
#include "LookState.h"
#include "ModState.h"
#include "RangedState.h"
#include "RestState.h"
#include "RunState.h"
#include "StringInputState.h"
#include "TargetState.h"
#include "UseState.h"

#include "DisplayText.h"
#ifdef RENDER_OPENGL
#include "Render.h"
#endif
#ifdef RENDER_ASCII
#include "RenderASCII.h"
#endif
#include "RenderNull.h"

#include "AIMgr.h"

#ifdef PROFILE
#include "utils/profile.h"
#endif

CGame::CGame()
    : m_pDungeon( NULL ),
      m_pPlayer( NULL ),
      m_pRender( NULL ),
      m_pCurState( NULL ),
      m_pClockStepState( NULL ),
      m_pCmdState( NULL ),
      m_pEndGameState( NULL ),
      m_pIntroState( NULL ),
      m_pLookState( NULL ),
      m_pModState( NULL ),
      m_pRangedState( NULL ),
      m_pRestState( NULL ),
      m_pRunState( NULL ),
      m_pStringInputState( NULL ),
      m_pTargetState( NULL ),
      m_pUseState( NULL ),
      m_eCurState( STATE_INVALID ),
      m_fGameTime( 0.0f ),
      m_eRenderMode( RenderMode::None ),
      m_bShowStats( true ),
      m_bShowInv( false ),
      m_bShowEquip( false )
{
    m_pClockStepState = new CClockStepState;
    m_pCmdState = new CCmdState;
    m_pEndGameState = new CEndGameState;
    m_pIntroState = new CIntroState;
    m_pLookState = new CLookState;
    m_pModState = new CModState;
    m_pRangedState = new CRangedState;
    m_pRestState = new CRestState;
    m_pRunState = new CRunState;
    m_pStringInputState = new CStringInputState;
    m_pTargetState = new CTargetState;
    m_pUseState = new CUseState;
#ifdef TURN_BASED
    m_bReadyForUpdate = false;
#endif // TURN_BASED
};

JResult CGame::Init( const char *szBasedir, RenderMode mode )
{
    JResult result = JSUCCESS;
    // Initialize all the game stuff, baby.

    g_Constants.Init();

    m_eRenderMode = mode;

    // Init the Render
    if( m_eRenderMode == RenderMode::None )
    {
        m_pRender = new CRenderNull;
    }
#ifdef RENDER_ASCII
    else if( m_eRenderMode == RenderMode::ASCII )
    {
        m_pRender = new CRenderASCII;
        result = m_pRender->Init( 80, 24, 0 );
    }
#endif
#ifdef RENDER_OPENGL
    else if( m_eRenderMode == RenderMode::OpenGL )
    {
        m_pRender = new CRender;
        result = m_pRender->Init( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );
    }
#endif
    else
    {
        JLog( LOG_LEVEL_ERROR, true, "No render mode specified.\n" );
        return 1;
    }
    if( m_pRender && result != JSUCCESS )
    {
        m_pRender->Term();
        return result;
    }

    m_pMsgsDT = new CDisplayText( szBasedir, JRect( 0, 0, 640, 40 ), 255 );
    m_pMsgsDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE );

    m_pStatsDT = new CDisplayText( szBasedir, JRect( 0, 50, 150, 480 ), 220 );
    m_pStatsDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX );

    m_pInvDT = new CDisplayText( szBasedir, JRect( 440, 50, 640, 340 ), 180 );
    m_pInvDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX );

    m_pEquipDT = new CDisplayText( szBasedir, JRect( 440, 345, 640, 480 ), 180 );
    m_pEquipDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX );

    m_pUseDT = new CDisplayText( szBasedir, JRect( 200, 40, 440, 480 ), 200 );
    m_pUseDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX );

    m_pEndGameDT = new CDisplayText( szBasedir, JRect( 0, 0, 640, 480 ), 255 );
    m_pEndGameDT->SetFlags( FLAG_TEXT_WRAP_WHITESPACE | FLAG_TEXT_BOUNDING_BOX );

    // Let the renderer configure display region rects for its coordinate system
    m_pRender->ConfigureDisplayRegions( m_pMsgsDT, m_pStatsDT, m_pInvDT, m_pEquipDT, m_pUseDT,
                                        m_pEndGameDT );

    m_bShowInv = m_pRender->ShouldAutoShowInventory();
    m_bShowEquip = m_pRender->ShouldAutoShowEquipment();

    m_pAIMgr = new CAIMgr;
    m_pAIMgr->Init();

    // Init the Player
    m_pPlayer = new CPlayer;
    m_pPlayer->Init( szBasedir );

    // Init the Dungeon
    m_pDungeon = new CDungeon;
    m_pDungeon->Init( szBasedir );

#ifdef PROFILE
    ProfileInit();
#endif // PROFILE

#ifdef CLOCKSTEP
    // Now, in exciting slow motion!
    SetState( STATE_CLOCKSTEP );
#else
    // Set up the initial game state
    // SetState( STATE_COMMAND );
    SetState( STATE_INTRO );
#endif // CLOCKSTEP
       // we're up.

    return JSUCCESS;
}

void CGame::Term()
{
    JLog( LOG_LEVEL_INFO, true, "Terminating the game..." );
    if( m_pRender )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Renderer..." );
        m_pRender->Term();
        delete m_pRender;
        m_pRender = NULL;
    }

    if( m_pDungeon )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Dungeon..." );
        m_pDungeon->Term();
        delete m_pDungeon;
        m_pDungeon = NULL;
    }

    if( m_pPlayer )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Player..." );
        m_pPlayer->Term();
        delete m_pPlayer;
        m_pPlayer = NULL;
    }

    if( m_pAIMgr )
    {
        delete m_pAIMgr;
        m_pAIMgr = NULL;
    }

    JLog( LOG_LEVEL_DEBUG, true, "States..." );
    if( m_pClockStepState )
    {
        delete m_pClockStepState;
        m_pClockStepState = NULL;
    }

    if( m_pCmdState )
    {
        delete m_pCmdState;
        m_pCmdState = NULL;
    }

    if( m_pEndGameState )
    {
        delete m_pEndGameState;
        m_pEndGameState = NULL;
    }

    if( m_pIntroState )
    {
        delete m_pIntroState;
        m_pIntroState = NULL;
    }

    if( m_pLookState )
    {
        delete m_pLookState;
        m_pLookState = NULL;
    }

    if( m_pModState )
    {
        delete m_pModState;
        m_pModState = NULL;
    }

    if( m_pRangedState )
    {
        delete m_pRangedState;
        m_pRangedState = NULL;
    }

    if( m_pRestState )
    {
        delete m_pRestState;
        m_pRestState = NULL;
    }

    if( m_pRunState )
    {
        delete m_pRunState;
        m_pRunState = NULL;
    }

    if( m_pStringInputState )
    {
        delete m_pStringInputState;
        m_pStringInputState = NULL;
    }

    if( m_pTargetState )
    {
        delete m_pTargetState;
        m_pTargetState = NULL;
    }

    if( m_pUseState )
    {
        delete m_pUseState;
        m_pUseState = NULL;
    }

    JLog( LOG_LEVEL_DEBUG, true, "Message boxes..." );
    if( m_pMsgsDT )
    {
        delete m_pMsgsDT;
        m_pMsgsDT = NULL;
    }

    if( m_pStatsDT )
    {
        delete m_pStatsDT;
        m_pStatsDT = NULL;
    }

    if( m_pInvDT )
    {
        delete m_pInvDT;
        m_pInvDT = NULL;
    }

    if( m_pEquipDT )
    {
        delete m_pEquipDT;
        m_pEquipDT = NULL;
    }

    if( m_pUseDT )
    {
        delete m_pUseDT;
        m_pUseDT = NULL;
    }

    if( m_pEndGameDT )
    {
        delete m_pEndGameDT;
        m_pEndGameDT = NULL;
    }
    JLog( LOG_LEVEL_DEBUG, true, "done.\n" );
}

void CGame::Quit( int returncode )
{
    Term();
    exit( returncode );
}

void CGame::SetState( int eNewState )
{
    m_eCurState = eNewState;
    switch( m_eCurState )
    {
    case STATE_COMMAND:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pCmdState );
        break;
    case STATE_LOOK:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pLookState );
        break;
    case STATE_MODIFY:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pModState );
        break;
    case STATE_STRINGINPUT:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pStringInputState );
        break;
    case STATE_RANGED:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pRangedState );
        break;
    case STATE_REST:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pRestState );
        break;
    case STATE_RUN:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pRunState );
        break;
    case STATE_TARGET:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pTargetState );
        break;
    case STATE_USE:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pUseState );
        break;
    case STATE_ENDGAME:
    {
        m_pCurState = reinterpret_cast<CStateBase *>( m_pEndGameState );
        JKeysym keysym;
        keysym.sym = JKEY_SPACE;
        m_pCurState->HandleKey( &keysym );
    }
    break;
    case STATE_INTRO:
    {
        m_pCurState = reinterpret_cast<CStateBase *>( m_pIntroState );
        JKeysym keysym;
        keysym.sym = JKEY_SPACE;
        m_pCurState->HandleKey( &keysym );
    }
    break;
    case STATE_CLOCKSTEP:
    {
        m_pCurState = reinterpret_cast<CStateBase *>( m_pClockStepState );
        JKeysym keysym;
        keysym.sym = JKEY_SPACE;
        m_pCurState->HandleKey( &keysym );
    }
    break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "Tried to change to unknown state.\n" );
        break;
    }
}

#ifdef CLOCKSTEP

float pow( float base, int exp )
{
    if( exp == 0 )
        return 1;
    else
        return base * pow( base, exp - 1 );
}
float plouffBig(
    int n ) // http://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula
{
    float pi = 0.0f;
    int k = 0;

    while( k < n )
    {
        pi += ( 1.0f / pow( 16, k ) ) * ( 4.0f / ( 8 * k + 1 ) ) - ( 2.0f / ( 8 * k + 4 ) ) -
              ( 1.0f / ( 8 * k + 5 ) ) - ( 1.0f / ( 8 * k + 6 ) );
        k++;
    }

    return pi * 2.0f;
}

bool CGame::WaitForTick()
{
    int count = 0;
    while( !m_bReadyForUpdate )
    {
        // Calculate Tau to some decimal places.
        int n = Util::Roll( 1, 100 );
        float tau = plouffBig( n );
        GetMsgs()->Printf( "n: %d, tau: %f\n", n, tau );
        count++;
        if( count > 10000 )
            break;
    }
    return true;
}
#endif // CLOCKSTEP
/*
int CGame::Update()
{
#ifdef PROFILE
                ProfileBegin( "Main Loop" );
#endif // PROFILE
                curTime = GetTickCount();
                if( curTime > nextTime )
                {
                        g_pGame->GetStats()->Printf( "\nstats go here...\n");
                        nextTime = curTime + 2000;
                }

                // handle the events in the queue
                g_pGame->HandleEvents(isActive, done);

                // draw the scene
                if ( isActive )
                {
#ifdef PROFILE
                        ProfileBegin( "Graphics Draw Routine" );
#endif // PROFILE

                        g_pGame->GetRender()->PreDraw();
                        // Draw the player
                        g_pGame->GetPlayer()->Draw();

                        // Draw the dungeon
                        g_pGame->GetDungeon()->Draw();
                        g_pGame->GetMsgs()->Draw();
                        g_pGame->GetStats()->Draw();

#ifdef PROFILE
                        ProfileDraw();

                        ProfileEnd( "Graphics Draw Routine" );
#endif // PROFILE
                        g_pGame->GetRender()->PostDraw();

                        // pageflip
                        SDL_GL_SwapBuffers( );
                } // if( isactive )

#ifdef PROFILE
                ProfileEnd( "Main Loop" );
                ProfileDumpOutputToBuffer();
#endif PROFILE
        }

        return done;
}/**/

#ifdef TURN_BASED
bool CGame::Update()
{
    float fCurTime = 1.0f;
    if( m_bReadyForUpdate )
    {
        m_fGameTime++;
        // fCurTime = 1.0f;
        m_bReadyForUpdate = false;
        // TODO: Why does the AI require 2 ticks to move the monster?
        GetAIMgr()->Update( fCurTime );
        // GetAIMgr()->Update( fCurTime );
    }
//    else
//    {
//        return false;
//    }
#else
bool CGame::Update( float fCurTime )
{
    m_fGameTime += fCurTime;
    // Don't update AI during use commands or ranged item selection
    if( m_eCurState != STATE_USE && m_eCurState != STATE_RANGED )
    {
        GetAIMgr()->Update( fCurTime );
    }
#endif // TURN_BASED

#ifdef CLOCKSTEP
    // During CLOCKSTEP generation, only update dungeon and stats.
    // After ESC transitions to CmdState, use the normal gameplay update path.
    if( m_eCurState == STATE_CLOCKSTEP )
    {
        GetDungeon()->Update( fCurTime );
        GetStats()->Update( fCurTime );
    }
    else
    {
        // Normal gameplay update path (post-CLOCKSTEP)
        GetPlayer()->Update( fCurTime );
        GetDungeon()->Update( fCurTime );
        GetMsgs()->Update( fCurTime );
        GetStats()->Update( fCurTime );
        GetInv()->Update( fCurTime );
        GetEquip()->Update( fCurTime );
        m_pCurState->Update( fCurTime );
    }
#else
    // Update the player
    GetPlayer()->Update( fCurTime );

    // Update the dungeon
    GetDungeon()->Update( fCurTime );
    GetMsgs()->Update( fCurTime );
    GetStats()->Update( fCurTime );
    GetInv()->Update( fCurTime );
    GetEquip()->Update( fCurTime );
    if( m_eCurState == STATE_USE )
    {
        eUseModifier mod = reinterpret_cast<CUseState *>( m_pCurState )->GetModifier();
        eInvFilter filter = INV_COMPLETE;
        switch( mod )
        {
        case USE_QUAFF:
            filter = INV_QUAFF;
            break;
        case USE_READ:
            filter = INV_READ;
            break;
        case USE_WIELD:
            filter = INV_WIELD;
            break;
        default:
            break;
        }
        switch( mod )
        {
        case USE_WIELD:
        case USE_DROP:
        case USE_READ:
        case USE_QUAFF:
            GetPlayer()->DisplayInventory( PLACEMENT_USE, filter );
            break;
        case USE_REMOVE:
            GetPlayer()->DisplayEquipment( PLACEMENT_USE );
            break;
        default:
            JLog( LOG_LEVEL_DEBUG, true, "Nothing to display for command\n" );
            break;
        }
        GetUse()->Update( fCurTime );
    }
    else if( m_eCurState == STATE_RANGED )
    {
        switch( reinterpret_cast<CRangedState *>( m_pCurState )->GetModifier() )
        {
        case RANGED_FIRE:
            GetPlayer()->DisplayEquipment( PLACEMENT_USE, INV_FIRE );
            break;
        case RANGED_ZAP:
            GetPlayer()->DisplayInventory( PLACEMENT_USE, INV_ZAP );
            break;
        default:
            JLog( LOG_LEVEL_DEBUG, true, "Nothing to display for command\n" );
            break;
        }
        GetUse()->Update( fCurTime );
    }
    else if( m_eCurState == STATE_ENDGAME )
    {
        GetEnd()->Update( fCurTime );
    }
    else if( m_eCurState == STATE_INTRO )
    {
        GetEnd()->Update( fCurTime );
    }
    m_pCurState->Update( fCurTime );
#endif // CLOCKSTEP
    return true;
}

void CGame::Draw()
{
    bool bResized = GetRender()->CheckResize();
    GetRender()->PreDraw();

    bool bASCII = ( m_eRenderMode == RenderMode::ASCII );

    // After resize, update DisplayText rects and auto-show/hide inventory
    if( bResized )
    {
        GetRender()->ConfigureDisplayRegions( m_pMsgsDT, m_pStatsDT, m_pInvDT, m_pEquipDT, m_pUseDT,
                                              m_pEndGameDT );
        if( bASCII )
            m_bShowInv = GetRender()->ShouldAutoShowInventory();
    }

    bool bOverlayState = ( m_eCurState == STATE_INTRO || m_eCurState == STATE_ENDGAME );

    // In ASCII mode, overlay states (intro/endgame) take the full screen
    // and shouldn't show dungeon/player/stats underneath
    if( !bASCII || !bOverlayState )
    {
        // Draw the dungeon
        GetDungeon()->Draw();

        // Draw the player
        GetPlayer()->Draw();

        GetMsgs()->Draw();

        // Panel visibility toggled by i/e/C keys
        if( m_bShowStats )
            GetStats()->Draw();
        if( m_bShowInv )
            GetInv()->Draw();
        if( m_bShowEquip )
            GetEquip()->Draw();
    }

    if( m_eCurState == STATE_USE )
    {
        GetUse()->Draw();
    }
    if( m_eCurState == STATE_RANGED )
    {
        if( m_pRangedState->NeedsSelection() )
        {
            GetUse()->Draw();
        }
    }
    else if( m_eCurState == STATE_ENDGAME )
    {
        GetEnd()->Draw();
    }
    else if( m_eCurState == STATE_INTRO )
    {
        GetEnd()->Draw();
    }

    GetRender()->PostDraw();

    // SDL_GL_SwapBuffers();
}

void CGame::HandleEvents( int &isActive, int &done )
{
    JInputEvent event;
    JResult retval;

    while( m_pRender->PollEvent( event ) )
    {
        switch( event.type )
        {
        case JInputEvent::KEY:
            retval = m_pCurState->HandleKey( &event.keysym );
            if( retval == JBOGUSKEY )
            {
                JLog( LOG_LEVEL_INFO, true, "Bogus command: 0x%x\n", event.keysym.sym );
                GetMsgs()->Printf( "Unrecognized command: 0x%x\n", event.keysym.sym );
            }
            else if( retval == JQUITREQUEST )
            {
                Quit( 0 );
            }
            break;
        case JInputEvent::QUIT:
            done = true;
            break;
        default:
            break;
        }
    }
}
