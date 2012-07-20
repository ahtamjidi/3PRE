#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <common/geometry.h>
#include <common/convexhull.h>

int main(int argc, char **argv)
{
    int npoints = 100;

    GRand *rng = g_rand_new ();

    pointlist2d_t *candidates = pointlist2d_new (npoints);
    for (int i=0; i<npoints; i++) {
        candidates->points[i].x = g_rand_double_range (rng, 0, 100);
        candidates->points[i].y = g_rand_double_range (rng, 0, 100);
    }

    for (int i=0; i<5000000; i++) {
        pointlist2d_t *a = pointlist2d_new (g_rand_int_range (rng, 3, npoints / 3));
        pointlist2d_t *b = pointlist2d_new (g_rand_int_range (rng, 3, npoints / 3));

        for (int j=0; j<a->npoints; j++) {
            a->points[j] = 
                candidates->points[g_rand_int_range (rng, 0, npoints)];
        }
        for (int j=0; j<b->npoints; j++) {
            b->points[j] = 
                candidates->points[g_rand_int_range (rng, 0, npoints)];
        }

        pointlist2d_t *ah = convexhull_graham_scan_2d (a);
        if (ah) {
            pointlist2d_t *bh = convexhull_graham_scan_2d (b);
            if (bh) {
                pointlist2d_t *isect = 
                    geom_convex_polygon_convex_polygon_intersect_2d (ah, bh);

                if (isect) 
                    pointlist2d_free (isect);
                pointlist2d_free (bh);
            }
            pointlist2d_free (ah);
        }

        pointlist2d_free (a);
        pointlist2d_free (b);
    }

    return 0;
}
