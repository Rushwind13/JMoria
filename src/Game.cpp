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
#include "ModState.h"
#include "RestState.h"
#include "StringInputState.h"
#include "UseState.h"

#include "DisplayText.h"
#include "Render.h"

#include "AIMgr.h"

#ifdef PROFILE
#include "utils/profile.h"
#endif

CGame::CGame()
    : m_pDungeon( NULL ),
      m_pPlayer( NULL ),
      m_pRender( NULL ),
      m_pCurState( NULL ),
      m_pCmdState( NULL ),
      m_pStringInputState( NULL ),
      m_pEndGameState( NULL ),
      m_pClockStepState( NULL ),
      m_pRestState( NULL ),
      m_eCurState( STATE_INVALID ),
      m_fGameTime( 0.0f )
{
    m_pCmdState = new CCmdState;
    m_pModState = new CModState;
    m_pUseState = new CUseState;
    m_pStringInputState = new CStringInputState;
    m_pEndGameState = new CEndGameState;
    m_pClockStepState = new CClockStepState;
    m_pRestState = new CRestState;
#ifdef TURN_BASED
    m_bReadyForUpdate = false;
#endif // TURN_BASED
};

JResult CGame::Init(const char *szBasedir)
{
    JResult result;
    // Initialize all the game stuff, baby.

    g_Constants.Init();

    // Init the Render
    m_pRender = new CRender;
    result = m_pRender->Init( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );
    if( result != JSUCCESS )
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

    m_pAIMgr = new CAIMgr;
    m_pAIMgr->Init();

    // Init the Dungeon
    m_pDungeon = new CDungeon;
    m_pDungeon->Init(szBasedir);

    // Init the Player
    m_pPlayer = new CPlayer;
    m_pPlayer->Init(szBasedir);

#ifdef PROFILE
    ProfileInit();
#endif // PROFILE

#ifdef CLOCKSTEP
    // Now, in exciting slow motion!
    SetState( STATE_CLOCKSTEP );
#else
    // Set up the initial game state
    SetState( STATE_COMMAND );
#endif // CLOCKSTEP
       // we're up.

    return JSUCCESS;
}

void CGame::Quit( int returncode )
{
    if( m_pRender )
    {
        m_pRender->Term();
        delete m_pRender;
        m_pRender = NULL;
    }

    if( m_pDungeon )
    {
        m_pDungeon->Term();
        delete m_pDungeon;
        m_pDungeon = NULL;
    }

    if( m_pPlayer )
    {
        m_pPlayer->Term();
        delete m_pPlayer;
        m_pPlayer = NULL;
    }

    if( m_pCmdState )
    {
        delete m_pCmdState;
        m_pCmdState = NULL;
    }

    if( m_pModState )
    {
        delete m_pModState;
        m_pModState = NULL;
    }

    if( m_pUseState )
    {
        delete m_pUseState;
        m_pUseState = NULL;
    }

    if( m_pStringInputState )
    {
        delete m_pStringInputState;
        m_pStringInputState = NULL;
    }

    if( m_pEndGameState )
    {
        delete m_pEndGameState;
        m_pEndGameState = NULL;
    }

    if( m_pClockStepState )
    {
        delete m_pClockStepState;
        m_pClockStepState = NULL;
    }

    if( m_pRestState )
    {
        delete m_pRestState;
        m_pRestState = NULL;
    }

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
    case STATE_MODIFY:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pModState );
        break;
    case STATE_USE:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pUseState );
        break;
    case STATE_STRINGINPUT:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pStringInputState );
        break;
    case STATE_ENDGAME:
    {
        m_pCurState = reinterpret_cast<CStateBase *>( m_pEndGameState );
        SDL_Keysym *keysym = new SDL_Keysym();
        keysym->sym = SDLK_SPACE;
        m_pCurState->HandleKey( keysym );
    }
    break;
    case STATE_CLOCKSTEP:
    {
        m_pCurState = reinterpret_cast<CStateBase *>( m_pClockStepState );
        SDL_Keysym *keysym = new SDL_Keysym();
        keysym->sym = SDLK_SPACE;
        m_pCurState->HandleKey( keysym );
    }
    break;
    case STATE_REST:
        m_pCurState = reinterpret_cast<CStateBase *>( m_pRestState );
        break;
    default:
        printf( "Tried to change to unknown state.\n" );
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

bool CGame::Update( float fCurTime )
{
#ifdef TURN_BASED
    if( m_bReadyForUpdate )
    {
        m_fGameTime++;
        fCurTime = 1.0f;
        m_bReadyForUpdate = false;
        // TODO: Why does the AI require 2 ticks to move the monster?
        GetAIMgr()->Update( fCurTime );
        GetAIMgr()->Update( fCurTime );
    }
//    else
//    {
//        return false;
//    }
#else
    m_fGameTime += fCurTime;
    // Update the AI
    GetAIMgr()->Update( fCurTime );
#endif // TURN_BASED

#ifdef CLOCKSTEP
    GetDungeon()->Update( fCurTime );
    GetStats()->Update( fCurTime );
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
        switch( reinterpret_cast<CUseState *>( m_pCurState )->GetModifier() )
        {
        case USE_WIELD:
        case USE_DROP:
        case USE_QUAFF:
            GetPlayer()->DisplayInventory( PLACEMENT_USE );
            break;
        case USE_REMOVE:
            GetPlayer()->DisplayEquipment( PLACEMENT_USE );
            break;
        default:
            printf( "Nothing to display for command\n" );
            break;
        }
        GetUse()->Update( fCurTime );
    }
    else if( m_eCurState == STATE_ENDGAME )
    {
        GetEnd()->Update( fCurTime );
    }
    m_pCurState->Update( fCurTime );
#endif // CLOCKSTEP
    return true;
}

void CGame::Draw()
{
    GetRender()->PreDraw();

    // Draw the dungeon
    GetDungeon()->Draw();

    // Draw the player
    GetPlayer()->Draw();

    GetMsgs()->Draw();
    GetStats()->Draw();
    GetInv()->Draw();
    GetEquip()->Draw();
    if( m_eCurState == STATE_USE )
    {
        GetUse()->Draw();
    }
    else if( m_eCurState == STATE_ENDGAME )
    {
        GetEnd()->Draw();
    }

    GetRender()->PostDraw();

    // SDL_GL_SwapBuffers();
}

void CGame::HandleEvents( int &isActive, int &done )
{
    // used to collect events
    SDL_Event event;
    JResult retval;

    while( SDL_PollEvent( &event ) )
    {
        switch( event.type )
        {
        case SDL_WINDOWEVENT:
            switch( event.window.event )
            {
            // Something's happend with our focus
            // If we lost focus or we are iconified, we
            // shouldn't draw the screen
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                isActive = true;
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                isActive = false;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                // used to be SDL_VIDEORESIZE:
                retval = GetRender()->ResizeWindow( event.window.data1, event.window.data2 );
                if( retval != JSUCCESS )
                {
                    Quit( retval );
                }
                break;
            }
            break;
        case SDL_KEYDOWN:
            // handle key presses
            retval = m_pCurState->HandleKey( &event.key.keysym );
            if( retval == JBOGUSKEY )
            {
                printf( "Bogus command: 0x%x\n", event.key.keysym.sym );
                GetMsgs()->Printf( "Unrecognized command: 0x%x\n", event.key.keysym.sym );
            }
            else if( retval == JQUITREQUEST )
            {
                Quit( 0 );
            }
            break;
        case SDL_QUIT:
            // handle quit requests
            done = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if( event.button.state == SDL_RELEASED )
            {
                // Nothing to see here.
                break;
            }
            switch( event.button.button )
            {
            case MOUSE_WHEEL_UP:
                g_pGame->GetDungeon()->Zoom( 3 );
                break;
            case MOUSE_WHEEL_DOWN:
                g_pGame->GetDungeon()->Zoom( -3 );
                break;
            }
            /*printf("Got up event type: %d which: %d button: %d state: %d at <%d %d>\n",
            event.button.type,
            event.button.which,
            event.button.button,
            event.button.state,
            event.button.x,
            event.button.y );/* */
            break;
        default:
            // printf("unhandled event type: %d\n", event.type );
            break;
        }
    }
}
