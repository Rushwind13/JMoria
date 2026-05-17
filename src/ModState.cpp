#include "ModState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"

#include "Dungeon.h"
#include "Item.h"
#include "Player.h"

extern CGame *g_pGame;

CModState::CModState() : m_cCommand( 0 ), m_vNewPos( 0, 0 )
{
    m_pKeyHandlers[MOD_OPEN] = &CModState::OnHandleOpen;
    m_pKeyHandlers[MOD_TUNNEL] = &CModState::OnHandleTunnel;
    m_pKeyHandlers[MOD_INIT] = &CModState::OnHandleInit;
    m_pKeyHandlers[MOD_CLOSE] = &CModState::OnHandleClose;
    m_pKeyHandlers[MOD_SPIKE] = &CModState::OnHandleSpike;

    m_eCurModifier = MOD_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CModState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CModState::OnHandleOpen( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling OPEN modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Open cmd still waiting for a directional key: Directional key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Direction(1 2 3 4 6 7 8 9):\n" );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "OPEN modifier got a directional\n" );
    if( TestOpen() )
    {
        if( DoOpen() )
        {
            // door opened; set up new state and crap
            g_pGame->GetMsgs()->Printf( "You have picked the lock.\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You failed to pick the lock.\n" );
        }
    }
    else
    {
        CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
        if( pTile && pTile->m_dtd->m_dwType == DUNG_IDX_DOOR &&
            pTile->HasFlags( DUNG_FLAG_LOCKED ) )
        {
            g_pGame->GetMsgs()->Printf( "The door is held fast.\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "I do not see anything to open there.\n" );
        }
    }

    JLog( LOG_LEVEL_DEBUG, true,
          "OPEN modifier resetting game state to COMMAND, OPEN state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CModState::OnHandleClose( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling CLOSE modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "close cmd still waiting for a directional key: Directional key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Direction(1 2 3 4 6 7 8 9):\n" );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "CLOSE modifier got a directional\n" );
    if( TestClose() )
    {
        if( DoClose() )
        {
            // door closed; set up new state and crap
            g_pGame->GetMsgs()->Printf( "You have closed the door.\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You failed to close the door.\n" );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "I do not see anything to close there.\n" );
    }

    JLog( LOG_LEVEL_DEBUG, true,
          "CLOSE modifier resetting game state to COMMAND, CLOSE state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CModState::OnHandleTunnel( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TUNNEL modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Tunnel cmd still waiting for a directional key: Directional key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Direction(1 2 3 4 6 7 8 9):\n" );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "TUNNEL modifier got a directional\n" );
    if( TestTunnel() )
    {
        if( DoTunnel() )
        {
            // Tunnel cleared; set up new state and crap
            g_pGame->GetMsgs()->Printf( "You have removed the rubble.\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You dig in the rubble...\n" );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "Tunnel through what? Empty air?.\n" );
    }

    JLog( LOG_LEVEL_DEBUG, true,
          "TUNNEL modifier resetting game state to COMMAND, TUNNEL state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CModState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing modify state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eModModifier mod = MOD_INIT;
        switch( m_cCommand )
        {
        case JKEY_o:
            mod = MOD_OPEN;
            break;
        case JKEY_c:
            mod = MOD_CLOSE;
            break;
        case JKEY_s:
            if( keysym->mod & JMOD_SHIFT )
            {
                mod = MOD_SPIKE;
            }
            else
            {
                JLog( LOG_LEVEL_INFO, true, "SEARCH not handled here.\n" );
                ResetToState( STATE_COMMAND );
                return 0;
            }
            break;
        case JKEY_t:
            if( keysym->mod & JMOD_SHIFT )
            {
                mod = MOD_TUNNEL;
            }
            else
            {
                JLog( LOG_LEVEL_INFO, true, "THROW not implemented yet.\n" );
                ResetToState( STATE_COMMAND );
                return 0;
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

int CModState::OnBaseHandleKey( JKeysym *keysym )
{
    if( IsDirectional( keysym ) )
    {
        m_vNewPos.Init();
        GetDir( keysym, m_vNewPos );
        m_vNewPos += g_pGame->GetPlayer()->m_vPos;

        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_ESCAPE )
    {
        // ESC key gets us out of modify mode
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    return -1;
}

void CModState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = MOD_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Open commands
bool CModState::TestOpen() { return ( g_pGame->GetDungeon()->IsOpenable( m_vNewPos ) ); }

bool CModState::DoOpen()
{
    if( Util::GetRandom( 1, 100 ) <= CHANCE_PICK_LOCK )
    {
        g_pGame->GetDungeon()->Modify( m_vNewPos );
        return true;
    }
    return false;
}

//// Tunnel commands
bool CModState::TestTunnel() { return ( g_pGame->GetDungeon()->IsTunnelable( m_vNewPos ) ); }

bool CModState::DoTunnel()
{
    if( Util::GetRandom( 0, 1 ) )
    {
        g_pGame->GetDungeon()->Modify( m_vNewPos );
        return true;
    }
    return false;
}

//// Close commands
bool CModState::TestClose() { return ( g_pGame->GetDungeon()->IsCloseable( m_vNewPos ) ); }

bool CModState::DoClose()
{
    if( Util::GetRandom( 1, 100 ) <= CHANCE_CLOSE_DOOR )
    {
        g_pGame->GetDungeon()->Modify( m_vNewPos );
        return true;
    }
    return false;
}

//// Spike commands
int CModState::OnHandleSpike( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling SPIKE modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        g_pGame->GetMsgs()->Printf( "Direction(1 2 3 4 6 7 8 9):\n" );
        return 0;
    }

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile || pTile->m_dtd->m_dwType != DUNG_IDX_DOOR )
    {
        g_pGame->GetMsgs()->Printf( "I do not see a closed door there.\n" );
    }
    else if( pTile->HasFlags( DUNG_FLAG_LOCKED ) )
    {
        // Already spiked — pull it free
        if( DoUnspike() )
        {
            g_pGame->GetMsgs()->Printf( "You pull the spike free.\n" );
        }
    }
    else
    {
        // Spike the door shut
        if( TestSpike() )
        {
            if( DoSpike() )
            {
                g_pGame->GetMsgs()->Printf( "You spike the door shut.\n" );
            }
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You have no iron spikes.\n" );
        }
    }

    ResetToState( STATE_COMMAND );
    return 0;
}

bool CModState::TestSpike()
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile || pTile->m_dtd->m_dwType != DUNG_IDX_DOOR )
        return false;
    if( pTile->HasFlags( DUNG_FLAG_LOCKED ) )
        return false;
    CLink<CItem> *pSpike = g_pGame->GetPlayer()->m_llInventory->GetLink( ITEM_IDX_SPIKE );
    return ( pSpike != nullptr && pSpike->m_lpData != nullptr );
}

bool CModState::DoSpike()
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile )
        return false;
    pTile->SetFlags( DUNG_FLAG_LOCKED );
    CLink<CItem> *pSpike = g_pGame->GetPlayer()->m_llInventory->GetLink( ITEM_IDX_SPIKE );
    g_pGame->GetPlayer()->ConsumeItem( pSpike );
    return true;
}

bool CModState::DoUnspike()
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile )
        return false;
    pTile->UnsetFlags( DUNG_FLAG_LOCKED );
    // Return the spike to inventory
    CLink<CItem> *pExisting = g_pGame->GetPlayer()->m_llInventory->GetLink( ITEM_IDX_SPIKE );
    if( pExisting && pExisting->m_lpData )
    {
        pExisting->m_lpData->m_dwCount++;
    }
    else
    {
        // Find the spike item def and create a new stack
        CLink<CItemDef> *pDefLink = g_pGame->GetDungeon()->GetItemDefs()->GetHead();
        while( pDefLink )
        {
            if( pDefLink->m_lpData && pDefLink->m_lpData->m_dwIndex == ITEM_IDX_SPIKE )
            {
                CItem *pItem = new CItem;
                pItem->Init( pDefLink->m_lpData );
                pItem->m_pllLink = g_pGame->GetPlayer()->m_llInventory->Add( pItem );
                break;
            }
            pDefLink = pDefLink->next;
        }
    }
    return true;
}
