#ifndef __MONSTERRECALL_H__
#define __MONSTERRECALL_H__
#include "JMDefs.h"

class CMonsterDef;
class CDisplayText;

#define RECALL_MAX_ENTRIES 256
#define RECALL_NAME_LEN 64
#define RECALL_EFFECT_NAME_LEN 32
#define RECALL_MAX_ATTACKS 8

// One observed attack type recorded for a species
struct CRecallAttack
{
    int dwType;                            // MON_FLAG_BITE/CLAW/etc
    char szEffect[RECALL_EFFECT_NAME_LEN]; // effect name, or "" for none
    int dwTimesObserved;
    int dwDamageMin;   // lowest damage seen
    int dwDamageMax;   // highest damage seen
    int dwDamageTotal; // sum for average
};

struct CRecallEntry
{
    char szName[RECALL_NAME_LEN];
    int dwEncounters;
    int dwKills;
    int dwDeepestFeet;
    int dwKnownFlags; // MON_FLAG_* bits the player has observed
    bool bDepthKnown; // set on first sighting
    bool bXPKnown;    // set on first kill

    // HP range observed from kills (killing blow damage used as proxy)
    int dwHPObsMin;
    int dwHPObsMax;

    // Attacks observed in combat
    CRecallAttack attacks[RECALL_MAX_ATTACKS];
    int nAttacks;

    void Init( const char *szN )
    {
        memset( this, 0, sizeof( CRecallEntry ) );
        strncpy( szName, szN, RECALL_NAME_LEN - 1 );
        szName[RECALL_NAME_LEN - 1] = '\0';
    }
};

class CMonsterRecall
{
public:
    CMonsterRecall();
    void Init( const char *szBasedir );  // normal game init (loads from ~/.jmoria/)
    void Save();                         // save to the file set by Init
    void InitFrom( const char *szPath ); // load directly from an explicit path (tests/fixtures)
    void SaveTo( const char *szPath );   // save directly to an explicit path (tests)
    // instanceId=0: species-level dedup (tests); non-zero: per-instance dedup (game)
    bool RecordSighting( const char *szName, int depthFeet, uint32 instanceId = 0 );
    void ResetLevelSightings(); // call on each new dungeon level
    void RecordKill( const char *szName, float fEstimatedHP );
    void RecordObservation( const char *szName, int dwFlag );
    void RecordAttackObservation( const char *szName, int dwType, float fDamage,
                                  const char *szEffect );
    const CRecallEntry *GetEntry( const char *szName ) const;
    void PrintRecall( const CMonsterDef *pmd, CDisplayText *pDT ) const;

private:
    CRecallEntry *FindOrCreate( const char *szName );

    CRecallEntry m_entries[RECALL_MAX_ENTRIES];
    int m_nCount;
    char m_szFilePath[512];
    bool m_bSeenThisSession[RECALL_MAX_ENTRIES]; // species-level dedup (instanceId==0)
    uint32 m_dwSeenInstances[512];               // instance-level dedup (instanceId!=0)
    int m_nSeenInstances;
};

#endif // __MONSTERRECALL_H__
