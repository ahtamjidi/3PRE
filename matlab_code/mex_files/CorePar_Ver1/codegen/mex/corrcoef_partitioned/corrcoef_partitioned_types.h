/*
 * corrcoef_partitioned_types.h
 *
 * Code generation for function 'corrcoef_partitioned'
 *
 * C source code generated on: Wed May  2 02:54:19 2012
 *
 */

#ifndef __CORRCOEF_PARTITIONED_TYPES_H__
#define __CORRCOEF_PARTITIONED_TYPES_H__

/* Type Definitions */
typedef struct
{
    boolean_T CaseSensitivity;
    boolean_T PartialMatching;
    boolean_T StructExpand;
} b_struct_T;
typedef struct
{
    struct
    {
        real_T x_data[1100220];
    } f0;
    struct
    {
        real_T c_patches_for_correlation_part_[25010001];
        real_T tempCorrelation_data[25000000];
        real_T patches_for_correlation_data[1100220];
    } f1;
    struct
    {
        real_T patches_for_correlation_data[1100000];
    } f2;
} corrcoef_partitionedStackData;
typedef struct
{
    uint32_T alpha;
    uint32_T rows;
} struct_T;

#endif
/* End of code generation (corrcoef_partitioned_types.h) */
