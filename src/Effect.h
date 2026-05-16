#ifndef __EFFECT_H__
#define __EFFECT_H__
// clang-format off
#include "JMDefs.h"
// clang-format on
#include "JColor.h"
#include "JLinkList.h"
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
          m_fRadius( 0.0f ),
          m_cBeamChar( '*' ),
          m_llColors( NULL )
    {
        m_llColors = new JLinkList<JColor>;
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
        if( m_llColors )
        {
            m_llColors->Terminate();
            delete m_llColors;
            m_llColors = NULL;
        }
    }
    char *m_szName;                // "Firebolt", "Light Ray", etc.
    int m_dwEffect;                // EFFECT_TYPE_HIT, EFFECT_TYPE_HEAL, etc.
    uint32 m_dwFlags;              // EFFECT_FLAG_FIRE, EFFECT_FLAG_LIGHT, etc.
    uint32 m_dwFlags2;             // EFFECT_FLAG_DOOR, EFFECT_FLAG_NO_COLLIDE, etc.
    int m_dwModifier;              // EFFECT_MOD_LINE, EFFECT_MOD_BALL, etc.
    char *m_szAmount;              // NdM dice string for damage/healing per use
    float m_fDuration;             // for timed effects
    float m_fRange;                // max range in tiles
    float m_fRadius;               // AoE radius (0 = single target)
    char m_cBeamChar;              // character for beam rendering (default '*')
    JLinkList<JColor> *m_llColors; // multicolor beam cycling
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

    void SetAmount( const char *szAmount );
    float Resist() const;
    bool HasFlag( const char *szFlag );
    const char *Effect();

    JResult Area( JVector vOrigin );
    JResult Ball( JVector vOrigin );
    JResult Line( JVector vOrigin );
    JResult LightRay( JVector vOrigin );
    JResult Elemental( JVector vOrigin );
    JResult Physical( JVector vOrigin );
    JResult Status( JVector vOrigin, uint32 dwFlag );
    JResult StoneToMud( JVector vOrigin );
    JResult TeleportAway( JVector vOrigin );
    JResult Probe( JVector vOrigin );
    JResult HealMonster( JVector vOrigin );
    JResult Aggravate( JVector vOrigin );

    // Top-level dispatch — called once per effect in the effect list.
    JResult Dispatch( float fDuration, int dwItemFlags );
    // Hit-effect sub-dispatch — selects Apply* based on modifier and flag.
    JResult DoHitEffects();
    JResult DoHitEffects( JVector vCasterPos, JVector vTargetPos );

    CEffectDef *m_ed; // pointer to shared effect definition (NULL for inline effects)
    int m_dwEffect;
    uint32 m_dwFlags;
    uint32 m_dwFlags2;
    int m_dwModifier;
    char *m_szAmount;
    float m_fDuration;
};
#endif // __EFFECT_H__