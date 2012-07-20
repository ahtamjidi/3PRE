/*
 * sign.c
 *
 * Code generation for function 'sign'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "sign.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */

/*
 * 
 */
void b_sign(real_T *x)
{
    if (*x > 0.0) {
        *x = 1.0;
    } else if (*x < 0.0) {
        *x = -1.0;
    } else {
        if (*x == 0.0) {
            *x = 0.0;
        }
    }
}
/* End of code generation (sign.c) */
