/*
 * corrcoef.h
 *
 * Code generation for function 'corrcoef'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

#ifndef __CORRCOEF_H__
#define __CORRCOEF_H__
/* Include files */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "mwmathutil.h"

#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include "blascompat32.h"
#include "rtwtypes.h"
#include "corrcoef_partitioned_types.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
extern void corrcoef(corrcoef_partitionedStackData *SD, const real_T x_data[1100220], const int32_T x_sizes[2], real_T r_data[25010001], int32_T r_sizes[2]);
#endif
/* End of code generation (corrcoef.h) */
