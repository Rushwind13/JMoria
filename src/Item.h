#ifndef __ITEM_H__
#define __ITEM_H__

#include "JLinkList.h"
#include "JMDefs.h"
#include "Util.h"

// Named effect template — shared catalog entry parsed from Effects.txt.
// Items, monsters, and spells reference these by name.
class CEffectDef
{
public:
    CEffectDef()
        : m_szName( NULL ),
          m_dwEffect( -1 ),
          m_dwFlags( 0 ),
          m_dwFlags2( 0 ),
          m_dwModifier( 0 ),
          m_szAmount( NULL ),
          m_fDuration( 0 ),
          m_fRange( 0.0f ),
          m_fRadius( 0.0f )
    {
    }
    ~CEffectDef()
    {
        if( m_szName )
        {
            delete[] m_szName;
            m_szName = NULL;
        }
        if( m_szAmount )
        {
            delete[] m_szAmount;
            m_szAmount = NULL;
        }
    }
    char *m_szName;    // "Firebolt", "Light Ray", etc.
    int m_dwEffect;    // EFFECT_TYPE_HIT, EFFECT_TYPE_HEAL, etc.
    uint32 m_dwFlags;  // EFFECT_FLAG_FIRE, EFFECT_FLAG_LIGHT, etc.
    uint32 m_dwFlags2; // EFFECT_FLAG_DOOR, EFFECT_FLAG_NO_COLLIDE, etc.
    int m_dwModifier;  // EFFECT_MOD_LINE, EFFECT_MOD_BALL, etc.
    char *m_szAmount;  // NdM dice string for damage/healing per use
    float m_fDuration; // for timed effects
    float m_fRange;    // max range in tiles
    float m_fRadius;   // AoE radius (0 = single target)
};

class CEffect
{
public:
    CEffect()
        : m_ed( NULL ),
          m_dwEffect( -1 ),
          m_dwFlags( 0 ),
          m_dwFlags2( 0 ),
          m_dwModifier( 0 ),
          m_szAmount( NULL ),
          m_fDuration( 0 )
    {
    }
    CEffect( const CEffect &other )
        : m_ed( other.m_ed ),
          m_dwEffect( other.m_dwEffect ),
          m_dwFlags( other.m_dwFlags ),
          m_dwFlags2( other.m_dwFlags2 ),
          m_dwModifier( other.m_dwModifier ),
          m_szAmount( NULL ),
          m_fDuration( other.m_fDuration )
    {
        SetAmount( other.m_szAmount );
    }
    CEffect &operator=( const CEffect &other )
    {
        if( this != &other )
        {
            m_ed = other.m_ed;
            m_dwEffect = other.m_dwEffect;
            m_dwFlags = other.m_dwFlags;
            m_dwFlags2 = other.m_dwFlags2;
            m_dwModifier = other.m_dwModifier;
            m_fDuration = other.m_fDuration;
            SetAmount( other.m_szAmount );
        }
        return *this;
    }
    ~CEffect()
    {
        if( m_szAmount )
        {
            delete[] m_szAmount;
            m_szAmount = NULL;
        }
    }
    void SetAmount( const char *szAmount )
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
    float Resist() const
    {
        if( m_dwModifier & EFFECT_MOD_IMMUNE )
            return 0.0f;
        if( m_dwModifier & EFFECT_MOD_RESIST )
            return 0.5f;
        if( m_dwModifier & EFFECT_MOD_WEAK )
            return 2.0f;
        return 1.0f;
    }
    bool HasFlag( const char *szFlag )
    {
        return g_Constants.CheckEffectFlag( szFlag, m_dwFlags, m_dwFlags2 );
    }
    CEffectDef *m_ed; // pointer to shared effect definition (NULL for inline effects)
    int m_dwEffect;
    uint32 m_dwFlags;
    uint32 m_dwFlags2;
    int m_dwModifier;
    char *m_szAmount;
    float m_fDuration;
};

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
          m_fSpeed( 0.0f ),
          m_szACBonus( NULL ),
          m_fBaseAC( 0.0f ),
          m_szBaseDamage( NULL ),
          m_szBonusToHit( NULL ),
          m_szBonusToDamage( NULL ),
          m_szCharges( NULL ),
          m_dwLevel( 0 ),
          m_fValue( 0.0f ),
          m_fWeight( 0.0f ),
          m_fRadius( 0.0f ),
          m_fDuration( 0.0f ),
          m_dwFlags( 0 ),
          m_dwIndex( ITEM_IDX_INVALID ),
          m_dwBaseHP( 0.0f ),
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
    float m_fSpeed;
    char *m_szACBonus; // NdM dice string for magical AC bonus (rolled per-instance)
    float m_fBaseAC;
    char *m_szBaseDamage;
    char *m_szBonusToHit;    // NdM dice string for magical to-hit bonus (rolled per-instance)
    char *m_szBonusToDamage; // NdM dice string for magical to-damage bonus (rolled per-instance)
    char *m_szCharges;       // NdM dice string for initial charges (rolled per-instance)
    int m_dwLevel;
    float m_fValue;
    float m_fWeight;
    int m_dwFlags;
    int m_dwIndex;     // ITEM_IDX_SWORD, ITEM_IDX_WAND, etc.
    int m_dwBaseHP;    // for busting down walls, disarming traps, etc.
    float m_fDuration; // for potions, scrolls, torches -- "How long will this last?"
    float m_fRadius;   // for AoE effects -- "How big is the badaboom?"
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
        RevealAllProperties();
    }
    bool KnowsProperty( uint32 prop ) { return ( m_dwKnownProps & prop ) != 0; }
    void RevealProperty( uint32 prop ) { m_dwKnownProps |= prop; }
    void RevealAllProperties()
    {
        m_dwKnownProps |= ( KNOWN_CURSED | KNOWN_BONUSES | KNOWN_CHARGES );
    }
    bool IsOpenable() { return false; }   // for chests, etc.
    bool IsCloseable() { return false; }  // closeable pickup?
    bool IsTunnelable() { return false; } // Tunnelable pickup? unlikely.
    int EquipType();

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
