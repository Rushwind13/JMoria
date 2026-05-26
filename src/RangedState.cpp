//
//  RangedState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 11/29/2024.
//  Copyright © 2024-2026 Jimbo S. Harris. All rights reserved.
//

#include "RangedState.h"

#include "DisplayText.h"
#include "Game.h"
#include "Player.h"
#include "Util.h"

#include "assert.h"

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

int CRangedState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CRangedState::OnHandleInit( JKeysym *keysym )
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
        case JKEY_f:
        {
            // Validate equipped primary weapon for fire
            CLink<CItem> *pMainWeapon =
                g_pGame->GetPlayer()->m_llEquipment->GetLink( EQUIP_IDX_MAIN_HAND );
            if( pMainWeapon == NULL ||
                !( pMainWeapon->m_lpData->m_id->m_dwFlags & ITEM_FLAG_NEEDSAMMO ) )
            {
                g_pGame->GetMsgs()->Printf( "You have nothing to fire with.\n" );
                ResetToState( STATE_COMMAND );
                return JRESETSTATE;
            }

            // Check if inventory has compatible ammo for this weapon
            CLink<CItem> *pAmmo = g_pGame->GetPlayer()->m_llInventory->GetHead();
            bool hasAmmo = false;
            while( pAmmo )
            {
                if( g_pGame->GetPlayer()->IsCompatibleAmmo( pAmmo ) )
                {
                    hasAmmo = true;
                    break;
                }
                pAmmo = g_pGame->GetPlayer()->m_llInventory->GetNext( pAmmo );
            }

            if( !hasAmmo )
            {
                g_pGame->GetMsgs()->Printf( "You have nothing to fire.\n" );
                ResetToState( STATE_COMMAND );
                return JRESETSTATE;
            }

            // Store the weapon so BuildTrajectory can access its range values
            g_pGame->GetPlayer()->m_pCurrentRangedWeapon = pMainWeapon->m_lpData;

            mod = RANGED_FIRE;
            g_pGame->GetPlayer()->DisplayInventory( PLACEMENT_USE, INV_FIRE );
            break;
        }
        case JKEY_z:
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
        case JKEY_f:
            m_eCurModifier = RANGED_FIRE;
            break;
        case JKEY_z:
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
int CRangedState::OnHandleFire( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling FIRE\n" );
    JLog( LOG_LEVEL_DEBUG, true, "OnHandleFire: pSelected=%p, ReadyToLaunch=%s\n", m_pSelected,
          ReadyToLaunch() ? "YES" : "NO" );

    if( m_pSelected == NULL )
    {
        retval = OnBaseHandleKey( keysym );

        if( retval == JRESETSTATE )
        {
            return JSUCCESS;
        }

        if( retval != JSUCCESS )
        {
            JLog( LOG_LEVEL_DEBUG, true,
                  "FIRE cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
            g_pGame->GetMsgs()->Printf( "Choose ammo from inventory (a to z):\n" );
            return JSUCCESS;
        }

        JLog( LOG_LEVEL_DEBUG, true,
              "OnHandleFire: After OnBaseHandleKey - pSelected=%p, m_dwSelected=%d\n", m_pSelected,
              m_dwSelected );
    }

    // We got an alpha key; do a "fire" of that ammo
    JLog( LOG_LEVEL_NOISE, true, "FIRE got a selection\n" );
    JLog( LOG_LEVEL_DEBUG, true,
          "OnHandleFire: After base handler - pSelected=%p, ReadyToLaunch=%s, TestFire=%s\n",
          m_pSelected, ReadyToLaunch() ? "YES" : "NO", TestFire() ? "YES" : "NO" );

    if( TestFire() ) // can fire
    {
        JLog( LOG_LEVEL_DEBUG, true, "OnHandleFire: TestFire passed. ReadyToLaunch=%s\n",
              ReadyToLaunch() ? "YES" : "NO" );
        if( ReadyToLaunch() ) // have target
        {
            // Ensure trajectory is built if we have a target
            if( !m_llTrajectory )
            {
                BuildTrajectory();
            }

            if( DoLaunch() ) // have ammo
            {
                JLog( LOG_LEVEL_DEBUG, true,
                      "OnHandleFire: DoLaunch returned true, setting RANGED_TRAJECTORY\n" );
                m_eCurModifier = RANGED_TRAJECTORY;
                m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
                return JSUCCESS;
            }
            else
            {
                // no ammo (shouldn't happen if validation in OnHandleInit worked)
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
        ResetToState( STATE_COMMAND );
        return JCOMPLETESTATE;
    }
    return JSUCCESS;
}

// z)ap a wand
int CRangedState::OnHandleZap( JKeysym *keysym )
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
            JLog( LOG_LEVEL_DEBUG, true,
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
            // Ensure trajectory is built if we have a target

            if( !m_llTrajectory )
            {
                BuildTrajectory();
            }

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
            g_pGame->GetUse()->Printf( "Choose target with * or Directional (1 2 3 4 6 7 8 9)\n" );
            m_eCurModifier = RANGED_TARGET;
            m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
            return JSUCCESS;
        }
    }
    else
    {
        g_pGame->GetUse()->Printf( "You can't zap a %s!\n", m_pSelected->m_lpData->GetName() );
        ResetToState( STATE_COMMAND );
        return JCOMPLETESTATE;
    }
    return JSUCCESS;
}

// choose a target either * or direction
int CRangedState::OnHandleTarget( JKeysym *keysym )
{
    int retval;
    if( !ReadyToLaunch() )
    {
        retval = OnBaseHandleKey( keysym );

        if( retval == JRESETSTATE )
        {
            return 0;
        }

        if( retval != JSUCCESS )
        {
            g_pGame->GetMsgs()->Printf( "Choose target: * or Direction (1 2 3 4 6 7 8 9)\n" );
            return JSUCCESS;
        }
    }

    eRangedModifier mod = RANGED_INIT;
    switch( m_cCommand )
    {
    case JKEY_f:
        mod = RANGED_FIRE;
        break;
    case JKEY_z:
        mod = RANGED_ZAP;
        break;
    default:
        ResetToState( STATE_COMMAND );
        return JBOGUSKEY;
        break;
    }
    m_eCurModifier = mod;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    // Re-process the keystroke through the new modifier handler (fire/zap) now that we have target
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

int CRangedState::BuildTrajectory()
{
    // Determine trajectory distance based on command type
    uint32 dwDistance = MAX_PROJECTILE_RANGE; // default fallback

    if( m_cCommand == JKEY_f )
    {
        // Bows: randomize between min and max range per shot
        if( g_pGame->GetPlayer()->m_pCurrentRangedWeapon != NULL )
        {
            uint32 dwMin = g_pGame->GetPlayer()->m_pCurrentRangedWeapon->m_id->m_dwMinRange;
            uint32 dwMax = g_pGame->GetPlayer()->m_pCurrentRangedWeapon->m_id->m_dwMaxRange;
            if( dwMin > 0 && dwMax >= dwMin )
            {
                dwDistance = (uint32)Util::GetRandom( (float)dwMin, (float)dwMax );
            }
        }
    }
    else if( m_cCommand == JKEY_z )
    {
        // Wands: full range (max distance) every shot
        if( m_pSelected != NULL && m_pSelected->m_lpData != NULL )
        {
            uint32 dwMax = m_pSelected->m_lpData->m_id->m_dwMaxRange;
            if( dwMax > 0 )
            {
                dwDistance = dwMax;
            }
        }
    }

    JLog( LOG_LEVEL_DEBUG, true, "BuildTrajectory: from <%d %d> to <%d %d>, distance=%d\n",
          VEC_EXPAND( m_vCurrentPosition ), VEC_EXPAND( m_vTarget ), dwDistance );
    m_llTrajectory = Util::GenerateLine( m_vCurrentPosition, m_vTarget, dwDistance );

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

int CRangedState::OnHandleTrajectory( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "TRAJECTORY modifier following projectile\n" );

    DoTrajectory();
    return 0;
}

int CRangedState::OnBaseHandleKey( JKeysym *keysym )
{
    if( keysym->sym == JKEY_ESCAPE )
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
                g_pGame->GetUse()->Printf( "Nothing valid to %s.\n",
                                           m_eCurModifier == RANGED_FIRE ? "fire" : "zap" );
                ResetToState( STATE_COMMAND );
                return JRESETSTATE;
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
        else if( keysym->sym == JKEY_8 && keysym->mod & JMOD_SHIFT )
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
    JKeysym newkey;
    newkey.sym = m_cCommand; // Use the actual command (JKEY_f or JKEY_z), not hardcoded JKEY_f
    newkey.mod = 0;
    g_pGame->GetGameState()->HandleKey( &newkey );
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
    g_pGame->GetDungeon()->SetProjectileEffect( NULL,
                                                NULL ); // Clear beam when exiting RANGED state
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
    // Check if we have ammo available
    // Arrows/bolts are stackable items (check m_dwCount)
    // Wands are charged items (check m_dwCharges)
    CItem *pItem = m_pSelected->m_lpData;
    bool isStackable = pItem->IsStackable();
    uint32 availability = isStackable ? pItem->m_dwCount : pItem->m_dwCharges;

    if( availability <= 0 )
    {
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
        return false;
    }
    JLog( LOG_LEVEL_DEBUG, true, "RANGED state firing projectile...\n" );

    // Get effect description from the current effect being used
    const char *szEffectDesc = NULL;
    CEffectDef *pEffectDef = NULL;
    CLink<CEffect> *plEffect = m_pSelected->m_lpData->m_id->m_llEffects->GetHead();
    if( plEffect && plEffect->m_lpData && plEffect->m_lpData->m_ed &&
        plEffect->m_lpData->m_ed->m_szName )
    {
        szEffectDesc = plEffect->m_lpData->m_ed->m_szName;
        pEffectDef = plEffect->m_lpData->m_ed;
    }

    if( szEffectDesc && pItem->IsIdentified() )
    {
        g_pGame->GetMsgs()->Printf( "The %s emits a %s.\n", pItem->GetName(), szEffectDesc );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s glows.\n", pItem->GetName() );
    }

    // Pass effect definition and trajectory to dungeon for multicolor beam rendering
    if( pEffectDef && m_llTrajectory )
    {
        g_pGame->GetDungeon()->SetProjectileEffect( pEffectDef, m_llTrajectory );
    }
    else if( m_llTrajectory && m_pSelected )
    {
        // No effect - trajectory will render at mundane projectile color (white fallback)
        g_pGame->GetDungeon()->SetProjectileEffect( NULL, m_llTrajectory );
    }

    // NOTE: Ammo and wand consumption now happens in CItem::Consume() called below,
    // unified handler for both fire and zap paths.
    // This ensures ammo is consumed immediately when fired, not on hit or impact.

    // For charged items in a stack: unstack before consuming to preserve charge counts
    bool isChargedStackable =
        ( pItem->m_id->m_dwIndex == ITEM_IDX_WAND || pItem->m_id->m_dwIndex == ITEM_IDX_STAFF ) &&
        pItem->m_dwCount > 1;

    if( isChargedStackable )
    {
        // Create a copy with count=1 to unstack (takes one out of the stack)
        CItem *pSingleCopy = pItem->Copy( 1 ); // Single item from this stack
        pSingleCopy->m_pllLink = g_pGame->GetPlayer()->m_llInventory->Add(
            pSingleCopy, pSingleCopy->m_id->m_dwIndex, pSingleCopy->GetInstanceId() );

        // Decrement the original stack
        pItem->m_dwCount--;

        // Consume the copy and remove if depleted
        g_pGame->GetPlayer()->ConsumeItem( pSingleCopy->m_pllLink );
    }
    else
    {
        // Non-charged or non-stacked: just consume and remove if empty
        g_pGame->GetPlayer()->ConsumeItem( m_pSelected );
    }

    g_pGame->SetReadyForUpdate( false );
    return true;
}
// TIMING COUPLING: DoTrajectory() relies on SetReadyForUpdate(false) being called
// by DoLaunch() before the trajectory animation begins. This prevents AIMgr::Update()
// from moving monsters while the projectile follows its pre-computed Bresenham path.
// The trajectory is a snapshot built at launch time — if monsters moved during flight,
// the projectile would follow a stale path. ResetToState() calls SetReadyForUpdate(true)
// to resume normal turn processing after the animation completes.
// See also: CGame::Update() TURN_BASED path, which gates AIMgr on m_bReadyForUpdate.
bool CRangedState::DoTrajectory()
{
    assert( !g_pGame->IsReadyForUpdate() );
    if( !ReadyToLaunch() )
    {
        JLog( LOG_LEVEL_DEBUG, true, "DoTrajectory: ReadyToLaunch=NO, returning early\n" );
        return true;
    }

    if( !m_llTrajectory )
    {
        if( m_vCurrentPosition.IsZero() )
        {
            m_vCurrentPosition.Init( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
        }
        BuildTrajectory();
    }

    JLog( LOG_LEVEL_DEBUG, true, "doing trajectory %d/%d\n", m_dwClock, m_llTrajectory->length() );
    JLog( LOG_LEVEL_DEBUG, true,
          "DoTrajectory: clock=%d, trajectory=%d, command=%c, pSelected=%p\n", m_dwClock,
          m_dwTrajectory, m_cCommand, m_pSelected );

    JIVector *pTrajectoryPos = (JIVector *)( m_llTrajectory->GetNthLink( m_dwClock )->m_lpData );
    JLog( LOG_LEVEL_DEBUG, true, ">>Trajectory[%d] raw = <%d %d>\n", m_dwClock, pTrajectoryPos->x,
          pTrajectoryPos->y );

    // Directly assign trajectory position coordinates
    m_vCurrentPosition.x = pTrajectoryPos->x;
    m_vCurrentPosition.y = pTrajectoryPos->y;
    JLog( LOG_LEVEL_DEBUG, true, ">>After assign: m_vCurrentPosition = <%d %d>\n",
          m_vCurrentPosition.x, m_vCurrentPosition.y );
    m_dwClock++;

    // Advance to next color in beam animation
    g_pGame->GetDungeon()->AdvanceProjectileColor();

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
        {
            // Light each tile the beam passes through if this is a light wand
            bool bHasLight = false;
            CLink<CEffect> *plEff = m_pSelected->m_lpData->m_id->m_llEffects->GetHead();
            while( plEff != NULL )
            {
                if( plEff->m_lpData->m_dwFlags & EFFECT_FLAG_LIGHT )
                {
                    bHasLight = true;
                    break;
                }
                plEff = plEff->next;
            }
            if( bHasLight )
                g_pGame->GetDungeon()->LightPosition( vTest );
        }
        break;
    case DUNG_COLL_MONSTER:
        JLog( LOG_LEVEL_DEBUG, true, "COLLISION WITH MONSTER at <%d %d>\n", (int)vTest.x,
              (int)vTest.y );
        g_pGame->GetPlayer()->SetRangedHitPosition( vTest );
        switch( m_cCommand )
        {
        case JKEY_f:
            DoFire();
            break;
        case JKEY_z:
            DoZap();
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true, "very confused in DoTrajectory: %d\n", m_cCommand );
            break;
        }
        // Check if any effect on this item has NO_COLLIDE (pass-through)
        {
            bool bNoCollide = false;
            CLink<CEffect> *plEff = m_pSelected->m_lpData->m_id->m_llEffects->GetHead();
            int effCount = 0;
            while( plEff != NULL )
            {
                CEffectDef *pDef = plEff->m_lpData->m_ed;
                if( pDef )
                {
                    JLog( LOG_LEVEL_DEBUG, true,
                          "DoTrajectory: Effect[%d]=%s flags2=0x%08x (NO_COLLIDE=0x%08x)\n",
                          effCount, pDef->m_szName, pDef->m_dwFlags2, EFFECT_FLAG_NO_COLLIDE );
                    if( pDef->m_dwFlags2 & EFFECT_FLAG_NO_COLLIDE )
                    {
                        bNoCollide = true;
                        break;
                    }
                }
                effCount++;
                plEff = plEff->next;
            }
            JLog( LOG_LEVEL_DEBUG, true, "DoTrajectory: Monster collision - bNoCollide=%s\n",
                  bNoCollide ? "YES" : "NO" );
            if( !bNoCollide )
            {
                JLog( LOG_LEVEL_DEBUG, true,
                      "Projectile collided. RANGED state complete, reset to CMD state.\n" );
                // Drop arrow after hitting target
                if( m_cCommand == JKEY_f && m_pSelected )
                {
                    JLog( LOG_LEVEL_DEBUG, true, "CALLING DropAmmo: monster collision\n" );
                    DropAmmo( vTest );
                }
                else
                {
                    JLog( LOG_LEVEL_DEBUG, true,
                          "NOT dropping arrow at monster: command=%c, pSelected=%p\n", m_cCommand,
                          m_pSelected );
                }
                ResetToState( STATE_COMMAND );
                return true;
            }
        }
        break;
    case DUNG_COLL_ITEM:
        break;
    case DUNG_COLL_PLAYER:
        break;
    default:
        // Wall or obstacle collision - drop arrow at impact point
        JLog( LOG_LEVEL_DEBUG, true, "Wall collision at <%f %f> type: %d\n", VEC_EXPAND( vTest ),
              collide_type );
        JLog( LOG_LEVEL_DEBUG, true,
              "DoTrajectory DEFAULT case: command=%c, pSelected=%p, m_vCurrentPosition=<%f %f>\n",
              m_cCommand, m_pSelected, VEC_EXPAND( m_vCurrentPosition ) );
        if( m_cCommand == JKEY_f && m_pSelected )
        {
            JLog( LOG_LEVEL_DEBUG, true, "CALLING DropAmmo: wall collision, vTest=<%f %f>\n",
                  VEC_EXPAND( vTest ) );
            DropAmmo( vTest );
        }
        if( m_cCommand == JKEY_z && m_pSelected )
        {
            // Wands that affect walls (e.g. Stone to Mud) trigger on wall collision
            g_pGame->GetPlayer()->SetRangedHitPosition( vTest );
            DoZap();
        }
        ResetToState( STATE_COMMAND );
        return false;
        break;
    }

    // failsafe: don't lock up
    if( m_dwClock > MAX_PROJECTILE_RANGE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "failsafe in TRAJECTORY\n" );
        // Arrow exceeded maximum trajectory steps; drop it at current position
        if( m_cCommand == JKEY_f && m_pSelected )
        {
            JVector vDropPos( VEC_EXPAND( m_vCurrentPosition ) );
            DropAmmo( vDropPos );
        }
        ResetToState( STATE_COMMAND );
        return false;
    }

    if( m_dwClock >= m_dwTrajectory )
    {
        JLog( LOG_LEVEL_DEBUG, true, "RANGED state complete: clock=%d >= trajectory=%d\n",
              m_dwClock, m_dwTrajectory );
        JLog( LOG_LEVEL_DEBUG, true, "RANGED state complete, reset to CMD state.\n" );
        // Arrow reached end of range; drop it at final position
        // (Wands already consumed in DoLaunch, so only handle arrows here)
        if( m_cCommand == JKEY_f && m_pSelected )
        {
            JLog( LOG_LEVEL_DEBUG, true, "CALLING DropAmmo at end\n" );
            JVector vDropPos( VEC_EXPAND( m_vCurrentPosition ) );
            DropAmmo( vDropPos );
        }
        else
        {
            JLog( LOG_LEVEL_DEBUG, true, "NOT calling DropAmmo: command=%c, pSelected=%p\n",
                  m_cCommand, m_pSelected );
        }
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
    {
        // Keep selection logic aligned with DisplayInventory
        pList = g_pGame->GetPlayer()->m_llInventory;
        pLink = pList->GetNthLink( m_dwSelected );
        break;
    }
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
bool CRangedState::TestFire() { return m_pSelected->m_lpData->IsFireable(); }
bool CRangedState::DoFire() { return g_pGame->GetPlayer()->Fire( m_pSelected ) == JSUCCESS; }

bool CRangedState::TestZap() { return m_pSelected->m_lpData->IsZappable(); }
bool CRangedState::DoZap() { return g_pGame->GetPlayer()->Zap( m_pSelected ) == JSUCCESS; }

void CRangedState::DropAmmo( JVector vFinalPos )
{
    JLog( LOG_LEVEL_DEBUG, true, ">>DropAmmo called at <%f %f>, m_pSelected=%p\n",
          VEC_EXPAND( vFinalPos ), m_pSelected );

    // Only arrows and bolts can be dropped on the ground
    if( !m_pSelected || m_pSelected->m_lpData == NULL )
    {
        JLog( LOG_LEVEL_DEBUG, true, ">>Early return: m_pSelected is NULL\n" );
        return;
    }

    CItem *pAmmo = m_pSelected->m_lpData;
    int eAmmoType = pAmmo->m_id->m_dwIndex;

    // Only ammo types (arrows, bolts) drop on terrain
    if( eAmmoType != ITEM_IDX_ARROW && eAmmoType != ITEM_IDX_BOLT )
    {
        JLog( LOG_LEVEL_DEBUG, true, ">>Early return: not arrow/bolt (type=%d)\n", eAmmoType );
        return;
    }

    // Check if arrow breaks on impact (before creating/placing anything)
    int breakChance = Util::GetRandom( 1, 100 );
    if( breakChance <= CHANCE_ARROW_BREAK )
    {
        g_pGame->GetMsgs()->Printf( "The arrow breaks.\n" );
        JLog( LOG_LEVEL_DEBUG, true, ">>Arrow broke on landing (roll=%d)\n", breakChance );
        return;
    }

    // Arrow survives: create new item and drop it
    // Drop() will handle stacking if same-type arrow is on ground, or scattering to adjacent
    CItem *pDropArrow = pAmmo->Copy(1);

    // Drop the arrow (Drop() now handles stacking and scattering)
    JLog( LOG_LEVEL_DEBUG, true, ">>About to call Drop with pos=<%f %f>\n",
          VEC_EXPAND( vFinalPos ) );
    g_pGame->GetDungeon()->Drop( pDropArrow, vFinalPos );
    JLog( LOG_LEVEL_DEBUG, true, ">>Arrow dropped at <%f %f>\n", VEC_EXPAND( vFinalPos ) );
}

void CRangedState::UsePlayerTarget()
{
    m_vCurrentPosition.Init( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    m_vTarget.Init( VEC_EXPAND( g_pGame->GetPlayer()->GetTarget()->GetPos() ) );

    JLog( LOG_LEVEL_DEBUG, true, "cur <%d %d> tar <%d %d>\n", VEC_EXPAND( m_vCurrentPosition ),
          VEC_EXPAND( m_vTarget ) );
}