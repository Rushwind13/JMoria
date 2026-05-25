//
//  RangedState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 11/29/2024.
//  Copyright © 2024-2026 Jimbo S. Harris. All rights reserved.
//

#ifndef RangedState_h
#define RangedState_h
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"

#include "DungeonConstants.h"

#define PROJECTILE_RANGE DUNG_PROJECTILE_RANGE
#define PROJECTILE_UPDATE_INTERVAL 2.5f
class CRangedState;
typedef int ( CRangedState::*RangedKeyHandler )( JKeysym *keysym );
enum eRangedModifier
{
    RANGED_INVALID = -1,
    RANGED_INIT = 0,
    RANGED_FIRE = 1,
    RANGED_LAUNCH,
    RANGED_TARGET,
    RANGED_TRAJECTORY,
    RANGED_ZAP,
    RANGED_MAX
};
class CRangedState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    RangedKeyHandler m_pKeyHandlers[RANGED_MAX];
    RangedKeyHandler m_pCurKeyHandler;

    eRangedModifier m_eCurModifier;

private:
    int m_dwClock;
    float m_fStateTicks;

    JIVector m_vTarget;
    JIVector m_vCurrentPosition;
    JLinkList<JIVector> *m_llTrajectory;
    uint32 m_dwTrajectory;
    int m_dwSelected;
    CLink<CItem> *m_pSelected;

    // Member Functions
public:
    CRangedState();
    ~CRangedState();

    virtual void OnUpdate( float fCurTime )
    {
        m_fStateTicks += fCurTime;
        JLog( LOG_LEVEL_DEBUG, true,
              "RangedState::OnUpdate CALLED - fCurTime=%f, m_fStateTicks=%f, threshold=%f, "
              "modifier=%d\n",
              fCurTime, m_fStateTicks, PROJECTILE_UPDATE_INTERVAL, m_eCurModifier );
        if( m_fStateTicks >= PROJECTILE_UPDATE_INTERVAL )
        {
            JLog( LOG_LEVEL_DEBUG, true,
                  "RangedState::OnUpdate - modifier=%d, clock=%d, trajectory=%d\n", m_eCurModifier,
                  m_dwClock, m_dwTrajectory );
            if( m_eCurModifier == RANGED_TRAJECTORY )
            {
                JLog( LOG_LEVEL_DEBUG, true, "  ->About to call DoTrajectory\n" );
                DoTrajectory();
            }
            m_fStateTicks -= PROJECTILE_UPDATE_INTERVAL;
        }
    };
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

    char GetCommand() { return m_cCommand; };
    int GetModifier() { return (int)m_eCurModifier; };
    bool NeedsSelection() { return ( m_pSelected == NULL ); };
    void SetTarget( JVector vTarget ) { m_vTarget = JIVector( vTarget.x, vTarget.y ); };
    void SetCurrentPosition( JVector vPos )
    {
        m_vCurrentPosition = JIVector( vPos.x, vPos.y );
        JLog( LOG_LEVEL_DEBUG, true,
              ">>SetCurrentPosition called: vPos=<%f %f>, m_vCurrentPosition now=<%d %d>\n", vPos.x,
              vPos.y, m_vCurrentPosition.x, m_vCurrentPosition.y );
    };

protected:
#ifdef UNIT_TEST
public:
#else
private:
#endif // UNIT_TEST    
    int OnHandleFire( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );
    int OnHandleLaunch( JKeysym *keysym );
    int OnHandleTarget( JKeysym *keysym );
    int OnHandleTrajectory( JKeysym *keysym );
    int OnHandleZap( JKeysym *keysym );

    void ResetToState( int newstate );
    void GosubState( int newstate );
    CLink<CItem> *GetResponse( eRangedModifier whichUse );
    
    bool ReadyToLaunch();
    bool DoLaunch();

    void UsePlayerTarget();
    int BuildTrajectory();
    bool DoTrajectory();
    void DropAmmo( JVector vFinalPos );

    bool TestFire();
    bool DoFire();

    bool TestZap();
    bool DoZap();
};
#endif /* RangedState_h */
