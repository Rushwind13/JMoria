//
//  RangedState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 11/29/2024.
//  Copyright © 2024 Jimbo S. Harris. All rights reserved.
//

#include "RangedState.h"

#include "DisplayText.h"
#include "Game.h"
#include "Player.h"

extern CGame *g_pGame;

CRangedState::CRangedState()
    : m_dwClock( 0 ),
      m_fStateTicks( 0.0f ),
      m_vTarget( 0, 0 ),
      m_vCurrentPosition( 0, 0 ),
      m_dwTrajectory( 0 ),
      m_llTrajectory( NULL )
{
    m_pKeyHandlers[RANGED_INIT] = &CRangedState::OnHandleInit;
    m_pKeyHandlers[RANGED_TARGET] = &CRangedState::OnHandleTarget;
    m_pKeyHandlers[RANGED_FIRE] = &CRangedState::OnHandleFire;
    m_pKeyHandlers[RANGED_TRAJECTORY] = &CRangedState::OnHandleTrajectory;
    m_pKeyHandlers[RANGED_ZAP] = &CRangedState::OnHandleZap;

    m_eCurModifier = RANGED_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CRangedState::~CRangedState()
{
    if( m_llTrajectory )
    {
        m_llTrajectory->Terminate();
        delete m_llTrajectory;
        m_llTrajectory = NULL;
    }
}

int CRangedState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CRangedState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing RANGED state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;
        m_pSelected = NULL;

        CMonster *pMon = g_pGame->GetPlayer()->GetTarget();
        if( pMon && ( g_pGame->GetDungeon()->PlayerCanSee(
                        pMon->GetPos(),
                        pMon->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND ) ) ) )
        {
            UsePlayerTarget();
        }

        eRangedModifier mod = RANGED_INIT;
        switch( m_cCommand )
        {
        case SDLK_f:
            mod = RANGED_FIRE;
            g_pGame->GetMsgs()->Printf( "Fire which weapon? [a-z]\n" );
            break;
        case SDLK_z:
            mod = RANGED_ZAP;
            g_pGame->GetMsgs()->Printf( "Zap which wand? [a-z]\n" );
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true,
                  "There seems to be some kind of mistake; I don't handle mod: %d\n", m_cCommand );
            ResetToState( STATE_COMMAND );
            return JRESETSTATE;
            break;
        }
        m_eCurModifier = mod;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        return JSUCCESS;
    }
    else // returning from STATE_TARGET
    {
        JLog( LOG_LEVEL_DEBUG, true, "RANGED returning from TARGET state\n" );
        if( !g_pGame->GetPlayer()->GetTarget() )
        {
            JLog( LOG_LEVEL_ERROR, true,
                  "badness returning from target state, still need target\n" );
            ResetToState( STATE_COMMAND );
            return JRESETSTATE;
        }

        UsePlayerTarget();
        BuildTrajectory();

        switch( m_cCommand )
        {
        case SDLK_f:
            m_eCurModifier = RANGED_FIRE;
            break;
        case SDLK_z:
            m_eCurModifier = RANGED_ZAP;
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true, "badness returning from target state, have command: %d\n",
                  m_cCommand );
            ResetToState( STATE_COMMAND );
            return JRESETSTATE;
            break;
        }
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        HandleKey( keysym );
        return JSUCCESS;
    }

    JLog( LOG_LEVEL_ERROR, true,
          "Error: tried to init ranged state when it was already initted...\n" );
    ResetToState( STATE_COMMAND );
    // shouldn't get here
    return JRESETSTATE;
}

// f)ire a projectile e.g. arrow
int CRangedState::OnHandleFire( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling FIRE\n" );
    if( m_pSelected == NULL )
    {
        retval = OnBaseHandleKey( keysym );

        if( retval == JRESETSTATE )
        {
            return JSUCCESS;
        }

        if( retval != JSUCCESS )
        {
            JLog( LOG_LEVEL_WARN, true,
                  "FIRE cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
            g_pGame->GetMsgs()->Printf( "Choose an item from equipment(a to z):\n" );
            return JSUCCESS;
        }
    }

    // We got a alpha key; do a "zap" of that item
    JLog( LOG_LEVEL_NOISE, true, "FIRE got a selection\n" );
    if( TestFire() ) // can fire
    {
        if( ReadyToLaunch() ) // have target
        {
            if( DoLaunch() ) // have charges
            {
                m_eCurModifier = RANGED_TRAJECTORY;
                m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
                return JSUCCESS;
            }
            else
            {
                // no charges
                g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
                ResetToState( STATE_COMMAND );
                return JCOMPLETESTATE;
            }
        }
        else
        {
            // set target
            g_pGame->GetMsgs()->Printf( "Choose target with * or Directional (1 2 3 4 6 7 8 9)\n" );
            m_eCurModifier = RANGED_TARGET;
            m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
            return JSUCCESS;
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't fire a %s!\n", m_pSelected->m_lpData->GetName() );
    }
    return JSUCCESS;
}

// z)ap a wand
int CRangedState::OnHandleZap( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling ZAP\n" );
    if( m_pSelected == NULL )
    {
        retval = OnBaseHandleKey( keysym );

        if( retval == JRESETSTATE )
        {
            return JSUCCESS;
        }

        if( retval != JSUCCESS )
        {
            JLog( LOG_LEVEL_WARN, true,
                  "ZAP cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
            g_pGame->GetMsgs()->Printf( "Choose an item from inventory(a to z):\n" );
            return JSUCCESS;
        }
    }
    else
    {
        JLog( LOG_LEVEL_DEBUG, true, "have selection: %s\n", m_pSelected->m_lpData->GetName() );
    }

    JLog( LOG_LEVEL_DEBUG, true, "success, %s\n", m_pSelected->m_lpData->GetName() );

    // We got a alpha key; do a "zap" of that item
    JLog( LOG_LEVEL_NOISE, true, "ZAP got a selection\n" );
    if( TestZap() ) // can zap
    {
        if( ReadyToLaunch() ) // have target
        {
            if( DoLaunch() ) // have charges
            {
                m_eCurModifier = RANGED_TRAJECTORY;
                m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
                return JSUCCESS;
            }
            else
            {
                // no charges
                g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
                ResetToState( STATE_COMMAND );
                return JCOMPLETESTATE;
            }
        }
        else
        {
            JLog( LOG_LEVEL_DEBUG, true, "need a target\n" );
            // set target
            g_pGame->GetMsgs()->Printf( "Choose target with * or Directional (1 2 3 4 6 7 8 9)\n" );
            m_eCurModifier = RANGED_TARGET;
            m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
            return JSUCCESS;
        }
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't zap a %s!\n", m_pSelected->m_lpData->GetName() );
    }
    return JSUCCESS;
}

// choose a target either * or direction
int CRangedState::OnHandleTarget( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TARGET modifier\n" );
    if( !ReadyToLaunch() )
    {
        retval = OnBaseHandleKey( keysym );

        if( retval == JRESETSTATE )
        {
            return 0;
        }

        if( retval != JSUCCESS )
        {
            JLog( LOG_LEVEL_WARN, true, "TARGET cmd still waiting for target.\n" );
            g_pGame->GetMsgs()->Printf( "Choose target: * or Direction (1 2 3 4 6 7 8 9)\n" );
            return JSUCCESS;
        }
    }

    JLog( LOG_LEVEL_DEBUG, true, "TARGET modifier complete, RANGED state reset to previous\n" );
    eRangedModifier mod = RANGED_INIT;
    switch( m_cCommand )
    {
    case SDLK_f:
        mod = RANGED_FIRE;
        break;
    case SDLK_z:
        mod = RANGED_ZAP;
        break;
    default:
        ResetToState( STATE_COMMAND );
        return JBOGUSKEY;
        break;
    }
    m_eCurModifier = mod;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    HandleKey( keysym );

    return JSUCCESS;
}

bool CollisionCheck( JVector &vTest )
{
    int collide_type = g_pGame->GetDungeon()->IsWalkableFor( vTest );
    switch( collide_type )
    {
    case DUNG_COLL_ITEM:
    case DUNG_COLL_PLAYER:
    case DUNG_COLL_MONSTER:
    case DUNG_COLL_NO_COLLISION:
        return true;
        break;
    default:
        break;
    }
    return false;
}

bool NoCollisionCheck( JVector &viTest ) { return true; }

int CRangedState::BuildTrajectory()
{
    m_llTrajectory = new JLinkList<JIVector>;
    Util::Bresenham( m_vCurrentPosition, m_vTarget, PROJECTILE_RANGE, NoCollisionCheck,
                     m_llTrajectory );

    if( m_llTrajectory && m_llTrajectory->length() > 0 )
    {
        JLog( LOG_LEVEL_DEBUG, true, "length: %d\n", m_llTrajectory->length() );
        m_dwTrajectory = m_llTrajectory->length();
    }
    else
    {
        JLog( LOG_LEVEL_DEBUG, true, "yikes bad bres\n" );
        return JBOGUSKEY;
    }
    return JSUCCESS;
}

int CRangedState::OnHandleTrajectory( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "TRAJECTORY modifier following projectile\n" );

    DoTrajectory();
    return 0;
}

int CRangedState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_ESCAPE )
    {
        // ESC key gets us out of modify mode
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    switch( m_eCurModifier )
    {
    case RANGED_TRAJECTORY:
        return JSUCCESS;
        break;
    case RANGED_FIRE:
    case RANGED_ZAP:
        if( m_pSelected != NULL )
            return JSUCCESS;
        m_dwSelected = GetAlpha( keysym );
        if( m_dwSelected != nul )
        {
            // convert selected item to list offset
            m_dwSelected -= 'a';
            JLog( LOG_LEVEL_DEBUG, true, "Use State got key: %c\n", m_dwSelected + 'a' );
            m_pSelected = GetResponse( m_eCurModifier );
            if( m_pSelected == NULL )
            {
                g_pGame->GetMsgs()->Printf( "Please select a valid item.\n" );

                return JBOGUSKEY;
            }

            return JSUCCESS;
        }
        break;
    case RANGED_TARGET:
        if( ReadyToLaunch() )
        {
            JLog( LOG_LEVEL_DEBUG, true, "have target: <%d %d>\n", VEC_EXPAND( m_vTarget ) );
            return JSUCCESS;
        }
        if( IsDirectional( keysym ) )
        {
            // if player chooses direction,
            // find position at end of ray from player pos to item range
            JVector vSource = g_pGame->GetPlayer()->m_vPos;
            JVector vTarget;
            GetDir( keysym, vTarget );
            m_vCurrentPosition.Init( VEC_EXPAND( vSource ) );
            m_vTarget.Init( VEC_EXPAND( vSource + vTarget ) );
            if( !Util::IsInWorld( vSource + vTarget ) )
            {
                m_vTarget.Init( CLAMP( m_vTarget.x, 0, DUNG_WIDTH - 1 ),
                                CLAMP( m_vTarget.y, 0, DUNG_HEIGHT - 1 ) );
            }
            JLog( LOG_LEVEL_DEBUG, true, "made target: <%d %d>\n", VEC_EXPAND( m_vTarget ) );

            BuildTrajectory();

            return JSUCCESS;
        }
        else if( keysym->sym == SDLK_8 && keysym->mod & KMOD_SHIFT )
        {
            GosubState( STATE_TARGET );
            return JRESETSTATE;
        }
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "no joy for that modifier\n" );
        return JBOGUSKEY;
    }

    return JBOGUSKEY;
}

void CRangedState::GosubState( int newstate )
{
    g_pGame->SetState( newstate );
    // target state will bounce back to here
    // do not overwrite m_cCommand nor m_pSelected
    SDL_Keysym *newkey = new SDL_Keysym();
    newkey->sym = SDLK_f;
    newkey->mod = 0;
    g_pGame->GetGameState()->HandleKey( newkey );
    delete newkey;
    m_eCurModifier = RANGED_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

void CRangedState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_pSelected = NULL;
    m_vTarget.Init();
    m_vCurrentPosition.Init();
    m_dwTrajectory = 0;
    if( m_llTrajectory != NULL )
    {
        m_llTrajectory->Terminate();
        delete m_llTrajectory;
        m_llTrajectory = NULL;
    }
    g_pGame->GetDungeon()->SetProjectilePosition( JVector( -1, -1 ) );
    m_dwClock = 0;
    m_fStateTicks = 0.0f;
    m_eCurModifier = RANGED_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    g_pGame->SetReadyForUpdate( true );
}

//////////////////////////////////////
/// command-specific fcns go below

bool CRangedState::DoLaunch()
{
    if( m_pSelected->m_lpData->m_dwCharges <= 0 )
        return false;
    JLog( LOG_LEVEL_DEBUG, true, "RANGED state firing projectile...\n" );
    g_pGame->GetMsgs()->Printf( "The %s emits a ray of %s.\n", m_pSelected->m_lpData->GetName(),
                                "blinding blue light" );
    m_pSelected->m_lpData->m_dwCharges--;
    g_pGame->SetReadyForUpdate( false );
    return true;
}
bool CRangedState::DoTrajectory()
{
    if( !ReadyToLaunch() )
        return true;

    if( !m_llTrajectory )
    {
        if( m_vCurrentPosition.IsZero() )
        {
            m_vCurrentPosition.Init( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
        }
        BuildTrajectory();
    }

    JLog( LOG_LEVEL_DEBUG, true, "doing trajectory %d/%d\n",
          m_llTrajectory->length() - m_dwTrajectory, m_llTrajectory->length() );
    m_dwClock++;
    m_vCurrentPosition.Init(
        VEC_EXPAND( *( m_llTrajectory->GetNthLink( m_dwTrajectory - 1 )->m_lpData ) ) );
    JVector vTest( VEC_EXPAND( m_vCurrentPosition ) );
    JLog( LOG_LEVEL_DEBUG, true, "pos <%d %d>\n", VEC_EXPAND( m_vCurrentPosition ) );

    int collide_type = g_pGame->GetDungeon()->IsWalkableFor( vTest );

    switch( collide_type )
    {
    case DUNG_COLL_NO_COLLISION:
        // Do projectile update (moves across screen before anyone gets to move)
        JLog( LOG_LEVEL_DEBUG, true,
              "RANGED state updating, projectile moves one step in the direction of the "
              "target.\n" );
        g_pGame->GetDungeon()->SetProjectilePosition( vTest );
        break;
    case DUNG_COLL_MONSTER:
        g_pGame->GetPlayer()->SetRangedHitPosition( vTest );
        switch( m_cCommand )
        {
        case SDLK_f:
            DoFire();
            break;
        case SDLK_z:
            DoZap();
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true, "very confused in DoTrajectory: %d\n", m_cCommand );
            break;
        }
        if( ( m_pSelected->m_lpData->m_dwFlags & ITEM_FLAG_NO_COLLIDE ) == 0 )
        {
            JLog( LOG_LEVEL_DEBUG, true,
                  "Projectile collided. RANGED state complete, reset to CMD state.\n" );
            ResetToState( STATE_COMMAND );
            return true;
        }
        break;
    case DUNG_COLL_ITEM:
        break;
    case DUNG_COLL_PLAYER:
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "invalid collision at <%f %f> type: %d\n", VEC_EXPAND( vTest ),
              collide_type );
        ResetToState( STATE_COMMAND );
        return false;
        break;
    }

    // failsafe: don't lock up
    if( m_dwClock > 20 )
    {
        JLog( LOG_LEVEL_ERROR, true, "failsafe in TRAJECTORY\n" );
        ResetToState( STATE_COMMAND );
        return false;
    }

    m_dwTrajectory--;

    if( m_dwTrajectory == 0 )
    {
        JLog( LOG_LEVEL_DEBUG, true, "RANGED state complete, reset to CMD state.\n" );
        ResetToState( STATE_COMMAND );
        return true;
    }

    g_pGame->SetReadyForUpdate( false );
    return true;
}

CLink<CItem> *CRangedState::GetResponse( eRangedModifier whichUse )
{
    JLinkList<CItem> *pList = NULL;
    CLink<CItem> *pLink = NULL;
    switch( whichUse )
    {
    case RANGED_FIRE:
    case RANGED_ZAP:
        pList = g_pGame->GetPlayer()->m_llInventory;
        pLink = pList->GetNthLink( m_dwSelected );
        break;
    default:
        JLog( LOG_LEVEL_ERROR, true, "Can't get response for : %d\n", whichUse );
        return NULL;
        break;
    }
    if( pLink != NULL )
        JLog( LOG_LEVEL_DEBUG, true, "got good pLink %s\n", pLink->m_lpData->GetName() );
    return pLink;
}

bool CRangedState::ReadyToLaunch() { return !m_vTarget.IsZero(); }

//// Zap commands
bool CRangedState::TestFire() { return g_pGame->GetPlayer()->IsFireable( m_pSelected ); }
bool CRangedState::DoFire() { return g_pGame->GetPlayer()->Fire( m_pSelected ) == JSUCCESS; }

bool CRangedState::TestZap() { return g_pGame->GetPlayer()->IsZappable( m_pSelected ); }
bool CRangedState::DoZap() { return g_pGame->GetPlayer()->Zap( m_pSelected ) == JSUCCESS; }

void CRangedState::UsePlayerTarget()
{
    m_vCurrentPosition.Init( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    // construct target pos in the direction of the player target (to allow passing through the
    // target)
    JVector vDelta = g_pGame->GetPlayer()->GetTarget()->GetPos() - g_pGame->GetPlayer()->m_vPos;
    vDelta.Norm();
    vDelta *= 8.0f; // TODO: actual range of item
    m_vTarget.Init( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos + vDelta ) );

    JLog( LOG_LEVEL_ERROR, true, "cur <%d %d> tar <%d %d> del <%.2f %.2f>\n",
          VEC_EXPAND( m_vCurrentPosition ), VEC_EXPAND( m_vTarget ), VEC_EXPAND( vDelta ) );
}