#ifndef __AIMGR_H__
#define __AIMGR_H__

#include "JLinkList.h"
#include "JMDefs.h"

#define AI_TURNS_PER_HP 16

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
    JFVector m_vPos;
    CLink<CAIBrain> *m_pllLink;

    bool SetRandomDest( float fCurTime );
    bool WalkSeek( float fCurTime );
    bool WalkSeek( float fCurTime, JVector &vPlayerPos, int dwCollideType );
    bool Update( float fCurTime );

    bool UpdateRest( float fCurTime );
    bool UpdateIdle( float fCurTime );
    bool UpdateGoToDest( float fCurTime );
    bool UpdateSeek( float fCurTime );

    void SetState( eBrainState newState );
    void SetParent( CMonster *newParent ) { m_pParent = newParent; };

    eBrainState GetState() { return m_eBrainState; };
    const char *GetStateString() { return m_szState; };
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
    const char *m_szState;

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
