#ifndef __ITEM_H__
#define __ITEM_H__

#include "Effect.h"
#include "JLinkList.h"
#include "JMDefs.h"
#include "Util.h"

class CItemDef
{
    // Member Variables
public:
    CItemDef()
        : m_szName( NULL ),
          m_szPlural( NULL ),
          m_szUnidentifiedName( NULL ),
          m_szUnidentifiedPlural( NULL ),
          m_szFlavor( NULL ),
          m_fAttackSpeed( 0.0f ),
          m_fSpeedBonus( 0.0f ),
          m_szACBonus( NULL ),
          m_fBaseAC( 0.0f ),
          m_szBaseDamage( NULL ),
          m_szBonusToHit( NULL ),
          m_szBonusToDamage( NULL ),
          m_szCharges( NULL ),
          m_dwLevel( 0 ),
          m_fLevelSigma( 10.0f ),
          m_fSpawnWeight( 0.0f ),
          m_fValue( 0.0f ),
          m_fWeight( 0.0f ),
          m_fRadius( 0.0f ),
          m_fDuration( 0.0f ),
          m_dwFlags( 0 ),
          m_dwIndex( ITEM_IDX_INVALID ),
          m_dwBaseHP( 0.0f ),
          m_dwMinRange( 0 ),
          m_dwMaxRange( 0 ),
          m_bIdentified( false ),
          m_bTried( false )
    {
        m_Colors = new JLinkList<JColor>;
        m_llEffects = new JLinkList<CEffect>;
    }
    ~CItemDef()
    {
        if( m_szName )
        {
            delete[] m_szName;
            m_szName = NULL;
        }
        if( m_szPlural )
        {
            delete[] m_szPlural;
            m_szPlural = NULL;
        }
        if( m_szUnidentifiedName )
        {
            delete[] m_szUnidentifiedName;
            m_szUnidentifiedName = NULL;
        }
        if( m_szUnidentifiedPlural )
        {
            delete[] m_szUnidentifiedPlural;
            m_szUnidentifiedPlural = NULL;
        }
        if( m_szFlavor )
        {
            delete[] m_szFlavor;
            m_szFlavor = NULL;
        }
        if( m_szBaseDamage )
        {
            delete[] m_szBaseDamage;
            m_szBaseDamage = NULL;
        }
        if( m_szACBonus )
        {
            delete[] m_szACBonus;
            m_szACBonus = NULL;
        }
        if( m_szBonusToHit )
        {
            delete[] m_szBonusToHit;
            m_szBonusToHit = NULL;
        }
        if( m_szBonusToDamage )
        {
            delete[] m_szBonusToDamage;
            m_szBonusToDamage = NULL;
        }
        if( m_szCharges )
        {
            delete[] m_szCharges;
            m_szCharges = NULL;
        }
        if( m_Colors )
        {
            m_Colors->Terminate();
            delete m_Colors;
            m_Colors = NULL;
        }
        if( m_llEffects )
        {
            m_llEffects->Terminate();
            delete m_llEffects;
            m_llEffects = NULL;
        }
    }
    char *m_szName; // what item is this?
    char *m_szPlural;
    char *m_szUnidentifiedName;
    char *m_szUnidentifiedPlural;
    char *m_szFlavor; // "Green" Potion
    float m_fAttackSpeed;
    float m_fSpeedBonus; // for rings of speed, boots/gloves of swiftness, etc.
    char *m_szACBonus; // NdM dice string for magical AC bonus (rolled per-instance)
    float m_fBaseAC;
    char *m_szBaseDamage;
    char *m_szBonusToHit;    // NdM dice string for magical to-hit bonus (rolled per-instance)
    char *m_szBonusToDamage; // NdM dice string for magical to-damage bonus (rolled per-instance)
    char *m_szCharges;       // NdM dice string for initial charges (rolled per-instance)
    int m_dwLevel;           // peak dungeon depth (center of bell curve)
    float m_fLevelSigma;     // spread of bell curve (default 10.0)
    float m_fSpawnWeight;    // scratch: Gaussian weight computed by ChooseItemForDepth
    float m_fValue;
    float m_fWeight;
    int m_dwFlags;
    int m_dwIndex;     // ITEM_IDX_SWORD, ITEM_IDX_WAND, etc.
    int m_dwBaseHP;    // for busting down walls, disarming traps, etc.
    float m_fDuration; // for potions, scrolls, torches -- "How long will this last?"
    float m_fRadius;   // for AoE effects -- "How big is the badaboom?"
    uint32 m_dwMinRange; // for ranged weapons -- minimum trajectory length
    uint32 m_dwMaxRange; // for ranged weapons -- maximum trajectory length
    JLinkList<JColor> *m_Colors;
    JLinkList<CEffect> *m_llEffects;
    JColor m_Color;
    bool m_bIdentified; // has this item type been identified?
    bool m_bTried;      // has this item type been used without identifying?

protected:
private:
    // Member Functions
public:
    void FormatProperties( char *szOut, int maxLen, uint32 knownProps, uint32 itemFlags,
                           uint32 charges, float fACBonus, float fBonusToHit,
                           float fBonusToDamage );
    CEffect *FindTargetingEffect() const
    {
        if( !m_llEffects )
            return nullptr;
        CLink<CEffect> *pLink = m_llEffects->GetHead();
        while( pLink != NULL )
        {
            CEffect *pEffect = pLink->m_lpData;
            if( pEffect && pEffect->GetTargetType() != EFFECT_TARGET_NONE )
                return pEffect;
            pLink = pLink->next;
        }
        return nullptr;
    }

protected:
private:
};
class CItem
{
    // Member Variables
public:
    JVector m_vPos;
    CItemDef *m_id;
    CLink<CItem> *m_pllLink;
    int m_dwCount; // how many of this item are being carried?
    int m_dwFlags; // item cursed, or other specific to this instance, rather than in the general
                   // CItemDef
    uint32 m_dwCharges;     // for wands and staves and other items that have an "ammo count"
    uint32 m_dwMaxCharges;  // lifetime charge limit for recharge explosion curve
    uint32 m_dwInstanceId;  // unique instance id for this item
    uint32 m_dwKnownProps;  // bitmask of known properties (KNOWN_CURSED, KNOWN_BONUSES, etc.)
    float m_fACBonus;       // per-instance rolled magical AC bonus
    float m_fBonusToHit;    // per-instance rolled magical to-hit bonus
    float m_fBonusToDamage; // per-instance rolled magical to-damage bonus
    float m_fSpeedBonus;    // per-instance speed bonus (rings: random 0.1-1.0; boots/gloves: fixed)
protected:
    float m_fColorChangeInterval;
    JColor m_Color;

    float m_fRemainingDuration; // for potions, scrolls, torches -- "How long will this last?"

private:
    // Member Functions
public:
    CItem()
        : m_vPos( 0, 0 ),
          m_dwFlags( 0 ),
          m_dwCharges( 0 ),
          m_dwMaxCharges( 0 ),
          m_dwInstanceId( 0 ),
          m_dwKnownProps( 0 ),
          m_fACBonus( 0.0f ),
          m_fBonusToHit( 0.0f ),
          m_fBonusToDamage( 0.0f ),
          m_fSpeedBonus( 0.0f ),
          m_dwCount( 1 ),
          m_pllLink( NULL ),
          m_id( NULL ),
          m_fRemainingDuration( 0.0f ) {};

    uint32 GetInstanceId() { return m_dwInstanceId; }
    void Init( CItemDef *pid );
    void SetCursed( int likelihood );
    void SetCursed( bool bCursed );
    const char *GetName();
    const char *GetPlural();
    bool IsStackable() { return ( m_id->m_dwFlags & ITEM_FLAG_STACKS ) == ITEM_FLAG_STACKS; }
    bool IsIdentified() { return m_id->m_bIdentified || ( m_dwFlags & ITEM_FLAG_IDENTIFIED ); }
    void Identify()
    {
        m_id->m_bIdentified = true;
        m_dwKnownProps |= ( KNOWN_CURSED | KNOWN_BONUSES | KNOWN_CHARGES );
    }
    bool KnowsProperty( uint32 prop ) { return ( m_dwKnownProps & prop ) != 0; }
    void RevealProperty( uint32 prop ) { m_dwKnownProps |= prop; }
    bool IsOpenable() { return false; }   // for chests, etc.
    bool IsCloseable() { return false; }  // closeable pickup?
    bool IsTunnelable() { return false; } // Tunnelable pickup? unlikely.
    inline bool IsRanged()
    {
        return m_id->m_dwIndex == ITEM_IDX_BOW || m_id->m_dwIndex == ITEM_IDX_XBOW;
    };
    inline bool NeedsFuel()
    {
        return m_id->m_dwIndex == ITEM_IDX_STAFF || m_id->m_dwIndex == ITEM_IDX_WAND ||
               ( m_id->m_dwIndex == ITEM_IDX_TORCH && ( m_id->m_dwFlags & ITEM_FLAG_NEEDSAMMO ) );
    };
    inline bool IsWeapon() { return EquipType() == EQUIP_IDX_MAIN_HAND && !IsRanged(); };
    inline bool IsArmor()
    {
        int type = EquipType();
        return type == EQUIP_IDX_ARMOR || type == EQUIP_IDX_OFF_HAND || type == EQUIP_IDX_HELMET ||
               type == EQUIP_IDX_CLOAK || type == EQUIP_IDX_GLOVES || type == EQUIP_IDX_BOOTS;
    };
    int EquipType();
    bool IsWeakTo( uint32 dwElement );

    float GetDuration() { return m_fRemainingDuration; };
    void ChangeDuration( float fValue, bool bReset = false )
    {
        if( bReset )
        {
            m_fRemainingDuration = fValue;
        }
        else
        {
            m_fRemainingDuration += fValue;
        }
    };

    float GetRadius() { return m_id->m_fRadius; };

    static JResult CreateItem( CItemDef *pid, JVector vSpawnPoint = JVector( -1, -1 ),
                               bool bNear = false );
    void Imbue( int depth );
    CItem *
    Copy( int quantity = 0 ); // Create a copy of this item with specified quantity (0 = full stack)
    void Consume(); // Decrement charges (wands) or count (ammo); unified handler for fire/zap
                    // consumption
    bool
    IsConsumed(); // Check if item is empty (0 charges/count) and should be removed from inventory
    JResult SpawnItem( JVector vSpawnPoint = JVector( -1, -1 ) );
    JResult SpawnAt( JVector vSpawnPoint );

    bool Update( float fCurTime );
    void PreDraw();
    void Draw();
    void PostDraw();

protected:
    void SetColor();

private:
};

#endif // __ITEMTILE_H__
