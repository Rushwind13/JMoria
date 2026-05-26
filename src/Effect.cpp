#include "Effect.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Item.h"
#include "Player.h"
void CEffect::SetAmount( const char *szAmount )
{
    if( m_szAmount )
    {
        delete[] m_szAmount;
        m_szAmount = NULL;
    }
    if( szAmount )
    {
        int len = Util::jstrlen( szAmount );
        m_szAmount = new char[len + 1];
        memset( m_szAmount, 0, len + 1 );
        Util::jstrcpy( m_szAmount, szAmount );
    }
}

float CEffect::Resist() const
{
    if( m_dwModifier & EFFECT_MOD_IMMUNE )
        return 0.0f;
    if( m_dwModifier & EFFECT_MOD_RESIST )
        return 0.5f;
    if( m_dwModifier & EFFECT_MOD_WEAK )
        return 2.0f;
    return 1.0f;
}

bool CEffect::HasFlag( const char *szFlag )
{
    return g_Constants.CheckEffectFlag( szFlag, m_dwFlags, m_dwFlags2 );
}

const char *CEffect::Effect()
{
    switch( m_dwFlags )
    {
    case EFFECT_FLAG_FIRE:
        return "fire";
    case EFFECT_FLAG_COLD:
        return "cold";
    case EFFECT_FLAG_ELECTRICITY:
        return "electricity";
    case EFFECT_FLAG_ACID:
        return "acid";
    case EFFECT_FLAG_POISON:
        return "poison gas";
    default:
        return "energy";
    }
}

JResult CEffect::Physical( JVector vOrigin )
{
    bool bCriticalHit = false;
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    CPlayer *pPlayer = g_pGame->GetPlayer();
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }
    const char *szMonName = pMon->GetName();
    if( pPlayer->m_pCurrentRangedAmmo )
    {
        CLink<CItem> *pArrowLink = NULL;
        CLink<CItem> *pInvItem = pPlayer->m_llInventory->GetHead();
        while( pInvItem )
        {
            if( pInvItem->m_lpData == pPlayer->m_pCurrentRangedAmmo )
            {
                pArrowLink = pInvItem;
                break;
            }
            pInvItem = pPlayer->m_llInventory->GetNext( pInvItem );
        }

        float fRoll = pPlayer->RangedAttack( pArrowLink );
        bool bHit = pMon->Hit( fRoll );

        if( !bHit )
        {
            g_pGame->GetMsgs()->Printf( "You miss the %s.\n", szMonName );
            JLog( LOG_LEVEL_INFO, true, "Ranged miss: roll %.2f vs AC %d\n", fRoll,
                  (int)pMon->m_fCurAC );
            return JSUCCESS;
        }

        if( fRoll > 80.0f )
        {
            g_pGame->GetMsgs()->Printf( "(Critical hit!)\n" );
            bCriticalHit = true;
        }
    }

    const char *szAmount = m_szAmount;
    if( !szAmount && m_ed )
        szAmount = m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "1d2";

    float fDamage = Util::Roll( szAmount );

    if( pPlayer->m_pCurrentRangedAmmo )
        fDamage += pPlayer->m_pCurrentRangedAmmo->m_fBonusToDamage;

    if( bCriticalHit )
        fDamage *= 2.0f;

    if( pPlayer->DamageMonster( pMon, fDamage ) )
        g_pGame->GetMsgs()->Printf( "The %s dies.\n", szMonName );
    else
        g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );

    return JSUCCESS;
}

JResult CEffect::LightRay( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    const char *szMonName = pMon->GetName();
    JLog( LOG_LEVEL_INFO, true, "monster: %s\n", szMonName );

    if( ( pMon->m_md->m_dwFlags & MON_FLAG_HURT_BY_LIGHT ) == MON_FLAG_HURT_BY_LIGHT )
    {
        JLog( LOG_LEVEL_NOISE, true, "that's gonna hurt\n" );
        float fDamage = Util::Roll( "1d5" );
        if( g_pGame->GetPlayer()->DamageMonster( pMon, fDamage ) )
            g_pGame->GetMsgs()->Printf( "The %s shrivels away in the bright light!\n", szMonName );
        else
            g_pGame->GetMsgs()->Printf( "The %s screams in agony.\n", szMonName );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "The %s is unaffected.\n", szMonName );
    }

    return JSUCCESS;
}

JResult CEffect::Elemental( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    const char *szEffect = Effect();
    const char *szMonName = pMon->GetName();
    JLog( LOG_LEVEL_INFO, true, "elemental hit (%s) on %s\n", szEffect, szMonName );

    if( m_ed && m_ed->m_szName )
        g_pGame->GetMsgs()->Printf( "The %s strikes the %s with %s.\n", m_ed->m_szName, szMonName,
                                    szEffect );

    const char *szAmount = m_szAmount;
    if( !szAmount && m_ed )
        szAmount = m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "1d6";

    float fDamage = Util::Roll( szAmount );

    CPlayer *pPlayer = g_pGame->GetPlayer();
    if( pPlayer->m_pCurrentRangedAmmo )
        fDamage += pPlayer->m_pCurrentRangedAmmo->m_fBonusToDamage;

    float fAffinityMult = CheckAffinity( m_dwFlags, pMon->GetElementFlags() );
    if( fAffinityMult == 0.0f )
    {
        pMon->m_fCurHP += fDamage;
        if( pMon->m_fCurHP > pMon->m_fHP )
            pMon->m_fCurHP = pMon->m_fHP;
        g_pGame->GetMsgs()->Printf( "The %s shrugs off the attack!\n", szMonName );
        return JSUCCESS;
    }
    fDamage *= fAffinityMult;
    if( fAffinityMult > 1.0f )
        g_pGame->GetMsgs()->Printf( "The %s is especially vulnerable!\n", szMonName );

    if( pPlayer->DamageMonster( pMon, fDamage ) )
        g_pGame->GetMsgs()->Printf( "The %s is destroyed!\n", szMonName );
    else
        g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );

    return JSUCCESS;
}

JResult CEffect::Area( JVector vOrigin )
{
    const char *szAmount = m_szAmount;
    if( !szAmount && m_ed )
        szAmount = m_ed->m_szAmount;

    uint8 radius = m_ed ? (uint8)m_ed->m_fRadius : 5;
    JIVector vCenter( (int)vOrigin.x, (int)vOrigin.y );

    int nAffected = 0;
    CLink<CMonster> *pLink = g_pGame->GetDungeon()->m_llMonsters->GetHead();
    while( pLink != NULL )
    {
        CMonster *pMon = pLink->m_lpData;
        pLink = g_pGame->GetDungeon()->m_llMonsters->GetNext( pLink );

        if( !pMon )
            continue;

        JIVector vMonPos( (int)pMon->GetPos().x, (int)pMon->GetPos().y );
        if( !Util::WithinRadius( vCenter, vMonPos, radius ) )
            continue;

        if( m_dwFlags &
            ( EFFECT_FLAG_FIRE | EFFECT_FLAG_COLD | EFFECT_FLAG_ELECTRICITY | EFFECT_FLAG_ACID ) )
        {
            float fDamage = szAmount ? Util::Roll( szAmount ) : Util::Roll( "1d6" );
            const char *szMonName = pMon->GetName();

            float fAffinityMult = CheckAffinity( m_dwFlags, pMon->GetElementFlags() );
            if( fAffinityMult == 0.0f )
            {
                pMon->m_fCurHP += fDamage;
                if( pMon->m_fCurHP > pMon->m_fHP )
                    pMon->m_fCurHP = pMon->m_fHP;
                g_pGame->GetMsgs()->Printf( "The %s shrugs off the attack!\n", szMonName );
            }
            else
            {
                fDamage *= fAffinityMult;
                if( fAffinityMult > 1.0f )
                    g_pGame->GetMsgs()->Printf( "The %s is especially vulnerable!\n", szMonName );
                if( g_pGame->GetPlayer()->DamageMonster( pMon, fDamage ) )
                    g_pGame->GetMsgs()->Printf( "The %s is destroyed!\n", szMonName );
                else
                    g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );
            }
        }
        else
        {
            if( !CheckAffinity( m_dwFlags, pMon->m_md->m_dwFlags ) )
            {
                pMon->m_dwActiveEffects |= m_dwFlags;
                if( m_dwFlags & ( EFFECT_FLAG_SLEEP | EFFECT_FLAG_PARALYZE ) )
                {
                    pMon->m_pBrain->m_nEffectTurns = (int)Util::Roll( "3d6" );
                    pMon->m_pBrain->SetState( BRAINSTATE_REST );
                }
            }
            else
            {
                continue;
            }
        }
        nAffected++;
    }

    if( nAffected > 0 )
    {
        const char *szEffName = ( m_ed && m_ed->m_szName ) ? m_ed->m_szName : "something";
        g_pGame->GetMsgs()->Printf( "The %s affects %d creature%s.\n", szEffName, nAffected,
                                    nAffected == 1 ? "" : "s" );
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
    }

    return JSUCCESS;
}

JResult CEffect::Ball( JVector vOrigin )
{
    const char *szAmount = m_szAmount;
    if( !szAmount && m_ed )
        szAmount = m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "2d6";

    uint8 radius = m_ed ? (uint8)m_ed->m_fRadius : 2;
    JIVector vCenter( (int)vOrigin.x, (int)vOrigin.y );

    int nAffected = 0;
    CLink<CMonster> *pLink = g_pGame->GetDungeon()->m_llMonsters->GetHead();
    while( pLink != NULL )
    {
        CMonster *pMon = pLink->m_lpData;
        pLink = g_pGame->GetDungeon()->m_llMonsters->GetNext( pLink );

        if( !pMon )
            continue;

        JIVector vMonPos( (int)pMon->GetPos().x, (int)pMon->GetPos().y );
        if( !Util::WithinRadius( vCenter, vMonPos, radius ) )
            continue;

        float fDamage = Util::Roll( szAmount );
        const char *szMonName = pMon->GetName();

        float fAffinityMult = CheckAffinity( m_dwFlags, pMon->GetElementFlags() );
        if( fAffinityMult == 0.0f )
        {
            pMon->m_fCurHP += fDamage;
            if( pMon->m_fCurHP > pMon->m_fHP )
                pMon->m_fCurHP = pMon->m_fHP;
            g_pGame->GetMsgs()->Printf( "The %s shrugs off the attack!\n", szMonName );
            nAffected++;
            continue;
        }
        fDamage *= fAffinityMult;
        if( fAffinityMult > 1.0f )
            g_pGame->GetMsgs()->Printf( "The %s is especially vulnerable!\n", szMonName );
        if( g_pGame->GetPlayer()->DamageMonster( pMon, fDamage ) )
            g_pGame->GetMsgs()->Printf( "The %s is destroyed!\n", szMonName );
        else
            g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );
        nAffected++;
    }

    if( nAffected == 0 )
        g_pGame->GetMsgs()->Printf( "The ball explodes harmlessly.\n" );

    return JSUCCESS;
}

JResult CEffect::Line( JVector vOrigin )
{
    const char *szAmount = m_szAmount;
    if( !szAmount && m_ed )
        szAmount = m_ed->m_szAmount;
    if( !szAmount )
        szAmount = "2d6";

    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JSUCCESS;
    }

    float fDamage = Util::Roll( szAmount );
    const char *szMonName = pMon->GetName();
    const char *szEffect = Effect();
    if( m_ed && m_ed->m_szName )
        g_pGame->GetMsgs()->Printf( "The %s strikes the %s with %s.\n", m_ed->m_szName, szMonName,
                                    szEffect );

    float fAffinityMult = CheckAffinity( m_dwFlags, pMon->GetElementFlags() );
    if( fAffinityMult == 0.0f )
    {
        pMon->m_fCurHP += fDamage;
        if( pMon->m_fCurHP > pMon->m_fHP )
            pMon->m_fCurHP = pMon->m_fHP;
        g_pGame->GetMsgs()->Printf( "The %s shrugs off the attack!\n", szMonName );
        return JSUCCESS;
    }
    fDamage *= fAffinityMult;
    if( fAffinityMult > 1.0f )
        g_pGame->GetMsgs()->Printf( "The %s is especially vulnerable!\n", szMonName );

    if( g_pGame->GetPlayer()->DamageMonster( pMon, fDamage ) )
        g_pGame->GetMsgs()->Printf( "The %s is destroyed!\n", szMonName );
    else
        g_pGame->GetMsgs()->Printf( "The %s is hit.\n", szMonName );

    return JSUCCESS;
}

JResult CEffect::Status( JVector vOrigin, uint32 dwFlag )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        // Apply status effect to the player
        CPlayer *pPlayer = g_pGame->GetPlayer();
        if( vOrigin == pPlayer->m_vPos )
        {
            const char *szStatusName = "affected";
            if( dwFlag & EFFECT_FLAG_SLEEP )
                szStatusName = "fall asleep";
            else if( dwFlag & EFFECT_FLAG_PARALYZE )
                szStatusName = "are paralyzed";
            else if( dwFlag & EFFECT_FLAG_AFRAID )
                szStatusName = "flee in terror";
            else if( dwFlag & EFFECT_FLAG_CONFUSE )
                szStatusName = "feel confused";
            g_pGame->GetMsgs()->Printf( "You %s.\n", szStatusName );
            pPlayer->SetIntrinsic( dwFlag );
            return JSUCCESS;
        }
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    if( CheckAffinity( dwFlag, pMon->m_md->m_dwFlags ) == 0.0f )
    {
        g_pGame->GetMsgs()->Printf( "The %s is unaffected.\n", pMon->GetName() );
        return JSUCCESS;
    }

    pMon->m_dwActiveEffects |= dwFlag;

    if( dwFlag & ( EFFECT_FLAG_SLEEP | EFFECT_FLAG_PARALYZE ) )
    {
        pMon->m_pBrain->m_nEffectTurns = (int)Util::Roll( "3d6" );
        pMon->m_pBrain->SetState( BRAINSTATE_REST );
    }

    const char *szStatusName = "affected";
    if( dwFlag & EFFECT_FLAG_SLEEP )
        szStatusName = "falls asleep";
    else if( dwFlag & EFFECT_FLAG_PARALYZE )
        szStatusName = "is paralyzed";
    else if( dwFlag & EFFECT_FLAG_AFRAID )
        szStatusName = "flees in terror";
    else if( dwFlag & EFFECT_FLAG_CONFUSE )
        szStatusName = "looks confused";

    g_pGame->GetMsgs()->Printf( "The %s %s.\n", pMon->GetName(), szStatusName );
    return JSUCCESS;
}

JResult CEffect::StoneToMud( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile || !pTile->m_dtd )
        return JBOGUSKEY;

    JResult retval = JBOGUSKEY;

    // Damage rock-type monsters on the tile
    CMonster *pMon = pTile->m_pCurMonster;
    if( pMon && ( pMon->m_md->m_dwFlags & MON_FLAG_ROCK ) )
    {
        float fDamage = Util::Roll( "3d8" );
        const char *szMonName = pMon->GetName();
        if( g_pGame->GetPlayer()->DamageMonster( pMon, fDamage ) )
            g_pGame->GetMsgs()->Printf( "The %s crumbles to dust!\n", szMonName );
        else
            g_pGame->GetMsgs()->Printf( "The %s cracks!\n", szMonName );
        retval = JSUCCESS;
    }

    // Melt stone tiles (closed door, secret door, or wall; not open/broken doors)
    switch( pTile->m_dtd->m_dwType )
    {
    case DUNG_IDX_WALL:
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );
        g_pGame->GetMsgs()->Printf( "The wall turns to mud and collapses!\n" );
        retval = JSUCCESS;
        break;
    case DUNG_IDX_DOOR:
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );
        g_pGame->GetMsgs()->Printf( "The door dissolves!\n" );
        retval = JSUCCESS;
        break;
    case DUNG_IDX_SECRET_DOOR:
        pTile->m_dtd = g_pGame->GetDungeon()->GetTileDef( DUNG_IDX_FLOOR );
        g_pGame->GetMsgs()->Printf( "The secret door dissolves!\n" );
        retval = JSUCCESS;
        break;
    }

    if( retval == JBOGUSKEY )
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );

    return retval;
}

JResult CEffect::TeleportAway( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    const char *szMonName = pMon->GetName();
    pTile->m_pCurMonster = NULL;

    JIVector vNew = pMon->GetSpawnPoint( JIVector( -1, -1 ) );
    if( vNew.IsWithinWorld() )
    {
        CDungeonTile *pNewTile = g_pGame->GetDungeon()->GetTile( JVector( vNew.x, vNew.y ) );
        if( pNewTile )
        {
            pMon->SetPos( JVector( vNew.x, vNew.y ) );
            pNewTile->m_pCurMonster = pMon;
            g_pGame->GetMsgs()->Printf( "The %s vanishes!\n", szMonName );
            return JSUCCESS;
        }
    }

    pTile->m_pCurMonster = pMon;
    g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
    return JSUCCESS;
}

JResult CEffect::Probe( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    const CMonsterDef *pmd = pMon->m_md;
    g_pGame->GetMsgs()->Printf( "The %s: HP %d/%d  AC %d  Lvl %d  Spd %.1f\n", pMon->GetName(),
                                (int)pMon->m_fCurHP, (int)pMon->m_fHP, (int)pmd->m_fBaseAC,
                                pmd->m_dwLevel, pmd->m_fSpeed );
    return JSUCCESS;
}

JResult CEffect::HealMonster( JVector vOrigin )
{
    CDungeonTile *pTile = g_pGame->GetDungeon()->GetTile( vOrigin );
    if( !pTile )
        return JBOGUSKEY;

    CMonster *pMon = pTile->m_pCurMonster;
    if( !pMon )
    {
        JLog( LOG_LEVEL_NOISE, true, "no monster\n" );
        return JBOGUSKEY;
    }

    float fHeal = m_szAmount ? Util::Roll( m_szAmount ) : 10.0f;
    pMon->m_fCurHP += fHeal;
    if( pMon->m_fCurHP > pMon->m_fHP )
        pMon->m_fCurHP = pMon->m_fHP;

    g_pGame->GetMsgs()->Printf( "The %s looks healthier.\n", pMon->GetName() );
    return JSUCCESS;
}

JResult CEffect::DoHitEffects()
{
    CPlayer *pPlayer = g_pGame->GetPlayer();
    return DoHitEffects( pPlayer->m_vPos, pPlayer->GetRangedHitPosition() );
}

JResult CEffect::DoHitEffects( JVector vCasterPos, JVector vTargetPos )
{
    CPlayer *pPlayer = g_pGame->GetPlayer();
    if( m_dwModifier & EFFECT_MOD_AREA )
        return Area( vCasterPos );
    if( m_dwModifier & EFFECT_MOD_BALL )
        return Ball( vTargetPos );
    if( m_dwModifier & EFFECT_MOD_LINE )
        return Line( vTargetPos );

    // Word-2 flags dispatch before word-1 flag switch.
    if( HasFlag( "EFFECT_FLAG_AGGRAVATE" ) )
        return Aggravate( vCasterPos );
    if( m_dwFlags2 & EFFECT_FLAG_LOCK )
        return LockDoor( vTargetPos );

    switch( m_dwFlags )
    {
    case EFFECT_FLAG_LIGHT:
        return LightRay( vTargetPos );
    case EFFECT_FLAG_FIRE:
    case EFFECT_FLAG_COLD:
    case EFFECT_FLAG_ELECTRICITY:
    case EFFECT_FLAG_ACID:
        return Elemental( vTargetPos );
    case EFFECT_FLAG_SLEEP:
    case EFFECT_FLAG_PARALYZE:
    case EFFECT_FLAG_AFRAID:
    case EFFECT_FLAG_CONFUSE:
        return Status( vTargetPos, m_dwFlags );
    case EFFECT_FLAG_STONE_TO_MUD:
        return StoneToMud( vTargetPos );
    case EFFECT_FLAG_TELEPORT:
        return TeleportAway( vTargetPos );
    case EFFECT_FLAG_IDENTIFY:
        return Probe( vTargetPos );
    case EFFECT_FLAG_AC:
        return pPlayer->DoACBuff( this );
    case EFFECT_FLAG_HP:
        return HealMonster( vTargetPos );
    default:
        return Physical( vTargetPos );
    }
    return JSUCCESS;
}

JResult CEffect::Dispatch( float fDuration, int dwItemFlags )
{
    // TODO: this is a bit of a mess. Refactor so that the player
    // doesn't have to know about all the different effect types.
    // TBD in Issue#315
    CPlayer *pPlayer = g_pGame->GetPlayer();
    switch( m_dwEffect )
    {
    case EFFECT_TYPE_HEAL:
        return pPlayer->DoHealEffects( this );
    case EFFECT_TYPE_HIT:
        return DoHitEffects();
    case EFFECT_TYPE_CREATE:
        return pPlayer->DoCreateEffects( this );
    case EFFECT_TYPE_DESTROY:
        JLog( LOG_LEVEL_DEBUG, true, "Destroying\n" );
        return pPlayer->DoDestroyEffects( this, dwItemFlags );
    case EFFECT_TYPE_INTRINSIC:
        JLog( LOG_LEVEL_DEBUG, true, "Setting intrinsic\n" );
        return pPlayer->DoIntrinsicEffects( this, fDuration );
    case EFFECT_TYPE_RESTORE:
        return pPlayer->DoRestoreEffects( this );
    case EFFECT_TYPE_GAIN:
        return pPlayer->DoGainEffects( this );
    case EFFECT_TYPE_LOSE:
        pPlayer->DoLoseEffects( this );
        return JSUCCESS;
    case EFFECT_TYPE_SEE:
        pPlayer->DoSeeEffects( this );
        return JSUCCESS;
    default:
        JLog( LOG_LEVEL_ERROR, true, "bad effect type: %d\n", m_dwEffect );
        return JSUCCESS;
    }
}

JResult CEffect::Aggravate( JVector vOrigin )
{
    uint8 radius = m_ed ? (uint8)m_ed->m_fRadius : 0;
    CDungeon *pDungeon = g_pGame->GetDungeon();
    CPlayer *pPlayer = g_pGame->GetPlayer();

    if( radius > 0 )
    {
        // Area variant: wake all monsters within radius, point them at the source.
        JIVector vCenter( (int)vOrigin.x, (int)vOrigin.y );
        int nWoken = 0;
        bool bOutOfSight = false;

        CLink<CMonster> *pLink = pDungeon->m_llMonsters->GetHead();
        while( pLink != NULL )
        {
            CMonster *pMon = pLink->m_lpData;
            pLink = pDungeon->m_llMonsters->GetNext( pLink );
            if( !pMon )
                continue;

            JIVector vMonPos( (int)pMon->GetPos().x, (int)pMon->GetPos().y );
            if( !Util::WithinRadius( vCenter, vMonPos, radius ) )
                continue;

            pMon->m_dwActiveEffects &= ~EFFECT_FLAG_SLEEP;
            pMon->m_dwActiveEffects2 |= EFFECT_FLAG_AGGRAVATE;
            pMon->m_pBrain->SetTargetPos( vOrigin );
            pMon->m_pBrain->SetState( BRAINSTATE_SEEK );

            if( pDungeon->PlayerCanSee( pMon->GetPos() ) )
                g_pGame->GetMsgs()->Printf( "The %s wakes up!\n", pMon->GetName() );
            else
                bOutOfSight = true;

            nWoken++;
        }

        if( nWoken > 0 )
        {
            const char *szName = ( m_ed && m_ed->m_szName ) ? m_ed->m_szName : "something";
            g_pGame->GetMsgs()->Printf( "The %s emits a horrible wail!\n", szName );
            if( bOutOfSight )
                g_pGame->GetMsgs()->Printf( "You hear a stirring in the distance!\n" );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "Nothing stirs.\n" );
        }
    }
    else
    {
        // Single-target variant: monster at ranged hit position.
        JVector vTargetPos = pPlayer->GetRangedHitPosition();
        CDungeonTile *pTile = pDungeon->GetTile( vTargetPos );
        if( !pTile )
            return JBOGUSKEY;

        CMonster *pMon = pTile->m_pCurMonster;
        if( !pMon )
        {
            g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
            return JBOGUSKEY;
        }

        pMon->m_dwActiveEffects &= ~EFFECT_FLAG_SLEEP;
        pMon->m_dwActiveEffects2 |= EFFECT_FLAG_AGGRAVATE;
        pMon->m_pBrain->SetTargetPos( vOrigin );
        pMon->m_pBrain->SetState( BRAINSTATE_SEEK );

        g_pGame->GetMsgs()->Printf( "The %s looks enraged!\n", pMon->GetName() );
    }

    return JSUCCESS;
}

JResult CEffect::LockDoor( JVector vOrigin )
{
    // Direct target (wand/spike): lock the specific door tile.
    if( !g_pGame->GetDungeon()->LockDoor( vOrigin ) )
    {
        g_pGame->GetMsgs()->Printf( "Nothing happens.\n" );
        return JSUCCESS;
    }
    g_pGame->GetMsgs()->Printf( "The door clicks shut.\n" );
    return JSUCCESS;
}

/*static*/ JResult CEffect::Fire( const char *szEffectName, JVector vOrigin )
{
    CEffectDef *pDef = g_pGame->GetDungeon()->GetEffectDef( szEffectName );
    if( !pDef )
    {
        JLog( LOG_LEVEL_ERROR, true, "CEffect::Fire: unknown effect '%s'\n", szEffectName );
        return JSUCCESS;
    }
    CEffect fx;
    fx.m_ed = pDef;
    fx.m_dwEffect = pDef->m_dwEffect;
    fx.m_dwFlags = pDef->m_dwFlags;
    fx.m_dwFlags2 = pDef->m_dwFlags2;
    fx.m_dwModifier = pDef->m_dwModifier;
    fx.SetAmount( pDef->m_szAmount );
    fx.m_fDuration = pDef->m_fDuration;
    return fx.DoHitEffects( vOrigin, vOrigin );
}

// ---------------------------------------------------------------------------
// Targeting helpers — CEffect owns what it needs and how to ask for it.
// ---------------------------------------------------------------------------

eEffectTargetType CEffect::GetTargetType() const
{
    // ITEM: effect applies to a player-chosen item
    switch( m_dwFlags )
    {
    case EFFECT_FLAG_IDENTIFY:
        if( m_dwEffect == EFFECT_TYPE_RESTORE )
            return EFFECT_TARGET_ITEM;
        break;
    case EFFECT_FLAG_FUEL:
        if( m_dwEffect == EFFECT_TYPE_RESTORE )
            return EFFECT_TARGET_ITEM;
        break;
    case EFFECT_FLAG_TOHIT:
    case EFFECT_FLAG_TODAM:
    case EFFECT_FLAG_AC:
        if( m_dwModifier & EFFECT_MOD_ENCHANT )
            return EFFECT_TARGET_ITEM;
        break;
    default:
        break;
    }

    switch( m_dwFlags2 )
    {
    case EFFECT_FLAG_CURSE:
        if( m_dwEffect == EFFECT_TYPE_DESTROY )
            return EFFECT_TARGET_ITEM;
        break;
    case EFFECT_FLAG_LOCK:
        return EFFECT_TARGET_DIRECTION;
    default:
        break;
    }

    return EFFECT_TARGET_NONE;
}

bool CEffect::IsValidTarget( CItem *pItem ) const
{
    if( !pItem || !pItem->m_id )
        return false;

    int slot = pItem->EquipType();

    switch( m_dwFlags )
    {
    case EFFECT_FLAG_IDENTIFY:
        return !pItem->IsIdentified();

    case EFFECT_FLAG_FUEL:
        return pItem->NeedsFuel(); // wands, staves, and lanterns

    case EFFECT_FLAG_TOHIT:
    case EFFECT_FLAG_TODAM:
        return pItem->IsWeapon() || pItem->IsRanged();

    case EFFECT_FLAG_AC:
        if( m_dwModifier & EFFECT_MOD_ENCHANT )
        {
            return pItem->IsArmor();
        }
        break;
    default:
        break;
    }

    if( ( m_dwFlags2 & EFFECT_FLAG_CURSE ) && m_dwEffect == EFFECT_TYPE_DESTROY )
        return true; // Remove Curse works on any item

    return true; // default: accept any item
}

const char *CEffect::GetTargetPrompt() const
{
    eEffectTargetType tt = GetTargetType();
    if( tt == EFFECT_TARGET_DIRECTION )
        return "Which direction? [arrow keys or numpad]";

    // Item-targeting prompts
    switch( m_dwFlags )
    {
    case EFFECT_FLAG_FUEL:
        return "Recharge which wand/staff? [a-z inv, A-J equip]";
    case EFFECT_FLAG_TOHIT:
    case EFFECT_FLAG_TODAM:
    case EFFECT_FLAG_AC:
        return "Enchant which weapon? [a-z inv, A-J equip]";
    case EFFECT_FLAG_IDENTIFY:
        if( m_dwEffect == EFFECT_TYPE_RESTORE )
            return "Identify which item? [a-z inv, A-J equip]";
    default:
        break;
    }
    switch( m_dwFlags2 )
    {
    case EFFECT_FLAG_CURSE:
        if( m_dwEffect == EFFECT_TYPE_DESTROY )
            return "Remove curse from which item? [a-z inv, A-J equip]";
    default:
        break;
    }

    return "Use on which item? [a-z inv, A-J equip]";
}

// Returns 0.0 (immune), 1.0 (normal), or 2.0 (weak) based on the relationship between
// the incoming effect's element and the subject's elemental nature.
//
// Immune:  incoming element overlaps with subject's own element (fire creature vs fire attack).
// Weak:    incoming element is the opposite of the subject's element
//          (fire creature vs cold attack).
// Normal:  no relationship.
//
// Also used for status-effect immunity: pass EFFECT_FLAG_SLEEP as dwEffect and
// (MON_FLAG_EMPTY_MIND | EFFECT_FLAG_FREE_ACTION) as dwSubject to block sleep on immune targets.
float CEffect::CheckAffinity( uint32 dwEffect, uint32 dwSubject )
{
    if( dwEffect == 0 || dwSubject == 0 )
        return 1.0f;

    // Direct overlap → immune.
    if( dwEffect & dwSubject )
        return 0.0f;

    // Static opposite table: fire<->cold, electricity<->acid.
    static const struct
    {
        uint32 a;
        uint32 b;
    } kOpposites[] = { { EFFECT_FLAG_FIRE, EFFECT_FLAG_COLD },
                       { EFFECT_FLAG_ELECTRICITY, EFFECT_FLAG_ACID },
                       { 0, 0 } };

    for( int i = 0; kOpposites[i].a; i++ )
    {
        uint32 pair_a = kOpposites[i].a;
        uint32 pair_b = kOpposites[i].b;
        // Subject has element A and incoming effect is element B (or vice versa) → weak.
        if( ( dwSubject & pair_a ) && ( dwEffect & pair_b ) )
            return 2.0f;
        if( ( dwSubject & pair_b ) && ( dwEffect & pair_a ) )
            return 2.0f;
    }

    return 1.0f;
}

JResult CEffect::SummonMonsters( JVector vOrigin )
{
    CDungeon *pDungeon = g_pGame->GetDungeon();
    JIVector vPos( VEC_EXPAND( vOrigin ) );
    int count = Util::Roll( 1, 3 );

    // Summon as if 20 levels deeper — punishes careless reading
    int effectiveDepth = pDungeon->depth + 20;
    if( effectiveDepth > DUNG_MAXDEPTH )
        effectiveDepth = DUNG_MAXDEPTH;

    for( int i = 0; i < count; i++ )
    {
        int which = pDungeon->ChooseMonsterForDepth( effectiveDepth, 15.0f );
        if( which == MON_IDX_INVALID )
            continue;
        CMonsterDef *pDef = pDungeon->GetMonsterDef( which );
        if( pDef )
            CMonster::CreateMonster( pDef, vPos, true );
    }

    g_pGame->GetMsgs()->Printf(
        "Monsters appear around %s!\n",
        vOrigin.WithinRange( g_pGame->GetPlayer()->m_vPos, 0.0f ) ? "you" : "the summoner" );
    return JSUCCESS;
}

JResult CEffect::DispatchAll( CLink<CEffect> *plEffect, float fDuration, int dwItemFlags )
{
    JResult bNoticed = JBOGUSKEY;
    while( plEffect != NULL )
    {
        CEffect *pEffect = plEffect->m_lpData;
        JLog( LOG_LEVEL_DEBUG, true, "Effect: %s Flag: %s Mod: %s\n",
              g_Constants.IndexToString( EFFECT_TYPE, pEffect->m_dwEffect ),
              g_Constants.EffectFlagToString( pEffect->m_dwFlags, pEffect->m_dwFlags2 ),
              g_Constants.IndexToString( EFFECT_MOD, pEffect->m_dwModifier ) );
        if( pEffect->GetTargetType() != EFFECT_TARGET_NONE )
        {
            // Targeting effects are dispatched by the state machine after player input.
            plEffect = plEffect->next;
            continue;
        }
        if( pEffect->Dispatch( fDuration, dwItemFlags ) == JSUCCESS )
            bNoticed = JSUCCESS;
        plEffect = plEffect->next;
    }
    return bNoticed;
}
