/*
 * corrcoef.c
 *
 * Code generation for function 'corrcoef'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "corrcoef.h"
#include "sign.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
static void cov_to_corrcoef(real_T r_data[25010001], int32_T r_sizes[2]);

/* Function Definitions */

/*
 * 
 */
static void cov_to_corrcoef(real_T r_data[25010001], int32_T r_sizes[2])
{
    int32_T m;
    int32_T k;
    real_T d_data[5001];
    int32_T i;
    real_T absrij;
    m = r_sizes[0];
    for (k = 0; k + 1 <= m; k++) {
        d_data[k] = muDoubleScalarSqrt(r_data[k + r_sizes[0] * k]);
    }
    for (k = 0; k + 1 <= m; k++) {
        for (i = k + 1; i + 1 <= m; i++) {
            r_data[i + r_sizes[0] * k] = r_data[i + r_sizes[0] * k] / d_data[i] / d_data[k];
        }
        for (i = k + 1; i + 1 <= m; i++) {
            absrij = muDoubleScalarAbs(r_data[i + r_sizes[0] * k]);
            if (absrij > 1.0) {
                r_data[i + r_sizes[0] * k] /= absrij;
            }
            r_data[k + r_sizes[0] * i] = r_data[i + r_sizes[0] * k];
        }
        if (r_data[k + r_sizes[0] * k] > 0.0) {
            b_sign(&r_data[k + r_sizes[0] * k]);
        } else {
            r_data[k + r_sizes[0] * k] = rtNaN;
        }
    }
}

/*
 * 
 */
void corrcoef(corrcoef_partitionedStackData *SD, const real_T x_data[1100220], const int32_T x_sizes[2], real_T r_data[25010001], int32_T r_sizes[2])
{
    int32_T loop_ub;
    int32_T fm;
    int16_T unnamed_idx_0;
    int16_T unnamed_idx_1;
    real_T d;
    int32_T i;
    int32_T k;
    loop_ub = x_sizes[0] * x_sizes[1] - 1;
    for (fm = 0; fm <= loop_ub; fm++) {
        SD->f0.x_data[fm] = x_data[fm];
    }
    unnamed_idx_0 = (int16_T)x_sizes[1];
    unnamed_idx_1 = (int16_T)x_sizes[1];
    r_sizes[0] = unnamed_idx_0;
    r_sizes[1] = unnamed_idx_1;
    loop_ub = unnamed_idx_0 * unnamed_idx_1 - 1;
    for (fm = 0; fm <= loop_ub; fm++) {
        r_data[fm] = 0.0;
    }
    if (x_sizes[0] < 2) {
        r_sizes[0] = unnamed_idx_0;
        r_sizes[1] = unnamed_idx_1;
        loop_ub = unnamed_idx_0 * unnamed_idx_1 - 1;
        for (fm = 0; fm <= loop_ub; fm++) {
            r_data[fm] = rtNaN;
        }
    } else {
        for (loop_ub = 0; loop_ub + 1 <= x_sizes[1]; loop_ub++) {
            d = 0.0;
            for (i = 1; i <= x_sizes[0]; i++) {
                d += SD->f0.x_data[(i + x_sizes[0] * loop_ub) - 1];
            }
            d /= (real_T)x_sizes[0];
            for (i = 0; i + 1 <= x_sizes[0]; i++) {
                SD->f0.x_data[i + x_sizes[0] * loop_ub] -= d;
            }
        }
        fm = x_sizes[0] - 1;
        for (loop_ub = 0; loop_ub + 1 <= x_sizes[1]; loop_ub++) {
            d = 0.0;
            for (k = 0; k + 1 <= x_sizes[0]; k++) {
                d += SD->f0.x_data[k + x_sizes[0] * loop_ub] * SD->f0.x_data[k + x_sizes[0] * loop_ub];
            }
            r_data[loop_ub + r_sizes[0] * loop_ub] = d / (real_T)fm;
            for (i = loop_ub + 1; i + 1 <= x_sizes[1]; i++) {
                d = 0.0;
                for (k = 0; k + 1 <= x_sizes[0]; k++) {
                    d += SD->f0.x_data[k + x_sizes[0] * i] * SD->f0.x_data[k + x_sizes[0] * loop_ub];
                }
                r_data[i + r_sizes[0] * loop_ub] = d / (real_T)fm;
            }
        }
    }
    cov_to_corrcoef(r_data, r_sizes);
}
/* End of code generation (corrcoef.c) */
