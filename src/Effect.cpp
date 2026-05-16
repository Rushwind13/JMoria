#include "Effect.h"
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