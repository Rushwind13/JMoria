#include "UseState.h"
#include "TargetState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Effect.h"
#include "Game.h"
#include "JMDefs.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CUseState::CUseState()
    : m_cCommand( 0 ),
      m_dwQuantityPrompt( -1 ),
      m_pPendingEffect( nullptr ),
      m_dwPendingItemFlags( 0 ),
      m_bFromEquipment( false )
{
    m_pKeyHandlers[USE_INIT] = &CUseState::OnHandleInit;
    m_pKeyHandlers[USE_WIELD] = &CUseState::OnHandleWield;
    m_pKeyHandlers[USE_REMOVE] = &CUseState::OnHandleRemove;
    m_pKeyHandlers[USE_DROP] = &CUseState::OnHandleDrop;
    m_pKeyHandlers[USE_QUAFF] = &CUseState::OnHandleQuaff;
    m_pKeyHandlers[USE_READ] = &CUseState::OnHandleRead;
    m_pKeyHandlers[USE_FUEL] = &CUseState::OnHandleFuel;
    m_pKeyHandlers[USE_CHOOSE_ITEM] = &CUseState::OnHandleChooseItem;
    m_pKeyHandlers[USE_STAFF] = &CUseState::OnHandleStaff;
    m_pKeyHandlers[USE_TARGET] = &CUseState::OnHandleTarget;

    m_eCurModifier = USE_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    memset( m_szQuantityBuffer, 0, sizeof( m_szQuantityBuffer ) );
}

int CUseState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

void CUseState::TargetEffect( CEffect *pEffect, uint32 dwFlags )
{
    m_pPendingEffect = pEffect;
    m_dwPendingItemFlags = dwFlags;
    g_pGame->GetMsgs()->Printf( "%s\n", pEffect->GetTargetPrompt() );
    switch( pEffect->GetTargetType() )
    {
    case EFFECT_TARGET_DIRECTION:
        m_eCurModifier = USE_TARGET;
        m_pCurKeyHandler = m_pKeyHandlers[USE_TARGET];
        break;
    case EFFECT_TARGET_ITEM:
        m_eCurModifier = USE_CHOOSE_ITEM;
        m_pCurKeyHandler = m_pKeyHandlers[USE_CHOOSE_ITEM];
        break;
    default:
        // EFFECT_TARGET_NONE: no player choice needed — fire immediately
        pEffect->Dispatch( 0.0f, dwFlags );
        m_pPendingEffect = nullptr;
        m_dwPendingItemFlags = 0;
        ResetToState( STATE_COMMAND );
        break;
    }
}

void CUseState::GosubState( int newstate )
{
    g_pGame->SetState( newstate );
    g_pGame->GetGameState()->Begin( STATE_USE );
    m_eCurModifier = USE_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[USE_INIT];
}

int CUseState::OnHandleTarget( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Handling USE_TARGET\n" );

    if( keysym->sym == JKEY_ESCAPE )
    {
        m_pPendingEffect = nullptr;
        m_dwPendingItemFlags = 0;
        g_pGame->GetMsgs()->Printf( "Cancelled.\n" );
        ResetToState( STATE_COMMAND );
        return 0;
    }

    if( keysym->sym == JKEY_8 && keysym->mod & JMOD_SHIFT )
    {
        GosubState( STATE_TARGET );
        return 0;
    }

    if( IsDirectional( keysym ) )
    {
        JVector vDir;
        GetDir( keysym, vDir );
        JVector vTarget = g_pGame->GetPlayer()->m_vPos + vDir;
        g_pGame->GetPlayer()->SetRangedHitPosition( vTarget );
        m_pPendingEffect->Dispatch( 0.0f, m_dwPendingItemFlags );
        m_pPendingEffect = nullptr;
        m_dwPendingItemFlags = 0;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    g_pGame->GetMsgs()->Printf( "%s\n", m_pPendingEffect ? m_pPendingEffect->GetTargetPrompt()
                                                         : "Which direction? (* for target)" );
    return 0;
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

    // If we're in quantity prompt mode, handle that instead
    if( m_dwQuantityPrompt >= 0 )
    {
        return OnHandleQuantityPrompt( keysym );
    }

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

    // We got an alpha key; check if this is a stackable item with multiple items
    JLog( LOG_LEVEL_NOISE, true, "DROP got a selection\n" );
    if( TestDrop() )
    {
        CItem *pItem = m_pSelected->m_lpData;

        // If stackable with count > 1, prompt for quantity
        if( pItem->IsStackable() && pItem->m_dwCount > 1 )
        {
            m_dwQuantityPrompt = 0; // Start quantity prompt mode
            memset( m_szQuantityBuffer, 0, sizeof( m_szQuantityBuffer ) );
            g_pGame->GetMsgs()->Printf( "How many? (1-%d, * for all): ", pItem->m_dwCount );
            return JSUCCESS;
        }

        // Non-stackable or single item: drop normally
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
    if( !TestRead() )
    {
        g_pGame->GetMsgs()->Printf( "You can't read a %s!\n", m_pSelected->m_lpData->GetName() );
        m_pSelected = NULL;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    // Pre-check: does any effect need player targeting before we can dispatch?
    CEffect *pTargetEffect = m_pSelected->m_lpData->m_id->FindTargetingEffect();
    if( pTargetEffect )
    {
        uint32 dwItemFlags = m_pSelected->m_lpData->m_dwFlags;
        g_pGame->GetMsgs()->Printf( "You read the %s.\n", m_pSelected->m_lpData->GetName() );
        g_pGame->GetPlayer()->ConsumeItem( m_pSelected );
        m_pSelected = NULL;
        TargetEffect( pTargetEffect, dwItemFlags );
        return 0;
    }

    // No targeting needed — dispatch immediately
    JResult readResult = DoRead();
    if( readResult == JSUCCESS )
    {
        g_pGame->GetMsgs()->Printf( "You read the %s.\n", m_pSelected->m_lpData->GetName() );
    }
    else if( readResult != JSUCCESS )
    {
        g_pGame->GetMsgs()->Printf( "The %s slips from your fingers and returns to your pack!\n",
                                    m_pSelected->m_lpData->GetName() );
    }

    JLog( LOG_LEVEL_DEBUG, true, "READ resetting game state to COMMAND, USE state to INIT\n" );
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
        case JKEY_z:
            mod = USE_STAFF;
            g_pGame->GetMsgs()->Printf( "Use which staff? [a-z]\n" );
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

    // Returning from STATE_TARGET — fire the pending effect at the selected target
    JLog( LOG_LEVEL_DEBUG, true, "USE returning from TARGET state\n" );
    if( !m_pPendingEffect )
    {
        JLog( LOG_LEVEL_ERROR, true, "returning from TARGET with no pending effect\n" );
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    if( !pTarget )
    {
        // Player cancelled targeting; item was already consumed
        m_pPendingEffect = nullptr;
        m_dwPendingItemFlags = 0;
        g_pGame->GetMsgs()->Printf( "Cancelled.\n" );
        ResetToState( STATE_COMMAND );
        return 0;
    }

    g_pGame->GetPlayer()->SetRangedHitPosition( pTarget->GetPos() );
    m_pPendingEffect->Dispatch( 0.0f, m_dwPendingItemFlags );
    m_pPendingEffect = nullptr;
    m_dwPendingItemFlags = 0;
    ResetToState( STATE_COMMAND );
    return 0;
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

int CUseState::OnHandleChooseItem( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Handling IDENTIFY\n" );

    if( keysym->sym == JKEY_ESCAPE )
    {
        m_pPendingEffect = nullptr;
        m_dwPendingItemFlags = 0;
        m_bFromEquipment = false;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    int ch = GetAlpha( keysym );
    if( ch == nul )
    {
        g_pGame->GetMsgs()->Printf( "%s\n", m_pPendingEffect
                                                ? m_pPendingEffect->GetTargetPrompt()
                                                : "Choose an item [a-z inv, A-J equip]:" );
        return 0;
    }

    m_bFromEquipment = ( ch >= 'A' && ch <= 'Z' );
    m_dwSelected = ch - ( m_bFromEquipment ? 'A' : 'a' );

    m_pSelected = GetResponse( USE_CHOOSE_ITEM );
    if( m_pSelected == NULL )
    {
        g_pGame->GetMsgs()->Printf( "No such item. %s\n",
                                    m_pPendingEffect ? m_pPendingEffect->GetTargetPrompt()
                                                     : "Choose an item [a-z inv, A-J equip]:" );
        m_bFromEquipment = false;
        return 0;
    }

    if( m_pPendingEffect && !m_pPendingEffect->IsValidTarget( m_pSelected->m_lpData ) )
    {
        g_pGame->GetMsgs()->Printf( "%s\n", m_pPendingEffect->GetTargetPrompt() );
        m_pSelected = NULL;
        m_bFromEquipment = false;
        return 0;
    }

    g_pGame->GetPlayer()->ApplyChosenItem( m_pSelected, m_pPendingEffect, m_dwPendingItemFlags );
    m_pPendingEffect = nullptr;
    m_dwPendingItemFlags = 0;
    m_pSelected = NULL;
    m_bFromEquipment = false;

    JLog( LOG_LEVEL_DEBUG, true, "IDENTIFY resetting game state to COMMAND, USE state to INIT\n" );
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleQuantityPrompt( JKeysym *keysym )
{
    CItem *pItem = m_pSelected->m_lpData;

    // Handle Enter key to confirm quantity
    if( keysym->sym == JKEY_RETURN )
    {
        int quantity = 0;

        // If empty/blank, cancelled
        if( m_szQuantityBuffer[0] == 0 )
        {
            g_pGame->GetMsgs()->Printf( "Cancelled.\n" );
            m_dwQuantityPrompt = -1;
            m_pSelected = NULL;
            ResetToState( STATE_COMMAND );
            return 0;
        }

        // Check for * or all
        if( m_szQuantityBuffer[0] == '*' || Util::jstrcmp( m_szQuantityBuffer, "all" ) == 0 )
        {
            quantity = pItem->m_dwCount;
        }
        else
        {
            quantity = atoi( m_szQuantityBuffer );
        }

        if( quantity <= 0 || quantity > pItem->m_dwCount )
        {
            g_pGame->GetMsgs()->Printf( "Invalid quantity.\n" );
            m_dwQuantityPrompt = -1;
            m_pSelected = NULL;
            ResetToState( STATE_COMMAND );
            return 0;
        }

        // Execute the drop with the specified quantity
        if( g_pGame->GetPlayer()->Drop( pItem, quantity ) )
        {
            g_pGame->GetMsgs()->Printf( "You dropped %d.\n", quantity );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "Could not drop items.\n" );
        }

        m_dwQuantityPrompt = -1;
        m_pSelected = NULL;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    // Handle Escape to cancel
    if( keysym->sym == JKEY_ESCAPE )
    {
        g_pGame->GetMsgs()->Printf( "Cancelled.\n" );
        m_dwQuantityPrompt = -1;
        m_pSelected = NULL;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    // Handle backspace to delete last character
    if( keysym->sym == JKEY_BACKSPACE )
    {
        int len = Util::jstrlen( m_szQuantityBuffer );
        if( len > 0 )
        {
            m_szQuantityBuffer[len - 1] = 0;
        }
        return JSUCCESS;
    }

    // Handle numeric input (0-9)
    if( keysym->sym >= JKEY_0 && keysym->sym <= JKEY_9 )
    {
        int len = Util::jstrlen( m_szQuantityBuffer );
        if( len < sizeof( m_szQuantityBuffer ) - 1 )
        {
            m_szQuantityBuffer[len] = '0' + ( keysym->sym - JKEY_0 );
            m_szQuantityBuffer[len + 1] = 0;
        }
        return JSUCCESS;
    }

    // Handle * (Shift+8) for "all"
    if( keysym->sym == JKEY_8 && keysym->mod & JMOD_SHIFT )
    {
        int len = Util::jstrlen( m_szQuantityBuffer );
        if( len < sizeof( m_szQuantityBuffer ) - 1 )
        {
            m_szQuantityBuffer[len] = '*';
            m_szQuantityBuffer[len + 1] = 0;
        }
        return JSUCCESS;
    }

    // Ignore other keys
    return JSUCCESS;
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
        // always inventory
    case USE_DROP:
    case USE_WIELD:
    case USE_READ:
    case USE_QUAFF:
    case USE_STAFF:
        pList = g_pGame->GetPlayer()->m_llInventory;
        break;
        // either equipment or inventory
    case USE_FUEL:
    case USE_CHOOSE_ITEM:
        if( m_bFromEquipment )
        {
            pList = g_pGame->GetPlayer()->m_llEquipment;
        }
        else
        {
            pList = g_pGame->GetPlayer()->m_llInventory;
        }
        break;
        // always equipment
    case USE_REMOVE:
        pList = g_pGame->GetPlayer()->m_llEquipment;
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "Can't get response for : %d\n", whichUse );
        return NULL;
        break;
    }

    pLink = pList->GetLink( m_dwSelected );

    return pLink;
}

//////////////////////////////////////
/// command-specific fcns go below

//// Open commands
bool CUseState::TestWield() { return m_pSelected->m_lpData->IsWieldable(); }

bool CUseState::DoWield() { return g_pGame->GetPlayer()->Wield( m_pSelected ) == JSUCCESS; }

//// Close commands
bool CUseState::TestRemove() { return g_pGame->GetPlayer()->IsRemovable( m_pSelected ); }

bool CUseState::DoRemove() { return g_pGame->GetPlayer()->RemoveEquipment( m_pSelected ); }

//// Drop commands
bool CUseState::TestDrop() { return g_pGame->GetPlayer()->CanDropHere(); }

bool CUseState::DoDrop() { return g_pGame->GetPlayer()->Drop( m_pSelected->m_lpData ); }
//// Quaff commands
bool CUseState::TestQuaff() { return m_pSelected->m_lpData->IsDrinkable(); }

bool CUseState::DoQuaff() { return g_pGame->GetPlayer()->Quaff( m_pSelected ) == JSUCCESS; }

//// Read commands
bool CUseState::TestRead() { return m_pSelected->m_lpData->IsReadable(); }

JResult CUseState::DoRead() { return g_pGame->GetPlayer()->Read( m_pSelected ); }

//// Fuel commands
bool CUseState::TestFuel() { return m_pSelected->m_lpData->IsFuel(); }

bool CUseState::DoFuel() { return g_pGame->GetPlayer()->Fuel( m_pSelected ) == JSUCCESS; }

//// Staff commands
bool CUseState::TestStaff() { return m_pSelected->m_lpData->IsStaff(); }

JResult CUseState::DoStaff() { return g_pGame->GetPlayer()->UseStaff( m_pSelected ); }

int CUseState::OnHandleStaff( JKeysym *keysym )
{
    JResult retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling STAFF\n" );
    retval = OnBaseHandleKey( keysym, USE_STAFF );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "STAFF cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf( "Choose an item from inventory (a to z):\n" );
        return 0;
    }

    JLog( LOG_LEVEL_DEBUG, true, "STAFF got a selection: %s\n", m_pSelected->m_lpData->GetName() );

    if( !TestStaff() )
    {
        g_pGame->GetMsgs()->Printf( "You can't use a %s as a staff!\n",
                                    m_pSelected->m_lpData->GetName() );
        m_pSelected = NULL;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    CItem *pItem = m_pSelected->m_lpData;
    if( pItem->m_dwCharges <= 0 )
    {
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
        m_pSelected = NULL;
        ResetToState( STATE_COMMAND );
        return 0;
    }

    // Staves fire at player position — no trajectory needed
    g_pGame->GetPlayer()->SetRangedHitPosition( g_pGame->GetPlayer()->m_vPos );
    CEffect *pTargetEffect = pItem->m_id->FindTargetingEffect();

    const char *szEffectDesc = NULL;
    CLink<CEffect> *plEffect = pItem->m_id->m_llEffects->GetHead();
    if( plEffect && plEffect->m_lpData && plEffect->m_lpData->m_ed &&
        plEffect->m_lpData->m_ed->m_szName )
    {
        szEffectDesc = plEffect->m_lpData->m_ed->m_szName;
    }

    // Pre-check: does any effect need player targeting before dispatch?
    if( pTargetEffect )
    {
        uint32 dwItemFlags = pItem->m_dwFlags;
        g_pGame->GetPlayer()->ConsumeItem( m_pSelected );
        m_pSelected = NULL;
        TargetEffect( pTargetEffect, dwItemFlags );
        return 0;
    }
    else if( szEffectDesc )
    {
        g_pGame->GetMsgs()->Printf( "The %s emits a %s.\n", pItem->GetName(), szEffectDesc );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s glows.\n", pItem->GetName() );
    }

    JResult staffResult = DoStaff();
    g_pGame->GetPlayer()->ConsumeItem( m_pSelected );
    m_pSelected = NULL;

    JLog( LOG_LEVEL_DEBUG, true, "STAFF resetting game state to COMMAND, USE state to INIT\n" );
    ResetToState( STATE_COMMAND );
    return 0;
}

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
