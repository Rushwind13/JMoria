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

  // RESULTS
  JVector result_vec;

};

#endif // __TEST_CONTEXT__
