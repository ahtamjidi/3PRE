#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "velpos_estimate.h"

int velpos_estimate_init(velpos_estimate_t *ev)
{
    memset(ev, 0, sizeof(velpos_estimate_t));
    return 0;
}

int velpos_estimate_update(velpos_estimate_t *ev, double x, double y, double dt, double W)
{
    ev->A[0][0] +=  W*1;
    ev->A[0][1] +=  W*dt;
    ev->A[1][0] +=  W*dt;
    ev->A[1][1] +=  W*dt*dt;

    ev->bx[0] += W*x;
    ev->bx[1] += W*x*dt;

    ev->by[0] += W*y;
    ev->by[1] += W*y*dt;

    ev->hits ++;

    return 0;
}

int velpos_estimate_finish(velpos_estimate_t *ev)
{
    if (ev->hits < 3) {
//        printf("velpos_estimate: only %d hits\n", ev->hits);
//        ev->vx = 0;
//        ev->vy = 0;
        return -1;
    }

    // invert A in place
    double _a = ev->A[0][0], _b = ev->A[0][1], _c = ev->A[1][0], _d = ev->A[1][1];
    double det = _a*_d - _b*_c;
    double a, b, c, d;

    a = _d / det;
    b = -_b / det;
    c = -_c / det;
    d = _a / det;

    // vx = inv(A)*bx
    ev->x0 = a*ev->bx[0] + b*ev->bx[1];
    ev->vx = c*ev->bx[0] + d*ev->bx[1];

    // vy = inv(A)*by
    ev->y0 = a*ev->by[0] + b*ev->by[1];
    ev->vy = c*ev->by[0] + d*ev->by[1];

    return 0;
}
