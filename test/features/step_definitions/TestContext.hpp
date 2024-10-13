#ifndef __TEST_CONTEXT__
#define __TEST_CONTEXT__

#include <JVector.h>
#include <JRect.h>

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

  int lo;
  int hi;
  int lo_f;
  int hi_f;

  // RESULTS
  JVector result_vec;
  int result_int;
  float result_float;

};

#endif // __TEST_CONTEXT__
