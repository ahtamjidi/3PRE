/*
 * fast_corner_detect_9_terminate.c
 *
 * Code generation for function 'fast_corner_detect_9_terminate'
 *
 * C source code generated on: Wed May  2 02:55:06 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "fast_corner_detect_9.h"
#include "fast_corner_detect_9_terminate.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */

void fast_corner_detect_9_atexit(void)
{
    emlrtExitTimeCleanup(&emlrtContextGlobal);
}

void fast_corner_detect_9_terminate(void)
{
    emlrtLeaveRtStack(&emlrtContextGlobal);
}
/* End of code generation (fast_corner_detect_9_terminate.c) */
