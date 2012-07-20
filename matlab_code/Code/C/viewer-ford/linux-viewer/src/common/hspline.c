#include <stdlib.h>
#include <math.h>

#include "hspline.h"

#define SQR(o) ((o)*(o))

#define HERMITE_P0(t) ((t)*(t)*(2*(t) - 3) + 1)
#define HERMITE_M0(t) ((t)*((t)*((t) - 2) + 1))
#define HERMITE_P1(t) ((t)*(t)*(-2*(t) + 3))
#define HERMITE_M1(t) ((t)*(t)*((t) - 1))

int
hspline_sample_segment_points (const hspline_point_t * p1,
        const hspline_point_t * p2,
        double * x, double * y, int num_samples, double scale)
{
    int j;
    double inv_num_samples = 1.0 / num_samples;
    double t = 0;
    for (j = 0; j < num_samples; ++j) {
        double t2 = t*t;
        double t3 = t*t2;

        double hp1 = -2*t3 + 3*t2;
        double hp0 = 1 - hp1;
        double hm0 = t3 - 2*t2 + t;
        double hm1 = t3 - t2;

        x[j] = hp0 * p1->pos[0] + hp1 * p2->pos[0] +
          scale * (hm0 * p1->tan[0] + hm1 * p2->tan[0]);
        y[j] = hp0 * p1->pos[1] + hp1 * p2->pos[1] +
            scale * (hm0 * p1->tan[1] + hm1 * p2->tan[1]);

        t += inv_num_samples;
    }
    return 0;
}

int
hspline_sample_points_autoscale  (const hspline_point_t * sp, int num_control,
        double * x, double * y, int num_output)
{
    double total_dist = 0;
    double dists[num_control-1];
    int i;
    for (i = 0; i < num_control-1; i++) {
        dists[i] = sqrt (SQR (sp[i+1].pos[0] - sp[i].pos[0]) +
                SQR (sp[i+1].pos[1] - sp[i].pos[1]));
        total_dist += dists[i];
    }

    int done = 0; 
    for (i = 0; i < num_control-1; i++) {
        int num_samples = (num_output - done - 1);
        if (i < num_control - 2)
            num_samples = num_samples * dists[i] / total_dist;
        total_dist -= dists[i];

        hspline_sample_segment_points (sp+i, sp+i+1, x+done, y+done,
                num_samples, dists[i]);
        done += num_samples;
    }

    x[done] = sp[num_control-1].pos[0];
    y[done] = sp[num_control-1].pos[1];

    return 0;
}

int
hspline_sample_points(const hspline_point_t * sp, int num_control,
        double * x, double * y, int num_output)
{
    double total_dist = 0;
    double dists[num_control-1];
    int i;
    for (i = 0; i < num_control-1; i++) {
        dists[i] = sqrt (SQR (sp[i+1].pos[0] - sp[i].pos[0]) +
                SQR (sp[i+1].pos[1] - sp[i].pos[1]));
        total_dist += dists[i];
    }

    int done = 0; 
    for (i = 0; i < num_control-1; i++) {
        int num_samples = (num_output - done - 1);
        if (i < num_control - 2)
            num_samples = num_samples * dists[i] / total_dist;
        total_dist -= dists[i];

        hspline_sample_segment_points (sp+i, sp+i+1, x+done, y+done,
                num_samples, 1);
        done += num_samples;
    }

    x[done] = sp[num_control-1].pos[0];
    y[done] = sp[num_control-1].pos[1];

    return 0;
}
