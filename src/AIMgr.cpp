#include "AIMgr.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
#include "JMDefs.h"
#include "MonsterRecall.h"
#include "Player.h"

CAIBrain::CAIBrain()
    : m_dwMoveType( 0 ),
      m_fSpeed( 0.0f ),
      m_eBrainState( BRAINSTATE_INVALID ),
      m_vTargetPos( 0, 0 )
{
}

CAIMgr::~CAIMgr()
{
    if( m_llAIBrains )
    {
        m_llAIBrains->Terminate();
        delete m_llAIBrains;
        m_llAIBrains = NULL;
    }
}

void CAIMgr::Init() { m_llAIBrains = new JLinkList<CAIBrain>; }

bool CAIMgr::Update( float fCurTime )
{
    m_fTicks += fCurTime;

    CLink<CAIBrain> *pLink = m_llAIBrains->GetHead();

    if( pLink == NULL )
    {
        return true;
    }

    while( pLink != NULL )
    {
        pLink->m_lpData->Update( fCurTime );
        pLink = m_llAIBrains->GetNext( pLink );
    }
    return true;
}

void CAIMgr::DestroyBrain( CAIBrain *delete_me ) { m_llAIBrains->Remove( delete_me->m_pllLink ); }

bool CAIBrain::Update( float fCurTime )
{
    if( m_vPos.IsZero() )
    {
        JLog( LOG_LEVEL_DEBUG, true, "%s has a bad position...\n", m_pParent->m_md->m_szName );
        // g_pGame->GetDungeon()->RemoveMonster( m_pParent);

        return false;
    }

    if( g_pGame->GetTime() > m_pParent->m_fLastHPTime + AI_TURNS_PER_HP )
    {
        m_pParent->m_fCurHP++;
        if( m_pParent->m_fCurHP > m_pParent->m_fHP )
        {
            m_pParent->m_fCurHP = m_pParent->m_fHP;
        }
        m_pParent->m_fLastHPTime = g_pGame->GetTime();
    }
    m_pParent->m_fColorChangeInterval += fCurTime;
    m_pParent->Breed();
    switch( m_eBrainState )
    {
    case BRAINSTATE_REST:
        return UpdateRest( fCurTime );
        break;
    case BRAINSTATE_GOTODEST:
        JLog( LOG_LEVEL_NOISE, true, "I'm going, I'm going\n" );
        return UpdateGoToDest( fCurTime );
        break;
    case BRAINSTATE_SEEK:
        JLog( LOG_LEVEL_NOISE, true, "I seek\n" );
        return UpdateSeek( fCurTime );
        break;
    case BRAINSTATE_IDLE:
        return UpdateIdle( fCurTime );
        break;
    default:
        return true;
        break;
    }

    if( !g_pGame->GetPlayer()->m_bIsDisturbed )
    {
        uint32 dwFlags = m_pParent->m_md->m_dwFlags & ( MON_FLAG_WARM | MON_FLAG_EMPTY_MIND );
        if( g_pGame->GetDungeon()->PlayerCanSee( m_vPos, dwFlags ) )
        {
            g_pGame->GetDungeon()->DisturbPlayer();
        }
    }

    return false;
}

bool CAIBrain::UpdateSeek( float fCurTime )
{
    switch( m_dwMoveType )
    {
    case MON_AI_100RANDOMMOVE:
    {
        JLog( LOG_LEVEL_NOISE, true, "rng\n" );
        SetRandomDest( fCurTime );
    }
    break;
    case MON_AI_75RANDOMMOVE:
    {
        JLog( LOG_LEVEL_NOISE, true, "chaos monkey\n" );
        if( Util::Roll( "1d100" ) <= 75 )
        {
            SetRandomDest( fCurTime );
        }
        else
        {
            WalkSeek( fCurTime );
        }
    }
    break;
    case MON_AI_DONTMOVE:
    {

        JLog( LOG_LEVEL_NOISE, true, "i no move\n" );
        SetRandomDest( fCurTime );
    }
    break;
    case MON_AI_SEEKPLAYER:
    {
        JLog( LOG_LEVEL_NOISE, true, "seek player\n" );
        m_vTargetPos = g_pGame->GetPlayer()->m_vPos;
        WalkSeek( fCurTime );
    }
    break;
    default:
        JLog( LOG_LEVEL_NOISE, true, "bad seek\n" );
        return false;
    }

    SetState( BRAINSTATE_GOTODEST );
    return true;
}

bool CAIBrain::UpdateRest( float fCurTime ) { return true; }

bool CAIBrain::UpdateGoToDest( float fCurTime )
{
    bool didWalk = false;
    m_fStateTicks += fCurTime * m_fSpeed;

    while( m_fStateTicks >= 1.0f )
    {
        m_fStateTicks -= 1.0f;
        didWalk = true;
        if( m_vVel.IsZero() )
            continue;
        JVector vTryPos( m_vPos + m_vVel );
        JLog( LOG_LEVEL_NOISE, true,
              "and we're walking... <%.2f %.2f> + <%.2f %.2f> = <%.2f %.2f> ", VEC_EXPAND( m_vPos ),
              VEC_EXPAND( m_vVel ), VEC_EXPAND( vTryPos ) );
        if( !Util::IsInWorld( vTryPos ) )
        {
            continue;
        }

        JLog( LOG_LEVEL_NOISE, true, "and we're in the world... " );
        int dwCollideType = g_pGame->GetDungeon()->IsWalkableFor( vTryPos );
        switch( dwCollideType )
        {
        case DUNG_COLL_NO_COLLISION:
            Move();
            break;
        case DUNG_COLL_PLAYER:
            CollideWithPlayer();
            break;
        default:
            JLog( LOG_LEVEL_NOISE, true, "you hit %d ", dwCollideType );
            break;
        }
    }

    JLog( LOG_LEVEL_NOISE, false, "\n" );
    if( didWalk )
    {
        JLog( LOG_LEVEL_NOISE, true, "looking\n" );
        SetState( BRAINSTATE_SEEK );
    }

    return true;
}

void CAIBrain::Move()
{
    JLog( LOG_LEVEL_NOISE, true, "swing and a miss " );
    if( m_dwMoveType != MON_AI_DONTMOVE )
    {
        JLog( LOG_LEVEL_NOISE, true, "on the move " );
        g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurMonster = NULL;
        m_vPos += m_vVel;
        g_pGame->GetDungeon()->GetTile( m_vPos )->m_pCurMonster = m_pParent;
    }
}

void CAIBrain::CollideWithPlayer()
{
    JLog( LOG_LEVEL_NOISE, true, "ouch! you ran into the player! " );
    char szStatus[32];
    float fDamageMult = 1.0f;
    // TODO: make this use all the attacks, not just the first one
    m_pParent->ChooseAttack();
    float fRoll = m_pParent->Attack();
    bool bHit = g_pGame->GetPlayer()->Hit( fRoll );
    if( bHit )
    {
        // TODO: make this differ based on attack effect type
        Util::jstrcpy( szStatus, m_pParent->AttackFlavorText() );
    }
    else
    {
        Util::jstrcpy( szStatus, "misses" );
    }
    g_pGame->GetMsgs()->Printf( "The %s %s you.\n", m_pParent->GetName(), szStatus );

    if( bHit )
    {
        if( fRoll > 80.0f )
        {
            g_pGame->GetMsgs()->Printf( "(It was an excellent hit! (x2 damage)\n" );
            fDamageMult = 2.0f;
        }

        float fDamage = m_pParent->Damage( fDamageMult );
        CAttack *pAtk = m_pParent->m_pCurrentAttack;
        uint32 dwElement = ( pAtk && pAtk->m_pEffect ) ? pAtk->m_pEffect->m_dwFlags : 0;
        g_pGame->GetPlayer()->TakeDamage( fDamage, m_pParent->GetName(), dwElement );
        if( g_pGame->RecallMonster() && m_pParent->m_md )
        {
            const char *szEffect = ( pAtk && pAtk->m_pEffect ) ? m_pParent->AttackEffect() : "";
            g_pGame->RecallMonster()->RecordAttackObservation(
                m_pParent->m_md->m_szName, pAtk ? pAtk->m_dwType : 0, fDamage, szEffect );
        }
        m_pParent->AttackDone();
    }
}

bool CAIBrain::UpdateIdle( float fCurTime ) { return true; }

bool CAIBrain::SetRandomDest( float fCurTime )
{
    JFVector delta;
    delta.Init( (int)( Util::GetRandom( -2.0f, 2.0f ) ), (int)( Util::GetRandom( -2.0f, 2.0f ) ) );

    JLog( LOG_LEVEL_NOISE, true, "and we're rnging... <%.2f %.2f> + <%.2f %.2f> = <%.2f %.2f> ",
          VEC_EXPAND( m_vPos ), VEC_EXPAND( delta ), VEC_EXPAND( m_vPos + delta ) );

    return GotoDest( fCurTime, delta );
}

bool CAIBrain::WalkSeek( float fCurTime )
{
    JVector delta( 0, 0 );

    float x_delta = m_vTargetPos.x - m_vPos.x;
    float y_delta = m_vTargetPos.y - m_vPos.y;

    if( x_delta > 1 )
        x_delta = 1;
    if( x_delta < -1 )
        x_delta = -1;
    if( y_delta > 1 )
        y_delta = 1;
    if( y_delta < -1 )
        y_delta = -1;

    delta.Init( x_delta, y_delta );

    JLog( LOG_LEVEL_NOISE, true, "and we're seeking... <%.2f %.2f> + <%.2f %.2f> = <%.2f %.2f> ",
          VEC_EXPAND( m_vPos ), VEC_EXPAND( delta ), VEC_EXPAND( m_vPos + delta ) );

    return GotoDest( fCurTime, delta );
}

bool CAIBrain::GotoDest( float fCurTime, JVector &delta )
{
    JLog( LOG_LEVEL_NOISE, true, "and we're going to dest... <%.2f %.2f> + <%.2f %.2f> = <%.2f %.2f> ",
          VEC_EXPAND( m_vPos ), VEC_EXPAND( delta ), VEC_EXPAND( m_vPos + delta ) );
    int dwCollideType = DUNG_COLL_NO_COLLISION;
    JVector dest = m_vPos + delta;

    if( dest.IsInWorld() )
    {
        JLog( LOG_LEVEL_NOISE, true, "and we're in the world... %d, %d", &g_pGame, g_pGame->GetDungeon()
     );
        dwCollideType = g_pGame->GetDungeon()->IsWalkableFor( dest );

        JLog( LOG_LEVEL_NOISE, true, "and we collide with %d... ", dwCollideType );
        switch( dwCollideType )
        {
        case DUNG_COLL_NO_COLLISION:
        case DUNG_COLL_PLAYER:
            m_vVel = delta;
            JLog( LOG_LEVEL_NOISE, true, "hit %d change course <%.2f %.2f>\n", dwCollideType,
                  VEC_EXPAND( m_vVel ) );
            break;
        default:
            JLog( LOG_LEVEL_NOISE, true, "hit %d steady course <%.2f %.2f>\n", dwCollideType,
                  VEC_EXPAND( m_vVel ) );
            // m_vVel.Init();
            break;
        }
    }
    else
    {
        JLog( LOG_LEVEL_NOISE, true, "and we're out of the world... steady course <%.2f %.2f>\n",
              VEC_EXPAND( m_vVel ) );
        // delta.Init();
    }

    JLog( LOG_LEVEL_NOISE, true, "\n" );
    return true;
}
