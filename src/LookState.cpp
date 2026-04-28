#include "LookState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"
#include "MonsterRecall.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CLookState::CLookState() : m_cCommand( 0 ), m_vDelta( 0, 0 )
{
    m_pKeyHandlers[LOOK_INIT] = &CLookState::OnHandleInit;
    m_pKeyHandlers[LOOK_LOOK] = &CLookState::OnHandleLook;

    m_eCurModifier = LOOK_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CLookState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CLookState::OnHandleLook( JKeysym *keysym )
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
        JLog( LOG_LEVEL_DEBUG, true,
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

int CLookState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing look state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eLookModifier mod = LOOK_INIT;
        switch( m_cCommand )
        {
        case JKEY_SEMICOLON:
            if( keysym->mod & JMOD_SHIFT )
            {
                mod = LOOK_LOOK;
                m_vDelta.Init();
                g_pGame->GetDungeon()->SetLookPosition( g_pGame->GetPlayer()->m_vPos );
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

int CLookState::OnBaseHandleKey( JKeysym *keysym )
{
    if( IsDirectional( keysym ) )
    {
        m_vDelta.Init();
        GetDir( keysym, m_vDelta );

        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_PERIOD )
    {
        // * key chooses look target, then gets us out of look mode
        CDungeonTile *pTile =
            g_pGame->GetDungeon()->GetTile( g_pGame->GetDungeon()->GetLookPosition() );

        // monster
        if( pTile->m_pCurMonster )
        {
            JLog( LOG_LEVEL_DEBUG, true, "LOOK command sees a monster\n" );
            g_pGame->GetMsgs()->Printf( "You see here a %s.\nTarget selected.\n",
                                        pTile->m_pCurMonster->GetName() );
            g_pGame->GetPlayer()->SetTarget( pTile->m_pCurMonster );
            if( g_pGame->RecallMonster() && pTile->m_pCurMonster->m_md )
            {
                g_pGame->GetMsgs()->Clear();
                g_pGame->RecallMonster()->PrintRecall( pTile->m_pCurMonster->m_md,
                                                       g_pGame->GetMsgs() );
            }
        }
        // item
        if( pTile->m_pCurItem )
        {
            JLog( LOG_LEVEL_DEBUG, true, "LOOK command sees an item\n" );
            g_pGame->GetMsgs()->Printf( "You see here a %s\n", pTile->m_pCurItem->GetName() );
        }
        // tile
        if( pTile->m_dtd->m_dwType != DUNG_IDX_WALL )
        {
            JLog( LOG_LEVEL_DEBUG, true, "LOOK command sees an item\n" );
            char dungeon[32];
            switch( pTile->m_dtd->m_dwType )
            {
            case DUNG_IDX_DOOR:
                Util::jstrcpy( dungeon, "a door. It is closed" );
                break;
            case DUNG_IDX_OPEN_DOOR:
                Util::jstrcpy( dungeon, "an open door" );
                break;
            case DUNG_IDX_SECRET_DOOR:
                Util::jstrcpy( dungeon, "You can't see that from here" );
                break;
            case DUNG_IDX_DOWNSTAIRS:
                Util::jstrcpy( dungeon, "a set of stairs, going down" );
                break;
            case DUNG_IDX_LONG_DOWNSTAIRS:
                Util::jstrcpy( dungeon, "a long set of stairs, going down" );
                break;
            case DUNG_IDX_UPSTAIRS:
                Util::jstrcpy( dungeon, "a staircase, going up" );
                break;
            case DUNG_IDX_LONG_UPSTAIRS:
                Util::jstrcpy( dungeon, "a long staircase, going up" );
                break;
            case DUNG_IDX_FLOOR:
                Util::jstrcpy( dungeon, "open floor" );
                break;
            case DUNG_IDX_RUBBLE:
                Util::jstrcpy( dungeon, "some rubble" );
                break;
            default:
                Util::jstrcpy( dungeon, "... what is *that*?! .." );
                break;
            }
            g_pGame->GetMsgs()->Printf( "You see %s.\n", dungeon );
        }
        // now reset
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }
    else if( keysym->sym == JKEY_ESCAPE )
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
bool CLookState::TestLook()
{
    return g_pGame->GetDungeon()->PlayerCanSee( g_pGame->GetDungeon()->GetLookPosition() +
                                                m_vDelta );
}

bool CLookState::DoLook()
{
    g_pGame->GetDungeon()->SetLookPosition( g_pGame->GetDungeon()->GetLookPosition() + m_vDelta );
    return true;
}
