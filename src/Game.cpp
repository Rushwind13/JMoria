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
#include "SDL2/SDL.h"
#endif
#ifdef RENDER_ASCII
#include "RenderASCII.h"
#endif

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
      m_eRenderMode( RenderMode::OpenGL ),
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
    JResult result;
    // Initialize all the game stuff, baby.

    g_Constants.Init();

    m_eRenderMode = mode;

    // Init the Render
#if defined( RENDER_ASCII ) && defined( RENDER_OPENGL )
    if( m_eRenderMode == RenderMode::ASCII )
    {
        m_pRender = new CRenderASCII;
        result = m_pRender->Init( 80, 24, 0 );
    }
    else
    {
        m_pRender = new CRender;
        result = m_pRender->Init( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );
    }
#elif defined( RENDER_ASCII )
    m_pRender = new CRenderASCII;
    result = m_pRender->Init( 80, 24, 0 );
#elif defined( RENDER_OPENGL )
    m_pRender = new CRender;
    result = m_pRender->Init( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );
#endif
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

    if( m_eRenderMode == RenderMode::ASCII )
    {
#ifdef RENDER_ASCII
        UpdateASCIILayout();
        m_bShowInv = ( m_pRender->GetScreenWidth() >= ASCIILayout::INV_AUTO_WIDTH );
#endif
    }

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
        case USE_READ:
        case USE_QUAFF:
            GetPlayer()->DisplayInventory( PLACEMENT_USE );
            break;
        case USE_REMOVE:
            GetPlayer()->DisplayEquipment( PLACEMENT_USE );
            break;
        default:
            JLog( LOG_LEVEL_WARN, true, "Nothing to display for command\n" );
            break;
        }
        GetUse()->Update( fCurTime );
    }
    else if( m_eCurState == STATE_RANGED )
    {
        switch( reinterpret_cast<CRangedState *>( m_pCurState )->GetCommand() )
        {
        case JKEY_f:
            GetPlayer()->DisplayEquipment( PLACEMENT_USE );
            break;
        case JKEY_z:
            GetPlayer()->DisplayInventory( PLACEMENT_USE );
            break;
        default:
            JLog( LOG_LEVEL_WARN, true, "Nothing to display for command\n" );
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

// Convert ASCIILayout regions (char coords) to pixel-space JRects
// that DisplayText expects (x*6, y*8).
#ifdef RENDER_ASCII
void CGame::UpdateASCIILayout()
{
    CRenderASCII *pASCII = static_cast<CRenderASCII *>( m_pRender );
    const ASCIILayout &l = pASCII->GetLayout();

    auto toPixelRect = []( const ASCIILayoutRegion &r ) {
        return JRect( r.left * 6, r.top * 8, r.right * 6, r.bottom * 8 );
    };

    m_pMsgsDT->SetRect( toPixelRect( l.messages ) );
    m_pStatsDT->SetRect( toPixelRect( l.stats ) );
    m_pInvDT->SetRect( toPixelRect( l.inventory ) );
    m_pEquipDT->SetRect( toPixelRect( l.equipment ) );
    m_pUseDT->SetRect( toPixelRect( l.use ) );
    m_pEndGameDT->SetRect( toPixelRect( l.endgame ) );
    m_pEndGameDT->SetContentMargin( 0, 0 );
}
#endif // RENDER_ASCII

void CGame::Draw()
{
    bool bResized = GetRender()->CheckResize();
    GetRender()->PreDraw();

    bool bASCII = ( m_eRenderMode == RenderMode::ASCII );

    // After resize, update DisplayText rects and auto-show/hide inventory
#ifdef RENDER_ASCII
    if( bASCII && bResized )
    {
        UpdateASCIILayout();
        m_bShowInv = ( GetRender()->GetScreenWidth() >= ASCIILayout::INV_AUTO_WIDTH );
    }
#endif

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

        // In ASCII mode, stats/inv/equip are fly-out panels toggled by c/i/e
        if( !bASCII )
        {
            GetStats()->Draw();
            GetInv()->Draw();
            GetEquip()->Draw();
        }
        else
        {
            if( m_bShowStats ) GetStats()->Draw();
            if( m_bShowInv ) GetInv()->Draw();
            if( m_bShowEquip ) GetEquip()->Draw();
        }
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
#ifdef RENDER_ASCII
    if( m_eRenderMode == RenderMode::ASCII )
    {
        HandleEventsASCII( isActive, done );
        return;
    }
#endif

#ifdef RENDER_OPENGL
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
                retval = static_cast<CRender*>(GetRender())->ResizeWindow( event.window.data1, event.window.data2 );
                if( retval != JSUCCESS )
                {
                    Quit( retval );
                }
                break;
            }
            break;
        case SDL_KEYDOWN:
        {
            // Translate SDL keysym to JMoria keysym at the boundary
            JKeysym jkey;
            jkey.sym = (JKeycode)event.key.keysym.sym;
            jkey.mod = (JKeymod)event.key.keysym.mod;
            retval = m_pCurState->HandleKey( &jkey );
            if( retval == JBOGUSKEY )
            {
                JLog( LOG_LEVEL_ERROR, true, "Bogus command: 0x%x\n", jkey.sym );
                GetMsgs()->Printf( "Unrecognized command: 0x%x\n", jkey.sym );
            }
            else if( retval == JQUITREQUEST )
            {
                Quit( 0 );
            }
            break;
        }
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
            /*JLog( LOG_LEVEL_NOISE, true, "Got up event type: %d which: %d button: %d state: %d at
            <%d %d>\n", event.button.type, event.button.which, event.button.button,
            event.button.state, event.button.x, event.button.y );/* */
            break;
        default:
            JLog( LOG_LEVEL_NOISE, true, "unhandled event type: %d\n", event.type );
            break;
        }
    }
#endif // RENDER_OPENGL
}

#ifdef RENDER_ASCII
void CGame::HandleEventsASCII( int &isActive, int &done )
{
    int ch = getch();
    if( ch == ERR )
        return; // no input available

    // Terminal resize: consume the event, PreDraw handles the actual resize
    if( ch == KEY_RESIZE )
        return;

    JKeysym keysym;
    memset( &keysym, 0, sizeof( keysym ) );

    // Map ncurses keys to SDL keysyms
    // For ASCII printable characters, SDLK values match ASCII codes
    if( ch >= 'a' && ch <= 'z' )
    {
        keysym.sym = (JKeycode)ch;
        keysym.mod = JMOD_NONE;
    }
    else if( ch >= 'A' && ch <= 'Z' )
    {
        // Uppercase: map to lowercase sym + shift modifier
        keysym.sym = (JKeycode)( ch - 'A' + 'a' );
        keysym.mod = JMOD_SHIFT;
    }
    else if( ch >= 1 && ch <= 26 )
    {
        // Ctrl+letter: ch 1 = Ctrl+A, ch 3 = Ctrl+C, etc.
        keysym.sym = (JKeycode)( 'a' + ch - 1 );
        keysym.mod = JMOD_CTRL;
    }
    else if( ch >= '0' && ch <= '9' )
    {
        keysym.sym = (JKeycode)ch;
        keysym.mod = JMOD_NONE;
    }
    else
    {
        // Map special keys
        switch( ch )
        {
        case '\n':
        case '\r':
        case KEY_ENTER:
            keysym.sym = JKEY_RETURN;
            break;
        case 27: // Escape
            keysym.sym = JKEY_ESCAPE;
            break;
        case ' ':
            keysym.sym = JKEY_SPACE;
            break;
        case '.':
            keysym.sym = JKEY_PERIOD;
            break;
        case '>': // Shift+.
            keysym.sym = JKEY_PERIOD;
            keysym.mod = JMOD_SHIFT;
            break;
        case ',':
            keysym.sym = JKEY_COMMA;
            break;
        case '<': // Shift+,
            keysym.sym = JKEY_COMMA;
            keysym.mod = JMOD_SHIFT;
            break;
        case ';':
            keysym.sym = JKEY_SEMICOLON;
            break;
        case KEY_BACKSPACE:
        case 127: // DEL on some terminals
            keysym.sym = JKEY_BACKSPACE;
            break;
        case KEY_DC: // ncurses Delete key
            keysym.sym = JKEY_DELETE;
            break;
        case KEY_F(1):
            keysym.sym = JKEY_F1;
            break;
        default:
            // Unknown key, ignore
            return;
        }
    }

    // ASCII fly-out panel toggles: i=inventory, e=equipment, C=character stats
    // These are display-only and don't consume a game turn.
    if( m_eCurState == STATE_COMMAND )
    {
        if( keysym.sym == JKEY_i && keysym.mod == JMOD_NONE )
        {
            ToggleInv();
            return;
        }
        if( keysym.sym == JKEY_e && keysym.mod == JMOD_NONE )
        {
            ToggleEquip();
            return;
        }
        if( keysym.sym == JKEY_c && ( keysym.mod & JMOD_SHIFT ) )
        {
            ToggleStats();
            return;
        }
    }

    JResult retval = m_pCurState->HandleKey( &keysym );
    if( retval == JBOGUSKEY )
    {
        JLog( LOG_LEVEL_ERROR, true, "Bogus command: 0x%x\n", keysym.sym );
        GetMsgs()->Printf( "Unrecognized command: 0x%x\n", keysym.sym );
    }
    else if( retval == JQUITREQUEST )
    {
        Quit( 0 );
    }
}
#endif // RENDER_ASCII
