// Game.cpp
//
// implementation of the Game Shell

#include "Game.h"
#include "Dungeon.h"
#include "Player.h"
#include "Tileset.h"

#include "CmdState.h"
#include "ModState.h"

#include "render.h"
#include "DisplayText.h"

#include "AIMgr.h"


#ifdef PROFILE
#include "utils/profile.h"
#endif

CGame::CGame():
m_pDungeon(NULL),
m_pPlayer(NULL),
m_pRender(NULL),
m_pCurState(NULL),
m_pCmdState(NULL),
m_eCurState(STATE_INVALID)
{
	m_pCmdState = new CCmdState;
	m_pModState = new CModState;
};

JResult CGame::Init()
{
	JResult result;
	// Initialize all the game stuff, baby.

	g_Constants.Init();
	
	// Init the Render
	//*
	m_pRender = new CRender;
	result = m_pRender->Init( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );
	if( result != JSUCCESS )
	{
		m_pRender->Term();
		return result;
	}/**/

	m_pMsgsDT = new CDisplayText( JRect( 0, 0, 640, 36 ) );
	m_pMsgsDT->SetFlags(FLAG_TEXT_WRAP_WHITESPACE);

	m_pStatsDT = new CDisplayText( JRect( 0,50,  150,480 ) );
	m_pStatsDT->SetFlags(FLAG_TEXT_WRAP_WHITESPACE|FLAG_TEXT_BOUNDING_BOX);


	m_pAIMgr = new CAIMgr;
	m_pAIMgr->Init();

	// Init the Dungeon
	m_pDungeon = new CDungeon;
	m_pDungeon->Init();

	// Init the Player
	m_pPlayer = new CPlayer;
	m_pPlayer->Init();

#ifdef PROFILE
	ProfileInit();
#endif // PROFILE

	// Set up the initial game state
	SetState(STATE_COMMAND);
	
	// we're up.
	return JSUCCESS;
}

void CGame::Quit( int returncode )
{
	//*
	if( m_pRender )
	{
		m_pRender->Term();
		delete m_pRender;
		m_pRender = NULL;
	}/**/
	
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
	exit( returncode );
}

void CGame::SetState( int eNewState )
{
	m_eCurState = eNewState;
	switch( m_eCurState )
	{
	case STATE_COMMAND:
		m_pCurState = reinterpret_cast<CStateBase *>(m_pCmdState);
		break;
	case STATE_MODIFY:
		m_pCurState = reinterpret_cast<CStateBase *>(m_pModState);
		break;
	default:
		break;
	}
}

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
	// Update the AI
	GetAIMgr()->Update(fCurTime);

	// Update the player
	GetPlayer()->Update(fCurTime);
	
	// Update the dungeon
	GetDungeon()->Update(fCurTime);
	GetMsgs()->Update(fCurTime);
	GetStats()->Update(fCurTime);

	return true;
}

void CGame::Draw()
{	
	GetRender()->PreDraw();
	// Draw the player
	GetPlayer()->Draw();
	
	// Draw the dungeon
	GetDungeon()->Draw();
	GetMsgs()->Draw();
	GetStats()->Draw();
	
	GetRender()->PostDraw();

	//SDL_GL_SwapBuffers();
}

void CGame::HandleEvents(int &isActive, int &done)
{
    // used to collect events
    SDL_Event event;
	JResult retval;

	while ( SDL_PollEvent( &event ) )
	{
		switch( event.type )
        {
        case SDL_WINDOWEVENT:
            switch (event.window.event)
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
                    Quit(retval);
                }
                break;
            }
            break;
		case SDL_KEYDOWN:
			// handle key presses
			retval = m_pCurState->HandleKey( &event.key.keysym );
			if( retval == JBOGUSKEY )
			{
				printf("Bogus command: 0x%x\n", event.key.keysym.sym);
				GetMsgs()->Printf("Unrecognized command: 0x%x\n", event.key.keysym.sym);
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
				g_pGame->GetDungeon()->Zoom(3);
				break;
			case MOUSE_WHEEL_DOWN:
				g_pGame->GetDungeon()->Zoom(-3);
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
			//printf("unhandled event type: %d\n", event.type );
			break;
		}
	}
}