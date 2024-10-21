#ifndef __AIMGR_H__
#define __AIMGR_H__

#include "JMDefs.h"
#include "JLinkList.h"

#define AI_TURNS_PER_HP 16

// all the states which the game can run in
// this modifies the event handling
enum eBrainState
{
	BRAINSTATE_INVALID=-1,
	BRAINSTATE_REST=0,
	BRAINSTATE_GOTODEST=1,
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
    CLink <CAIBrain> *m_pllLink;

	bool Update( float fCurTime );

	bool SetRandomDest(float fCurTime);
	bool WalkSeek(float fCurTime);

	bool UpdateRest( float fCurTime );
	bool UpdateIdle( float fCurTime );
	bool UpdateGoToDest( float fCurTime );
	bool UpdateSeek( float fCurTime );

	void SetState( eBrainState newState ) { m_eBrainState = newState; m_fStateTicks = 0.0f; }
	void SetParent( CMonster *newParent ) { m_pParent = newParent; }
protected:
	float m_fStateTicks;
	JVector m_vVel;
	eBrainState m_eBrainState;

    CMonster *m_pParent;
};

class CAIMgr
{
public:
	CAIMgr();
	virtual ~CAIMgr() {}
	void Init();

	bool Update( float fCurTime );
    void DestroyBrain( CAIBrain *delete_me );
	JLinkList<CAIBrain> *m_llAIBrains;
protected:
private:
	float m_fTicks; // Ticks since last update
};

#endif // __AIMGR_H__
