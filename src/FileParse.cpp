#include "FileParse.h"
#include "Dungeon.h"
#include "EndGameState.h"
#include "Item.h"
#include "Monster.h"

CEffect *CDataFile::EffectFromName( const char *szName )
{
    if( m_pDungeon == NULL || szName == NULL || *szName == '\0' )
        return NULL;

    CEffectDef *pFound = m_pDungeon->GetEffectDef( szName );
    if( pFound == NULL )
    {
        JLog( LOG_LEVEL_WARN, true, "EffectFromName: unknown effect '%s'\n", szName );
        return NULL;
    }

    CEffect *pEffect = new CEffect;
    pEffect->m_ed = pFound;
    pEffect->m_dwEffect = pFound->m_dwEffect;
    pEffect->m_dwFlags = pFound->m_dwFlags;
    pEffect->m_dwFlags2 = pFound->m_dwFlags2;
    pEffect->m_dwModifier = pFound->m_dwModifier;
    pEffect->m_fDuration = pFound->m_fDuration;
    pEffect->SetAmount( pFound->m_szAmount );
    JLog( LOG_LEVEL_NOISE, true, "EffectFromName: resolved '%s'\n", szName );
    return pEffect;
}

bool CDataFile::Open( const char *szFilename )
{
    m_fp = fopen( szFilename, "r" );

    if( m_fp == NULL )
    {
        return false;
    }

    Util::Shuffle( PotionIndex, NUM_POTION_TYPES );
    Util::Shuffle( ScrollIndex, NUM_SCROLL_TYPES );
    Util::Shuffle( WandIndex, NUM_LUMBER_TYPES );
    Util::Shuffle( StaffIndex, NUM_LUMBER_TYPES );

    return true;
}

bool CDataFile::Append( const char *szFilename )
{
    m_fp = fopen( szFilename, "a" );

    if( m_fp == NULL )
    {
        return false;
    }

    return true;
}

bool CDataFile::Close()
{
    fclose( m_fp );
    m_fp = NULL;
    return true;
}

char *CDataFile::Strip( char *szLine )
{
    char *c;

    if( szLine == NULL || *szLine == nul || *szLine == '#' )
    {
        return NULL;
    }

    if( szLine[Util::jstrlen( szLine ) - 1] == '\n' )
    {
        szLine[Util::jstrlen( szLine ) - 1] = NULL;
    }

    if( *szLine == nul )
    {
        return NULL;
    }

    c = szLine;
    while( *c == ' ' || *c == '\t' || *c == '\n' )
    {
        c++;
    }
    szLine = c;

    c = strtok( szLine, "#" );
    if( c != NULL )
    {
        return c;
    }

    return szLine;
}

CMonsterDef *CDataFile::ReadMonster( CMonsterDef &mdIn )
{
    char szRaw[1024];
    char *szLine;
    char *szValue = NULL;
    bool bFoundMonster = false;
    bool bStartMonster = false;
    bool bEndMonster = false;

    while( !bEndMonster && fgets( szRaw, 1024, m_fp ) != NULL )
    {
        szLine = Strip( szRaw );
        if( szLine == NULL )
        {
            continue;
        }
        if( !bFoundMonster )
        {
            if( strncasecmp( szLine, "monster", 7 ) == 0 )
            {
                bFoundMonster = true;
                mdIn.m_szName = GetValue( szLine, mdIn.m_szName );
            }
            continue;
        }

        if( !bStartMonster )
        {
            if( *szLine == '{' )
            {
                bStartMonster = true;
            }
            continue;
        }

        // Once you get here, you know that you're
        // parsing a monster entry. Everything
        // from here to the next } is going to be
        // data for this monster.
        if( !bEndMonster )
        {
            if( strncasecmp( szLine, "plural", 6 ) == 0 )
            {
                mdIn.m_szPlural = GetValue( szLine, mdIn.m_szPlural );
            }
            else if( strncasecmp( szLine, "appear", 6 ) == 0 )
            {
                mdIn.m_szAppear = GetValue( szLine, mdIn.m_szAppear );
            }
            else if( strncasecmp( szLine, "speed", 5 ) == 0 )
            {
                mdIn.m_fSpeed = GetValue( szLine, mdIn.m_fSpeed );
            }
            else if( strncasecmp( szLine, "movetype", 8 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                if( g_Constants.CompareType( "MON_AI", szValue ) )
                {
                    mdIn.m_dwMoveType = g_Constants.LookupString( szValue );
                }
            }
            else if( strncasecmp( szLine, "hd", 2 ) == 0 )
            {
                mdIn.m_szHD = GetValue( szLine, mdIn.m_szHD );
            }
            else if( strncasecmp( szLine, "ac", 2 ) == 0 )
            {
                mdIn.m_fBaseAC = GetValue( szLine, mdIn.m_fBaseAC );
            }
            else if( strncasecmp( szLine, "levelsigma", 10 ) == 0 )
            {
                mdIn.m_fLevelSigma = GetValue( szLine, mdIn.m_fLevelSigma );
            }
            else if( strncasecmp( szLine, "level", 5 ) == 0 )
            {
                GetValue( szLine, mdIn.m_dwLevel );
            }
            else if( strncasecmp( szLine, "expvalue", 8 ) == 0 )
            {
                GetValue( szLine, mdIn.m_fExpValue );
            }
            else if( strncasecmp( szLine, "type", 4 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                if( g_Constants.CompareType( "MON_IDX", szValue ) )
                {
                    mdIn.m_dwIndex = g_Constants.LookupString( szValue );
                }
            }
            else if( strncasecmp( szLine, "flags", 5 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );

                char *c = strtok( szValue, "," );
                while( c != NULL )
                {
                    while( *c == ' ' || *c == '\t' )
                        c++;
                    // if( g_Constants.CompareType( "MON_FLAG", c ) )
                    {
                        JLog( LOG_LEVEL_NOISE, true, "found flag: %s\n", c );
                        mdIn.m_dwFlags |= g_Constants.LookupString( c );
                    }
                    c = strtok( NULL, "," );
                }
            }
            else if( strncasecmp( szLine, "attack", 6 ) == 0 )
            {
                CAttack *curAttack;
                curAttack = new CAttack;
                char *begin;
                char *end;
                char *cur;

                // Two supported formats:
                //   Old inline: Attack <EFFECT_TYPE_HIT>,<MON_FLAG_TOUCH>,1d2
                //               Attack
                //               <EFFECT_TYPE_HIT>,<MON_FLAG_BREATHE>,<EFFECT_FLAG_FIRE>,15d8,5
                //   New named:  Attack <MON_FLAG_BITE>,<Physical Hit>,2d6
                //               Attack <MON_FLAG_BREATHE>,<Firebolt>,2d8,15
                // Detect by checking whether the first token starts with "EFFECT_TYPE_".

                // first token
                begin = strchr( szLine, '<' );
                end = strchr( szLine, '>' );
                if( begin == NULL || end == NULL )
                {
                    JLog( LOG_LEVEL_ERROR, true,
                          "error parsing attack: first token not found: %s\n", szLine );
                    delete curAttack;
                    continue;
                }
                *end++ = NULL;
                begin++;
                cur = end;

                if( strncasecmp( begin, "EFFECT_TYPE_", 12 ) == 0 )
                {
                    // --- Old inline format ---
                    CEffect *pEffect = new CEffect;
                    pEffect->m_dwEffect = g_Constants.LookupString( begin );

                    // attack type (MON_FLAG_*)
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin == NULL || end == NULL )
                    {
                        JLog( LOG_LEVEL_ERROR, true,
                              "error parsing attack: attack type not found: %s\n", cur );
                        delete pEffect;
                        delete curAttack;
                        continue;
                    }
                    *end++ = NULL;
                    begin++;
                    curAttack->m_dwType = g_Constants.LookupString( begin );
                    cur = end;

                    // effect flag (optional)
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin != NULL && end != NULL )
                    {
                        *end++ = NULL;
                        begin++;
                        pEffect->m_dwFlags = g_Constants.LookupString( begin );
                        JLog( LOG_LEVEL_NOISE, true, "Found an Effect Flag: %s\n", begin );
                        cur = end;
                    }

                    curAttack->m_pEffect = pEffect;

                    // damage
                    begin = strchr( cur, ',' );
                    if( begin == NULL )
                    {
                        JLog( LOG_LEVEL_ERROR, true, "error parsing attack: damage not found: %s\n",
                              cur );
                        delete curAttack;
                        continue;
                    }
                    begin++;
                    cur = Strip( begin );
                    curAttack->m_szDamage = new char[Util::jstrlen( cur ) + 1];
                    Util::jstrcpy( curAttack->m_szDamage, cur );
                }
                else if( m_pDungeon != NULL )
                {
                    // --- New named-effect format ---
                    // first token is delivery type (MON_FLAG_*)
                    curAttack->m_dwType = g_Constants.LookupString( begin );

                    // second token: named effect
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin == NULL || end == NULL )
                    {
                        JLog( LOG_LEVEL_ERROR, true,
                              "error parsing attack: effect name not found: %s\n", cur );
                        delete curAttack;
                        continue;
                    }
                    char nameBuf[128];
                    int nameLen = (int)( end - begin - 1 );
                    if( nameLen > 0 && nameLen < (int)sizeof( nameBuf ) )
                    {
                        memcpy( nameBuf, begin + 1, nameLen );
                        nameBuf[nameLen] = '\0';
                    }
                    else
                    {
                        nameBuf[0] = '\0';
                    }
                    *end++ = NULL;
                    cur = end;

                    CEffect *pEffect = EffectFromName( nameBuf );
                    if( pEffect != NULL )
                    {
                        curAttack->m_pEffect = pEffect;

                        // optional dice override
                        begin = strchr( cur, ',' );
                        if( begin != NULL )
                        {
                            begin++;
                            cur = Strip( begin );
                            if( cur != NULL && *cur != '\0' )
                            {
                                curAttack->m_szDamage = new char[Util::jstrlen( cur ) + 1];
                                Util::jstrcpy( curAttack->m_szDamage, cur );
                            }
                        }
                        else if( pEffect->m_ed != NULL && pEffect->m_ed->m_szAmount != NULL )
                        {
                            // no dice override — fall back to effect's default amount
                            curAttack->m_szDamage =
                                new char[Util::jstrlen( pEffect->m_ed->m_szAmount ) + 1];
                            Util::jstrcpy( curAttack->m_szDamage, pEffect->m_ed->m_szAmount );
                        }
                    }
                }
                else
                {
                    JLog( LOG_LEVEL_WARN, true,
                          "Attack uses named effect but dungeon not set: %s\n", szLine );
                    delete curAttack;
                    continue;
                }

                // store it
                mdIn.m_llAttacks->Add( curAttack );
            }
            else if( strncasecmp( szLine, "color", 5 ) == 0 )
            {
                char *color = chomp( szLine, szValue );
                if( strchr( color, '<' ) != NULL )
                {
                    // multi-hued
                    // <<rgb1>,<rgb2>,...,<rgbn>>
                    JLog( LOG_LEVEL_DEBUG, true, "Found multi-hued monster: %s\n", color );
                    mdIn.m_Colors = ParseColors( color );

                    mdIn.m_dwFlags |= MON_COLOR_MULTI;
                }
                else
                {
                    // single-hued
                    // <rgb1>
                    mdIn.m_Color.SetColor( color );
                }
                delete[] color;
            }
            else if( *szLine == '}' )
            {
                bEndMonster = true;
            }
            else
            {
                JLog( LOG_LEVEL_WARN, true, "Unparseable line:%s\n", szLine );
            }
        }
    }
    if( !bEndMonster )
    {
        return NULL;
    }

    return &mdIn;
}

CItemDef *CDataFile::ReadItem( CItemDef &idIn )
{
    char szRaw[1024];
    char *szLine;
    char *szValue = NULL;
    bool bFoundItem = false;
    bool bStartItem = false;
    bool bEndItem = false;

    while( !bEndItem && fgets( szRaw, 1024, m_fp ) != NULL )
    {
        szLine = Strip( szRaw );
        if( szLine == NULL )
        {
            continue;
        }
        if( !bFoundItem )
        {
            if( strncasecmp( szLine, "Item", 4 ) == 0 )
            {
                bFoundItem = true;
                idIn.m_szName = GetValue( szLine, idIn.m_szName );
            }
            continue;
        }

        if( !bStartItem )
        {
            if( *szLine == '{' )
            {
                bStartItem = true;
            }
            continue;
        }

        // Once you get here, you know that you're
        // parsing a Item entry. Everything
        // from here to the next } is going to be
        // data for this Item.
        if( !bEndItem )
        {
            if( strncasecmp( szLine, "plural", 6 ) == 0 )
            {
                idIn.m_szPlural = GetValue( szLine, idIn.m_szPlural );
            }
            else if( strncasecmp( szLine, "speed", 5 ) == 0 )
            {
                idIn.m_fSpeed = GetValue( szLine, idIn.m_fSpeed );
            }
            else if( strncasecmp( szLine, "acbonus", 7 ) == 0 )
            {
                idIn.m_szACBonus = GetValue( szLine, idIn.m_szACBonus );
            }
            else if( strncasecmp( szLine, "ac", 2 ) == 0 )
            {
                idIn.m_fBaseAC = GetValue( szLine, idIn.m_fBaseAC );
            }
            else if( strncasecmp( szLine, "damage", 6 ) == 0 )
            {
                idIn.m_szBaseDamage = GetValue( szLine, idIn.m_szBaseDamage );
            }
            else if( strncasecmp( szLine, "to-hitbonus", 11 ) == 0 )
            {
                idIn.m_szBonusToHit = GetValue( szLine, idIn.m_szBonusToHit );
            }
            else if( strncasecmp( szLine, "to-dambonus", 11 ) == 0 )
            {
                idIn.m_szBonusToDamage = GetValue( szLine, idIn.m_szBonusToDamage );
            }
            else if( strncasecmp( szLine, "charges", 7 ) == 0 )
            {
                idIn.m_szCharges = GetValue( szLine, idIn.m_szCharges );
            }
            else if( strncasecmp( szLine, "levelsigma", 10 ) == 0 )
            {
                idIn.m_fLevelSigma = GetValue( szLine, idIn.m_fLevelSigma );
            }
            else if( strncasecmp( szLine, "level", 5 ) == 0 )
            {
                GetValue( szLine, idIn.m_dwLevel );
            }
            else if( strncasecmp( szLine, "value", 5 ) == 0 )
            {
                GetValue( szLine, idIn.m_fValue );
            }
            else if( strncasecmp( szLine, "weight", 6 ) == 0 )
            {
                GetValue( szLine, idIn.m_fWeight );
            }
            else if( strncasecmp( szLine, "radius", 6 ) == 0 )
            {
                idIn.m_fRadius = GetValue( szLine, idIn.m_fRadius );
            }
            else if( strncasecmp( szLine, "duration", 8 ) == 0 )
            {
                idIn.m_fDuration = GetValue( szLine, idIn.m_fDuration );
            }
            else if( strncasecmp( szLine, "type", 4 ) == 0 )
            {
                // TODO: Add validation that this is ITEM_IDX_ and not...
                szValue = GetValue( szLine, szValue );
                if( g_Constants.CompareType( "ITEM_IDX", szValue ) )
                {
                    idIn.m_dwIndex = g_Constants.LookupString( szValue );
                    if( idIn.m_dwIndex == ITEM_IDX_POTION )
                    {
                        int potion_index = PotionIndex[m_dwPotionCount];
                        idIn.m_szFlavor =
                            new char[Util::jstrlen( g_Constants.PotionColor( potion_index ) ) + 1];
                        Util::jstrcpy( idIn.m_szFlavor, g_Constants.PotionColor( potion_index ) );
                        char szUnID[100];
                        sprintf( szUnID, "%s Potion", idIn.m_szFlavor );
                        JLog( LOG_LEVEL_DEBUG, true, "Potion #%d - index %d color %s\n",
                              m_dwPotionCount, potion_index, szUnID );
                        idIn.m_szUnidentifiedName = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedName, szUnID );

                        sprintf( szUnID, "%s Potions", idIn.m_szFlavor );
                        idIn.m_szUnidentifiedPlural = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedPlural, szUnID );

                        sprintf( szUnID, "%s", g_Constants.PotionRGBA( potion_index ) );
                        idIn.m_Color.SetColor( szUnID );
                        m_dwPotionCount++;
                    }
                    else if( idIn.m_dwIndex == ITEM_IDX_SCROLL )
                    {
                        int scroll_index = ScrollIndex[m_dwScrollCount];
                        idIn.m_szFlavor =
                            new char[Util::jstrlen( g_Constants.ScrollName( scroll_index ) ) + 1];
                        Util::jstrcpy( idIn.m_szFlavor, g_Constants.ScrollName( scroll_index ) );
                        char szUnID[100];
                        sprintf( szUnID, "Scroll labeled %s", idIn.m_szFlavor );
                        idIn.m_szUnidentifiedName = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedName, szUnID );
                        sprintf( szUnID, "Scrolls labeled %s", idIn.m_szFlavor );
                        idIn.m_szUnidentifiedPlural = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedPlural, szUnID );
                        m_dwScrollCount++;
                    }
                    else if( idIn.m_dwIndex == ITEM_IDX_STAFF )
                    {
                        int staff_index = StaffIndex[m_dwStaffCount];
                        idIn.m_szFlavor =
                            new char[Util::jstrlen( g_Constants.Lumber( staff_index ) ) + 1];
                        Util::jstrcpy( idIn.m_szFlavor, g_Constants.Lumber( staff_index ) );
                        char szUnID[100];
                        sprintf( szUnID, "%s Staff", idIn.m_szFlavor );
                        JLog( LOG_LEVEL_DEBUG, true, "Staff #%d - index %d color %s\n",
                              m_dwStaffCount, staff_index, szUnID );
                        idIn.m_szUnidentifiedName = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedName, szUnID );

                        sprintf( szUnID, "%s Staves", idIn.m_szFlavor );
                        idIn.m_szUnidentifiedPlural = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedPlural, szUnID );

                        sprintf( szUnID, "%s", g_Constants.LumberRGBA( staff_index ) );
                        idIn.m_Color.SetColor( szUnID );
                        m_dwStaffCount++;
                    }
                    else if( idIn.m_dwIndex == ITEM_IDX_WAND )
                    {
                        int wand_index = WandIndex[m_dwWandCount];
                        idIn.m_szFlavor =
                            new char[Util::jstrlen( g_Constants.Lumber( wand_index ) ) + 1];
                        Util::jstrcpy( idIn.m_szFlavor, g_Constants.Lumber( wand_index ) );
                        char szUnID[100];
                        sprintf( szUnID, "%s Wand", idIn.m_szFlavor );
                        JLog( LOG_LEVEL_DEBUG, true, "Wand #%d - index %d color %s\n",
                              m_dwWandCount, wand_index, szUnID );
                        idIn.m_szUnidentifiedName = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedName, szUnID );

                        sprintf( szUnID, "%s Wands", idIn.m_szFlavor );
                        idIn.m_szUnidentifiedPlural = new char[Util::jstrlen( szUnID ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedPlural, szUnID );

                        sprintf( szUnID, "%s", g_Constants.LumberRGBA( wand_index ) );
                        idIn.m_Color.SetColor( szUnID );
                        m_dwWandCount++;
                    }
                    else
                    {
                        idIn.m_szUnidentifiedName = new char[Util::jstrlen( idIn.m_szName ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedName, idIn.m_szName );
                        idIn.m_szUnidentifiedPlural =
                            new char[Util::jstrlen( idIn.m_szPlural ) + 1];
                        Util::jstrcpy( idIn.m_szUnidentifiedPlural, idIn.m_szPlural );
                    }
                }
            }
            else if( strncasecmp( szLine, "flags", 5 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );

                char *c = strtok( szValue, "," );
                while( c != NULL )
                {
                    // if( g_Constants.CompareType( "ITEM_FLAG", c ) )
                    {
                        idIn.m_dwFlags |= g_Constants.LookupString( c );
                    }
                    c = strtok( NULL, "," );
                }
            }
            else if( strncasecmp( szLine, "color", 5 ) == 0 )
            {
                char *color = chomp( szLine, szValue );
                if( strchr( color, '<' ) != NULL )
                {
                    // multi-hued
                    // <<rgb1>,<rgb2>,...,<rgbn>>
                    JLog( LOG_LEVEL_DEBUG, true, "Found multi-hued item: %s\n", color );
                    idIn.m_Colors = ParseColors( color );

                    idIn.m_dwFlags |= ITEM_COLOR_MULTI;
                }
                else
                {
                    // single-hued
                    // <rgb1>
                    idIn.m_Color.SetColor( color );
                }
                delete[] color;
            }
            else if( strncasecmp( szLine, "effect", 6 ) == 0 )
            {
                CEffect *curEffect;
                curEffect = new CEffect;
                char *begin;
                char *end;
                char *cur;

                // Named reference: Effect <Firebolt>
                // Inline:          Effect <EFFECT_TYPE_HEAL>,<EFFECT_FLAG_HP>
                // Detect by checking for comma after the first <...> pair
                begin = strchr( szLine, '<' );
                end = strchr( szLine, '>' );
                if( begin == NULL || end == NULL )
                {
                    JLog( LOG_LEVEL_ERROR, true, "Error parsing effect: type not found\n" );
                    delete curEffect;
                    continue;
                }

                // Check if there's a comma after the first <...> — inline vs named
                char *comma = strchr( end + 1, ',' );
                char *nextAngle = strchr( end + 1, '<' );
                bool bIsNamed = ( comma == NULL && nextAngle == NULL );

                if( bIsNamed && m_pDungeon != NULL )
                {
                    char nameBuf[128];
                    int nameLen = (int)( end - begin - 1 );
                    if( nameLen > 0 && nameLen < (int)sizeof( nameBuf ) )
                    {
                        memcpy( nameBuf, begin + 1, nameLen );
                        nameBuf[nameLen] = '\0';
                    }
                    else
                    {
                        nameBuf[0] = '\0';
                    }

                    delete curEffect; // EffectFromName allocates its own
                    curEffect = EffectFromName( nameBuf );
                    if( curEffect != NULL )
                        idIn.m_llEffects->Add( curEffect );
                }
                else
                {
                    // Inline format: <EFFECT_TYPE>,<EFFECT_FLAG>[,<EFFECT_MOD>][,<amount>]
                    *end++ = NULL;
                    begin++;
                    {
                        JLog( LOG_LEVEL_NOISE, true, "%s ", begin );
                        curEffect->m_dwEffect = g_Constants.LookupString( begin );
                    }
                    cur = end;

                    // flag
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin == NULL || end == NULL )
                    {
                        JLog( LOG_LEVEL_ERROR, true, "Error parsing effect: flag not found\n" );
                        delete curEffect;
                        continue;
                    }
                    *end++ = NULL;
                    begin++;

                    JLog( LOG_LEVEL_NOISE, false, "%s ", begin );
                    g_Constants.LookupEffectFlag( begin, curEffect->m_dwFlags,
                                                  curEffect->m_dwFlags2 );
                    cur = end;

                    // modifier (optional)
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin != NULL && end != NULL )
                    {
                        JLog( LOG_LEVEL_NOISE, true, "Found effect modifier\n" );
                        *end++ = NULL;
                        begin++;
                        {
                            JLog( LOG_LEVEL_NOISE, false, "%s ", begin );
                            curEffect->m_dwModifier = g_Constants.LookupString( begin );
                        }
                        cur = end;
                    }

                    // amount (optional — 4th comma-separated <NdM> field)
                    begin = strchr( cur, ',' );
                    if( begin != NULL )
                    {
                        begin++;
                        cur = Strip( begin );
                        begin = strchr( cur, '<' );
                        end = strchr( cur, '>' );
                        if( begin != NULL && end != NULL )
                        {
                            *end = NULL;
                            begin++;
                            curEffect->SetAmount( begin );
                            JLog( LOG_LEVEL_NOISE, false, "amount=%s ", curEffect->m_szAmount );
                        }
                    }

                    JLog( LOG_LEVEL_NOISE, false, "\n" );
                    // store it
                    idIn.m_llEffects->Add( curEffect );
                }
            }
            else if( *szLine == '}' )
            {
                bEndItem = true;
            }
            else
            {
                JLog( LOG_LEVEL_WARN, true, "Unparseable line:%s\n", szLine );
            }
        }
    }
    if( !bEndItem )
    {
        return NULL;
    }

    return &idIn;
}

CEffectDef *CDataFile::ReadEffect( CEffectDef &edIn )
{
    char szRaw[1024];
    char *szLine;
    char *szValue = NULL;
    bool bFoundEffect = false;
    bool bStartEffect = false;
    bool bEndEffect = false;

    while( !bEndEffect && fgets( szRaw, 1024, m_fp ) != NULL )
    {
        szLine = Strip( szRaw );
        if( szLine == NULL )
        {
            continue;
        }
        if( !bFoundEffect )
        {
            if( strncasecmp( szLine, "effect", 6 ) == 0 )
            {
                bFoundEffect = true;
                edIn.m_szName = GetValue( szLine, edIn.m_szName );
                JLog( LOG_LEVEL_NOISE, true, "Found effect: %s\n", edIn.m_szName );
            }
            continue;
        }

        if( !bStartEffect )
        {
            if( *szLine == '{' )
            {
                bStartEffect = true;
            }
            continue;
        }

        if( !bEndEffect )
        {
            if( strncasecmp( szLine, "type", 4 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                edIn.m_dwEffect = g_Constants.LookupString( szValue );
            }
            else if( strncasecmp( szLine, "flag2", 5 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                g_Constants.LookupEffectFlag( szValue, edIn.m_dwFlags, edIn.m_dwFlags2 );
            }
            else if( strncasecmp( szLine, "flag", 4 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                g_Constants.LookupEffectFlag( szValue, edIn.m_dwFlags, edIn.m_dwFlags2 );
            }
            else if( strncasecmp( szLine, "modifier", 8 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );
                edIn.m_dwModifier = g_Constants.LookupString( szValue );
            }
            else if( strncasecmp( szLine, "amount", 6 ) == 0 )
            {
                edIn.m_szAmount = GetValue( szLine, edIn.m_szAmount );
            }
            else if( strncasecmp( szLine, "duration", 8 ) == 0 )
            {
                edIn.m_fDuration = GetValue( szLine, edIn.m_fDuration );
            }
            else if( strncasecmp( szLine, "range", 5 ) == 0 )
            {
                edIn.m_fRange = GetValue( szLine, edIn.m_fRange );
            }
            else if( strncasecmp( szLine, "radius", 6 ) == 0 )
            {
                edIn.m_fRadius = GetValue( szLine, edIn.m_fRadius );
            }
            else if( *szLine == '}' )
            {
                bEndEffect = true;
            }
            else
            {
                JLog( LOG_LEVEL_WARN, true, "Unparseable line:%s\n", szLine );
            }
        }
    }
    if( !bEndEffect )
    {
        return NULL;
    }

    return &edIn;
}

CScore *CDataFile::ReadScore( CScore &sIn )
{
    char szRaw[1024];
    char *szLine;
    char *szValue = NULL;
    bool bFoundScore = false;
    bool bStartScore = false;
    bool bEndScore = false;

    while( !bEndScore && fgets( szRaw, 1024, m_fp ) != NULL )
    {
        szLine = Strip( szRaw );
        if( szLine == NULL )
        {
            continue;
        }
        if( !bFoundScore )
        {
            if( strncasecmp( szLine, "Score", 4 ) == 0 )
            {
                bFoundScore = true;
                sIn.m_szName = GetValue( szLine, sIn.m_szName );
            }
            continue;
        }

        if( !bStartScore )
        {
            if( *szLine == '{' )
            {
                bStartScore = true;
            }
            continue;
        }

        // Once you get here, you know that you're
        // parsing a Score entry. Everything
        // from here to the next } is going to be
        // data for this Score.
        if( !bEndScore )
        {
            if( strncasecmp( szLine, "level", 5 ) == 0 )
            {
                sIn.m_dwLevel = GetValue( szLine, sIn.m_dwLevel );
            }
            else if( strncasecmp( szLine, "depth", 5 ) == 0 )
            {
                sIn.m_dwDepth = GetValue( szLine, sIn.m_dwDepth );
            }
            else if( strncasecmp( szLine, "reason", 6 ) == 0 )
            {
                sIn.m_szKilledBy = GetValue( szLine, sIn.m_szKilledBy );
            }
            else if( strncasecmp( szLine, "class", 5 ) == 0 )
            {
                sIn.m_szClass = GetValue( szLine, sIn.m_szClass );
            }
            else if( strncasecmp( szLine, "race", 4 ) == 0 )
            {
                sIn.m_szRace = GetValue( szLine, sIn.m_szRace );
            }
            else if( strncasecmp( szLine, "score", 5 ) == 0 )
            {
                sIn.m_dwScore = GetValue( szLine, sIn.m_dwScore );
            }
            else if( strncasecmp( szLine, "date", 4 ) == 0 )
            {
                sIn.m_dwDate = GetValue( szLine, sIn.m_dwDate );
            }
            else if( *szLine == '}' )
            {
                bEndScore = true;
            }
            else
            {
                JLog( LOG_LEVEL_WARN, true, "Unparseable line:%s\n", szLine );
            }
        }
    }
    if( !bEndScore )
    {
        return NULL;
    }

    return &sIn;
}

bool CDataFile::WriteScore( CScore *sIn )
{
    fprintf( m_fp,
             "Score "
             "<%s>\n{\n\tClass\t<%s>\n\tRace\t<%s>\n\tLevel\t%d\n\tDepth\t%d\n\tReason\t<%s>"
             "\n\tScore\t%d\n\tDate\t%ld\n}\n",
             sIn->m_szName, sIn->m_szClass, sIn->m_szRace, sIn->m_dwLevel, sIn->m_dwDepth,
             sIn->m_szKilledBy, sIn->m_dwScore, sIn->m_dwDate );
    fflush( m_fp );
    return true;
}

// Removes outermost <> from data entry
char *CDataFile::GetValue( char *szLine, char *szIn ) { return chomp( szLine, szIn ); }
char *CDataFile::chomp( const char *szLine, char *szIn )
{
    if( szLine == NULL || *szLine == nul )
    {
        return NULL;
    }

    char *copy;

    copy = new char[Util::jstrlen( szLine ) + 1];
    Util::jstrcpy( copy, szLine );
    char *begin;
    char *end;
    begin = strchr( copy, '<' );
    end = strrchr( copy, '>' );

    if( begin == NULL || end == NULL )
    {
        return NULL;
    }
    *end = NULL;
    begin++;

    szIn = new char[Util::jstrlen( begin ) + 1];
    Util::jstrcpy( szIn, begin );

    delete[] copy;
    return szIn;
}

JLinkList<JColor> *CDataFile::ParseColors( char *szLine )
{
    JColor *outcolor = NULL;
    char szToken[32][32];
    int count = 0;
    char *temp = NULL;
    JLinkList<JColor> *retval = new JLinkList<JColor>;
    char *c = strtok( szLine, ";" );
    while( c != NULL )
    {
        memset( szToken[count], 0, 32 );
        Util::jstrcpy( szToken[count++], c );
        c = strtok( NULL, ";" );
    }

    for( int i = 0; i < count; i++ )
    {
        char *tok = szToken[i];
        temp = chomp( tok, temp );
        outcolor = new JColor();
        outcolor->SetColor( temp );
        retval->Add( outcolor );
        delete[] temp;
    }

    return retval;
}

int CDataFile::GetValue( char *szLine, long &dwIn )
{
    char label[100];
    char value[100];
    sscanf( szLine, "%s %s\n", label, value );
    dwIn = atoi( value );

    return (int)dwIn;
}
int CDataFile::GetValue( char *szLine, int &dwIn )
{
    char label[100];
    char value[100];
    sscanf( szLine, "%s %s\n", label, value );
    dwIn = atoi( value );

    return dwIn;
}

float CDataFile::GetValue( char *szLine, float &fIn )
{
    char label[100];
    char value[100];
    sscanf( szLine, "%s %s\n", label, value );
    fIn = (float)atof( value );

    return fIn;
}
