#include "JMDefs.h"
#include "JTimer.h"
#include "RenderMode.h"
#include <cstring>
#include <cstdlib>
#include <ctime>

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

    Util::SeedRandom( (unsigned)time( NULL ) );

    // Parse command-line arguments
    RenderMode renderMode = RenderMode::None;
    for( int i = 1; i < argc; i++ )
    {
        if( strcmp( argv[i], "--renderer=ascii" ) == 0 )
            renderMode = RenderMode::ASCII;
        else if( strcmp( argv[i], "--renderer=opengl" ) == 0 )
            renderMode = RenderMode::OpenGL;
    }

    // Apply defaults / validate based on compiled renderer support
#if defined( RENDER_ASCII ) && defined( RENDER_OPENGL )
    if( renderMode == RenderMode::None )
    {
        printf( "Usage: %s --renderer=ascii|opengl\n", argv[0] );
        exit( 1 );
    }
#elif defined( RENDER_ASCII )
    if( renderMode == RenderMode::OpenGL )
        printf( "Warning: OpenGL renderer not compiled in, using ASCII.\n" );
    renderMode = RenderMode::ASCII;
#elif defined( RENDER_OPENGL )
    if( renderMode == RenderMode::ASCII )
        printf( "Warning: ASCII renderer not compiled in, using OpenGL.\n" );
    renderMode = RenderMode::OpenGL;
#endif

    JResult result;
    g_pGame = new CGame;

    result = g_pGame->Init( "../JMoria/", renderMode );
    if( result != JSUCCESS )
    {
        JLog( LOG_LEVEL_ERROR, true, "Error in game initialization. Terminating.\n" );
        exit( 1 );
    }

    atexit( Term );

    unsigned int curTime = 0;
#ifndef TURN_BASED
    unsigned int lastTick = JTimer::GetTicks();
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
            frameStartTime = JTimer::GetTicks();
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
            frameElapsedTime = JTimer::GetTicks() - frameStartTime;
            if( frameElapsedTime < TARGET_FRAME_TIME )
            {
                JTimer::Delay( TARGET_FRAME_TIME - frameElapsedTime );
            }
#endif // LIMIT_FRAMERATE
        }
#else
        {
#ifdef LIMIT_FRAMERATE
            frameStartTime = JTimer::GetTicks();
#endif
            curTime = JTimer::GetTicks();
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
            frameElapsedTime = JTimer::GetTicks() - frameStartTime;
            if( frameElapsedTime < TARGET_FRAME_TIME )
            {
                JTimer::Delay( TARGET_FRAME_TIME - frameElapsedTime );
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
    // g_pGame->GetRender()->Term();
}
