#ifndef __UTIL_H__
#define __UTIL_H__

#include "JVector.h"
#include "JRect.h"

#define MAX(a,b) (a>=b ? a : b)
#define MIN(a,b) (a<=b ? a : b)
#define CLAMP( val, lo, hi ) ( val <= lo ? lo : (val >= hi ? hi : val) )

namespace Util
{
	float GetRandom( float lo, float hi );
	int GetRandom( int lo, int hi );
	float Roll( int dice, int sides );
	float Roll( const char *szFormat );

	bool IsInWorld( JFVector vIn );
	bool IsInWorld( JRect rcIn );
    
    unsigned int GetTickCount();
};

#endif // __UTIL_H__
