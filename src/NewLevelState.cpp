#include "NewLevelState.h"

#include "JMDefs.h"

int CNewLevelState::OnHandleKey(SDL_Keysym *keysym)
{
	int retval = JSUCCESS;

	if( keysym->sym == SDLK_SPACE )
	{
		// run one step of dungeon creation
		if( retval == JRESETSTATE )
		{
			// we're done here.
			ResetToState(STATE_COMMAND);
		}
	}
	return retval;
}