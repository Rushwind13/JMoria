#include "TargetState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CTargetState::CTargetState()
    : m_cCommand( 0 ),
      m_llTargets( NULL ),
      m_dwCurrentSelection( 0 ),
      m_dwPreviousState( STATE_COMMAND )
{
    m_pKeyHandlers[TARGET_INIT] = &CTargetState::OnHandleInit;
    m_pKeyHandlers[TARGET_TARGET] = &CTargetState::OnHandleTarget;

    m_eCurModifier = TARGET_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CTargetState::~CTargetState()
{
    if( m_llTargets )
    {
        m_llTargets->Terminate();
        delete m_llTargets;
        m_llTargets = NULL;
    }
}

int CTargetState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CTargetState::OnHandleTarget( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TARGET modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_WARN, true, "TARGET cmd still waiting for a valid key.\n" );
        g_pGame->GetMsgs()->Printf( "(* for target, . to choose, ESC to exit):\n" );
        return 0;
    }

    // We got a directional key; do an "open" in that direction
    JLog( LOG_LEVEL_NOISE, true, "TARGET modifier got a valid key\n" );
    // if( TestTarget() )
    // {
    //     if( DoTarget() )
    //     {
    //         // you are continuing to look
    //     }
    //     else
    //     {
    //         g_pGame->GetMsgs()->Printf( "something happened? ...\n" );
    //     }
    // }
    // else
    // {
    //     g_pGame->GetMsgs()->Printf( "You can't see that target.\n" );
    // }

    // JLog( LOG_LEVEL_DEBUG, true,
    //       "TARGET modifier resetting game state to previous state, TARGET state to INIT\n" );
    // // One way or another, we're done with this state now.
    // ResetToState( m_dwPreviousState );
    return 0;
}

int CTargetState::DoInit()
{
    m_eCurModifier = TARGET_TARGET;
    m_dwCurrentSelection = 0;
    m_llTargets = new JLinkList<uint32>;
    // CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    // if( pTarget )
    // {
    //     if( g_pGame->GetDungeon()->PlayerCanSee(
    //             pTarget->GetPos(),
    //             pTarget->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND ) )
    //             )
    //     {
    //         uint32 *dwTarget = new uint32( pTarget->m_pllLink->m_dwIndex );
    //         m_llTargets->Add( dwTarget );
    //     }
    //     else
    //     {

    //         pTarget->UnsetAsTarget();
    //     }
    // }
    CLink<CMonster> *pLink = g_pGame->GetDungeon()->m_llMonsters->GetHead();
    CMonster *pMon = NULL;
    CDungeonTile *monPos = NULL;
    uint32 count = 0;
    while( pLink != NULL )
    {
        if( !pLink->m_lpData )
        {
            pLink = pLink->next;
            continue;
        }
        pMon = pLink->m_lpData;
        monPos = g_pGame->GetDungeon()->GetTile( pMon->GetPos() );
        bool bTargeted = false; //( pMon == g_pGame->GetPlayer()->GetTarget() );
        bool bSeen = true;      //( ( monPos->m_dwFlags & DUNG_FLAG_SEEN ) == DUNG_FLAG_SEEN );
        bool bPlayerSees = ( g_pGame->GetDungeon()->PlayerCanSee(
            pMon->GetPos(), pMon->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND ) ) );

        JLog( LOG_LEVEL_NOISE, true, "mon: %s target %d seen %d sees %d\n", pMon->GetName(),
              bTargeted, bSeen, bPlayerSees );
        if( /*!bTargeted && bSeen && /**/ bPlayerSees )
        {
            uint32 *dwTargetable = new uint32( count );
            JLog( LOG_LEVEL_DEBUG, true, "Adding %s to targets mon %d at idx %d\n", pMon->GetName(),
                  pMon->m_pllLink->m_dwIndex, *dwTargetable );
            m_llTargets->Add( dwTargetable );
            // if( !g_pGame->GetPlayer()->GetTarget() )
            g_pGame->GetPlayer()->SetTarget( pMon );
        }
        pLink = pLink->next;
        count++;
    }
    if( m_llTargets->length() )
    {
        JLog( LOG_LEVEL_INFO, true, "Total targetable monsters: %d\n", m_llTargets->length() );
    }
    else
    {
        JLog( LOG_LEVEL_INFO, true, "No targets available.\n" );
        ResetToState( m_dwPreviousState );
        return JCOMPLETESTATE;
    }

    return JSUCCESS;
}
int CTargetState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing look state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eTargetModifier mod = TARGET_INIT;
        switch( m_cCommand )
        {
        case JKEY_8:
            if( keysym->mod & JMOD_SHIFT )
                DoInit();
            break;
        case JKEY_f:
        case JKEY_z:
            if( keysym->mod == 0 )
            {
                m_dwPreviousState = STATE_RANGED;
                DoInit();
            }
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true,
                  "There seems to be some kind of mistake; I don't handle mod: %d\n", m_cCommand );
            ResetToState( m_dwPreviousState );
            return 0;
            break;
        }
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        return 0;
    }

    JLog( LOG_LEVEL_ERROR, true,
          "Error: tried to init modify state when it was already initted...\n" );
    ResetToState( m_dwPreviousState );
    // shouldn't get here
    return JRESETSTATE;
}

int CTargetState::OnBaseHandleKey( JKeysym *keysym )
{
    if( keysym->sym == JKEY_8 && keysym->mod & JMOD_SHIFT )
    {
        m_dwCurrentSelection++;
        JLog( LOG_LEVEL_NOISE, true, "Choosing next target... %d/%d\n", m_dwCurrentSelection,
              m_llTargets->length() );

        if( m_dwCurrentSelection >= m_llTargets->length() )
        {
            m_dwCurrentSelection = 0;
            JLog( LOG_LEVEL_NOISE, true, "wrapping... %d/%d\n", m_dwCurrentSelection,
                  m_llTargets->length() );
        }
        uint32 *dwTarget = m_llTargets->GetNthLink( m_dwCurrentSelection )->m_lpData;
        CMonster *pMon = g_pGame->GetDungeon()->m_llMonsters->GetNthLink( *dwTarget )->m_lpData;
        JLog( LOG_LEVEL_INFO, true, "desired monster index: %d retrieved idx %d, monster: %s\n",
              *dwTarget, pMon->m_pllLink->m_dwIndex, pMon->GetName() );
        g_pGame->GetPlayer()->SetTarget( pMon );
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_PERIOD )
    {
        g_pGame->GetMsgs()->Printf( "Target selected.\n" );
        // now reset
        ResetToState( m_dwPreviousState );
        return JRESETSTATE;
    }
    else if( keysym->sym == JKEY_ESCAPE )
    {
        // ESC key gets us out of target mode
        ResetToState( m_dwPreviousState );
        return JRESETSTATE;
    }

    return -1;
}

void CTargetState::ResetToState( int newstate )
{
    if( m_llTargets )
    {
        m_llTargets->Terminate();
        delete m_llTargets;
        m_llTargets = NULL;
    }
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = TARGET_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Target commands
// bool CTargetState::TestTarget()
// {
//     return true;
// }

// bool CTargetState::DoTarget()
// {
//     return true;
// }
