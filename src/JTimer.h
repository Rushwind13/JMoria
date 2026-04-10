// JTimer.h
//
// Platform-independent timing utilities
// Replaces SDL_GetTicks / SDL_Delay with std::chrono / std::thread

#ifndef __JTIMER_H__
#define __JTIMER_H__

#include <chrono>
#include <thread>

// Frame rate limiting configuration
// #define DISPLAY_FRAMERATE  // Enable FPS counter display
#define LIMIT_FRAMERATE // Lock rendering to 30 FPS
#define TARGET_FPS 30
#define TARGET_FRAME_TIME ( 1000 / TARGET_FPS ) // milliseconds per frame

namespace JTimer
{

inline unsigned int GetTicks()
{
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>( now - startTime )
        .count();
}

inline void Delay( unsigned int ms )
{
    std::this_thread::sleep_for( std::chrono::milliseconds( ms ) );
}

} // namespace JTimer

#endif // __JTIMER_H__
