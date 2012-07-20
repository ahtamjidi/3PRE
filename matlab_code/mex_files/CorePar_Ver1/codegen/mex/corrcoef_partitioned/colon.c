/*
 * colon.c
 *
 * Code generation for function 'colon'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "colon.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */

/*
 * 
 */
void float_colon_length(real_T a, real_T b, int32_T *n, real_T *anew, real_T *bnew, boolean_T *n_too_large)
{
    boolean_T b0;
    real_T ndbl;
    real_T cdiff;
    real_T absa;
    real_T absb;
    if (muDoubleScalarIsNaN(a) || muDoubleScalarIsNaN(b)) {
        *n = 1;
        *anew = rtNaN;
        *bnew = b;
        *n_too_large = FALSE;
    } else if (b < a) {
        *n = 0;
        *anew = a;
        *bnew = b;
        *n_too_large = FALSE;
    } else if (muDoubleScalarIsInf(a) || muDoubleScalarIsInf(b)) {
        *n = 1;
        *anew = rtNaN;
        *bnew = b;
        if (a == b) {
            b0 = TRUE;
        } else {
            b0 = FALSE;
        }
        *n_too_large = !b0;
    } else {
        *anew = a;
        ndbl = muDoubleScalarFloor((b - a) + 0.5);
        *bnew = a + ndbl;
        cdiff = *bnew - b;
        absa = muDoubleScalarAbs(a);
        absb = muDoubleScalarAbs(b);
        if (absa > absb) {
            absb = absa;
        }
        if (muDoubleScalarAbs(cdiff) < 4.4408920985006262E-16 * absb) {
            ndbl++;
            *bnew = b;
        } else if (cdiff > 0.0) {
            *bnew = a + (ndbl - 1.0);
        } else {
            ndbl++;
        }
        *n_too_large = FALSE;
        if (ndbl >= 0.0) {
            *n = (int32_T)ndbl;
        } else {
            *n = 0;
        }
    }
}
/* End of code generation (colon.c) */
