/*
 * corrcoef_partitioned.c
 *
 * Code generation for function 'corrcoef_partitioned'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "corrcoef_partitioned.h"
#include "corrcoef.h"
#include "colon.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */
static emlrtDCInfo emlrtDCI = { 14, 71, "corrcoef_partitioned", "/home/amirhossein/Desktop/Current_Work/april_2012/SR4000/EKF_monoSLAM_1pRANSAC/matlab_code/mex_files/CorePar_Ver1/corrcoef_partitioned.m", 1 };

/* Function Declarations */

/* Function Definitions */

/*
 * function  correlation_matrix = corrcoef_partitioned( patches_for_correlation )
 */
void corrcoef_partitioned(corrcoef_partitionedStackData *SD, const real_T patches_for_correlation_data[1100000], const int32_T patches_for_correlation_sizes[2], real_T correlation_matrix_data[5000], int32_T correlation_matrix_sizes[1])
{
    int32_T size_partition;
    int32_T c_patches_for_correlation_part_[2];
    int32_T loop_ub;
    int32_T i0;
    int32_T nm1;
    int32_T i;
    boolean_T n_too_large;
    real_T bnew;
    real_T anew;
    int32_T n;
    real_T y_data[5000];
    int32_T nm1d2;
    int32_T k;
    real_T tmp_data[5001];
    int32_T b_patches_for_correlation_sizes[2];
    /* %%% TAMADD */
    /* 'corrcoef_partitioned:4' coder.varsize('correlation_matrix', [5000 1]); */
    /* 'corrcoef_partitioned:5' correlation_matrix =[]; */
    correlation_matrix_sizes[0] = 0;
    /* 'corrcoef_partitioned:6' size_partition = floor(size(patches_for_correlation,2)/100); */
    size_partition = (int32_T)muDoubleScalarFloor((real_T)patches_for_correlation_sizes[1] / 100.0);
    /* 'corrcoef_partitioned:7' if size_partition==0 */
    if (size_partition == 0) {
        /* 'corrcoef_partitioned:8' tempCorrelation = corrcoef(patches_for_correlation); */
        corrcoef(SD, patches_for_correlation_data, patches_for_correlation_sizes, SD->f1.c_patches_for_correlation_part_, c_patches_for_correlation_part_);
        loop_ub = c_patches_for_correlation_part_[0] * c_patches_for_correlation_part_[1] - 1;
        for (i0 = 0; i0 <= loop_ub; i0++) {
            SD->f1.tempCorrelation_data[i0] = SD->f1.c_patches_for_correlation_part_[i0];
        }
        /* 'corrcoef_partitioned:9' correlation_matrix = tempCorrelation(2:end,1); */
        if (2 > c_patches_for_correlation_part_[0]) {
            i0 = 0;
            nm1 = 0;
        } else {
            i0 = 1;
            nm1 = c_patches_for_correlation_part_[0];
        }
        correlation_matrix_sizes[0] = nm1 - i0;
        loop_ub = (nm1 - i0) - 1;
        for (nm1 = 0; nm1 <= loop_ub; nm1++) {
            correlation_matrix_data[nm1] = SD->f1.tempCorrelation_data[i0 + nm1];
        }
    } else {
        /* 'corrcoef_partitioned:13' for i =1:100 */
        for (i = 0; i < 100; i++) {
            /* 'corrcoef_partitioned:14' patches_for_correlation_part = corrcoef(patches_for_correlation(:,[1,(i-1)*size_partition+1:i*size_partition])); */
            float_colon_length((real_T)(i * size_partition) + 1.0, (real_T)((i + 1) * size_partition), &n, &anew, &bnew, &n_too_large);
            if (n > 0) {
                y_data[0] = anew;
                if (n > 1) {
                    y_data[n - 1] = bnew;
                    nm1 = n - 1;
                    i0 = nm1;
                    nm1d2 = (int32_T)((uint32_T)i0 >> 1);
                    loop_ub = nm1d2 - 1;
                    for (k = 1; k <= loop_ub; k++) {
                        y_data[k] = anew + (real_T)k;
                        y_data[(n - k) - 1] = bnew - (real_T)k;
                    }
                    if (nm1d2 << 1 == nm1) {
                        y_data[nm1d2] = (anew + bnew) / 2.0;
                    } else {
                        y_data[nm1d2] = anew + (real_T)nm1d2;
                        y_data[nm1d2 + 1] = bnew - (real_T)nm1d2;
                    }
                }
            }
            nm1d2 = 1 + n;
            tmp_data[0] = 1.0;
            loop_ub = n - 1;
            for (i0 = 0; i0 <= loop_ub; i0++) {
                tmp_data[i0 + 1] = y_data[i0];
            }
            b_patches_for_correlation_sizes[0] = patches_for_correlation_sizes[0];
            b_patches_for_correlation_sizes[1] = nm1d2;
            loop_ub = nm1d2 - 1;
            for (i0 = 0; i0 <= loop_ub; i0++) {
                nm1d2 = patches_for_correlation_sizes[0] - 1;
                for (nm1 = 0; nm1 <= nm1d2; nm1++) {
                    SD->f1.patches_for_correlation_data[nm1 + b_patches_for_correlation_sizes[0] * i0] = patches_for_correlation_data[nm1 + patches_for_correlation_sizes[0] * ((int32_T)emlrtIntegerCheckR2011a(tmp_data[i0], &emlrtDCI, &emlrtContextGlobal) - 1)];
                }
            }
            corrcoef(SD, SD->f1.patches_for_correlation_data, b_patches_for_correlation_sizes, SD->f1.c_patches_for_correlation_part_, c_patches_for_correlation_part_);
            /* 'corrcoef_partitioned:15' correlation_matrix = [correlation_matrix;patches_for_correlation_part(2:end,1)]; */
            if (2 > c_patches_for_correlation_part_[0]) {
                i0 = 0;
                nm1 = 0;
            } else {
                i0 = 1;
                nm1 = c_patches_for_correlation_part_[0];
            }
            nm1d2 = (correlation_matrix_sizes[0] + nm1) - i0;
            loop_ub = correlation_matrix_sizes[0] - 1;
            for (k = 0; k <= loop_ub; k++) {
                y_data[k] = correlation_matrix_data[k];
            }
            loop_ub = (nm1 - i0) - 1;
            for (nm1 = 0; nm1 <= loop_ub; nm1++) {
                y_data[nm1 + correlation_matrix_sizes[0]] = SD->f1.c_patches_for_correlation_part_[i0 + nm1];
            }
            correlation_matrix_sizes[0] = nm1d2;
            loop_ub = nm1d2 - 1;
            for (i0 = 0; i0 <= loop_ub; i0++) {
                correlation_matrix_data[i0] = y_data[i0];
            }
        }
        /*  correlation_matrix = [1;correlation_matrix]; */
    }
    /* %%% */
}
/* End of code generation (corrcoef_partitioned.c) */
