//
// Util.cpp - common utilities library
//
// Created 5/24/2 Jimbo S. Harris
//
#ifdef __WIN32__
#include <windows.h>
#endif //__WIN32__
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "JMDefs.h"
#include "Util.h"

namespace Util
{
// The RNG in all its glory
float GetRandom( float lo, float hi )
{
	float mult = hi - lo;
	float num;
	num = ((float)rand() / RAND_MAX);
	num = lo + mult * num;

	return( num );
}

// In case we ever need a random int
int GetRandom( int lo, int hi )
{
	// deal with coinflip "getrandom(0,1)"
	if( (hi - lo) == 1 )
	{
		return( (rand() & 1) ? hi : lo );
	}

	int num;
	num = rand();
	num %= ( hi - lo + 1 );
	num += lo;

	return( num );
}

// Roll some dice
float Roll( int dice, int sides )
{
	if( sides <= 0 )
	{
		return 0.0f;
	}
	
	float total = 0.0f;
	int die;
	
	for( die = 0; die < dice; die++ )
	{
		total += GetRandom( 1.0f, (float)sides );
	}
	
	return total;
}

// function overload; pass in "2d5"
float Roll( const char *szFormat )
{
	if( !szFormat || *szFormat == nul )
	{
		// You passed in an empty or NULL string!
		return 0.0f;
	}
	
	char *szToken;
	int dice;
	int sides;
	char *c;
	szToken = new char[strlen(szFormat)+1];
	
	strcpy( szToken, szFormat );
	
	c = strtok( szToken, "d" );
	if( c == NULL )
	{
		// You passed in a malformed string ( no 'd' )
		return 0.0f;
	}
	
	dice = atoi( c );
	c = strtok( NULL, "d" );
	if( c == NULL )
	{
		// You passed in a malformed string (nothing after the 'd')
		return 0.0f;
	}
	
	sides = atoi( c );
	return Roll( dice, sides ); 
}

bool IsInWorld( JFVector vIn )
{
	return vIn.IsInWorld();
}

bool IsInWorld( JRect rcIn )
{
	return rcIn.IsInWorld();
}
    
int GetTickCount()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
        return 0;
    
    return (int)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

}