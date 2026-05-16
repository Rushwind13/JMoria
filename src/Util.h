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
void SeedRandomFromClock();
float GetRandom( float lo, float hi );
int GetRandom( int lo, int hi );
JIVector GetRandomPoint( const JRect rcIn );
float windowed_bell( float delta, float sigma );
float Roll( int dice, int sides );
float Roll( const char *szFormat );
float RollMax( const char *szFormat );

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

// Returns an elemental affinity multiplier for an incoming effect vs a subject's element flags.
// dwEffect   — EFFECT_FLAG_* bits describing the incoming attack/effect.
// dwSubject  — EFFECT_FLAG_* bits describing the subject's elemental nature (attacks / intrinsics).
// Returns: 0.0 = immune, 1.0 = normal, 2.0 = weak.
// Opposite pairs: FIRE<->COLD, ELECTRICITY<->ACID.
float CheckAffinity( uint32 dwEffect, uint32 dwSubject );
}; // namespace Util

#endif // __UTIL_H__
