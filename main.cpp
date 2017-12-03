#include "JMDefs.h"
#include "Game.h"
#include <time.h>

#include "DisplayText.h"
//#include "Render.h"

#include "player.h"

// The global game pointer
CGame *g_pGame;
JIVector g_vDirDelta[] = {JIVector(0,-1), JIVector(0,1), JIVector(-1,0), JIVector(1,0)};

void Term(void);

int main( int argc, char **argv )
{
    // main loop variable
    int done = false;
    // whether or not the window is active
    int isActive = true;
	
	srand( (unsigned)time(NULL) );
	
	JResult result;
	g_pGame = new CGame;

	result = g_pGame->Init();
	if( result != JSUCCESS )
	{
		printf("Error in game initialization. Terminating.\n");
		exit(1);
	}

	atexit( Term );

	int curTime = 0;
    int lastTick = Util::GetTickCount();
	int nextTime = 0;

	bool bRetVal;
	try
	{
    // Main Game Loop
    while ( !done )
	{
        curTime = Util::GetTickCount();
		if( curTime > nextTime )
		{
			if( g_pGame->GetPlayer() != NULL )
			{
			g_pGame->GetStats()->Printf( "\nplayer pos:<%f %f>\n", VEC_EXPAND(g_pGame->GetPlayer()->m_vPos));
			}
			nextTime = curTime + 2000;
		}

		// handle the events in the queue
		g_pGame->HandleEvents(isActive, done);

		bRetVal = g_pGame->Update( (float)(curTime-lastTick)/1000.0f );
		
		// draw the scene
		//if ( isActive )
		{
			// Draw the dungeon, player, text
			g_pGame->Draw();
		}
		lastTick = curTime;
	}
	}
	catch(...)
	{
		throw;
	}
	
    // clean ourselves up and exit
   Term();
	
    // Should never get here
    return( 0 );
}

void Term()
{
	//g_pGame->GetRender()->Term();
}

// If you want to add profiling 
// instrumentation to the code,
// an example is below.
/*
#ifdef PROFILE
#include "utils/text.h"
#include "utils/profile.h"
#endif // PROFILE

int main( int argc, char **argv )
{
    // main loop variable
    int done = FALSE;
    // whether or not the window is active
    int isActive = TRUE;
	
	srand( time(NULL) );
	
	JResult result;
	g_pGame = new CGame;

	result = g_pGame->Init();
	if( result != JSUCCESS )
	{
		printf("Error in game initialization. Terminating.\n");
		exit(1);
	}

	int curTime = 0;
	int nextTime = 0;
	int count = 0;
#ifdef PROFILE
	ProfileInit();
#endif // PROFILE

    // wait for events
    while ( !done )
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

			// Draw the dungeon, player, text
			g_pGame->Draw();
			
#ifdef PROFILE
			ProfileDraw();
			
			ProfileEnd( "Graphics Draw Routine" );
#endif // PROFILE
			
			// pageflip
			SDL_GL_SwapBuffers( );
		} // if( isactive )
		
#ifdef PROFILE
		ProfileEnd( "Main Loop" );
		ProfileDumpOutputToBuffer();
#endif PROFILE	
	}
	
    // clean ourselves up and exit
    g_pGame->GetRender()->Term();
	
    // Should never get here
    return( 0 );
}
/**/