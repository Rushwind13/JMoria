#ifndef __AIMGR_H__
#define __AIMGR_H__

#include "JLinkList.h"
#include "JMDefs.h"

#define AI_TURNS_PER_HP 16

#define SLEEP_BASE_WAKE_RANGE 5.0f    // base radius (tiles) at which player can wake sleeper
#define SLEEP_LIT_ROOM_PENALTY 4.0f   // additional wake range when monster is in a lit room
#define SLEEP_WAKE_CHANCE_SCALE 0.10f // probability scale: (range-dist)/range * scale per turn

// all the states which the game can run in
// this modifies the event handling
enum eBrainState
{
    BRAINSTATE_INVALID = -1,
    BRAINSTATE_REST = 0,
    BRAINSTATE_GOTODEST = 1,
    BRAINSTATE_SEEK,
    BRAINSTATE_IDLE,
    BRAINSTATE_MAX
};

class CMonster;

class CAIBrain
{
public:
    CAIBrain();
    virtual ~CAIBrain() {}

    float m_fSpeed;
    int m_dwMoveType;
    int m_nEffectTurns; // remaining turns for active timed status effect
    JFVector m_vPos;
    JVector m_vTargetPos;
    CLink<CAIBrain> *m_pllLink;

    void SetTargetPos( JVector vTarget ) { m_vTargetPos = vTarget; }
    JVector GetTargetPos() { return m_vTargetPos; }

    bool SetRandomDest( float fCurTime );
    bool WalkSeek( float fCurTime );
    bool GotoDest( float fCurTime, JVector &vVel );
    bool Update( float fCurTime );

    bool UpdateRest( float fCurTime );
    bool UpdateIdle( float fCurTime );
    bool UpdateGoToDest( float fCurTime );
    bool UpdateSeek( float fCurTime );

    void SetState( eBrainState newState )
    {
        m_eBrainState = newState;
        m_fStateTicks = 0.0f;
    }
    void SetParent( CMonster *newParent ) { m_pParent = newParent; };

    eBrainState GetState() { return m_eBrainState; };
#ifdef UNIT_TEST
public:
#else
protected:
#endif
    void Move();
    void CollideWithPlayer();
    float m_fStateTicks;
    JVector m_vVel;
    eBrainState m_eBrainState;

    CMonster *m_pParent;
};

class CAIMgr
{
public:
    CAIMgr() : m_fTicks( 0.0f ), m_llAIBrains( NULL ) {};
    ~CAIMgr();
    void Init();

    bool Update( float fCurTime );
    void DestroyBrain( CAIBrain *delete_me );
    JLinkList<CAIBrain> *m_llAIBrains;

protected:
private:
    float m_fTicks; // Ticks since last update
};

#endif // __AIMGR_H__
