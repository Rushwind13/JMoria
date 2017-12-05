#ifndef __STATEBASE_H__
#define __STATEBASE_H__
#ifdef __WIN32__
#include "sdl.h"
#else
#include "SDL2/SDL.h"
#endif //__WIN32__

class CStateBase
{
	// Member Variables
public:
protected:
private:
	
	// Member Functions
public:
	CStateBase() {};
	~CStateBase() {};
	
	int HandleKey( SDL_Keysym *keysym )
	{
		switch ( keysym->sym )
		{
		case SDLK_c:
			if( keysym->mod & KMOD_CTRL )
			{
				// ^C was pressed; bye bye.
				return( JQUITREQUEST );
			}
			break;
		case SDLK_F1:
			// F1 key was pressed
			// this toggles fullscreen mode
			
			//SDL_WM_ToggleFullScreen( surface );
			break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            // Just a modifier key, nothing to see here.
            return( JSUCCESS );
            break;
		default:
			break;
		}

		return OnHandleKey(keysym);
	};
	void Update();
	
protected:
	virtual int OnHandleKey( SDL_Keysym *keysym ) =0;
	virtual void OnUpdate()=0;
    virtual void ResetToState( int newstate ) =0;
	
private:
};
#endif //__STATEBASE_H__