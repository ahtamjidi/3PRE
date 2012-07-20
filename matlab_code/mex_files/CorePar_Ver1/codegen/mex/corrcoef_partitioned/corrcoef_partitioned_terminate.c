/*
 * corrcoef_partitioned_terminate.c
 *
 * Code generation for function 'corrcoef_partitioned_terminate'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "corrcoef_partitioned_terminate.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */

void corrcoef_partitioned_atexit(void)
{
    emlrtExitTimeCleanup(&emlrtContextGlobal);
}

void corrcoef_partitioned_terminate(void)
{
    emlrtLeaveRtStack(&emlrtContextGlobal);
}
/* End of code generation (corrcoef_partitioned_terminate.c) */
