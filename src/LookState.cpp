#include "LookState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CLookState::CLookState() : m_cCommand( 0 ), m_vLookPos( 0, 0 )
{
    m_pKeyHandlers[LOOK_INIT] = &CLookState::OnHandleInit;
    m_pKeyHandlers[LOOK_LOOK] = &CLookState::OnHandleLook;

    m_eCurModifier = LOOK_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CLookState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CLookState::OnHandleLook( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling LOOK modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_WARN, true,
              "LOOK cmd still waiting for a directional key: Directional key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Direction(1 2 3 4 6 7 8 9):\n" );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "LOOK modifier got a directional\n" );
    if( TestLook() )
    {
        if( DoLook() )
        {
            // you are continuing to look
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "something happened? ...\n" );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't see that from here.\n" );
    }

    // JLog( LOG_LEVEL_DEBUG, true,
    //       "LOOK modifier resetting game state to COMMAND, LOOK state to INIT\n" );
    // // One way or another, we're done with this state now.
    // ResetToState( STATE_COMMAND );
    return 0;
}

int CLookState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing look state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eLookModifier mod = LOOK_INIT;
        switch( m_cCommand )
        {
        case SDLK_SEMICOLON:
            if( keysym->mod & KMOD_SHIFT )
            {
                mod = LOOK_LOOK;
                m_vLookPos = g_pGame->GetPlayer()->m_vPos;
                g_pGame->GetDungeon()->SetLookPosition( m_vLookPos );
            }
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true,
                  "There seems to be some kind of mistake; I don't handle mod: %d\n", m_cCommand );
            ResetToState( STATE_COMMAND );
            return 0;
            break;
        }

        m_eCurModifier = mod;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        return 0;
    }

    JLog( LOG_LEVEL_ERROR, true,
          "Error: tried to init modify state when it was already initted...\n" );
    ResetToState( STATE_COMMAND );
    // shouldn't get here
    return JRESETSTATE;
}

int CLookState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( IsDirectional( keysym ) )
    {
        JVector vDelta;
        GetDir( keysym, vDelta );
        m_vLookPos += vDelta; // the previous position // g_pGame->GetPlayer()->m_vPos;
        g_pGame->GetDungeon()->SetLookPosition( m_vLookPos );

        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_8 )
    {
        if( keysym->mod & KMOD_SHIFT )
        {
            // * key chooses look target, then gets us out of look mode
            CDungeonTile *pTile =
                g_pGame->GetDungeon()->GetTile( g_pGame->GetDungeon()->GetLookPosition() );

            // monster
            if( pTile->m_pCurMonster )
            {
                printf( "monster\n" );
                // you see here a %s
                // target selected.
            }
            // item
            if( pTile->m_pCurItem )
            {
                printf( "item\n" );
                // you see here a %s
            }
            // tile
            if( pTile->m_dtd->m_dwType != DUNG_IDX_WALL )
            {
                printf( "dungeon\n" );
                // you see here a %s
            }
            // now reset
            ResetToState( STATE_COMMAND );
            return JRESETSTATE;
        }
    }
    else if( keysym->sym == SDLK_ESCAPE )
    {
        // ESC key gets us out of look mode
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    return -1;
}

void CLookState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = LOOK_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Look commands
bool CLookState::TestLook() { return g_pGame->GetDungeon()->PlayerCanSee( m_vLookPos ); }

bool CLookState::DoLook()
{
    // save off ( m_vLookPos );
    return true;
}
