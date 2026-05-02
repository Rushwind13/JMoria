#ifndef __PLAYER_H__
#define __PLAYER_H__
#include "JLinkList.h"
#include "JMDefs.h"
#include "Monster.h"

class CTileset;
class CItem;
class CEffect;
class CMonster;
#define PLAYER_TURNS_PER_HP 4
#define PLAYER_TURNS_PER_MP 16
#define PLAYER_BASE_DAMAGE "1d2"
#define CLASS_HD_WARRIOR "1d10"
#define PLAYER_MAX_LEVEL 11
#include "DungeonConstants.h"

enum eInvFilter
{
    INV_COMPLETE = 0,
    INV_QUAFF,
    INV_READ,
    INV_WIELD,
    INV_ZAP,
    INV_FIRE,
};

#define SIGHT_DISTANCE_PLAYER 5
#define SIGHT_DISTANCE_LIT DUNG_ROOM_MAX_DIAGONAL
#define SIGHT_DISTANCE_INFRA 10
#define SIGHT_DISTANCE_ESP 15

#define PHASE_DOOR_RANGE 20
#define MAGIC_MAPPING_RANGE 25

class CClass
{
    // Methods
public:
    CClass() : m_szHD( NULL )
    {
        m_fExpNeeded[0] = 20.0f;
        m_fExpNeeded[1] = 40.0f;
        m_fExpNeeded[2] = 80.0f;
        m_fExpNeeded[3] = 180.0f;
        m_fExpNeeded[4] = 350.0f;
        m_fExpNeeded[5] = 700.0f;
        m_fExpNeeded[6] = 1250.0f;
        m_fExpNeeded[7] = 2500.0f;
        m_fExpNeeded[8] = 5000.0f;
        m_fExpNeeded[9] = 7500.0f;
        m_fExpNeeded[10] = 10000.0f;

        m_szHD = new char[10];
        sprintf( m_szHD, CLASS_HD_WARRIOR );

        memset( m_szName, 0, MAX_STRING_LENGTH );
        Util::jstrcpy( m_szName, "Warrior" );
    };
    ~CClass()
    {
        if( m_szHD )
        {
            delete[] m_szHD;
            m_szHD = NULL;
        }
    };

protected:
private:
    // Fields
public:
    float m_fExpNeeded[PLAYER_MAX_LEVEL];
    char *m_szHD;
    char m_szName[MAX_STRING_LENGTH];

protected:
private:
};

class CRace
{
public:
    CRace() : m_szName( NULL )
    {
        m_szName = new char[MAX_STRING_LENGTH];
        memset( m_szName, 0, MAX_STRING_LENGTH );
        Util::jstrcpy( m_szName, "Human" );
    }

    ~CRace()
    {
        if( m_szName )
        {
            delete[] m_szName;
            m_szName = NULL;
        }
    }

public:
    char *m_szName;
};

class CPlayer
{
public:
    CPlayer()
        : m_bHasSpawned( false ),
          m_szDamage( NULL ),
          m_szKilledBy( NULL ),
          m_bIsRested( true ),
          m_bIsDisturbed( false ),
          m_bPendingIdentify( false ),
          m_bLastEffectNoticed( false ),
          m_fDamageModifier( 0.0f ),
          m_fToHitModifier( 0.0f ),
          m_fArmorClass( 1.0f ),
          m_fSpeed( 1.0f ),
          m_fHitPoints( 0.0f ),
          m_fLastHPTime( 0.0f ),
          m_fLastMPTime( 0.0f ),
          m_fLastLightTime( 0.0f ),
          m_fCurHitPoints( 0.0f ),
          m_fExperience( 0.0f ),
          m_fLevel( 1.0f ),
          m_dwIntrinsics( 0 ),
          m_dwRecallDepth( 1 ),
          m_bWizardMode( false ),
          m_pClass( NULL ),
          m_pTarget( NULL ),
          m_pCurrentRangedAmmo( NULL ),
          m_pCurrentRangedWeapon( NULL ),
          m_vRangedHitPosition( 0, 0 ),
          m_llVisibleMonsters( NULL )
    {
        memset( m_szName, 0, MAX_STRING_LENGTH );
        Util::jstrcpy( m_szName, "Anonymous" );
        m_pClass = new CClass;
        m_pRace = new CRace;
        m_llInventory = new JLinkList<CItem>;
        m_llEquipment = new JLinkList<CItem>;
        m_llActiveEffects = new JLinkList<CEffect>;
        m_szDamage = new char[10];
        sprintf( m_szDamage, PLAYER_BASE_DAMAGE );

        m_fHitPoints = Util::Roll( m_pClass->m_szHD );
        {
            const char *d = strchr( m_pClass->m_szHD, 'd' );
            float fMinHP = d ? atoi( d + 1 ) / 2.0f : 1.0f;
            if( m_fHitPoints < fMinHP )
                m_fHitPoints = fMinHP;
        }
        m_fCurHitPoints = m_fHitPoints;

        // Keep derived combat stats in sync with the canonical recalc path.
        RecalcCombatStats();
    };
    ~CPlayer() { Term(); }

    void Init( const char *szBasedir );
    JResult SpawnPlayer();
    void Term()
    {
        if( m_pClass )
        {
            delete m_pClass;
            m_pClass = NULL;
        }
        if( m_pRace )
        {
            delete m_pRace;
            m_pRace = NULL;
        }
        if( m_llInventory )
        {
            m_llInventory->Terminate();
            delete m_llInventory;
            m_llInventory = NULL;
        }
        if( m_llEquipment )
        {
            m_llEquipment->Terminate();
            delete m_llEquipment;
            m_llEquipment = NULL;
        }
        if( m_llActiveEffects )
        {
            m_llActiveEffects->Terminate();
            delete m_llActiveEffects;
            m_llActiveEffects = NULL;
        }
        if( m_TileSet )
        {
            delete m_TileSet;
            m_TileSet = NULL;
        }
        if( m_szDamage )
        {
            delete[] m_szDamage;
            m_szDamage = NULL;
        }
        if( m_szKilledBy )
        {
            delete[] m_szKilledBy;
            m_szKilledBy = NULL;
        }
        ClearVisibleMonsters();
    };
    const char *GetName() { return m_szName; }
    float GetLevel() { return m_fLevel; }
    CClass *GetClass() { return m_pClass; }
    CRace *GetRace() { return m_pRace; }
    float GetExperience() { return m_fExperience; }
    bool Update( float fCurTime );
    void UpdateActiveEffects( float fCurTime );
    void CheckDisturbance();
    void PreDraw();
    void Draw();
    void PostDraw();
    void DisplayStats();
    void DisplayInventory( uint8 dwPlacement, eInvFilter filter = INV_COMPLETE );
    void DisplayEquipment( uint8 dwPlacement, eInvFilter filter = INV_COMPLETE );
    void DisplayVisibleMonsters();
    void DisplayMonsterRecall();
    void DisplayItemRecall();
    void DisplayMap();
    void PickUp( JVector &vPickupPos );
    bool Drop( CItem *pItem );
    bool Drop( CItem *pItem, int quantity ); // Drop a partial stack

    bool CanDropHere();
    void ConsolidateInventory(); // Auto-consolidate stacks with matching charges/identification

    void SetIntrinsic( const uint32 dwIntrinsic ) { m_dwIntrinsics |= dwIntrinsic; };
    void UnsetIntrinsic( const uint32 dwIntrinsic ) { m_dwIntrinsics &= ~dwIntrinsic; };
    int GetIntrinsic( const uint32 dwIntrinsic ) { return m_dwIntrinsics & dwIntrinsic; }

    bool IsWieldable( CLink<CItem> *pLink );
    JResult Wield( CLink<CItem> *pItem );

    bool IsRemovable( CLink<CItem> *pLink );
    bool RemoveEquipment( CLink<CItem> *pLink );

    void RecalcCombatStats();
    void XchangeWeapons();

    bool IsDrinkable( CLink<CItem> *pLink );
    JResult Quaff( CLink<CItem> *pLink );

    bool IsFireable( CLink<CItem> *pLink );
    bool IsCompatibleAmmo( CLink<CItem> *pLink );
    JResult Fire( CLink<CItem> *pLink );

    bool IsReadable( CLink<CItem> *pLink );
    JResult Read( CLink<CItem> *pLink );

    bool IsZappable( CLink<CItem> *pLink );
    JResult Zap( CLink<CItem> *pLink );

    void
    ConsumeItem( CLink<CItem> *pLink ); // Unified consumption for ammo/charges: decrement or remove
    void ConsumeAndRemoveIfEmpty(
        CLink<CItem> *pLink ); // Consume item and remove from inventory if empty

    bool IsCastable( CLink<CItem> *pLink );
    JResult Magic( CLink<CItem> *pLink );

    bool IsFuel( CLink<CItem> *pLink );
    JResult Fuel( CLink<CItem> *pLink );

    void Search();
    void PassiveSearch();

    float LightSource();
    float LightRadius();
    void UpdateLight( float fValue, bool bReset = false );

    JResult DoEffects( CLink<CEffect> *plEffect, float fDuration, int dwFlags );
    JResult DoHealEffects( CEffect *pEffect );
    JResult DoHealHP( CEffect *pEffect );
    JResult DoHitEffects( CEffect *pEffect );
    JResult DoPhysicalHit( CEffect *pEffect );
    JResult DoLightRay( CEffect *pEffect );
    JResult DoElementalHit( CEffect *pEffect );
    JResult DoDamageInventory( uint32 dwElement );
    JResult DoDamageEquipment( uint32 dwElement );
    JResult DoCreateEffects( CEffect *pEffect );
    JResult DoLightArea();
    JResult DoDestroyEffects( CEffect *pEffect, int dwFlags );
    JResult DoRemoveCurse();
    JResult DoIntrinsicEffects( CEffect *pEffect, float fDuration );
    JResult UndoIntrinsicEffects( CEffect *pEffect );
    JResult DoApplyCurse();
    JResult DoRestoreEffects( CEffect *pEffect );
    JResult DoIdentify();
    JResult DoGainEffects( CEffect *pEffect );
    JResult DoLoseEffects( CEffect *pEffect );
    JResult DoSeeEffects( CEffect *pEffect );
    JResult DoTeleport( CEffect *pEffect );
    JResult DoMagicMapping( CEffect *pEffect );
    JResult DoRecall();
    JResult DoSummonMonsters();

    bool SetName( const char *szName );

    int Move( JVector vDir );
    void HandleCollision( JVector vPos, int dwCollideType );

    CMonster *GetTarget() { return m_pTarget; }
    void SetTarget( CMonster *pMon )
    {
        if( m_pTarget )
        {
            m_pTarget->UnsetAsTarget();
        }
        m_pTarget = pMon;
        if( pMon )
            pMon->SetAsTarget();
    }
    JVector GetRangedHitPosition() { return m_vRangedHitPosition; }
    void SetRangedHitPosition( const JVector vPos )
    {
        m_vRangedHitPosition.Init( VEC_EXPAND( vPos ) );
    }

    float Attack();
    float RangedAttack( CLink<CItem> *pArrow );
    float Damage( float fDamageMult );

    bool Hit( float &fRoll );
    int TakeDamage( float fDamage, const char *szMon, uint32 dwElement = 0 );
    float Resist( uint32 dwElement );
    static float ItemDestroyChance( float fResistMult )
    {
        return fResistMult * ITEM_DESTROY_CHANCE;
    }

    void OnKillMonster( CMonster *pMon, float fKillingBlow );
    bool DamageMonster( CMonster *pMon, float fDamage );

    void SetWizard();
    void ClearWizard();
    bool IsWizard() { return m_bWizardMode; };

    JVector m_vPos;
    JVector m_vVel;
    CTileset *m_TileSet;
    bool m_bHasSpawned;

    // Inventory
    JLinkList<CItem> *m_llInventory;
    JLinkList<CItem> *m_llEquipment;

    // Combat
    char *m_szDamage;
    float m_fDamageModifier;
    float m_fToHitModifier;
    float m_fSpeed;              // action economy: 1.0 = base (10), 2.0 = fast (20), 0.8 = slow (8)
    CItem *m_pCurrentRangedAmmo; // Current arrow/bolt being fired, for to-hit calculation
    CItem *m_pCurrentRangedWeapon; // Current bow/wand being used, for range calculation

    char *m_szKilledBy;

    bool m_bIsRested;
    bool m_bIsDisturbed;
    bool m_bPendingIdentify;
    bool m_bLastEffectNoticed;

    bool HasPendingIdentify() { return m_bPendingIdentify; }
    void ClearPendingIdentify() { m_bPendingIdentify = false; }
    float GetSpeed() { return m_fSpeed; }

protected:
    void GainLevel();

    float m_fArmorClass;
    float m_fHitPoints;
    float m_fCurHitPoints;
    float m_fLastHPTime;
    float m_fLastMPTime;
    float m_fLastLightTime;

    float m_fExperience;
    float m_fLevel;

    uint32 m_dwIntrinsics;
    uint8 m_dwRecallDepth;
    JLinkList<CEffect> *m_llActiveEffects;

    CClass *m_pClass;
    CRace *m_pRace;
    char m_szName[MAX_STRING_LENGTH];
    bool m_bWizardMode;

    CMonster *m_pTarget;
    JVector m_vRangedHitPosition;
    JLinkList<CMonster> *m_llVisibleMonsters;

public:
    void UpdateVisibleMonsters();
    void ClearVisibleMonsters();
    JLinkList<CMonster> *GetVisibleMonsters();
};
#endif // __PLAYER_H__
