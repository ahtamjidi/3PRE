#ifndef __cam_fov_test_h__
#define __cam_fov_test_h__

#include <common/small_linalg.h>
#include <common/config.h>
#include <common/camtrans.h>
#include <dgc/ctrans.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CamFOVTesterFrustum {
    double position[3];
    double view_ray[3];

    double plane_left[4];
    double plane_right[4];
    double plane_top[4];
    double plane_bottom[4];

    double min_dist_sq;
    double max_dist_sq;
} CamFOVTesterFrustum;


typedef struct _CamFOVTester {
    Config *config;
    CamTrans *camtrans;

    CamFOVTesterFrustum orig_frustum;
    CamFOVTesterFrustum cur_frustum;  

} CamFOVTester;

CamFOVTester * cam_fov_tester_new (const char *cam_name);

void cam_fov_tester_destroy (CamFOVTester * self);

/**
 * cam_fov_tester_set_transformation:
 * 
 * Sets a transformation matrix that will be applied to the viewing frustum.
 * Use this to transform the FOV frustum from the calibration frame into the
 * local frame.
 *
 * The effects of calling this function multiple times are not cumulative.
 * i.e. calling it N times with N different matrices has the same effect as
 * calling it only once with the last matrix.
 */
void cam_fov_tester_set_transformation (CamFOVTester *self, 
        double calib_to_new[12]);


/*
 * test of point against full view frustum
 * currently does not test against minimum distance
 */
static inline int
cam_fov_tester_test_xyz (CamFOVTester *self, double pt[3])
{
    // Is point behind camera?
    if (vector_dot_3d (self->cur_frustum.view_ray, pt) < 0)
        return 0;

#if 0
    CamFOVTesterFrustum *fr = &self->cur_frustum;

    // Is point too far away?
    if (vector_dist_squared_3d (pt, fr->position) > fr->max_dist_sq)
        return 0;
    
    // Is point within side planes of frustum?
    if (((vector_dot_3d (fr->plane_left, pt) + fr->plane_left[3]) > 0) ||
        ((vector_dot_3d (fr->plane_right, pt) + fr->plane_right[3]) > 0) ||
        ((vector_dot_3d (fr->plane_top, pt) + fr->plane_top[3]) > 0) ||
        ((vector_dot_3d (fr->plane_bottom, pt) + fr->plane_bottom[3]) > 0))
        return 0;
#endif

    return 1;
}

/**
 * cam_fov_tester_find_sun:
 *
 * Checks to see if the sun is in the FOV of the camera, and computes the
 * center of the projection of the sun into the image.
 *
 * returns: 0 if the sun is in the FOV of the camera at the specified pose, and
 * -1 if not.
 */
int cam_fov_tester_find_sun (CamFOVTester *self, CTrans *ctrans,
        lcmtypes_pose_t pose, double sun_img[2], double gps_ray[3]);

#ifdef __cplusplus
}
#endif

#endif
