#include "ModState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"
#include "Strings.h"

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
    m_pKeyHandlers[MOD_BASH] = &CModState::OnHandleBash;

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
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "OPEN modifier got a directional\n" );
    if( TestOpen() )
    {
        if( DoOpen() )
        {
            // door opened; set up new state and crap
            g_pGame->GetMsgs()->Printf( g_Strings[STR_LOCK_PICKED] );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_LOCK_FAILED] );
        }
    }
    else
    {
        CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
        if( pTile && pTile->m_dtd->m_dwType == DUNG_IDX_DOOR &&
            pTile->HasFlags( DUNG_FLAG_LOCKED ) )
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_DOOR_HELD] );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_NO_OPEN_THERE] );
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
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "CLOSE modifier got a directional\n" );
    if( TestClose() )
    {
        if( DoClose() )
        {
            // door closed; set up new state and crap
            g_pGame->GetMsgs()->Printf( g_Strings[STR_DOOR_CLOSED_MSG] );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_CLOSE_FAILED] );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_NO_CLOSE_THERE] );
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
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "TUNNEL modifier got a directional\n" );
    if( TestTunnel() )
    {
        if( DoTunnel() )
        {
            // Tunnel cleared; set up new state and crap
            g_pGame->GetMsgs()->Printf( g_Strings[STR_RUBBLE_REMOVED] );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_DIG_RUBBLE] );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_TUNNEL_AIR] );
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
            if( keysym->mod & JMOD_SHIFT )
                mod = MOD_BASH;
            else
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
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
        return 0;
    }

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile || pTile->m_dtd->m_dwType != DUNG_IDX_DOOR )
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_NO_CLOSED_DOOR] );
    }
    else if( pTile->HasFlags( DUNG_FLAG_LOCKED ) )
    {
        // Already spiked — break it free (spikes are single-use)
        if( DoUnspike() )
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_SPIKE_BREAKS] );
        }
    }
    else
    {
        // Spike the door shut
        if( TestSpike() )
        {
            if( DoSpike() )
            {
                g_pGame->GetMsgs()->Printf( g_Strings[STR_DOOR_SPIKED] );
            }
        }
        else
        {
            g_pGame->GetMsgs()->Printf( g_Strings[STR_NO_SPIKES] );
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
    if( !g_pGame->GetDungeon()->LockDoor( m_vNewPos ) )
        return false;
    CLink<CItem> *pSpike = g_pGame->GetPlayer()->m_llInventory->GetLink( ITEM_IDX_SPIKE );
    g_pGame->GetPlayer()->ConsumeItem( pSpike );
    return true;
}

bool CModState::DoUnspike() { return g_pGame->GetDungeon()->UnlockDoor( m_vNewPos ); }

//// Bash commands
int CModState::OnHandleBash( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling BASH modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DIRECTION_PROMPT] );
        return 0;
    }

    if( !TestBash() )
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_NO_DOOR_THERE] );
    }
    else if( DoBash() )
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DOOR_BASHED] );
        g_pGame->GetDungeon()->Aggravate( m_vNewPos );
        g_pGame->GetDungeon()->DisturbPlayer();
    }
    else
    {
        g_pGame->GetMsgs()->Printf( g_Strings[STR_DOOR_HOLDS] );
    }

    ResetToState( STATE_COMMAND );
    return 0;
}

bool CModState::TestBash()
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    return pTile && pTile->m_dtd && pTile->m_dtd->m_dwType == DUNG_IDX_DOOR;
}

bool CModState::DoBash()
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( m_vNewPos );
    if( !pTile || !pTile->m_dtd )
        return false;

    bool bLocked = pTile->HasFlags( DUNG_FLAG_LOCKED );
    int nDC = bLocked ? 15 : 10;

    float fBonus = g_pGame->GetPlayer()->m_fDamageModifier;

    // Metal armor/boots add heft to the blow
    CLink<CItem> *pEquip = g_pGame->GetPlayer()->m_llEquipment->GetHead();
    while( pEquip )
    {
        CItem *pItem = pEquip->m_lpData;
        if( pItem && pItem->m_id && ( pItem->m_id->m_dwFlags & ITEM_FLAG_METAL ) )
        {
            fBonus += 2.0f;
            break;
        }
        pEquip = g_pGame->GetPlayer()->m_llEquipment->GetNext( pEquip );
    }

    float fRoll = Util::Roll( 1, 20 ) + fBonus;
    JLog( LOG_LEVEL_INFO, true, "DoBash: roll=%.1f bonus=%.1f DC=%d locked=%d\n", fRoll, fBonus,
          nDC, bLocked );
    if( fRoll >= nDC )
    {
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_BROKEN_DOOR );
        pTile->UnsetFlags( DUNG_FLAG_LOCKED );
        return true;
    }
    return false;
}
