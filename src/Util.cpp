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
#include <sys/time.h>

#include "JMDefs.h"
#include "Util.h"

namespace Util
{
// RNG seed management for deterministic generation
static unsigned int g_RandomSeed = 0;

void SeedRandom( unsigned int seed )
{
    g_RandomSeed = seed;
    srand( seed );
    JLog( LOG_LEVEL_DEBUG, true, "RNG seeded with: %u\n", seed );
}

unsigned int GetRandomSeed()
{
    return g_RandomSeed;
}

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
    delete[] szToken;
    return Roll( dice, sides );
}

void Shuffle( int *array, const uint32 size )
{
    for( int i = 0; i < size; i++ )
    {
        array[i] = i;
    }

    for( int i = size - 1; i > 0; i-- )
    {
        int j = rand() % ( i + 1 );
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
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

float sqrt( const float a, const float epsilon )
{
    if( a < 0.0f )
        return -1.0f;

    float guess = 1.0f;
    uint32 count = 0;
    while( abs( ( guess * guess ) - a ) > epsilon )
    {
        guess = ( guess + a / guess ) * 0.5f;
        count++;
    }
    JLog( LOG_LEVEL_DEBUG, true, "sqrt count: %d\n", count );
    return guess;
}
int max( const int a, const int b ) { return ( a >= b ) ? a : b; }
int abs( const int a ) { return ( a >= 0 ) ? a : -a; }

int gcd( const int a, const int b )
{
    if( b == 0 )
        return a;
    return ( Util::gcd( b, a % b ) );
}

bool Bresenham( const JIVector vSource, const JIVector vTarget, const uint8 distance,
                bool ( *isWalkable )( JVector & ), JLinkList<JIVector> *llLine )
{
    if( distance == 8 )
        JLog( LOG_LEVEL_INFO, true, "bres from <%d %d> to  <%d %d> both should be in output\n",
              VEC_EXPAND( vSource ), VEC_EXPAND( vTarget ) );
    // Bresenham Line Algorithm
    JIVector vDelta( abs( vTarget.x - vSource.x ), abs( vTarget.y - vSource.y ) );
    JIVector vStep( vSource.x < vTarget.x ? 1 : -1, vSource.y < vTarget.y ? 1 : -1 );
    int error = 2 * ( vDelta.y - vDelta.x );

    JVector vTest;
    JIVector vCurrent = vSource;
    bool alreadyAdded = false;
    uint8 steps_remaining = distance;
    // while( vCurrent.x != vTarget.x || vCurrent.y != vTarget.y )
    while( steps_remaining > 0 )
    {
        if( distance == 8 && vCurrent.x == vTarget.x && vCurrent.y == vTarget.y )
        {
            JLog( LOG_LEVEL_INFO, true, "Bres reached target point with remaining steps: %d\n",
                  steps_remaining );
        }
        if( !alreadyAdded )
        {
            JLog( LOG_LEVEL_WARN, true, "bres added <%d %d> error: %d\n", VEC_EXPAND( vCurrent ),
                  error );
            if( llLine )
                llLine->Add( new JIVector( vCurrent ) );
            alreadyAdded = true;
            steps_remaining--;
            JLog( LOG_LEVEL_NOISE, true, "bres remaining: %d\n", steps_remaining );
            if( steps_remaining == 0 )
            {
                JLog( LOG_LEVEL_DEBUG, true, "bres finished\n" );
                break;
            }
        }
        if( vCurrent != vSource )
        {
            vTest.Init( VEC_EXPAND( vCurrent ) );
            if( !isWalkable( vTest ) )
            {
                JLog( LOG_LEVEL_DEBUG, true, "bres not walkable\n" );
                return false;
            }
        }

        if( error > 0 )
        {
            vCurrent.y += vStep.y; // Increment y if error is positive
            error -= 2 * vDelta.x;
            alreadyAdded = false;
        }
        else
        {
            vCurrent.x += vStep.x; // Increment x if error is negative
            error += 2 * vDelta.y;
            alreadyAdded = false;
        }
        JLog( LOG_LEVEL_NOISE, true, "bres still going\n" );
    }
    return true;
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
    while( dwBitmask > 1 )
    {
        dwBitmask >>= 1;
        exp++;
    }
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

// High-resolution timing for performance measurement
// Returns current time in milliseconds with microsecond precision
double GetTimeInMillis()
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return ( tv.tv_sec * 1000.0 ) + ( tv.tv_usec / 1000.0 );
}

} // namespace Util
