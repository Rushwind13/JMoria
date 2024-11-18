//
// Util.cpp - common utilities library
//
// Created 5/24/2 Jimbo S. Harris
//
#ifdef __WIN32__
#include <windows.h>
#endif // __WIN32__
#include <stdio.h>
#include <string.h>
#ifndef TURN_BASED
#include <sys/time.h>
#endif

#include "JMDefs.h"
#include "Util.h"

namespace Util
{
// The RNG in all its glory
float GetRandom( float lo, float hi )
{
    float mult = hi - lo;
    float num;
    num = ( (float)rand() / RAND_MAX );
    num = lo + mult * num;

    return ( num );
}

// In case we ever need a random int
int GetRandom( int lo, int hi )
{
    // deal with coinflip "getrandom(0,1)"
    if( ( hi - lo ) == 1 )
    {
        return ( ( rand() & 1 ) ? hi : lo );
    }

    // int num;
    // num = rand();
    // num %= ( hi - lo + 1 );
    // num += lo;

    // return ( num );
    return (int)GetRandom( (float)lo, (float)hi );
}

JIVector GetRandomPoint( const JRect rcIn )
{
    JIVector vPoint( GetRandom( rcIn.left, rcIn.right ), GetRandom( rcIn.top, rcIn.bottom ) );
    return vPoint;
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
    szToken = new char[Util::jstrlen( szFormat ) + 1];

    Util::jstrcpy( szToken, szFormat );

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

bool IsInWorld( JIVector vIn ) { return vIn.IsInWorld(); }
bool IsWithinWorld( JIVector vIn ) { return vIn.IsWithinWorld(); }

bool IsInWorld( JVector vIn )
{
    JIVector vReturn( VEC_EXPAND( vIn ) );
    return vReturn.IsInWorld();
}
bool IsWithinWorld( JVector vIn )
{
    JIVector vReturn( VEC_EXPAND( vIn ) );
    return vReturn.IsWithinWorld();
}

bool IsInWorld( JRect rcIn ) { return rcIn.IsInWorld(); }
bool IsWithinWorld( JRect rcIn ) { return rcIn.IsWithinWorld(); }

JRect Edges( JRect rcIn ) { return rcIn.Edges(); }
JVector Near( const JVector vOrig, int distance )
{
    // want an integer position within a certain distance
    // but never the same as the original position
    int direction = GetRandom( 0, 7 );
    int x = 0;
    int y = 0;

    switch( direction )
    {
    case DIR_NORTH:
        x = 0;
        y = 1;
        break;
    case DIR_NE:
        x = 1;
        y = 1;
        break;
    case DIR_EAST:
        x = 1;
        y = 0;
        break;
    case DIR_SE:
        x = 1;
        y = -1;
        break;
    case DIR_SOUTH:
        x = 0;
        y = -1;
        break;
    case DIR_SW:
        x = -1;
        y = -1;
        break;
    case DIR_WEST:
        x = -1;
        y = 0;
        break;
    case DIR_NW:
        x = -1;
        y = 1;
        break;
    }
    JVector vDelta( x * GetRandom( 1, distance ), y * GetRandom( 1, distance ) );
    return JVector( vOrig + vDelta );
}

// Returns square area
JRect Nearby( const JIVector vTarget, const int radius )
{
    JIVector delta( radius, radius );
    return JRect( VEC_EXPAND( vTarget - delta ), VEC_EXPAND( vTarget + delta ) );
}

// returns diamond-shaped area
bool Taxicab( const JIVector vOrigin, const JIVector vTarget, const uint8 distance )
{
    JIVector vDelta = vOrigin - vTarget;
    return abs( vDelta.x ) + abs( vDelta.y ) <= distance;
}

// returns circular area
bool WithinRadius( const JIVector vOrigin, const JIVector vTarget, const uint8 distance )
{
    // Euclidean offset between the points
    JIVector vDelta = vOrigin - vTarget;
    // to avoid a sqrt(), square everything
    vDelta.x *= vDelta.x;
    vDelta.y *= vDelta.y;
    uint8 dsquared = distance * distance;
    return vDelta.x + vDelta.y <= dsquared;
}

#ifndef TURN_BASED
unsigned int GetTickCount()
{
    struct timeval tv;
    if( gettimeofday( &tv, NULL ) != 0 )
        return 0;

    return (unsigned int)( ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 ) );
}
#endif

uint32 jlog2( uint32 dwBitmask )
{
    int exp = 0;
    while( dwBitmask >> 1 )
        exp++;
    return exp;
}

void jstrcat( char *dest, const char *src )
{
    int i, j;

    char *d = dest;
    const char *s = src;

    // Find the end of the destination string
    while( *d != NULL )
        d++;

    // Copy the source string to the end of the destination string
    while( *s != NULL )
    {
        *d = *s;
        d++;
        s++;
    }

    // Null-terminate the concatenated string
    *d = NULL;
}

void jstrcpy( char *dest, const char *src )
{
    const char *p = src;
    while( *p != NULL )
    {
        *dest = *p;
        dest++;
        p++;
    }
    *dest = NULL; // Null-terminate the destination string
}

int jstrlen( const char *s1 )
{
    const char *p1 = s1;
    int count = 0;
    while( *p1 != NULL )
    {
        count++;
        p1++;
    }
    return count;
}

int jstrcmp( const char *s1, const char *s2 )
{
    const char *p1 = s1;
    const char *p2 = s2;
    while( *p1 != NULL && *p2 != NULL )
    {
        if( *p1 != *p2 )
        {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return *p1 - *p2;
}

int jstrncmp( const char *s1, const char *s2, const uint32 count )
{
    int i = 0;
    const char *p1 = s1;
    const char *p2 = s2;
    while( i < count )
    {
        if( *p1 == NULL || *p2 == NULL || *p1 != *p2 )
        {
            return *p1 - *p2;
        }

        i++;
        p1++;
        p2++;
    }
    return 0;
}

} // namespace Util
