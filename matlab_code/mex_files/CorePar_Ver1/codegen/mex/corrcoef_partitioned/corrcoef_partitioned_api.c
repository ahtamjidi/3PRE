/*
 * corrcoef_partitioned_api.c
 *
 * Code generation for function 'corrcoef_partitioned_api'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "corrcoef_partitioned_api.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
static void b_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId, real_T y_data[1100000], int32_T y_sizes[2]);
static void c_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId, real_T ret_data[1100000], int32_T ret_sizes[2]);
static void emlrt_marshallIn(const mxArray *patches_for_correlation, const char_T *identifier, real_T y_data[1100000], int32_T y_sizes[2]);
static const mxArray *emlrt_marshallOut(real_T u_data[5000], int32_T u_sizes[1]);

/* Function Definitions */

static void b_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId, real_T y_data[1100000], int32_T y_sizes[2])
{
    c_emlrt_marshallIn(emlrtAlias(u), parentId, y_data, y_sizes);
    emlrtDestroyArray(&u);
}

static void c_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId, real_T ret_data[1100000], int32_T ret_sizes[2])
{
    int32_T i2;
    int32_T iv0[2];
    boolean_T bv0[2];
    for (i2 = 0; i2 < 2; i2++) {
        iv0[i2] = 220 + 4780 * i2;
        bv0[i2] = TRUE;
    }
    emlrtCheckVsBuiltInR2011a(msgId, src, "double", FALSE, 2U, iv0, bv0, ret_sizes);
    emlrtImportArrayR2008b(src, ret_data, 8);
    emlrtDestroyArray(&src);
}

static void emlrt_marshallIn(const mxArray *patches_for_correlation, const char_T *identifier, real_T y_data[1100000], int32_T y_sizes[2])
{
    emlrtMsgIdentifier thisId;
    thisId.fIdentifier = identifier;
    thisId.fParent = NULL;
    b_emlrt_marshallIn(emlrtAlias(patches_for_correlation), &thisId, y_data, y_sizes);
    emlrtDestroyArray(&patches_for_correlation);
}

static const mxArray *emlrt_marshallOut(real_T u_data[5000], int32_T u_sizes[1])
{
    const mxArray *y;
    const mxArray *m0;
    real_T (*pData)[];
    int32_T i1;
    int32_T i;
    y = NULL;
    m0 = mxCreateNumericArray(1, (int32_T *)u_sizes, mxDOUBLE_CLASS, mxREAL);
    pData = (real_T (*)[])mxGetPr(m0);
    i1 = 0;
    for (i = 0; i < u_sizes[0]; i++) {
        (*pData)[i1] = u_data[i];
        i1++;
    }
    emlrtAssign(&y, m0);
    return y;
}

void corrcoef_partitioned_api(corrcoef_partitionedStackData *SD, const mxArray * const prhs[1], const mxArray *plhs[1])
{
    int32_T patches_for_correlation_sizes[2];
    int32_T correlation_matrix_sizes;
    real_T correlation_matrix_data[5000];
    /* Marshall function inputs */
    emlrt_marshallIn(emlrtAliasP(prhs[0]), "patches_for_correlation", SD->f2.patches_for_correlation_data, patches_for_correlation_sizes);
    /* Invoke the target function */
    corrcoef_partitioned(SD, SD->f2.patches_for_correlation_data, patches_for_correlation_sizes, correlation_matrix_data, &correlation_matrix_sizes);
    /* Marshall function outputs */
    plhs[0] = emlrt_marshallOut(correlation_matrix_data, *(int32_T (*)[1])&correlation_matrix_sizes);
}
/* End of code generation (corrcoef_partitioned_api.c) */
