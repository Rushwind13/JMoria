#include "JMDefs.h"
#include <time.h>
#include "SDL2/SDL.h"
#include "AILog.h"

// Frame rate limiting configuration
// #define DISPLAY_FRAMERATE  // Enable FPS counter display
#define LIMIT_FRAMERATE       // Lock rendering to 30 FPS
#define TARGET_FPS 30
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)  // milliseconds per frame

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

    // Initialize AI logging for game observability
    JLog_InitAI( "../JMoria/" );

    atexit( Term );

    unsigned int curTime = 0;
#ifndef TURN_BASED
    unsigned int lastTick = Util::GetTickCount();
#endif // TURN_BASED
    unsigned int nextTime = 0;
#ifdef LIMIT_FRAMERATE
    unsigned int frameStartTime = 0;
    unsigned int frameElapsedTime = 0;
#endif // LIMIT_FRAMERATE

    bool bRetVal;
    try
    {
        // Main Game Loop
        while( !done )
#ifdef TURN_BASED
        {
#ifdef LIMIT_FRAMERATE
            frameStartTime = SDL_GetTicks();
#endif
            // handle the events in the queue
            g_pGame->HandleEvents( isActive, done );
            bRetVal = g_pGame->Update();
            // Draw the dungeon, player, text
            g_pGame->Draw();

#ifdef LIMIT_FRAMERATE
            /**
             * Frame rate limiting for turn-based mode:
             * Without this delay, the main loop runs as fast as possible,
             * consuming 100% CPU while waiting for player input. Since this
             * is a turn-based game, we don't need thousands of frames per
             * second - 30 FPS is more than sufficient for responsive input
             * handling while keeping CPU usage reasonable.
             */
            frameElapsedTime = SDL_GetTicks() - frameStartTime;
            if( frameElapsedTime < TARGET_FRAME_TIME )
            {
                SDL_Delay( TARGET_FRAME_TIME - frameElapsedTime );
            }
#endif // LIMIT_FRAMERATE
        }
#else
        {
#ifdef LIMIT_FRAMERATE
            frameStartTime = SDL_GetTicks();
#endif
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
#ifdef LIMIT_FRAMERATE
            /**
             * Frame rate limiting for real-time mode:
             * Cap rendering at 30 FPS to reduce CPU usage. Note: This affects
             * the deltaTime passed to Update(), which may impact game speed if
             * physics/movement calculations rely on consistent timing.
             */
            frameElapsedTime = SDL_GetTicks() - frameStartTime;
            if( frameElapsedTime < TARGET_FRAME_TIME )
            {
                SDL_Delay( TARGET_FRAME_TIME - frameElapsedTime );
            }
#endif // LIMIT_FRAMERATE
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
    // Terminate AI logging
    JLog_TermAI();
    // g_pGame->GetRender()->Term();
}
