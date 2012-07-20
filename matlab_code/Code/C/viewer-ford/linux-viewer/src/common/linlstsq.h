#ifndef __linlstsq_h__
#define __linlstsq_h__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * linlstsq_3d_t:
 *
 * data structure for solving linear least squares.
 * Finds the 3d vector x that minimizes ||Ax - b||
 */
typedef struct _linlstsq_3d_t {
    double AtA[9];
    double Atb[9];

    double x[3];
    // TODO chi square error

    uint8_t finished;
} linlstsq_3d_t;

linlstsq_3d_t * linlstsq_3d_new ();

/**
 *
 */
void linlstsq_3d_add_row (linlstsq_3d_t *self, const double row[3],
        double b);
/**
 * returns: 0 if an estimate was made, -1 if not
 */
int linlstsq_3d_finish (linlstsq_3d_t *self);

void linlstsq_3d_free (linlstsq_3d_t *self);

#ifdef __cplusplus
}
#endif

#endif
