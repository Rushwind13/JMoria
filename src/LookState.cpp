#include "LookState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"
#include "MonsterRecall.h"
#include "Strings.h"

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
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
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
            g_pGame->GetMsgs()->Printf( g_Strings[STR_SOMETHING_HAPPENED] );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_CANT_SEE_THAT] );
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
            g_pGame->GetMsgs()->Printf( g_Strings[STR_LOOK_SEE], pTile->m_pCurMonster->GetName() );
            g_pGame->GetMsgs()->Printf( g_Strings[STR_TARGET_SELECTED] );
            g_pGame->GetPlayer()->SetTarget( pTile->m_pCurMonster );
            if( g_pGame->RecallMonster() && pTile->m_pCurMonster->m_md )
            {
                g_pGame->GetMonsterRecall()->Clear();
                g_pGame->RecallMonster()->PrintRecall( pTile->m_pCurMonster->m_md,
                                                       g_pGame->GetMonsterRecall() );
            }
        }
        // item
        if( pTile->m_pCurItem )
        {
            JLog( LOG_LEVEL_DEBUG, true, "LOOK command sees an item\n" );
            g_pGame->GetMsgs()->Printf( g_Strings[STR_LOOK_SEE], pTile->m_pCurItem->GetName() );
        }
        // tile
        if( pTile->m_dtd->m_dwType != DUNG_IDX_WALL )
        {
            JLog( LOG_LEVEL_DEBUG, true, "LOOK command sees an item\n" );
            int strIdx = STR_DUNGEON_UNKNOWN;
            switch( pTile->m_dtd->m_dwType )
            {
            case DUNG_IDX_DOOR:
                strIdx = STR_DUNGEON_DOOR;
                break;
            case DUNG_IDX_OPEN_DOOR:
                strIdx = STR_DUNGEON_OPEN_DOOR;
                break;
            case DUNG_IDX_SECRET_DOOR:
                strIdx = STR_DUNGEON_SECRET_DOOR;
                break;
            case DUNG_IDX_DOWNSTAIRS:
                strIdx = STR_DUNGEON_STAIRS_DOWN;
                break;
            case DUNG_IDX_LONG_DOWNSTAIRS:
                strIdx = STR_DUNGEON_STAIRS_DOWN_LONG;
                break;
            case DUNG_IDX_UPSTAIRS:
                strIdx = STR_DUNGEON_STAIRS_UP;
                break;
            case DUNG_IDX_LONG_UPSTAIRS:
                strIdx = STR_DUNGEON_STAIRS_UP_LONG;
                break;
            case DUNG_IDX_FLOOR:
                strIdx = STR_DUNGEON_FLOOR;
                break;
            case DUNG_IDX_RUBBLE:
                strIdx = STR_DUNGEON_RUBBLE;
                break;
            default:
                strIdx = STR_DUNGEON_UNKNOWN;
                break;
            }
            g_pGame->GetMsgs()->Printf( g_Strings[STR_YOU_SEE], g_Strings[strIdx] );
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
