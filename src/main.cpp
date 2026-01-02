#include "JMDefs.h"
#include <time.h>
#include "SDL2/SDL.h"

// The global game pointer
CGame *g_pGame = NULL;
eLogLevel g_eLogLevel = LOG_LEVEL_INFO;

JIVector g_vDirDelta[] = { JIVector( 0, -1 ), JIVector( 0, 1 ), JIVector( -1, 0 ),
                           JIVector( 1, 0 ) };

void Term( void );

int main( int argc, char **argv )
{
    // main loop variable
    int done = false;
    // whether or not the window is active
    int isActive = true;

    srand( (unsigned)time( NULL ) );

    JResult result;
    g_pGame = new CGame;

    result = g_pGame->Init( "../JMoria/" );
    if( result != JSUCCESS )
    {
        JLog( LOG_LEVEL_ERROR, true, "Error in game initialization. Terminating.\n" );
        exit( 1 );
    }

    atexit( Term );

    unsigned int curTime = 0;
#ifndef TURN_BASED
    unsigned int lastTick = Util::GetTickCount();
#endif // TURN_BASED
    unsigned int nextTime = 0;

    bool bRetVal;
    try
    {
        // Main Game Loop
        while( !done )
#ifdef TURN_BASED
        {
            // handle the events in the queue
            g_pGame->HandleEvents( isActive, done );
            bRetVal = g_pGame->Update();
            // Draw the dungeon, player, text
            g_pGame->Draw();

            /**
             * Frame rate limiting for turn-based mode:
             * Without this delay, the main loop runs as fast as possible,
             * consuming 100% CPU while waiting for player input. Since this
             * is a turn-based game, we don't need thousands of frames per
             * second - 60 FPS (16ms per frame) is more than sufficient for
             * responsive input handling while keeping CPU usage reasonable.
             */
            SDL_Delay( 16 );
        }
#else
        {
            curTime = Util::GetTickCount();
            if( curTime > nextTime )
            {
// 			if( g_pGame->GetPlayer() != NULL )
//			{
// 			g_pGame->GetStats()->Printf( "\nplayer pos:<%f %f>\n",
// VEC_EXPAND(g_pGame->GetPlayer()->m_vPos));
//			}
#ifdef CLOCKSTEP
                g_pGame->SetState( STATE_CLOCKSTEP );
#endif
                nextTime = curTime + 10;
            }

            // handle the events in the queue
            g_pGame->HandleEvents( isActive, done );

            bRetVal = g_pGame->Update( (float)( curTime - lastTick ) / 1000.0f );

            // draw the scene
            // if ( isActive )
            {
                // Draw the dungeon, player, text
                g_pGame->Draw();
            }
            lastTick = curTime;
        }
#endif // TURN_BASED
    }
    catch( ... )
    {
        throw;
    }

    // clean ourselves up and exit
    Term();

    // Should never get here
    return ( 0 );
}

void Term()
{
    // g_pGame->GetRender()->Term();
}
