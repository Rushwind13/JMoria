#ifndef __TEST_CONTEXT__
#define __TEST_CONTEXT__

// #include <JMDefs.h>
#include <Constants.h>
#include <JRect.h>
#include <JVector.h>
#include <TextEntry.h>
#include <TileSet.h>

CGame *g_pGame = NULL;


/*#######
##
## CONTEXT
##
#######*/
struct TestCtx
{
  JVector vec;
  JVector vec_b;

  JRect area;

  // Tileset
  CTileset *tileset;
  char szTileset[1024];
  JIVector aspect;
  JIVector vTile;

  // TextEntry
  TextEntry pair;

  // TextEntry
  Constants constants;

  int index;
  int lo;
  int hi;
  int lo_f;
  int hi_f;

  // RESULTS
  int result_int;
  float result_float;
  JResult result;

};

#endif // __TEST_CONTEXT__
