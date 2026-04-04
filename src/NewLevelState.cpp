#include "NewLevelState.h"

#include "JMDefs.h"

int CNewLevelState::OnHandleKey( JKeysym *keysym )
{
    int retval = JSUCCESS;

    if( keysym->sym == JKEY_SPACE )
    {
        // run one step of dungeon creation
        if( retval == JRESETSTATE )
        {
            // we're done here.
            ResetToState( STATE_COMMAND );
        }
    }
    return retval;
}