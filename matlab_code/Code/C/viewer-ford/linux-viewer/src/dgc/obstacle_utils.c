#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <common/fasttrig.h>

#include <common/convexhull.h>
#include "obstacle_utils.h"

pointlist2d_t *
orutil_project_obstacle_to_image (const double x0, const double y0, 
        const double z0,
        const float sx, const float sy, const float sz,
        const float theta,
        const double to_camera[16],
        CamTrans *camtrans,
        CamFOVTester *fov_tester)
{
    // gather obstacle points
    point2d_t pts_local[4] = {
        { -sx/2, -sy/2 },
        {  sx/2, -sy/2 },
        {  sx/2,  sy/2 },
        { -sx/2,  sy/2 },
    };
    double sintheta, costheta;
    fasttrig_sincos (theta, &sintheta, &costheta);

    for (int j=0; j<4; j++) {
        double x = pts_local[j].x;
        double y = pts_local[j].y;
        pts_local[j].x = costheta * x - sintheta * y + x0;
        pts_local[j].y = sintheta * x + costheta * y + y0;
    }

    int npoints_in_camfov = 0;
    double img_width = camtrans_get_width (camtrans);
    double img_height = camtrans_get_height (camtrans);

    point2d_t pts_img[8];
    for (int i=0; i<8; ++i) {
        double pt_local[3] = { pts_local[i/2].x, pts_local[i/2].y, z0 };

        if (i % 2) pt_local[2] += sz;

        double pt_img_undist[3];
        matrix_vector_multiply_4x4_3d (to_camera, pt_local, pt_img_undist);

        point2d_t *pt_img = &pts_img[i];
        pt_img_undist[0] /= pt_img_undist[2];
        pt_img_undist[1] /= pt_img_undist[2];
        camtrans_distort_pixel (camtrans, pt_img_undist[0], pt_img_undist[1],
                                &pt_img->x, &pt_img->y);

        // bad shit happens when projecting something that's partially behind
        // the camera.
        if (pt_img_undist[2] < 0) return NULL;
        
        npoints_in_camfov += pt_img->x > 0 && pt_img->y > 0 && 
            pt_img->x < img_width && pt_img->y < img_height;
    }
    if (!npoints_in_camfov) {
        return NULL;
    }

    // compute the convex hull of the obstacle's projection into the image
    pointlist2d_t ptlist_pts_img = { .npoints = 8, .points = pts_img };

    return convexhull_graham_scan_2d (&ptlist_pts_img);
}
