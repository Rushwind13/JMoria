#ifndef __UTIL_H__
#define __UTIL_H__

#include "JRect.h"
#include "JVector.h"

#define MAX( a, b ) ( a >= b ? a : b )
#define MIN( a, b ) ( a <= b ? a : b )
#define CLAMP( val, lo, hi ) ( val <= lo ? lo : ( val >= hi ? hi : val ) )

namespace Util
{
float GetRandom( float lo, float hi );
int GetRandom( int lo, int hi );
JIVector GetRandomPoint( const JRect rcIn );
float Roll( int dice, int sides );
float Roll( const char *szFormat );

JRect Edges( const JRect rcIn );

bool IsInWorld( JFVector vIn );
bool IsInWorld( JRect rcIn );

bool IsWithinWorld( JFVector vIn );
bool IsWithinWorld( JRect rcIn );

JVector Near( const JVector vOrig, int distance = 1 );
JRect Nearby( const JIVector vTarget, const int radius = 2 );
bool Taxicab( const JIVector vOrigin, const JIVector vTarget, const uint8 distance = 2 );
bool WithinRadius( const JIVector vOrigin, const JIVector vTarget, const uint8 distance = 2 );

unsigned int GetTickCount();
}; // namespace Util

#endif // __UTIL_H__
