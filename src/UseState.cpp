#include "UseState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CUseState::CUseState() : m_cCommand( 0 )
{
    m_pKeyHandlers[USE_INIT] = &CUseState::OnHandleInit;
    m_pKeyHandlers[USE_WIELD] = &CUseState::OnHandleWield;
    m_pKeyHandlers[USE_REMOVE] = &CUseState::OnHandleRemove;
    m_pKeyHandlers[USE_DROP] = &CUseState::OnHandleDrop;
    m_pKeyHandlers[USE_QUAFF] = &CUseState::OnHandleQuaff;
    m_pKeyHandlers[USE_READ] = &CUseState::OnHandleRead;
    m_pKeyHandlers[USE_FUEL] = &CUseState::OnHandleFuel;

    m_eCurModifier = USE_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CUseState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CUseState::OnHandleWield( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling WIELD \n" );
    retval = OnBaseHandleKey( keysym, USE_WIELD );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
        return 0;
    }

    // We got a alpha key; do a "wield" of that item
    JLog( LOG_LEVEL_NOISE, true, "WIELD got a selection\n" );
    if( TestWield() )
    {
        if( DoWield() )
        {
            g_pGame->GetMsgs()->Printf( "You are now wielding the %s.\n",
                                        m_pSelected->m_lpData->GetName() );
            JLog( LOG_LEVEL_INFO, true, "Now wielding %s\n", m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf(
                "The %s slips from your fingers and returns to your pack!\n",
                m_pSelected->m_lpData->GetName() );
            JLog( LOG_LEVEL_INFO, true, "failed to wield %s\n", m_pSelected->m_lpData->GetName() );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't wield a %s!\n", m_pSelected->m_lpData->GetName() );
        JLog( LOG_LEVEL_INFO, true, "can't wield %s\n", m_pSelected->m_lpData->GetName() );
    }
    m_pSelected = NULL;

    JLog( LOG_LEVEL_DEBUG, true, "WIELD resetting game state to COMMAND, WIELD state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleRemove( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling REMOVE \n" );
    retval = OnBaseHandleKey( keysym, USE_REMOVE );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from equipment(a to z):\n" );
        return 0;
    }

    // We got a alpha key; do a "remove" of that item
    JLog( LOG_LEVEL_NOISE, true, "REMOVE  got a selection\n" );
    if( TestRemove() )
    {
        if( DoRemove() )
        {
            g_pGame->GetMsgs()->Printf( "You take off the %s.\n",
                                        m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You can't remove that!\n" );
        }
        m_pSelected = NULL;
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s is welded to your body!\n",
                                    m_pSelected->m_lpData->GetName() );
    }

    JLog( LOG_LEVEL_DEBUG, true, "REMOVE resetting game state to COMMAND, REMOVE state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleDrop( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling DROP \n" );
    retval = OnBaseHandleKey( keysym, USE_DROP );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
        return 0;
    }

    // We got a alpha key; do a "drop" of that item
    JLog( LOG_LEVEL_NOISE, true, "DROP  got a selection\n" );
    if( TestDrop() )
    {
        if( DoDrop() )
        {
            g_pGame->GetMsgs()->Printf( "You dropped the %s.\n", m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf(
                "The %s slips from your fingers and returns to your pack!\n",
                m_pSelected->m_lpData->GetName() );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't drop a %s here!\n",
                                    m_pSelected->m_lpData->GetName() );
    }
    m_pSelected = NULL;

    JLog( LOG_LEVEL_DEBUG, true, "DROP resetting game state to COMMAND, USE state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleRead( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling READ\n" );
    retval = OnBaseHandleKey( keysym, USE_READ );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
        return 0;
    }
    JLog( LOG_LEVEL_NOISE, true, "READ got a selection\n" );
    if( TestRead() )
    {
        if( DoRead() )
        {
            g_pGame->GetMsgs()->Printf( "You read the %s.\n", m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf(
                "The %s slips from your fingers and returns to your pack!\n",
                m_pSelected->m_lpData->GetName() );
        }
    }

    JLog( LOG_LEVEL_DEBUG, true, "READ resetting game state to COMMAND, USE state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleQuaff( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling QUAFF\n" );
    retval = OnBaseHandleKey( keysym, USE_QUAFF );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
        return 0;
    }

    // We got a alpha key; do a "quaff" of that item
    JLog( LOG_LEVEL_NOISE, true, "QUAFF  got a selection\n" );
    if( TestQuaff() )
    {
        if( DoQuaff() )
        {
            g_pGame->GetMsgs()->Printf( "You drank the %s.\n", m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf(
                "The %s slips from your fingers and returns to your pack!\n",
                m_pSelected->m_lpData->GetName() );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't drink a %s!\n", m_pSelected->m_lpData->GetName() );
    }
    m_pSelected = NULL;

    JLog( LOG_LEVEL_DEBUG, true, "QUAFF resetting game state to COMMAND, USE state to INIT\n" );
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing USE state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eUseModifier mod = USE_INIT;
        switch( m_cCommand )
        {
        case JKEY_w:
            mod = USE_WIELD;
            g_pGame->GetMsgs()->Printf( "Wield which item? [a-z]\n" );
            break;
        case JKEY_t:
            mod = USE_REMOVE;
            g_pGame->GetMsgs()->Printf( "Remove which item? [a-j]\n" );
            break;
        case JKEY_d:
            mod = USE_DROP;
            g_pGame->GetMsgs()->Printf( "Drop which item? [a-z]\n" );
            break;
        case JKEY_r:
            mod = USE_READ;
            g_pGame->GetMsgs()->Printf( "Read which item? [a-z]\n" );
            break;
        case JKEY_q:
            mod = USE_QUAFF;
            g_pGame->GetMsgs()->Printf( "Quaff which item? [a-z]\n" );
            break;
        case JKEY_f:
            mod = USE_FUEL;
            g_pGame->GetMsgs()->Printf( "Fill with which fuel? [a-z]\n" );
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
          "Error: tried to init USE state when it was already initted...\n" );
    ResetToState( STATE_COMMAND );
    // shouldn't get here
    return JRESETSTATE;
}

int CUseState::OnBaseHandleKey( JKeysym *keysym, eUseModifier whichUse )
{
    m_dwSelected = GetAlpha( keysym );
    if( m_dwSelected != nul )
    {
        // convert selected item to list offset
        m_dwSelected -= 'a';
        JLog( LOG_LEVEL_DEBUG, true, "Use State got key: %c\n", m_dwSelected + 'a' );
        m_pSelected = GetResponse( whichUse );
        if( m_pSelected == NULL )
        {
            g_pGame->GetMsgs()->Printf( "Please select a valid item.\n" );

            return -1;
        }

        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_ESCAPE )
    {
        // ESC key gets us out of  mode
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    return -1;
}

void CUseState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = USE_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CLink<CItem> *CUseState::GetResponse( eUseModifier whichUse )
{
    JLinkList<CItem> *pList = NULL;
    CLink<CItem> *pLink = NULL;
    switch( whichUse )
    {
    case USE_DROP:
    case USE_READ:
    case USE_QUAFF:
    case USE_WIELD:
    case USE_FUEL:
        pList = g_pGame->GetPlayer()->m_llInventory;
        pLink = pList->GetNthLink( m_dwSelected );
        break;
    case USE_REMOVE:
        pList = g_pGame->GetPlayer()->m_llEquipment;
        pLink = pList->GetLink( m_dwSelected );
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "Can't get response for : %d\n", whichUse );
        return NULL;
        break;
    }

    return pLink;
}

//////////////////////////////////////
/// command-specific fcns go below

//// Open commands
bool CUseState::TestWield() { return g_pGame->GetPlayer()->IsWieldable( m_pSelected ); }

bool CUseState::DoWield() { return g_pGame->GetPlayer()->Wield( m_pSelected ) == JSUCCESS; }

//// Close commands
bool CUseState::TestRemove() { return g_pGame->GetPlayer()->IsRemovable( m_pSelected ); }

bool CUseState::DoRemove() { return g_pGame->GetPlayer()->RemoveEquipment( m_pSelected ); }

//// Drop commands
bool CUseState::TestDrop() { return g_pGame->GetPlayer()->CanDropHere(); }

bool CUseState::DoDrop() { return g_pGame->GetPlayer()->Drop( m_pSelected->m_lpData ); }
//// Quaff commands
bool CUseState::TestQuaff() { return g_pGame->GetPlayer()->IsDrinkable( m_pSelected ); }

bool CUseState::DoQuaff() { return g_pGame->GetPlayer()->Quaff( m_pSelected ) == JSUCCESS; }

//// Read commands
bool CUseState::TestRead() { return g_pGame->GetPlayer()->IsReadable( m_pSelected ); }

bool CUseState::DoRead() { return g_pGame->GetPlayer()->Read( m_pSelected ) == JSUCCESS; }

//// Fuel commands
bool CUseState::TestFuel() { return g_pGame->GetPlayer()->IsFuel( m_pSelected ); }

bool CUseState::DoFuel() { return g_pGame->GetPlayer()->Fuel( m_pSelected ) == JSUCCESS; }

int CUseState::OnHandleFuel( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling FUEL\n" );
    retval = OnBaseHandleKey( keysym, USE_FUEL );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
        return 0;
    }

    JLog( LOG_LEVEL_NOISE, true, "FUEL got a selection\n" );
    if( TestFuel() )
    {
        if( DoFuel() )
        {
            g_pGame->GetMsgs()->Printf( "You fill your lantern with the %s.\n",
                                        m_pSelected->m_lpData->GetName() );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You have no lantern to fill.\n" );
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't use a %s as fuel!\n",
                                    m_pSelected->m_lpData->GetName() );
    }
    m_pSelected = NULL;

    JLog( LOG_LEVEL_DEBUG, true, "FUEL resetting game state to COMMAND, USE state to INIT\n" );
    ResetToState( STATE_COMMAND );
    return 0;
}
