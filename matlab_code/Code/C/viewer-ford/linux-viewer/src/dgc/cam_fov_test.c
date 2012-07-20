#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <common/ephemeris.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>

#include "cam_fov_test.h"

//#define dbgf(...) fprintf (stderr, __FILE__ ":" __VA_ARGS__);
#define dbgf(...) 

CamFOVTester * 
cam_fov_tester_new (const char *cam_name)
{
    CamFOVTester *self = (CamFOVTester*) calloc (1, sizeof (CamFOVTester));

    self->config = globals_get_config ();

    self->camtrans = config_util_get_new_camtrans (self->config, cam_name);
    if (!self->camtrans) goto fail;

    double width = camtrans_get_image_width (self->camtrans);
    double height = camtrans_get_image_height (self->camtrans);

    // compute an approximate viewing pyramid in the sensor frame.
    // This will not be completely accurate due to radial distortions, but 
    // assuming a normal lens, the entire viewing frustum will be represented,
    // plus some space that's not actually visible
    CamFOVTesterFrustum *frust = &self->orig_frustum;
    camtrans_get_position (self->camtrans, frust->position);

    double tl_ray_sensor[3];
    double tr_ray_sensor[3];
    double bl_ray_sensor[3];
    double br_ray_sensor[3];
    camtrans_pixel_to_ray (self->camtrans, 0, 0, tl_ray_sensor);
    camtrans_pixel_to_ray (self->camtrans, width, 0, tr_ray_sensor);
    camtrans_pixel_to_ray (self->camtrans, 0, height, bl_ray_sensor);
    camtrans_pixel_to_ray (self->camtrans, width, height, br_ray_sensor);
    camtrans_pixel_to_ray (self->camtrans, width/2, height/2, frust->view_ray);
    vector_normalize_3d (frust->view_ray);


    // Compute plane normals (sensor frame)
    vector_cross_3d (tl_ray_sensor, bl_ray_sensor, frust->plane_left);
    vector_cross_3d (br_ray_sensor, tr_ray_sensor, frust->plane_right);
    vector_cross_3d (tr_ray_sensor, tl_ray_sensor, frust->plane_top);
    vector_cross_3d (bl_ray_sensor, br_ray_sensor, frust->plane_bottom);
    vector_normalize_3d (frust->plane_left);
    vector_normalize_3d (frust->plane_right);
    vector_normalize_3d (frust->plane_top);
    vector_normalize_3d (frust->plane_bottom);

    // Compute plane offsets (sensor frame)
    frust->plane_left[3] = -vector_dot_3d (frust->plane_left,
                                           frust->position);
    frust->plane_right[3] = -vector_dot_3d (frust->plane_right,
                                            frust->position);
    frust->plane_top[3] = -vector_dot_3d (frust->plane_top,
                                          frust->position);
    frust->plane_bottom[3] = -vector_dot_3d (frust->plane_bottom,
                                             frust->position);

    frust->min_dist_sq = 0.2*0.2;
    frust->max_dist_sq = 100*100;

    self->cur_frustum = self->orig_frustum;

    return self;

fail:
    cam_fov_tester_destroy (self);
    return NULL;
}

void 
cam_fov_tester_destroy (CamFOVTester * self)
{
    if (self->config) globals_release_config (self->config);
    if (self->camtrans) camtrans_destroy (self->camtrans);

    memset (self, 0, sizeof (CamFOVTester));
    free (self);
}

static void
transform_plane (double src[4], double m[12], double dst[4])
{
    dst[0] = src[0]*m[0] + src[1]*m[1] + src[2]*m[2];
    dst[1] = src[0]*m[4] + src[1]*m[5] + src[2]*m[6];
    dst[2] = src[0]*m[8] + src[1]*m[9] + src[2]*m[10];

    double v[3] = {m[0]*m[3] + m[4]*m[7] + m[8]*m[11],
                   m[1]*m[3] + m[5]*m[7] + m[9]*m[11],
                   m[2]*m[3] + m[6]*m[7] + m[10]*m[11]};

    dst[3] = -vector_dot_3d (v, src) - src[3];
}

void
cam_fov_tester_set_transformation (CamFOVTester *self, 
        double calib_to_new[12])
{
    CamFOVTesterFrustum *orig = &self->orig_frustum;
    CamFOVTesterFrustum *cur = &self->cur_frustum;
    double *m = (double*)calib_to_new;

    transform_plane (orig->plane_left, calib_to_new, cur->plane_left);
    transform_plane (orig->plane_right, calib_to_new, cur->plane_right);
    transform_plane (orig->plane_top, calib_to_new, cur->plane_top);
    transform_plane (orig->plane_bottom, calib_to_new, cur->plane_bottom);

    matrix_vector_multiply_4x4_3d (calib_to_new, orig->position,
                                   cur->position);

    double *vr = (double*)orig->view_ray;
    cur->view_ray[0] = m[0]*vr[0] + m[1]*vr[1] + m[2]*vr[2];
    cur->view_ray[1] = m[4]*vr[0] + m[5]*vr[1] + m[6]*vr[2];
    cur->view_ray[2] = m[8]*vr[0] + m[9]*vr[1] + m[10]*vr[2];
}

int 
cam_fov_tester_find_sun (CamFOVTester *self, CTrans *ctrans,
        lcmtypes_pose_t pose, double sun_pos[2], double gps_ray[3])
{
    int sun_in_view = 1;

    // Get pose in gps coords
    double lat_lon_el[3];
    ctrans_local_to_gps (ctrans, pose.pos, lat_lon_el, NULL);

    // Get the sun vector in local coords
    double local_ray[3];
    solar_ephemeris_get_sun_vector (lat_lon_el[0], lat_lon_el[1],
                                    pose.utime, gps_ray);
    ctrans_gps_to_local_dir (ctrans, gps_ray, local_ray);

    // See if sun is below horizon
    if (gps_ray[2] <= 0) {
        sun_in_view = 0;
        return -1;
    }

    // Rotate sun ray (local coords) into camera frame
    double calibration_to_local[16];
    ctrans_calibration_to_local_matrix (ctrans, calibration_to_local);
    double *m = calibration_to_local;
    double local_to_calib_rot[9] =
        { m[0],m[4],m[8],m[1],m[5],m[9],m[2],m[6],m[10] };

    double calib_to_cam[12];
    camtrans_get_world_to_cam_matrix (self->camtrans, calib_to_cam);
    m = calib_to_cam;
    double calib_to_cam_rot[9] =
        { m[0],m[1],m[2],m[4],m[5],m[6],m[8],m[9],m[10] };

    double cam_ray[3], rot[9];
    matrix_multiply_3x3_3x3 (calib_to_cam_rot, local_to_calib_rot, rot);
    matrix_vector_multiply_3x3_3d (rot, local_ray, cam_ray);

    // See if sun is behind camera
    if (cam_ray[2] <= 0) {
        sun_in_view = 0;
        return -1;
    }

    // Distort point
    double u, v;
    camtrans_distort_pixel (self->camtrans, cam_ray[0]/cam_ray[2],
                            cam_ray[1]/cam_ray[2], &u, &v);
    // Check against image bounds
    int img_width = camtrans_get_image_width (self->camtrans);
    int img_height = camtrans_get_image_height (self->camtrans);
    int pad = 30;
    if ((u < -pad) || (u >= img_width-1+pad) ||
        (v < -pad) || (v >= img_height-1+pad)) {
        sun_in_view = 0;
        return -1;
    }

    if (sun_in_view) {
        sun_pos[0] = u;
        sun_pos[1] = v;
        return 0;
    }

    return -1;
}
