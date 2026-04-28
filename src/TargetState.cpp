#include "TargetState.h"

#include "DisplayText.h"
#include "Game.h"
#include "JMDefs.h"
#include "MonsterRecall.h"

#include "Dungeon.h"
#include "Player.h"
#include "Util.h"

extern CGame *g_pGame;

CTargetState::CTargetState()
    : m_cCommand( 0 ),
      m_dwCurrentSelection( 0 ),
      m_dwPreviousState( STATE_COMMAND )
{
    m_pKeyHandlers[TARGET_INIT] = &CTargetState::OnHandleInit;
    m_pKeyHandlers[TARGET_TARGET] = &CTargetState::OnHandleTarget;

    m_eCurModifier = TARGET_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CTargetState::~CTargetState() {}

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
        JLog( LOG_LEVEL_DEBUG, true, "TARGET cmd still waiting for a valid key.\n" );
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

    JLinkList<CMonster> *pVisible = g_pGame->GetPlayer()->GetVisibleMonsters();

    if( pVisible && pVisible->length() )
    {
        JLog( LOG_LEVEL_INFO, true, "Total targetable monsters: %d\n", pVisible->length() );
        // Set initial target to nearest monster (first in distance-sorted list)
        CMonster *pFirst = pVisible->GetHead()->m_lpData;
        if( pFirst )
        {
            g_pGame->GetPlayer()->SetTarget( pFirst );
            if( g_pGame->GetMonsterRecall() && pFirst->m_md )
            {
                g_pGame->GetMsgs()->Clear();
                g_pGame->GetMonsterRecall()->PrintRecall( pFirst->m_md, g_pGame->GetMsgs() );
            }
        }
        UpdateLOSLine();
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
    JLinkList<CMonster> *pVisible = g_pGame->GetPlayer()->GetVisibleMonsters();

    if( keysym->sym == JKEY_8 && keysym->mod & JMOD_SHIFT )
    {
        if( !pVisible || !pVisible->length() )
            return JSUCCESS;

        m_dwCurrentSelection++;
        JLog( LOG_LEVEL_NOISE, true, "Choosing next target... %d/%d\n", m_dwCurrentSelection,
              pVisible->length() );

        if( m_dwCurrentSelection >= pVisible->length() )
        {
            m_dwCurrentSelection = 0;
            JLog( LOG_LEVEL_NOISE, true, "wrapping... %d/%d\n", m_dwCurrentSelection,
                  pVisible->length() );
        }
        CLink<CMonster> *pTargetLink = pVisible->GetNthLink( m_dwCurrentSelection );
        CMonster *pMon = pTargetLink->m_lpData;
        if( !pMon )
        {
            JLog( LOG_LEVEL_WARN, true, "Target instance %d no longer exists, skipping\n",
                  pTargetLink->m_dwInstanceId );
            return JSUCCESS;
        }

        JLog( LOG_LEVEL_INFO, true, "desired monster instance: %d, monster: %s\n",
              pMon->GetInstanceId(), pMon->GetName() );
        g_pGame->GetPlayer()->SetTarget( pMon );
        UpdateLOSLine();
        if( g_pGame->GetMonsterRecall() && pMon->m_md )
        {
            g_pGame->GetMsgs()->Clear();
            g_pGame->GetMonsterRecall()->PrintRecall( pMon->m_md, g_pGame->GetMsgs() );
        }
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_PERIOD )
    {
        // Re-validate LOS before confirming target
        CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
        if( pTarget && g_pGame->GetDungeon()->PlayerCanSee(
                           pTarget->GetPos(),
                           pTarget->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND ) ) )
        {
            g_pGame->GetMsgs()->Printf( "Target selected.\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "You can no longer see that target.\n" );
            g_pGame->GetPlayer()->SetTarget( NULL );
        }
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
    g_pGame->GetDungeon()->ClearLOSLine();
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

void CTargetState::UpdateLOSLine()
{
    CMonster *pTarget = g_pGame->GetPlayer()->GetTarget();
    if( !pTarget )
    {
        g_pGame->GetDungeon()->ClearLOSLine();
        return;
    }

    JIVector vSource( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    JIVector vTarget( VEC_EXPAND( pTarget->GetPos() ) );
    JLinkList<JIVector> *pLine = Util::GenerateLine( vSource, vTarget, SIGHT_DISTANCE_PLAYER );
    g_pGame->GetDungeon()->SetLOSLine( pLine );
}
