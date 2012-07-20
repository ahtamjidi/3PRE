/*
 * fast_corner_detect_9.h
 *
 * Code generation for function 'fast_corner_detect_9'
 *
 * C source code generated on: Wed May  2 02:55:06 2012
 *
 */

#ifndef __FAST_CORNER_DETECT_9_H__
#define __FAST_CORNER_DETECT_9_H__
/* Include files */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include "blascompat32.h"
#include "rtwtypes.h"
#include "fast_corner_detect_9_types.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
extern void fast_corner_detect_9(const real_T im_data[24048], const int32_T im_sizes[2], real_T threshold, real_T coords_data[10000], int32_T coords_sizes[2]);
#endif
/* End of code generation (fast_corner_detect_9.h) */
