#include "FileParse.h"
#include "Monster.h"
#include <string.h>

bool CDataFile::Open( const char *szFilename )
{
	m_fp = fopen(szFilename, "r");

	if( m_fp == NULL )
	{
		return false;
	}

	return true;
}

char *CDataFile::Strip( char *szLine )
{
	char *c;

	if( szLine == NULL || *szLine == nul || *szLine == '#' )
	{
		return NULL;
	}

	if( szLine[strlen(szLine)-1] == '\n' )
	{
		szLine[strlen(szLine)-1] = NULL;
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


CMonsterDef *CDataFile::ReadMonster(CMonsterDef &mdIn)
{
	char szRaw[1024];
	char *szLine;
	char *szValue = NULL;
	bool bFoundMonster = false;
	bool bStartMonster = false;
	bool bEndMonster = false;
	
	while( !bEndMonster && fgets( szRaw, 1024, m_fp ) != NULL )
	{
		szLine = Strip(szRaw);
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
			else if( strncasecmp( szLine, "speed", 5 ) == 0 )
			{
				mdIn.m_fSpeed = GetValue( szLine, mdIn.m_fSpeed );
			}
			else  if( strncasecmp( szLine, "movetype", 8 ) == 0 )
			{
				szValue = GetValue( szLine, szValue );
                mdIn.m_dwMoveType = g_Constants.LookupString(szValue);
			}
			else if( strncasecmp( szLine, "hd", 2 ) == 0 )
			{
				mdIn.m_szHD = GetValue( szLine, mdIn.m_szHD );
			}
			else  if( strncasecmp( szLine, "ac", 2 ) == 0 )
			{
				mdIn.m_fBaseAC = GetValue( szLine, mdIn.m_fBaseAC );
			}
			else if( strncasecmp( szLine, "level", 5 ) == 0 )
			{
				GetValue( szLine, mdIn.m_dwLevel );
			}
			else if( strncasecmp( szLine, "expvalue", 8 ) == 0 )
			{
				GetValue( szLine, mdIn.m_dwExpValue );
			}
			else if( strncasecmp( szLine, "type", 4 ) == 0 )
			{
				szValue = GetValue( szLine, szValue );
				mdIn.m_dwIndex = g_Constants.LookupString(szValue);
			}
			else if( strncasecmp( szLine, "attack", 6 ) == 0 )
			{
				CAttack *curAttack;
				curAttack = new CAttack;
				char *begin;
				char *end;
				char *cur;
				bool bDone = false;
				// Attack	<MON_AI_TOUCH>, 1d2, <FLAG_POISON>
				// type
				begin = strchr( szLine, '<' );
				end = strchr( szLine, '>' );
				if( begin == NULL || end == NULL )
				{
					continue;
				}
				*end++ = NULL;
				begin++;
				curAttack->m_dwType = g_Constants.LookupString(begin);
				cur = end;

				// damage
				begin = strchr(cur, ',');
				if( begin == NULL ) continue;
				begin++;
				// from here on out, you've got enough info to do this.
				end = strchr(begin, ',');
				if( end != NULL )
				{
					*end++ = NULL;
				}
				else
				{
					bDone = true;
				}
				
				cur = Strip(begin);
				curAttack->m_szDamage = new char[strlen(cur)+1];
				strcpy( curAttack->m_szDamage, cur );
				
				if( !bDone )
				{
					cur = end;
					// effect
					begin = strchr( cur, '<' );
					end = strchr( cur, '>' );
					if( begin == NULL || end == NULL )
					{
						continue;
					}
					*end = NULL;
					begin++;
					curAttack->m_dwEffect = g_Constants.LookupString(begin);
				}
				

				// store it
				mdIn.m_llAttacks->Add(curAttack);
			}
			else if( strncasecmp( szLine, "color", 5 ) == 0 )
			{
				char *color = chomp( szLine, szValue );
                char *token = strchr( color, '<' );
                while( token != NULL )
                {
                    // multi-hued
                    // <<rgb1>,<rgb2>,...,<rgbn>>
                    char *colorList[] = split(color);
                    mdIn.m_Colors.append
                    
                    mdIn.m_dwFlags |= MON_COLOR_MULTI;
                }
                // <rgb1>
                mdIn.m_Color.SetColor(szValue);
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

// Removes outermost <> from data entry
char *CDataFile::GetValue(char *szLine, char *szIn) { return chomp(szLine, szIn); }
char *CDataFile::chomp(char *szLine, char *szIn)
{
	if( szLine == NULL || *szLine == nul )
	{
		return NULL;
	}

	char *copy;

	copy = new char[strlen(szLine)+1];
	strcpy( copy, szLine);
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

	szIn = new char[strlen(begin) +1];
	strcpy( szIn, begin );

	delete [] copy;
	return szIn;
}

char *[] split(char *szLine)
{
    char *c;
    while( (c = strtok( szColor, "," )) != NULL )
    {
        
    }
    m_vRG.x = atoi(c);
    c = strtok( NULL, "," );
    m_vRG.y = atoi(c);
    c = strtok( NULL, "," );
    m_vBA.x = atoi(c);
    c = strtok( NULL, "," );
    m_vBA.y = atoi(c);
};

int CDataFile::GetValue(char *szLine, int &dwIn)
{
	char label[100];
	char value[100];
	sscanf( szLine, "%s %s\n", label, value );
	dwIn = atoi(value);

	return dwIn;
}

float CDataFile::GetValue(char *szLine, float &fIn)
{
	char label[100];
	char value[100];
	sscanf( szLine, "%s %s\n", label, value );
	fIn = (float)atof(value);

	return fIn;
}