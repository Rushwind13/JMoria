#ifndef __EFFECT_H__
#define __EFFECT_H__
// clang-format off
#include "JMDefs.h"
// clang-format on
#include "JColor.h"
#include "JLinkList.h"
#include "Util.h"

class CItem;
class CItemDef;

// Targeting requirement for an effect — what, if anything, must the player choose before dispatch.
enum eEffectTargetType
{
    EFFECT_TARGET_NONE = 0,  // no player choice needed; fires immediately
    EFFECT_TARGET_DIRECTION, // player must choose a direction first
    EFFECT_TARGET_ITEM,      // player must choose an item (inv or equip) first
};

// Per-effect string slot indices — used to index CEffectDef::m_dwStrIds[].
// Each slot stores a eStringId value (or STR_INVALID if not set).
enum eEffectStrSlot
{
    EFFECT_STR_NAME = 0,       // display name for "The %s emits a %s." etc.
    EFFECT_STR_HIT_PLAYER,     // "You are hit by %s."  (overrides generic)
    EFFECT_STR_HIT_MONSTER,    // "The %s is hit by %s."  (overrides generic)
    EFFECT_STR_EMIT_PLAYER,    // emitting string for player-fired beams
    EFFECT_STR_EMIT_MONSTER,   // emitting string for monster-fired beams
    EFFECT_STR_STATUS_PLAYER,  // status verb fragment for player ("are paralyzed")
    EFFECT_STR_STATUS_MONSTER, // status verb fragment for monster ("is paralyzed")
    EFFECT_STR_STAT_LABEL,     // stats-panel status label ("Paralyzed")
    EFFECT_STR_SLOT_MAX
};

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
        for( int i = 0; i < EFFECT_STR_SLOT_MAX; i++ )
            m_dwStrIds[i] = STR_INVALID;
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
    int m_dwStrIds[EFFECT_STR_SLOT_MAX]; // per-slot string IDs (eStringId values)
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
    JResult LockDoor( JVector vOrigin );

    // Returns an elemental affinity multiplier for an incoming effect vs a subject's element flags.
    // dwEffect   — EFFECT_FLAG_* bits describing the incoming attack/effect.
    // dwSubject  — EFFECT_FLAG_* bits describing the subject's elemental nature (attacks /
    // intrinsics). Returns: 0.0 = immune, 1.0 = normal, 2.0 = weak. Opposite pairs: FIRE<->COLD,
    // ELECTRICITY<->ACID.
    static float CheckAffinity( uint32 dwEffect, uint32 dwSubject );

    // Fire a named effect from the loaded definitions at a given origin.
    // Stack-allocated; no heap allocation or memory leak.
    static JResult Fire( const char *szEffectName, JVector vOrigin );
    static JResult DispatchAll( CLink<CEffect> *plEffect, float fDuration, int dwItemFlags );

    JResult SummonMonsters( JVector vOrigin );

    // Top-level dispatch — called once per effect in the effect list.
    JResult Dispatch( float fDuration, int dwItemFlags );
    // Hit-effect sub-dispatch — selects Apply* based on modifier and flag.
    JResult DoHitEffects();
    JResult DoHitEffects( JVector vCasterPos, JVector vTargetPos );

    // Targeting: what player choice (if any) is needed before this effect can fire.
    eEffectTargetType GetTargetType() const;
    // Returns true if pItem is a legal target for this effect.
    bool IsValidTarget( CItem *pItem ) const;
    // Human-readable prompt to show the player when selecting a target.
    const char *GetTargetPrompt() const;

    CEffectDef *m_ed; // pointer to shared effect definition (NULL for inline effects)
    int m_dwEffect;
    uint32 m_dwFlags;
    uint32 m_dwFlags2;
    int m_dwModifier;
    char *m_szAmount;
    float m_fDuration;
};
#endif // __EFFECT_H__