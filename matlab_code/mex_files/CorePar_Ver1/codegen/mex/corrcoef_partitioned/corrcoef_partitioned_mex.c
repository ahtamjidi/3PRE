/*
 * corrcoef_partitioned_mex.c
 *
 * Code generation for function 'corrcoef_partitioned'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "mex.h"
#include "corrcoef_partitioned_api.h"
#include "corrcoef_partitioned_initialize.h"
#include "corrcoef_partitioned_terminate.h"

/* Type Definitions */

/* Function Declarations */
static void corrcoef_partitioned_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

/* Variable Definitions */
emlrtContext emlrtContextGlobal = { true, false, EMLRT_VERSION_INFO, NULL, "corrcoef_partitioned", NULL, false, NULL };

/* Function Definitions */
static void corrcoef_partitioned_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  /* Temporary copy for mex outputs. */
  mxArray *outputs[1];
  int n = 0;
  int nOutputs = (nlhs < 1 ? 1 : nlhs);
  corrcoef_partitionedStackData* c_corrcoef_partitionedStackData = (corrcoef_partitionedStackData*)mxCalloc(1,sizeof(corrcoef_partitionedStackData));
  /* Check for proper number of arguments. */
  if(nrhs != 1) {
    mexErrMsgIdAndTxt("emlcoder:emlmex:WrongNumberOfInputs","1 input required for entry-point 'corrcoef_partitioned'.");
  } else if(nlhs > 1) {
    mexErrMsgIdAndTxt("emlcoder:emlmex:TooManyOutputArguments","Too many output arguments for entry-point 'corrcoef_partitioned'.");
  }
  /* Module initialization. */
  corrcoef_partitioned_initialize(&emlrtContextGlobal);
  /* Call the function. */
  corrcoef_partitioned_api(c_corrcoef_partitionedStackData, prhs,(const mxArray**)outputs);
  /* Copy over outputs to the caller. */
  for (n = 0; n < nOutputs; ++n) {
    plhs[n] = emlrtReturnArrayR2009a(outputs[n]);
  }
  /* Module finalization. */
  corrcoef_partitioned_terminate();
  mxFree(c_corrcoef_partitionedStackData);
}
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  /* Initialize the memory manager. */
  mexAtExit(corrcoef_partitioned_atexit);
  emlrtClearAllocCount(&emlrtContextGlobal, 0, 0, NULL);
  /* Dispatch the entry-point. */
  corrcoef_partitioned_mexFunction(nlhs, plhs, nrhs, prhs);
}
/* End of code generation (corrcoef_partitioned_mex.c) */
