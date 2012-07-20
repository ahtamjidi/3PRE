/*
 * corrcoef_partitioned.h
 *
 * Code generation for function 'corrcoef_partitioned'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

#ifndef __CORRCOEF_PARTITIONED_H__
#define __CORRCOEF_PARTITIONED_H__
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
extern void corrcoef_partitioned(corrcoef_partitionedStackData *SD, const real_T patches_for_correlation_data[1100000], const int32_T patches_for_correlation_sizes[2], real_T correlation_matrix_data[5000], int32_T correlation_matrix_sizes[1]);
#endif
/* End of code generation (corrcoef_partitioned.h) */
