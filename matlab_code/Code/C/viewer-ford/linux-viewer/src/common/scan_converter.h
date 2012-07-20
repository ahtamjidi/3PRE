#ifndef __scan_converter_h__
#define __scan_converter_h__

#include "geometry.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ScanConverter ScanConverter;

ScanConverter *scan_converter_create ();
void scan_converter_destroy (ScanConverter *self);

void scan_converter_init (ScanConverter *self,
                          const pointlist2d_t *poly,
                          const int x_min, const int x_max,
                          const int y_min, const int y_max);
int scan_converter_next (ScanConverter *self,
                         int *x_min, int *x_max, int *y, const int clip);
int scan_converter_done (ScanConverter *self);

#ifdef __cplusplus
}
#endif

#endif
