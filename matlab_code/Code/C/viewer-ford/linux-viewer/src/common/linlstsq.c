#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>

#include <glib.h>

#include "small_linalg.h"
#include "linlstsq.h"

linlstsq_3d_t * 
linlstsq_3d_new ()
{
    linlstsq_3d_t *self = g_slice_new0 (linlstsq_3d_t);
    self->finished = 0;
    self->x[0] = self->x[1] = self->x[2] = NAN;
    return self;
}

void
linlstsq_3d_add_row (linlstsq_3d_t *self, const double row[3],
        double b)
{
    assert (!self->finished);

    double row_outer_product[9];
    vector_vector_outer_product_3d (row, row, row_outer_product);

    for (int i=0; i<9; i++) {
        self->AtA[i] += row_outer_product[i];
    }
    self->Atb[0] += row[0] * b;
    self->Atb[1] += row[1] * b;
    self->Atb[2] += row[2] * b;
}

int 
linlstsq_3d_finish (linlstsq_3d_t *self)
{
    assert (!self->finished);

    double AtA_inv[9];
    if (0 != matrix_inverse_3x3d (self->AtA, AtA_inv)) return -1;

    matrix_vector_multiply_3x3_3d (AtA_inv, self->Atb, self->x);
    self->finished = 1;
    return 0;
}

void 
linlstsq_3d_free (linlstsq_3d_t *self)
{
    g_slice_free (linlstsq_3d_t, self);
}
