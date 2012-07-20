#ifndef _VELPOS_ESTIMATE_H
#define _VELPOS_ESTIMATE_H

typedef struct velpos_estimate velpos_estimate_t;
struct velpos_estimate 
{
    // We estimate 4 variables: x0, vx and y0, vy.  These are the A
    // and b matrices used in the least-squares computation.
    double A[2][2];    double bx[2];
    double by[2];
    int hits;
    double x0, vx;
    double y0, vy;
};

int velpos_estimate_init(velpos_estimate_t *ev);

// provide position information at some point in time relative to the
// time you're interested in. E.g., if you're computing the current position
// of an object based on historical values, then dt < 0.
int velpos_estimate_update(velpos_estimate_t *ev, double x, double y, double dt, double W);

// returns <0 if not enough data. Computes the position and velocity at dt=0
int velpos_estimate_finish(velpos_estimate_t *ev);

#endif
