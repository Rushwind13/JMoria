#include "MonsterRecall.h"
#include "Constants.h"
#include "DisplayText.h"
#include "FileParse.h"
#include "JLog.h"
#include "Monster.h"
#include "Util.h"
#include <sys/stat.h>

CMonsterRecall::CMonsterRecall() : m_nCount( 0 )
{
    memset( m_entries, 0, sizeof( m_entries ) );
    memset( m_szFilePath, 0, sizeof( m_szFilePath ) );
    memset( m_bSeenThisSession, 0, sizeof( m_bSeenThisSession ) );
    memset( m_dwSeenInstances, 0, sizeof( m_dwSeenInstances ) );
    m_nSeenInstances = 0;
}

void CMonsterRecall::Init( const char *szBasedir )
{
    (void)szBasedir;

    // Reset session-local state for fresh init
    memset( m_bSeenThisSession, 0, sizeof( m_bSeenThisSession ) );
    memset( m_dwSeenInstances, 0, sizeof( m_dwSeenInstances ) );
    m_nSeenInstances = 0;

    const char *szHome = getenv( "HOME" );
    if( !szHome )
        szHome = ".";

    char szDir[480];
    snprintf( szDir, sizeof( szDir ), "%s/.jmoria", szHome );

#ifdef _WIN32
    _mkdir( szDir );
#else
    mkdir( szDir, 0755 );
#endif

    snprintf( m_szFilePath, sizeof( m_szFilePath ), "%s/monster_recall.txt", szDir );

    // If basedir is empty, skip file loading (used for tests)
    if( !szBasedir || szBasedir[0] == '\0' )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "MonsterRecall: Init with empty basedir, skipping file load\n" );
        return;
    }

    CDataFile df;
    if( !df.Open( m_szFilePath ) )
    {
        JLog( LOG_LEVEL_INFO, true, "MonsterRecall: no existing file at %s\n", m_szFilePath );
        return;
    }

    while( m_nCount < RECALL_MAX_ENTRIES )
    {
        CRecallEntry tmp;
        memset( &tmp, 0, sizeof( tmp ) );
        if( !df.ReadMonsterRecall( tmp ) )
            break;
        if( tmp.szName[0] == '\0' )
            break;
        m_entries[m_nCount++] = tmp;
    }

    df.Close();
    JLog( LOG_LEVEL_INFO, true, "MonsterRecall: loaded %d entries from %s\n", m_nCount,
          m_szFilePath );
}

void CMonsterRecall::InitFrom( const char *szPath )
{
    // Reset so this is a fresh load, not an append
    m_nCount = 0;
    memset( m_bSeenThisSession, 0, sizeof( m_bSeenThisSession ) );
    memset( m_dwSeenInstances, 0, sizeof( m_dwSeenInstances ) );
    m_nSeenInstances = 0;
    memset( m_dwSeenInstances, 0, sizeof( m_dwSeenInstances ) );
    m_nSeenInstances = 0;

    CDataFile df;
    if( !df.Open( szPath ) )
        return;
    CRecallEntry tmp;
    while( CRecallEntry *pEntry = df.ReadMonsterRecall( tmp ) )
    {
        if( m_nCount < RECALL_MAX_ENTRIES )
            m_entries[m_nCount++] = *pEntry;
    }
    df.Close();
}

void CMonsterRecall::SaveTo( const char *szPath )
{
    CDataFile df;
    if( !df.Write( szPath ) )
        return;
    for( int i = 0; i < m_nCount; i++ )
        df.WriteMonsterRecall( &m_entries[i] );
    df.Close();
}

void CMonsterRecall::Save()
{
    if( m_szFilePath[0] == '\0' )
        return;

    CDataFile df;
    if( !df.Write( m_szFilePath ) )
    {
        JLog( LOG_LEVEL_WARN, true, "MonsterRecall: could not save to %s\n", m_szFilePath );
        return;
    }

    for( int i = 0; i < m_nCount; i++ )
        df.WriteMonsterRecall( &m_entries[i] );

    df.Close();
    JLog( LOG_LEVEL_INFO, true, "MonsterRecall: saved %d entries to %s\n", m_nCount, m_szFilePath );
}

CRecallEntry *CMonsterRecall::FindOrCreate( const char *szName )
{
    if( !szName )
        return NULL;

    CRecallEntry *pExisting = const_cast<CRecallEntry *>( GetEntry( szName ) );
    if( pExisting )
        return pExisting;

    if( m_nCount >= RECALL_MAX_ENTRIES )
    {
        JLog( LOG_LEVEL_WARN, true, "MonsterRecall: entry table full, cannot add %s\n", szName );
        return NULL;
    }

    CRecallEntry *pEntry = &m_entries[m_nCount];
    pEntry->Init( szName );
    m_nCount++;
    return pEntry;
}

void CMonsterRecall::ResetLevelSightings()
{
    // Instance IDs are only valid within one dungeon level; clear on level change.
    // Species-level dedup (bSeenThisSession) intentionally persists across levels.
    memset( m_dwSeenInstances, 0, sizeof( m_dwSeenInstances ) );
    m_nSeenInstances = 0;
}

bool CMonsterRecall::RecordSighting( const char *szName, int depthFeet, uint32 instanceId )
{
    CRecallEntry *pEntry = FindOrCreate( szName );
    if( !pEntry )
        return false;

    if( instanceId != 0 )
    {
        // Per-instance dedup: check if we've already counted this exact monster
        for( int i = 0; i < m_nSeenInstances; i++ )
        {
            if( m_dwSeenInstances[i] == instanceId )
                return false;
        }
        if( m_nSeenInstances < 512 )
            m_dwSeenInstances[m_nSeenInstances++] = instanceId;
    }
    else
    {
        // Species-level dedup (used by tests)
        int idx = (int)( pEntry - m_entries );
        if( m_bSeenThisSession[idx] )
            return false;
        m_bSeenThisSession[idx] = true;
    }

    bool wasKnown = ( pEntry->dwEncounters > 0 );
    pEntry->dwEncounters++;
    if( depthFeet > pEntry->dwDeepestFeet )
        pEntry->dwDeepestFeet = depthFeet;

    return wasKnown;
}

void CMonsterRecall::RecordKill( const char *szName, float fEstimatedHP )
{
    CRecallEntry *pEntry = FindOrCreate( szName );
    if( !pEntry )
        return;
    pEntry->dwKills++;
    pEntry->bXPKnown = true;
    pEntry->bDepthKnown = true;

    int iHP = (int)fEstimatedHP;
    if( iHP > 0 )
    {
        if( pEntry->dwHPObsMin == 0 || iHP < pEntry->dwHPObsMin )
            pEntry->dwHPObsMin = iHP;
        if( iHP > pEntry->dwHPObsMax )
            pEntry->dwHPObsMax = iHP;
    }
}

void CMonsterRecall::RecordObservation( const char *szName, int dwFlag )
{
    CRecallEntry *pEntry = FindOrCreate( szName );
    if( !pEntry )
        return;
    pEntry->dwKnownFlags |= dwFlag;
}

void CMonsterRecall::RecordAttackObservation( const char *szName, int dwType, float fDamage,
                                              const char *szEffect, uint32 dwEffectFlags )
{
    CRecallEntry *pEntry = FindOrCreate( szName );
    if( !pEntry )
        return;

    // Check if we already have this attack type recorded
    for( int i = 0; i < pEntry->nAttacks; i++ )
    {
        if( pEntry->attacks[i].dwType == dwType )
        {
            int iDmg = (int)fDamage;
            pEntry->attacks[i].dwTimesObserved++;
            pEntry->attacks[i].dwDamageTotal += iDmg;
            if( iDmg < pEntry->attacks[i].dwDamageMin )
                pEntry->attacks[i].dwDamageMin = iDmg;
            if( iDmg > pEntry->attacks[i].dwDamageMax )
                pEntry->attacks[i].dwDamageMax = iDmg;
            pEntry->attacks[i].dwElementFlags |= dwEffectFlags;
            return;
        }
    }

    // New attack type
    if( pEntry->nAttacks >= RECALL_MAX_ATTACKS )
        return;

    CRecallAttack &atk = pEntry->attacks[pEntry->nAttacks++];
    atk.dwType = dwType;
    atk.dwTimesObserved = 1;
    int iDmg = (int)fDamage;
    atk.dwDamageMin = iDmg;
    atk.dwDamageMax = iDmg;
    atk.dwDamageTotal = iDmg;
    strncpy( atk.szEffect, szEffect ? szEffect : "", RECALL_EFFECT_NAME_LEN - 1 );
    atk.szEffect[RECALL_EFFECT_NAME_LEN - 1] = '\0';
    atk.dwElementFlags = dwEffectFlags;
}

const CRecallEntry *CMonsterRecall::GetEntry( const char *szName ) const
{
    if( !szName )
        return NULL;
    for( int i = 0; i < m_nCount; i++ )
    {
        if( Util::jstrcmp( m_entries[i].szName, szName ) == 0 )
            return &m_entries[i];
    }
    return NULL;
}

// Verb for each attack type (mirrors AttackFlavorText on CMonster)
static const char *RecallAttackVerb( int dwType )
{
    switch( dwType )
    {
    case MON_FLAG_BITE:
        return "bites";
    case MON_FLAG_CRAWL:
        return "crawls on you";
    case MON_FLAG_CLAW:
        return "claws";
    case MON_FLAG_TRAMPLE:
        return "tramples";
    case MON_FLAG_SPORE:
        return "releases spores";
    case MON_FLAG_TOUCH:
        return "touches";
    case MON_FLAG_DROOL:
        return "drools on you";
    case MON_FLAG_BREATHE:
        return "breathes";
    default:
        return "hits";
    }
}

void CMonsterRecall::PrintRecall( const CMonsterDef *pmd, CDisplayText *pDT ) const
{
    if( !pmd || !pDT )
        return;

    const CRecallEntry *pEntry = GetEntry( pmd->m_szName );

    // ── Line 1: name + sighting/kill counts ──────────────────────────────────
    pDT->Printf( g_Strings[STR_RECALL_MONSTER_HEADER], pmd->m_szName );

    bool anyKnowledge = pEntry && ( pEntry->dwEncounters > 0 || pEntry->dwKills > 0 ||
                                    pEntry->nAttacks > 0 || pEntry->dwKnownFlags != 0 );
    if( !anyKnowledge )
    {
        pDT->Printf( g_Strings[STR_NO_KNOWLEDGE] );
        return;
    }

    if( pEntry->dwEncounters > 0 )
        pDT->Printf( g_Strings[STR_SEEN], pEntry->dwEncounters );
    if( pEntry->dwKills > 0 )
        pDT->Printf( g_Strings[STR_KILLED], pEntry->dwKills );
    pDT->Printf( g_Strings[STR_LF] );

    // ── Line 2: depth / HP / XP / flags (all on one line, skip missing) ──────
    bool anyLine2 = false;
    auto sep = [&]()
    {
        if( anyLine2 )
            pDT->Printf( g_Strings[STR_INDENT] );
        anyLine2 = true;
    };

    if( pEntry->bDepthKnown && pmd->m_dwLevel > 0 )
    {
        sep();
        pDT->Printf( g_Strings[STR_DEPTH], pmd->m_dwLevel * 50 );
    }
    if( pEntry->dwHPObsMin > 0 )
    {
        sep();
        if( pEntry->dwHPObsMin == pEntry->dwHPObsMax )
            pDT->Printf( g_Strings[STR_HP_RANGE_APPROX], pEntry->dwHPObsMin );
        else
            pDT->Printf( g_Strings[STR_HP_RANGE], pEntry->dwHPObsMin, pEntry->dwHPObsMax );
    }
    if( pEntry->bXPKnown )
    {
        sep();
        pDT->Printf( g_Strings[STR_N_TIMESP], (int)pmd->m_fExpValue );
    }

    static const struct
    {
        int flag;
        const char *label;
    } kFlagLabels[] = { { MON_FLAG_WARM, "Warm." },
                        { MON_FLAG_REGENERATE, "Regen." },
                        { MON_FLAG_HURT_BY_LIGHT, "HurtLight." },
                        { MON_FLAG_BREED, "Breeds." },
                        { MON_FLAG_INVISIBLE, "Invis." },
                        { MON_FLAG_EMPTY_MIND, "NoMind." },
                        { 0, NULL } };
    for( int i = 0; kFlagLabels[i].flag; i++ )
    {
        if( pEntry->dwKnownFlags & kFlagLabels[i].flag )
        {
            sep();
            pDT->Printf( g_Strings[STR_FMT_STRING], kFlagLabels[i].label );
        }
    }

    // Elemental immunity/weakness — only shown for elements the player has actually observed.
    if( pEntry->nAttacks > 0 )
    {
        uint32 dwMonElements = 0;
        for( int i = 0; i < pEntry->nAttacks; i++ )
            dwMonElements |= pEntry->attacks[i].dwElementFlags;

        static const struct
        {
            uint32 flag;
            const char *immuneLabel;
            const char *weakLabel;
        } kElems[] = { { EFFECT_FLAG_FIRE, "ImuFire.", "WkFire." },
                       { EFFECT_FLAG_COLD, "ImuCold.", "WkCold." },
                       { EFFECT_FLAG_ELECTRICITY, "ImuElec.", "WkElec." },
                       { EFFECT_FLAG_ACID, "ImuAcid.", "WkAcid." },
                       { 0, NULL, NULL } };

        for( int i = 0; kElems[i].flag; i++ )
        {
            if( dwMonElements & kElems[i].flag )
            {
                sep();
                pDT->Printf( g_Strings[STR_FMT_STRING], kElems[i].immuneLabel );
            }
        }
        for( int i = 0; kElems[i].flag; i++ )
        {
            if( CEffect::CheckAffinity( kElems[i].flag, dwMonElements ) > 1.0f )
            {
                sep();
                pDT->Printf( g_Strings[STR_FMT_STRING], kElems[i].weakLabel );
            }
        }
    }

    if( anyLine2 )
        pDT->Printf( g_Strings[STR_LF] );

    // ── Line 3: attacks, all comma-separated on one line ─────────────────────
    if( pEntry->nAttacks > 0 )
    {
        for( int i = 0; i < pEntry->nAttacks; i++ )
        {
            const CRecallAttack &atk = pEntry->attacks[i];
            if( i > 0 )
                pDT->Printf( g_Strings[STR_INDENT] );
            int avg = atk.dwTimesObserved > 0 ? atk.dwDamageTotal / atk.dwTimesObserved : 0;
            pDT->Printf( g_Strings[STR_FMT_STRING], RecallAttackVerb( atk.dwType ) );
            if( atk.szEffect[0] )
                pDT->Printf( g_Strings[STR_FMT_BRACKETED], atk.szEffect );
            pDT->Printf( g_Strings[STR_DMGAVG], atk.dwDamageMin, atk.dwDamageMax, avg );
            if( atk.dwTimesObserved > 1 )
                pDT->Printf( g_Strings[STR_N_TIMES], atk.dwTimesObserved );
        }
        pDT->Printf( g_Strings[STR_LF] );
    }
}
