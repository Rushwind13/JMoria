#include "AIMgr.h"
#include "JMDefs.h"
#include "Dungeon.h"
#include "Player.h"
#include "DisplayText.h"

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

void CAIMgr::DestroyBrain( CAIBrain *delete_me )
{
    m_llAIBrains->Remove(delete_me->m_pllLink);
}


CAIBrain::CAIBrain()
: m_dwMoveType(0),
m_fSpeed(0.0f),
m_eBrainState(BRAINSTATE_INVALID)
{
}

bool CAIBrain::Update( float fCurTime )
{
    if( g_pGame->GetTime() >  m_pParent->m_fLastHPTime + AI_TURNS_PER_HP )
    {
        m_pParent->m_fCurHP++;
        if( m_pParent->m_fCurHP > m_pParent->m_fHP )
        {
            m_pParent->m_fCurHP = m_pParent->m_fHP;
        }
        m_pParent->m_fLastHPTime = g_pGame->GetTime();
    }
    m_pParent->m_fColorChangeInterval += fCurTime;
    m_pParent->m_fBreedInterval += fCurTime;
    m_pParent->Breed();
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
            SetRandomDest( fCurTime );
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
    bool didWalk = false;
	m_fStateTicks += fCurTime * m_fSpeed;

	while( m_fStateTicks >= 1.0f )
	 {
        didWalk = true;
        JVector vTryPos(m_vPos+m_vVel);
        if( Util::IsInWorld(vTryPos) )
        {
            int dwCollideType = g_pGame->GetDungeon()->IsWalkableFor(vTryPos);
            switch(dwCollideType)
            {
            case DUNG_COLL_NO_COLLISION:
                if( m_dwMoveType != MON_AI_DONTMOVE )
                {
                    g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurMonster = NULL;
                    m_vPos += m_vVel;
                    g_pGame->GetDungeon()->GetTile(m_vPos)->m_pCurMonster = m_pParent;
                }
                break;
            case DUNG_COLL_PLAYER:
                char szStatus[16];
                float fDamageMult = 1.0f;
                // TODO: make this use all the attacks, not just the first one
                float fRoll = m_pParent->Attack();
                bool bHit = g_pGame->GetPlayer()->Hit(fRoll);
                if( bHit )
                {
                    // TODO: make this differ based on attack effect type
                    sprintf(szStatus, "hits");
                }
                else
                {
                    sprintf(szStatus, "misses");
                }
                g_pGame->GetMsgs()->Printf( "The %s %s you.\n", m_pParent->GetName(), szStatus );

                if(bHit)
                {
                    if( fRoll > 80.0f )
                    {
                        g_pGame->GetMsgs()->Printf( "(It was an excellent hit! (x2 damage)\n" );
                        fDamageMult = 2.0f;
                    }

                    float fDamage = m_pParent->Damage(fDamageMult);
                    g_pGame->GetPlayer()->TakeDamage(fDamage, m_pParent->GetName());
                }
                break;
            }
        }
        m_fStateTicks -= 1.0f;
	}
	/*JVector delta;
	delta *= (m_fSpeed * fCurTime);

	JVector junk = m_vPos + delta;
	if( Util::IsInWorld(junk) )
	{
		printf( "moving from <%f %f> to <%f %f>...\n", VEC_EXPAND(m_vPos), VEC_EXPAND(m_vPos+delta) );
		m_vPos += delta;
     }/* */
    if( didWalk )
    {
        SetState( BRAINSTATE_SEEK );
    }

	return true;
}

bool CAIBrain::UpdateIdle( float fCurTime )
{
	return true;
}

bool CAIBrain::SetRandomDest(float fCurTime)
{
	JFVector delta, dest;
	delta.Init( (float)(Util::GetRandom(-1, 1)), (float)(Util::GetRandom(-1, 1)) );

	dest = m_vPos + delta;

	if( Util::IsInWorld(dest) )
    {
        int dwCollideType = g_pGame->GetDungeon()->IsWalkableFor(dest);
        switch(dwCollideType)
        {
        case DUNG_COLL_NO_COLLISION:
        case DUNG_COLL_PLAYER:
            m_vVel = delta;
            break;
        default:
            m_vVel.Init();
            break;
        }
    }

	return true;
}

bool CAIBrain::WalkSeek(float fCurTime)
{
	return true;
}
