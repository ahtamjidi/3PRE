/*
 * fast_corner_detect_9_mex.c
 *
 * Code generation for function 'fast_corner_detect_9'
 *
 * C source code generated on: Wed May  2 02:55:06 2012
 *
 */

/* Include files */
#include "mex.h"
#include "fast_corner_detect_9_api.h"
#include "fast_corner_detect_9_initialize.h"
#include "fast_corner_detect_9_terminate.h"

/* Type Definitions */

/* Function Declarations */
static void fast_corner_detect_9_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

/* Variable Definitions */
emlrtContext emlrtContextGlobal = { true, false, EMLRT_VERSION_INFO, NULL, "fast_corner_detect_9", NULL, false, NULL };

/* Function Definitions */
static void fast_corner_detect_9_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  /* Temporary copy for mex outputs. */
  mxArray *outputs[1];
  int n = 0;
  int nOutputs = (nlhs < 1 ? 1 : nlhs);
  fast_corner_detect_9StackData* c_fast_corner_detect_9StackData = (fast_corner_detect_9StackData*)mxCalloc(1,sizeof(fast_corner_detect_9StackData));
  /* Check for proper number of arguments. */
  if(nrhs != 2) {
    mexErrMsgIdAndTxt("emlcoder:emlmex:WrongNumberOfInputs","2 inputs required for entry-point 'fast_corner_detect_9'.");
  } else if(nlhs > 1) {
    mexErrMsgIdAndTxt("emlcoder:emlmex:TooManyOutputArguments","Too many output arguments for entry-point 'fast_corner_detect_9'.");
  }
  /* Module initialization. */
  fast_corner_detect_9_initialize(&emlrtContextGlobal);
  /* Call the function. */
  fast_corner_detect_9_api(c_fast_corner_detect_9StackData, prhs,(const mxArray**)outputs);
  /* Copy over outputs to the caller. */
  for (n = 0; n < nOutputs; ++n) {
    plhs[n] = emlrtReturnArrayR2009a(outputs[n]);
  }
  /* Module finalization. */
  fast_corner_detect_9_terminate();
  mxFree(c_fast_corner_detect_9StackData);
}
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  /* Initialize the memory manager. */
  mexAtExit(fast_corner_detect_9_atexit);
  emlrtClearAllocCount(&emlrtContextGlobal, 0, 0, NULL);
  /* Dispatch the entry-point. */
  fast_corner_detect_9_mexFunction(nlhs, plhs, nrhs, prhs);
}
/* End of code generation (fast_corner_detect_9_mex.c) */
