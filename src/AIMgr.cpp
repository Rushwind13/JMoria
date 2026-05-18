#include "AIMgr.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
#include "JMDefs.h"
#include "Monster.h"
#include "MonsterRecall.h"
#include "Player.h"

CAIBrain::CAIBrain()
    : m_dwMoveType( 0 ),
      m_fSpeed( 0.0f ),
      m_nEffectTurns( 0 ),
      m_eBrainState( BRAINSTATE_INVALID ),
      m_pEligibleAttacks( new JLinkList<CAttack>( false ) ),
      m_vTargetPos( 0, 0 )
{
}

CAIBrain::~CAIBrain()
{
    if( m_pEligibleAttacks )
    {
        m_pEligibleAttacks->Terminate();
        delete m_pEligibleAttacks;
        m_pEligibleAttacks = NULL;
    }
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
    case BRAINSTATE_ATTACK:
        return UpdateAttack( fCurTime );
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
    // Afraid monsters flee from the player regardless of their movement type.
    // Empty-minded creatures are immune to fear.
    if( ( m_pParent->m_dwActiveEffects & EFFECT_FLAG_AFRAID ) &&
        !m_pParent->IsImmuneToEffect( EFFECT_FLAG_AFRAID ) )
    {
        JVector vPlayerPos = g_pGame->GetPlayer()->m_vPos;
        float dx = m_vPos.x - vPlayerPos.x;
        float dy = m_vPos.y - vPlayerPos.y;
        m_vTargetPos.Init( m_vPos.x + ( dx > 0 ? 5 : ( dx < 0 ? -5 : 0 ) ),
                           m_vPos.y + ( dy > 0 ? 5 : ( dy < 0 ? -5 : 0 ) ) );
        WalkSeek( fCurTime );
        SetState( BRAINSTATE_GOTODEST );
        return true;
    }

    // Aggravated monsters seek the effect source.  Clear once they reach it or
    // regain sight of the player, then fall through to normal AI.
    if( m_pParent->m_dwActiveEffects2 & EFFECT_FLAG_AGGRAVATE )
    {
        JIVector vPlayerPos( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
        JIVector vPos( VEC_EXPAND( m_vPos ) );
        JIVector vTarget( VEC_EXPAND( m_vTargetPos ) );
        bool bReachedTarget = Util::Taxicab( vPos, vTarget, 1 );
        bool bSeesPlayer = g_pGame->GetDungeon()->CanSeeEachOther( vPlayerPos, vPos );
        if( bReachedTarget || bSeesPlayer )
        {
            m_pParent->m_dwActiveEffects2 &= ~EFFECT_FLAG_AGGRAVATE;
            // Fall through: normal AI picks up from here.
        }
        else
        {
            WalkSeek( fCurTime );
            SetState( BRAINSTATE_GOTODEST );
            return true;
        }
    }

    BuildEligibleAttacks();
    if( ChooseAction( fCurTime ) )
        return true;

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

bool CAIBrain::UpdateRest( float fCurTime )
{
    // Sleeping or paralyzed monsters are held in REST state; tick down the effect duration.
    const uint32 dwStunFlags = EFFECT_FLAG_SLEEP | EFFECT_FLAG_PARALYZE;
    if( m_pParent->m_dwActiveEffects & dwStunFlags )
    {
        m_fStateTicks += fCurTime * m_fSpeed;
        while( m_fStateTicks >= 1.0f )
        {
            m_fStateTicks -= 1.0f;
            if( m_nEffectTurns > 0 )
            {
                m_nEffectTurns--;
                if( m_nEffectTurns == 0 )
                {
                    m_pParent->m_dwActiveEffects &= ~dwStunFlags;
                    SetState( BRAINSTATE_SEEK );
                }
            }

            // Proximity wake check: sleeping monsters only (not paralyzed)
            if( m_pParent->m_dwActiveEffects & EFFECT_FLAG_SLEEP )
            {
                JVector vDiff = m_vPos - g_pGame->GetPlayer()->m_vPos;
                float fDist = vDiff.Length(); // Length() returns squared distance

                float fStealth = g_pGame->GetPlayer()->GetStealth();
                CRoom *pRoom = g_pGame->GetDungeon()->InRoom( m_vPos );
                bool inLitRoom = pRoom && pRoom->HasFlags( DUNG_FLAG_LIT );

                float fWakeRange = SLEEP_BASE_WAKE_RANGE + fStealth +
                                   ( inLitRoom ? SLEEP_LIT_ROOM_PENALTY : 0.0f );
                if( fWakeRange < 0.0f )
                    fWakeRange = 0.0f;
                fWakeRange *= fWakeRange; // compare squared distances to avoid sqrt

                if( fDist < fWakeRange && Util::GetRandom( 0.0f, fDist ) <
                                              SLEEP_WAKE_CHANCE_SCALE * ( fWakeRange - fDist ) )
                {
                    m_pParent->m_dwActiveEffects &= ~EFFECT_FLAG_SLEEP;
                    SetState( BRAINSTATE_SEEK );
                    g_pGame->GetMsgs()->Printf( "The %s wakes up!\n", m_pParent->GetName() );
                }
            }
        }
        return true;
    }
    return true;
}

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
        case DUNG_IDX_DOOR:
        {
            CDungeonTile *pDoorTile = g_pGame->GetDungeon()->GetTile( vTryPos );
            bool bLocked = pDoorTile && pDoorTile->HasFlags( DUNG_FLAG_LOCKED );
            bool bHasHands = m_pParent->m_md && ( m_pParent->m_md->m_dwFlags & MON_FLAG_HANDS );
            bool bIsLarge = m_pParent->m_md && ( m_pParent->m_md->m_dwFlags & MON_FLAG_LARGE );

            JVector vPlayerPos = g_pGame->GetPlayer()->m_vPos;
            JVector vDiff = vTryPos - vPlayerPos;
            bool bPlayerNearby =
                vDiff.Length() <= ( DOOR_OPEN_NOISE_RADIUS * DOOR_OPEN_NOISE_RADIUS );

            if( bIsLarge )
            {
                // Large monsters bash through any door (locked or not)
                pDoorTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_BROKEN_DOOR );
                pDoorTile->UnsetFlags( DUNG_FLAG_LOCKED );
                g_pGame->GetDungeon()->Aggravate( vTryPos );
                g_pGame->GetDungeon()->DisturbPlayer();
                if( bPlayerNearby )
                    g_pGame->GetMsgs()->Printf( "You hear a door smash open.\n" );
                Move();
            }
            else if( bHasHands && !bLocked )
            {
                // Handed monsters open unlocked doors
                g_pGame->GetDungeon()->Modify( vTryPos );
                g_pGame->GetDungeon()->Aggravate( vTryPos );
                if( bPlayerNearby )
                    g_pGame->GetMsgs()->Printf( "You hear a door creak open.\n" );
                Move();
            }
            else
            {
                JLog( LOG_LEVEL_NOISE, true, "it hits a %d ", dwCollideType );
            }
            break;
        }
        default:
            JLog( LOG_LEVEL_NOISE, true, "it hits a %d ", dwCollideType );
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
            g_pGame->RecallMonster()->RecordAttackObservation( m_pParent->m_md->m_szName,
                                                               pAtk ? pAtk->m_dwType : 0, fDamage,
                                                               szEffect, dwElement );
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

void CAIBrain::BuildEligibleAttacks()
{
    m_pEligibleAttacks->Terminate();
    if( !m_pParent || !m_pParent->m_md || !m_pParent->m_md->m_llAttacks )
        return;

    JIVector vPlayer( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
    JIVector vPos( VEC_EXPAND( m_vPos ) );

    bool bAdjacent = Util::Taxicab( vPlayer, vPos, 1 );

    bool bInLOS = g_pGame->GetDungeon()->CanSeeEachOther( vPos, vPlayer );

    CLink<CAttack> *pLink = m_pParent->m_md->m_llAttacks->GetHead();
    while( pLink )
    {
        CAttack *pAtk = pLink->m_lpData;
        if( pAtk )
        {
            bool bRanged =
                pAtk->m_pEffect && pAtk->m_pEffect->m_ed && pAtk->m_pEffect->m_ed->m_fRange > 0.0f;
            if( bAdjacent || ( bInLOS && bRanged ) )
                m_pEligibleAttacks->Add( pAtk );
        }
        pLink = m_pParent->m_md->m_llAttacks->GetNext( pLink );
    }
}

bool CAIBrain::ChooseAction( float fCurTime )
{
    int nMoveW, nAttackW, nIdleW;
    switch( m_dwMoveType )
    {
    case MON_AI_SEEKPLAYER:
        nMoveW = 24;
        nAttackW = 75;
        nIdleW = 1;
        break;
    case MON_AI_75RANDOMMOVE:
        nMoveW = 40;
        nAttackW = 55;
        nIdleW = 5;
        break;
    case MON_AI_100RANDOMMOVE:
        nMoveW = 60;
        nAttackW = 30;
        nIdleW = 10;
        break;
    case MON_AI_DONTMOVE:
        nMoveW = 1;
        nAttackW = 75;
        nIdleW = 19;
        break;
    default:
        nMoveW = 50;
        nAttackW = 40;
        nIdleW = 10;
        break;
    }

    int nRoll = Util::GetRandom( 1, 100 );
    if( m_pEligibleAttacks->length() > 0 && nRoll <= nAttackW )
    {
        m_vTargetPos = g_pGame->GetPlayer()->m_vPos;
        SetState( BRAINSTATE_ATTACK );
        return true;
    }

    // Redistribute idle vs. move over remaining probability space.
    int nDenom = nMoveW + nIdleW;
    int nIdleThresh = ( nDenom > 0 ) ? ( nIdleW * 100 / nDenom ) : 0;
    nRoll = Util::GetRandom( 1, 100 );
    if( nRoll <= nIdleThresh )
    {
        SetState( BRAINSTATE_IDLE );
        return true;
    }

    return false; // fall through to brain-type move switch
}

bool CAIBrain::UpdateAttack( float fCurTime )
{
    if( m_pEligibleAttacks->length() == 0 )
    {
        SetState( BRAINSTATE_SEEK );
        return true;
    }

    int nIdx = Util::GetRandom( 0, m_pEligibleAttacks->length() - 1 );
    CAttack *pAtk = m_pEligibleAttacks->GetNthLink( nIdx )->m_lpData;
    m_pParent->m_pCurrentAttack = pAtk;

    bool bRanged =
        pAtk->m_pEffect && pAtk->m_pEffect->m_ed && pAtk->m_pEffect->m_ed->m_fRange > 0.0f;
    if( bRanged )
    {
        JVector vMonPos( m_vPos.x, m_vPos.y );
        JVector vPlayerPos = g_pGame->GetPlayer()->m_vPos;
        pAtk->m_pEffect->DoHitEffects( vMonPos, vPlayerPos );
        g_pGame->GetMsgs()->Printf( "The %s %s you.\n", m_pParent->GetName(),
                                    m_pParent->AttackFlavorText() );
    }
    else
    {
        CollideWithPlayer();
    }

    m_pParent->AttackDone();
    SetState( BRAINSTATE_SEEK );
    return true;
}
