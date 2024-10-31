#ifndef __STATEBASE_H__
#define __STATEBASE_H__
#ifdef __WIN32__
#include "sdl.h"
#else
#include "SDL2/SDL.h"
#endif // __WIN32__

class CStateBase
{
    // Member Variables
public:
protected:
private:
    // Member Functions
public:
    CStateBase() {}
    ~CStateBase() {}

    int HandleKey( SDL_Keysym *keysym )
    {
        switch( keysym->sym )
        {
        case SDLK_c:
            if( keysym->mod & KMOD_CTRL )
            {
                // ^C was pressed; bye bye.
                return ( JQUITREQUEST );
            }
            break;
        case SDLK_F1:
            // F1 key was pressed
            // this toggles fullscreen mode

            // SDL_WM_ToggleFullScreen( surface );
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            // Just a modifier key, nothing to see here.
            return ( JSUCCESS );
            break;
        default:
            break;
        }

        return OnHandleKey( keysym );
    };
    void Update( float fCurTime ) { OnUpdate( fCurTime ); }

protected:
    virtual int OnHandleKey( SDL_Keysym *keysym ) = 0;
    virtual void OnUpdate( float fCurTime ) = 0;
    virtual void ResetToState( int newstate ) = 0;

    char GetAlpha( SDL_Keysym *keysym )
    {
        char cBase = 'a';
        if( keysym->mod & KMOD_SHIFT )
        {
            cBase = 'A';
        }
        // All the alphabet keys
        // have sequential SDLK_ symbols,
        // so we can handle them with this check
        if( keysym->sym >= SDLK_a && keysym->sym <= SDLK_z )
        {
            return cBase + keysym->sym - SDLK_a;
        }

        return nul;
    }

    char GetNumeric( SDL_Keysym *keysym )
    {
        char cBase = '0';
        // All the alphabet keys
        // have sequential SDLK_ symbols,
        // so we can handle them with this check
        if( keysym->sym >= SDLK_KP_1 && keysym->sym <= SDLK_KP_0 )
        {
            // keypad codes are 1,2,...,9,0
            if( keysym->sym == SDLK_KP_0 )
                return cBase;
            else
                return cBase + keysym->sym + SDLK_KP_1 + 1;
        }
        else if( keysym->sym >= SDLK_0 && keysym->sym <= SDLK_9 )
        {
            return cBase + keysym->sym - SDLK_0;
        }

        return nul;
    }

    char GetAlphaNumeric( SDL_Keysym *keysym )
    {
        if( keysym->sym == SDLK_SPACE )
            return ' ';

        char retval = GetAlpha( keysym );
        if( retval == nul )
        {
            retval = GetNumeric( keysym );
        }

        return retval;
    }

    bool IsDirectional( SDL_Keysym *keysym )
    {
        // All the movement keys
        // (arrows and numberpad keys)
        // have sequential SDLK_ symbols,
        // so we can handle them with this check
        if( ( keysym->sym >= SDLK_KP_1 && keysym->sym <= SDLK_KP_0 ) ||
            ( keysym->sym >= SDLK_RIGHT && keysym->sym <= SDLK_UP ) )
        {
            return true;
        }

        // Use "roguelike" directional keybinds
        if( strchr("hjklyubn", GetAlpha(keysym)) != NULL )
        {
            return true;
        }

        return false;
    }

    void GetDir( SDL_Keysym *keysym, JVector &vDir )
    {
        switch( keysym->sym )
        {
        case SDLK_UP:
        case SDLK_KP_8:
        case SDLK_k:
            // up
            vDir.y = -1;
            break;
        case SDLK_DOWN:
        case SDLK_KP_2:
        case SDLK_j:
            // down
            vDir.y = 1;
            break;
        case SDLK_LEFT:
        case SDLK_KP_4:
        case SDLK_h:
            // left
            vDir.x = -1;
            break;
        case SDLK_RIGHT:
        case SDLK_KP_6:
        case SDLK_l:
            vDir.x = 1;
            // right
            break;
        case SDLK_KP_7:
        case SDLK_y:
            // up + left
            vDir.x = -1;
            vDir.y = -1;
            break;
        case SDLK_KP_9:
        case SDLK_u:
            // up + right
            vDir.x = 1;
            vDir.y = -1;
            break;
        case SDLK_KP_1:
        case SDLK_b:
            // down + left
            vDir.x = -1;
            vDir.y = 1;
            break;
        case SDLK_KP_3:
        case SDLK_n:
            // down + right
            vDir.x = 1;
            vDir.y = 1;
            break;
        case SDLK_KP_5:
            // rest
            break;
        case SDLK_KP_0:
        default:
            // nothing
            break;
        }
    }

private:
};
#endif // __STATEBASE_H__
