#include "AIMgr.h"
#include "JMDefs.h"
#include "Dungeon.h"

CAIMgr::CAIMgr()
{
}

void CAIMgr::Init()
{
	m_llAIBrains = new JLinkList<CAIBrain>;
}

bool CAIMgr::Update( float fCurTime )
{
	m_fTicks += fCurTime;

	CLink <CAIBrain> *pLink = m_llAIBrains->GetHead();

	if( pLink == NULL )
	{
		return true;
	}

	while( pLink != NULL )
	{
		pLink->m_lpData->Update(fCurTime);
		pLink = m_llAIBrains->GetNext(pLink);
	}
	return true;
}


CAIBrain::CAIBrain()
: m_dwMoveType(0),
m_fSpeed(0.0f),
m_eBrainState(BRAINSTATE_INVALID)
{
}

bool CAIBrain::Update( float fCurTime )
{
    m_pParent->m_fColorChangeInterval += fCurTime;
	switch( m_eBrainState )
	{
	case BRAINSTATE_REST:
		return UpdateRest(fCurTime);
		break;
	case BRAINSTATE_GOTODEST:
		return UpdateGoToDest(fCurTime);
		break;
	case BRAINSTATE_SEEK:
		return UpdateSeek(fCurTime);
		break;
	case BRAINSTATE_IDLE:
		return UpdateIdle(fCurTime);
		break;
	default:
		return true;
		break;
	}

	return false;
}

bool CAIBrain::UpdateSeek( float fCurTime )
{
	switch( m_dwMoveType )
	{
	case MON_AI_100RANDOMMOVE:
		{
			SetRandomDest( fCurTime );
		}
		break;
	case MON_AI_75RANDOMMOVE:
		{
		}
		break;
	case MON_AI_DONTMOVE:
		{
		}
		break;
	}

	SetState( BRAINSTATE_GOTODEST );
	return true;
}

bool CAIBrain::UpdateRest( float fCurTime )
{
	return true;
}

bool CAIBrain::UpdateGoToDest( float fCurTime )
{
	m_fStateTicks += fCurTime * m_fSpeed;

	if( m_fStateTicks >= 1.0f )
	{
        JVector vTryPos(m_vPos+m_vVel);
        if( Util::IsInWorld(vTryPos) && (g_pGame->GetDungeon()->IsWalkableFor(vTryPos) == DUNG_COLL_NO_COLLISION) )
        {
            g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurMonster = NULL;
            m_vPos += m_vVel;
            g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurMonster = m_pParent;
            SetState( BRAINSTATE_SEEK );
        }
	}
	/*JVector delta;
	delta *= (m_fSpeed * fCurTime);

	JVector junk = m_vPos + delta;
	if( Util::IsInWorld(junk) )
	{
		printf( "moving from <%f %f> to <%f %f>...\n", VEC_EXPAND(m_vPos), VEC_EXPAND(m_vPos+delta) );
		m_vPos += delta;
	}/* */

	return true;
}

bool CAIBrain::UpdateIdle( float fCurTime )
{
	return true;
}

bool CAIBrain::SetRandomDest(float fCurTime)
{
	JFVector delta, dest;
	delta.Init( (float)(Util::GetRandom(-1,1)), (float)(Util::GetRandom(-1,1)) );

	dest = m_vPos + delta;

	if( Util::IsInWorld(dest) && (g_pGame->GetDungeon()->IsWalkableFor(dest) == DUNG_COLL_NO_COLLISION) )
	{
		//printf( "dest <%f %f> delta <%f %f>\n", VEC_EXPAND(dest), VEC_EXPAND(delta) );
		//m_vDest = dest;
		m_vVel = delta;
	}
	else
	{
		//printf( "velocity killed. dest <%f %f> delta <%f %f>\n", VEC_EXPAND(dest), VEC_EXPAND(delta) );
		m_vVel.Init();
	}

	return true;
}

bool CAIBrain::WalkSeek(float fCurTime)
{
	return true;
}
