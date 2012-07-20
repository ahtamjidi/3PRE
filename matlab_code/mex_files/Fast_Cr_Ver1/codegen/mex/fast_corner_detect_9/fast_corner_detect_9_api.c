/*
 * fast_corner_detect_9_api.c
 *
 * Code generation for function 'fast_corner_detect_9_api'
 *
 * C source code generated on: Wed May  2 02:55:06 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "fast_corner_detect_9.h"
#include "fast_corner_detect_9_api.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
static void b_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId, real_T y_data[24048], int32_T y_sizes[2]);
static real_T c_emlrt_marshallIn(const mxArray *threshold, const char_T *identifier);
static real_T d_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId);
static void e_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId, real_T ret_data[24048], int32_T ret_sizes[2]);
static void emlrt_marshallIn(const mxArray *im, const char_T *identifier, real_T y_data[24048], int32_T y_sizes[2]);
static const mxArray *emlrt_marshallOut(real_T u_data[10000], int32_T u_sizes[2]);
static real_T f_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId);

/* Function Definitions */

static void b_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId, real_T y_data[24048], int32_T y_sizes[2])
{
    e_emlrt_marshallIn(emlrtAlias(u), parentId, y_data, y_sizes);
    emlrtDestroyArray(&u);
}

static real_T c_emlrt_marshallIn(const mxArray *threshold, const char_T *identifier)
{
    real_T y;
    emlrtMsgIdentifier thisId;
    thisId.fIdentifier = identifier;
    thisId.fParent = NULL;
    y = d_emlrt_marshallIn(emlrtAlias(threshold), &thisId);
    emlrtDestroyArray(&threshold);
    return y;
}

static real_T d_emlrt_marshallIn(const mxArray *u, const emlrtMsgIdentifier *parentId)
{
    real_T y;
    y = f_emlrt_marshallIn(emlrtAlias(u), parentId);
    emlrtDestroyArray(&u);
    return y;
}

static void e_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId, real_T ret_data[24048], int32_T ret_sizes[2])
{
    int32_T i1;
    int32_T iv0[2];
    boolean_T bv0[2];
    for (i1 = 0; i1 < 2; i1++) {
        iv0[i1] = 144 + 23 * i1;
        bv0[i1] = TRUE;
    }
    emlrtCheckVsBuiltInR2011a(msgId, src, "double", FALSE, 2U, iv0, bv0, ret_sizes);
    emlrtImportArrayR2008b(src, ret_data, 8);
    emlrtDestroyArray(&src);
}

static void emlrt_marshallIn(const mxArray *im, const char_T *identifier, real_T y_data[24048], int32_T y_sizes[2])
{
    emlrtMsgIdentifier thisId;
    thisId.fIdentifier = identifier;
    thisId.fParent = NULL;
    b_emlrt_marshallIn(emlrtAlias(im), &thisId, y_data, y_sizes);
    emlrtDestroyArray(&im);
}

static const mxArray *emlrt_marshallOut(real_T u_data[10000], int32_T u_sizes[2])
{
    const mxArray *y;
    const mxArray *m0;
    real_T (*pData)[];
    int32_T i0;
    int32_T i;
    int32_T b_i;
    y = NULL;
    m0 = mxCreateNumericArray(2, (int32_T *)u_sizes, mxDOUBLE_CLASS, mxREAL);
    pData = (real_T (*)[])mxGetPr(m0);
    i0 = 0;
    for (i = 0; i < 2; i++) {
        for (b_i = 0; b_i < u_sizes[0]; b_i++) {
            (*pData)[i0] = u_data[b_i + u_sizes[0] * i];
            i0++;
        }
    }
    emlrtAssign(&y, m0);
    return y;
}

static real_T f_emlrt_marshallIn(const mxArray *src, const emlrtMsgIdentifier *msgId)
{
    real_T ret;
    emlrtCheckBuiltInR2011a(msgId, src, "double", FALSE, 0U, 0);
    ret = *(real_T *)mxGetData(src);
    emlrtDestroyArray(&src);
    return ret;
}

void fast_corner_detect_9_api(fast_corner_detect_9StackData *SD, const mxArray * const prhs[2], const mxArray *plhs[1])
{
    int32_T im_sizes[2];
    real_T threshold;
    int32_T coords_sizes[2];
    real_T coords_data[10000];
    /* Marshall function inputs */
    emlrt_marshallIn(emlrtAliasP(prhs[0]), "im", SD->f0.im_data, im_sizes);
    threshold = c_emlrt_marshallIn(emlrtAliasP(prhs[1]), "threshold");
    /* Invoke the target function */
    fast_corner_detect_9(SD->f0.im_data, im_sizes, threshold, coords_data, coords_sizes);
    /* Marshall function outputs */
    plhs[0] = emlrt_marshallOut(coords_data, coords_sizes);
}
/* End of code generation (fast_corner_detect_9_api.c) */
