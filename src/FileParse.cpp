#include "FileParse.h"
#include "EndGameState.h"
#include "Item.h"
#include "Monster.h"
#include <string.h>

bool CDataFile::Open( const char *szFilename )
{
    m_fp = fopen( szFilename, "r" );

    if( m_fp == NULL )
    {
        return false;
    }

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

    if( szLine[strlen( szLine ) - 1] == '\n' )
    {
        szLine[strlen( szLine ) - 1] = NULL;
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
                mdIn.m_dwMoveType = g_Constants.LookupString( szValue );
            }
            else if( strncasecmp( szLine, "hd", 2 ) == 0 )
            {
                mdIn.m_szHD = GetValue( szLine, mdIn.m_szHD );
            }
            else if( strncasecmp( szLine, "ac", 2 ) == 0 )
            {
                mdIn.m_fBaseAC = GetValue( szLine, mdIn.m_fBaseAC );
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
                mdIn.m_dwIndex = g_Constants.LookupString( szValue );
            }
            else if( strncasecmp( szLine, "flags", 5 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );

                char *c = strtok( szValue, "," );
                while( c != NULL )
                {
                    mdIn.m_dwFlags |= g_Constants.LookupString( c );
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
                bool bDone = false;
                // Attack <EFFECT_TYPE_HIT>,<MON_FLAG_TOUCH>,1d2 -or-
                // Attack <EFFECT_TYPE_HIT>,<MON_FLAG_BREATHE>,<EFFECT_FLAG_FIRE>,15d8,5
                // effect type
                begin = strchr( szLine, '<' );
                end = strchr( szLine, '>' );
                if( begin == NULL || end == NULL )
                {
                    printf( "error parsing attack: effect type not found %s\n", cur );
                    continue;
                }
                *end++ = NULL;
                begin++;
                curAttack->m_dwEffect = g_Constants.LookupString( begin );
                cur = end;

                // attack type
                begin = strchr( cur, '<' );
                end = strchr( cur, '>' );
                if( begin == NULL || end == NULL )
                {
                    printf( "error parsing attack: attack type not found %s\n", cur );
                    continue;
                }
                *end++ = NULL;
                begin++;
                curAttack->m_dwType = g_Constants.LookupString( begin );
                cur = end;

                // effect flag (optional)
                if( !bDone )
                {
                    cur = end;
                    // effect
                    begin = strchr( cur, '<' );
                    end = strchr( cur, '>' );
                    if( begin != NULL && end != NULL )
                    {
                        *end = NULL;
                        begin++;
                        curAttack->m_dwEffect = g_Constants.LookupString( begin );
                    }
                }

                // damage
                begin = strchr( cur, ',' );
                if( begin == NULL )
                {
                    printf( "error parsing attack: damage not found %s\n", cur );
                    continue;
                }
                begin++;
                // from here on out, you've got enough info to do this.
                end = strchr( begin, ',' );
                if( end != NULL )
                {
                    *end++ = NULL;
                }
                else
                {
                    bDone = true;
                }

                cur = Strip( begin );
                curAttack->m_szDamage = new char[strlen( cur ) + 1];
                strcpy( curAttack->m_szDamage, cur );

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
                    printf( "Found multi-hued monster: %s\n", color );
                    mdIn.m_Colors = ParseColors( color );

                    mdIn.m_dwFlags |= MON_COLOR_MULTI;
                }
                else
                {
                    // single-hued
                    // <rgb1>
                    mdIn.m_Color.SetColor( color );
                }
            }
            else if( *szLine == '}' )
            {
                bEndMonster = true;
            }
            else
            {
                printf( "Unparseable line:%s\n", szLine );
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
                idIn.m_fACBonus = GetValue( szLine, idIn.m_fACBonus );
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
                idIn.m_fBonusToHit = GetValue( szLine, idIn.m_fBonusToHit );
            }
            else if( strncasecmp( szLine, "to-dambonus", 11 ) == 0 )
            {
                idIn.m_fBonusToDamage = GetValue( szLine, idIn.m_fBonusToDamage );
            }
            else if( strncasecmp( szLine, "level", 5 ) == 0 )
            {
                GetValue( szLine, idIn.m_dwLevel );
            }
            else if( strncasecmp( szLine, "value", 5 ) == 0 )
            {
                GetValue( szLine, idIn.m_fValue );
            }
            else if( strncasecmp( szLine, "weight", 5 ) == 0 )
            {
                GetValue( szLine, idIn.m_fWeight );
            }
            else if( strncasecmp( szLine, "type", 4 ) == 0 )
            {
                // TODO: Add validation that this is ITEM_IDX_ and not...
                szValue = GetValue( szLine, szValue );
                idIn.m_dwIndex = g_Constants.LookupString( szValue );
            }
            else if( strncasecmp( szLine, "flags", 5 ) == 0 )
            {
                szValue = GetValue( szLine, szValue );

                char *c = strtok( szValue, "," );
                while( c != NULL )
                {
                    idIn.m_dwFlags |= g_Constants.LookupString( c );
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
                    printf( "Found multi-hued item: %s\n", color );
                    idIn.m_Colors = ParseColors( color );

                    idIn.m_dwFlags |= ITEM_COLOR_MULTI;
                }
                else
                {
                    // single-hued
                    // <rgb1>
                    idIn.m_Color.SetColor( color );
                }
            }
            else if( strncasecmp( szLine, "effect", 6 ) == 0 )
            {
                CEffect *curEffect;
                curEffect = new CEffect;
                char *begin;
                char *end;
                char *cur;
                // Effect    <ITEM_FLAG_HEAL>, 1d20
                // Effect    <ITEM_FLAG_CURE_BLINDNESS>
                // type
                begin = strchr( szLine, '<' );
                end = strchr( szLine, '>' );
                if( begin == NULL || end == NULL )
                {
                    continue;
                }
                *end++ = NULL;
                begin++;
                curEffect->m_dwEffect = g_Constants.LookupString( begin );
                cur = end;

                // amount
                begin = strchr( cur, ',' );
                if( begin == NULL )
                    continue;
                begin++;
                cur = Strip( begin );
                begin = strchr( cur, '<' );
                end = strchr( cur, '>' );
                if( begin == NULL || end == NULL )
                {
                    continue;
                }
                *end = NULL;
                begin++;
                curEffect->m_szAmount = new char[strlen( begin ) + 1];
                strcpy( curEffect->m_szAmount, begin );

                // store it
                idIn.m_llEffects->Add( curEffect );
            }
            else if( *szLine == '}' )
            {
                bEndItem = true;
            }
            else
            {
                printf( "Unparseable line:%s\n", szLine );
            }
        }
    }
    if( !bEndItem )
    {
        return NULL;
    }

    return &idIn;
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
                printf( "Unparseable line:%s\n", szLine );
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

    copy = new char[strlen( szLine ) + 1];
    strcpy( copy, szLine );
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

    szIn = new char[strlen( begin ) + 1];
    strcpy( szIn, begin );

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
        strcpy( szToken[count++], c );
        c = strtok( NULL, ";" );
    }

    for( int i = 0; i < count; i++ )
    {
        char *tok = szToken[i];
        temp = chomp( tok, temp );
        outcolor = new JColor();
        outcolor->SetColor( temp );
        retval->Add( outcolor );
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
