#ifndef __UTIL_H__
#define __UTIL_H__

#include "JRect.h"
#include "JVector.h"

#include "JLinkList.h"

#define MAX( a, b ) ( a >= b ? a : b )
#define MIN( a, b ) ( a <= b ? a : b )
#define CLAMP( val, lo, hi ) ( val <= lo ? lo : ( val >= hi ? hi : val ) )

namespace Util
{
void SeedRandom( unsigned int seed );
unsigned int GetRandomSeed();
float GetRandom( float lo, float hi );
int GetRandom( int lo, int hi );
JIVector GetRandomPoint( const JRect rcIn );
float Roll( int dice, int sides );
float Roll( const char *szFormat );

void Shuffle( int *array, const uint32 size );

JRect Edges( const JRect rcIn );

bool IsInWorld( JFVector vIn );
bool IsInWorld( JRect rcIn );

bool IsWithinWorld( JFVector vIn );
bool IsWithinWorld( JRect rcIn );

JVector Near( const JVector vOrig, int distance = 1 );
JRect Nearby( const JIVector vTarget, const int radius = 2 );
bool Taxicab( const JIVector vOrigin, const JIVector vTarget, const uint8 distance = 2 );
bool WithinRadius( const JIVector vOrigin, const JIVector vTarget, const uint8 distance = 2 );
JLinkList<JIVector> *GenerateLine( const JIVector vSource, const JIVector vTarget,
                                   const uint8 distance );
bool CheckLineCollision( JLinkList<JIVector> *llLine, const JIVector vSource,
                         bool ( *isWalkable )( JVector & ) );
bool Bresenham( const JIVector vSource, const JIVector vTarget, const uint8 distance,
                bool ( *isWalkable )( JVector & ), JLinkList<JIVector> *llLine = NULL );
int abs( const int a );
int max( const int a, const int b );
int gcd( const int a, const int b );
float sqrt( const float a, const float epsilon = 0.0001f );
#ifndef TURN_BASED
unsigned int GetTickCount();
#endif
uint32 jlog2( uint32 dwBitmask );
void jstrcat( char *dest, const char *src );
void jstrcpy( char *dest, const char *src );
int jstrlen( const char *s1 );
int jstrcmp( const char *s1, const char *s2 );
int jstrncmp( const char *s1, const char *s2, const uint32 count );
double GetTimeInMillis(); // High-resolution timing for performance measurement
}; // namespace Util

#endif // __UTIL_H__
