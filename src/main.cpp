#include "JMDefs.h"
#include "Game.h"
#include <time.h>

#include "DisplayText.h"
// #include "Render.h"

#include "Player.h"

// The global game pointer
CGame *g_pGame = NULL;
JIVector g_vDirDelta[] = {JIVector(0, -1), JIVector(0, 1), JIVector(-1, 0), JIVector(1, 0)};

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

	unsigned int curTime = 0;
    unsigned int lastTick = Util::GetTickCount();
	unsigned int nextTime = 0;

	bool bRetVal;
	try
	 {
    // Main Game Loop
    while ( !done )
	 {
        curTime = Util::GetTickCount();
		if( curTime > nextTime )
		 {
// 			if( g_pGame->GetPlayer() != NULL )
//			{
// 			g_pGame->GetStats()->Printf( "\nplayer pos:<%f %f>\n", VEC_EXPAND(g_pGame->GetPlayer()->m_vPos));
//			}
#ifdef CLOCKSTEP
            g_pGame->SetState(STATE_CLOCKSTEP);
#endif
			nextTime = curTime + 10;
		}

		// handle the events in the queue
		g_pGame->HandleEvents(isActive, done);

		bRetVal = g_pGame->Update( (float)(curTime-lastTick)/1000.0f );

		// draw the scene
		// if ( isActive )
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
	// g_pGame->GetRender()->Term();
}
