#ifndef __ITEM_H__
#define __ITEM_H__

#include "JLinkList.h"
#include "JMDefs.h"

class CEffect
{
public:
    CEffect() : m_dwEffect( -1 ), m_szAmount( NULL ), m_fDuration( 0 ) {}
    ~CEffect() {}
    int m_dwEffect;
    int m_dwFlags;
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
          m_fSpeed( 0.0f ),
          m_fACBonus( 0.0f ),
          m_fBaseAC( 0.0f ),
          m_szBaseDamage( NULL ),
          m_fBonusToHit( 0.0f ),
          m_fBonusToDamage( 0.0f ),
          m_dwLevel( 0 ),
          m_fValue( 0.0f ),
          m_fWeight( 0.0f ),
          m_fRadius( 0.0f ),
          m_fDuration( 0.0f ),
          m_dwFlags( 0 ),
          m_dwIndex( ITEM_IDX_INVALID ),
          m_dwBaseHP( 0.0f )
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
        if( m_szBaseDamage )
        {
            delete[] m_szBaseDamage;
            m_szBaseDamage = NULL;
        }
        if( m_Colors )
        {
            m_Colors->Terminate();
            m_Colors = NULL;
        }
        if( m_llEffects )
        {
            m_llEffects->Terminate();
            m_llEffects = NULL;
        }
    }
    char *m_szName; // what item is this?
    char *m_szPlural;
    float m_fSpeed;
    float m_fACBonus;
    float m_fBaseAC;
    char *m_szBaseDamage;
    float m_fBonusToHit;
    float m_fBonusToDamage;
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

protected:
private:
    // Member Functions
public:
    //    virtual int HandleModify(/*cmd?*/); // how do you deal with a modify cmd?
    // ... need one of these per state (virt in base, defined in subclasses)?
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
          m_dwCount( 1 ),
          m_pllLink( NULL ),
          m_id( NULL ),
          m_fRemainingDuration( 0.0f ) {};
    void Init( CItemDef *pid );
    void SetCursed( int likelihood );
    void SetCursed( bool bCursed );
    char *GetName() { return m_id->m_szName; }
    char *GetPlural() { return m_id->m_szPlural; }
    bool IsStackable() { return ( m_id->m_dwFlags & ITEM_FLAG_STACKS ) == ITEM_FLAG_STACKS; }
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
