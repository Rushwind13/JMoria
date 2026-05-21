#ifndef __STATEBASE_H__
#define __STATEBASE_H__
#include "JKeys.h"

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

    int HandleKey( JKeysym *keysym )
    {
        switch( keysym->sym )
        {
        case JKEY_c:
            if( keysym->mod & JMOD_CTRL )
            {
                // ^C was pressed; bye bye.
                return ( JQUITREQUEST );
            }
            break;
        case JKEY_F1:
            // F1 key was pressed
            // this toggles fullscreen mode

            // SDL_WM_ToggleFullScreen( surface );
            break;
        case JKEY_LSHIFT:
        case JKEY_RSHIFT:
        case JKEY_LCTRL:
        case JKEY_RCTRL:
            // Just a modifier key, nothing to see here.
            return ( JSUCCESS );
            break;
        default:
            break;
        }

        return OnHandleKey( keysym );
    };
    void Update( float fCurTime ) { OnUpdate( fCurTime ); }
    virtual void Begin( int previousState ) {}

protected:
    virtual int OnHandleKey( JKeysym *keysym ) = 0;
    virtual void OnUpdate( float fCurTime ) = 0;
    virtual void ResetToState( int newstate ) = 0;

    char GetAlpha( JKeysym *keysym )
    {
        char cBase = 'a';
        if( keysym->mod & JMOD_SHIFT )
        {
            cBase = 'A';
        }
        // All the alphabet keys
        // have sequential JKEY_ symbols,
        // so we can handle them with this check
        if( keysym->sym >= JKEY_a && keysym->sym <= JKEY_z )
        {
            return cBase + keysym->sym - JKEY_a;
        }

        return nul;
    }

    char GetNumeric( JKeysym *keysym )
    {
        char cBase = '0';
        // All the alphabet keys
        // have sequential JKEY_ symbols,
        // so we can handle them with this check
        if( keysym->sym >= JKEY_KP_1 && keysym->sym <= JKEY_KP_0 )
        {
            // keypad codes are 1,2,...,9,0
            if( keysym->sym == JKEY_KP_0 )
                return cBase;
            else
                return cBase + keysym->sym + JKEY_KP_1 + 1;
        }
        else if( keysym->sym >= JKEY_0 && keysym->sym <= JKEY_9 )
        {
            return cBase + keysym->sym - JKEY_0;
        }

        return nul;
    }

    char GetAlphaNumeric( JKeysym *keysym )
    {
        if( keysym->sym == JKEY_SPACE )
            return ' ';

        if( keysym->sym == JKEY_MINUS && keysym->mod & JMOD_SHIFT )
            return '_';

        char retval = GetAlpha( keysym );
        if( retval == nul )
        {
            retval = GetNumeric( keysym );
        }

        return retval;
    }

    bool IsDirectional( JKeysym *keysym )
    {
        // All the movement keys
        // (arrows and numberpad keys)
        // have sequential JKEY_ symbols,
        // so we can handle them with this check
        if( ( keysym->sym >= JKEY_KP_1 && keysym->sym <= JKEY_KP_0 ) ||
            ( keysym->sym >= JKEY_RIGHT && keysym->sym <= JKEY_UP ) )
        {
            return true;
        }

        // Plain digit keys 1-9 as directional (ASCII renderer has no numpad)
        // Exclude shifted digits (e.g. Shift+8 = '*' is the target command)
        if( keysym->sym >= JKEY_1 && keysym->sym <= JKEY_9 && !( keysym->mod & JMOD_SHIFT ) )
        {
            return true;
        }

        // Use "roguelike" directional keybinds
        // Ctrl+letter is never directional (e.g. ^n = name character, not SE move)
        if( keysym->mod & JMOD_CTRL )
            return false;
        char sym = GetAlpha( keysym );
        if( sym == NULL )
            return false;
        const char *c = strchr( "HJKLYUBNhjklyubn", sym );
        if( c != NULL )
        {
            return true;
        }

        return false;
    }

    void GetDir( JKeysym *keysym, JVector &vDir )
    {
        switch( keysym->sym )
        {
        case JKEY_UP:
        case JKEY_KP_8:
        case JKEY_8:
        case JKEY_k:
            // up
            vDir.y = -1;
            break;
        case JKEY_DOWN:
        case JKEY_KP_2:
        case JKEY_2:
        case JKEY_j:
            // down
            vDir.y = 1;
            break;
        case JKEY_LEFT:
        case JKEY_KP_4:
        case JKEY_4:
        case JKEY_h:
            // left
            vDir.x = -1;
            break;
        case JKEY_RIGHT:
        case JKEY_KP_6:
        case JKEY_6:
        case JKEY_l:
            vDir.x = 1;
            // right
            break;
        case JKEY_KP_7:
        case JKEY_7:
        case JKEY_y:
            // up + left
            vDir.x = -1;
            vDir.y = -1;
            break;
        case JKEY_KP_9:
        case JKEY_9:
        case JKEY_u:
            // up + right
            vDir.x = 1;
            vDir.y = -1;
            break;
        case JKEY_KP_1:
        case JKEY_1:
        case JKEY_b:
            // down + left
            vDir.x = -1;
            vDir.y = 1;
            break;
        case JKEY_KP_3:
        case JKEY_3:
        case JKEY_n:
            // down + right
            vDir.x = 1;
            vDir.y = 1;
            break;
        case JKEY_KP_5:
        case JKEY_5:
            // rest
            break;
        case JKEY_KP_0:
        default:
            // nothing
            break;
        }
    }

private:
};
#endif // __STATEBASE_H__
