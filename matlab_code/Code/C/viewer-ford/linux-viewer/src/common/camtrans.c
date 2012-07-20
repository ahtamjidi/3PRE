#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#if 0
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#endif

#include <common/small_linalg.h>
#include <common/rotations.h>

#include "camtrans.h"

#define CAMERA_EPSILON 1e-10

// Function that produces values for the warp tables
typedef void (*warp_func_t)(const double, const double, const void*,
                            double*, double*);


// Structure for spherical distortion model
typedef struct {
  double w;   // Original image width
  double h;   // Original image height
  double cx;  // Center of distortion (normalized coords)
  double cy;
  double a;   // Spherical distortion parameter

  // Temporaries for faster computation
  double inv_w;
  double x_trans;
  double y_trans;
} SphericalDistortion;


// Construct spherical distortion object, initialized with parameters
static SphericalDistortion*
spherical_distortion_create (const double w, const double h,
                             const double cx, const double cy,
                             const double a)
{
  SphericalDistortion* dist =
    (SphericalDistortion*)malloc(sizeof(SphericalDistortion));
  dist->w = w;
  dist->h = h;
  dist->cx = cx;
  dist->cy = cy;
  dist->a = a*a*a*a;
  dist->inv_w = 1/w;
  dist->x_trans = 0.5 + cx;
  dist->y_trans = 0.5*h/w + cy;  
  return dist;
}

// Destructor
static void
spherical_distortion_destroy(SphericalDistortion* dist)
{
  free(dist);
}


// Undistort according to spherical model
static void
spherical_undistort_func(const double x, const double y, const void* data,
                         double* ox, double* oy)
{
  SphericalDistortion* dist = (SphericalDistortion*)data;

  *ox = x * dist->inv_w - dist->x_trans;
  *oy = y * dist->inv_w - dist->y_trans;

  double r2 = *ox * *ox + *oy * *oy;
  double new_r2 = r2/(1 - r2*dist->a);

  double ratio;
  if (fabs(r2) < 1e-8)
    ratio = 0;
  else
    ratio = new_r2/r2;

  if (ratio >= 0) {
    ratio = sqrt(ratio);
    *ox = (*ox*ratio + dist->x_trans)*dist->w;
    *oy = (*oy*ratio + dist->y_trans)*dist->w;
  }
  else {
    *ox = *oy = -1e50;
  }
}


// Distort according to spherical model
static void
spherical_distort_func(const double x, const double y, const void* data,
                       double* ox, double* oy)
{
  SphericalDistortion* dist = (SphericalDistortion*)data;

  *ox = x * dist->inv_w - dist->x_trans;
  *oy = y * dist->inv_w - dist->y_trans;

  double r2 = *ox * *ox + *oy * *oy;
  double new_r2 = r2/(1 + r2*dist->a);

  double ratio;
  if (fabs(r2) < 1e-8)
    ratio = 0;
  else
    ratio = new_r2/r2;

  ratio = sqrt(ratio);
  *ox = (*ox*ratio + dist->x_trans)*dist->w;
  *oy = (*oy*ratio + dist->y_trans)*dist->w;
}




// Structure to hold camera data
struct _CamTrans {
  double width;             // Image width in pixels
  double height;            // Image height in pixels

  double position[3];       // Position in world (sensor) coords
  double orientation[4];    // Orientation in world (sensor) coords as
                            // quaternion
  double fx;                // Focal length in x (pixels)
  double fy;                // Focal length in y (pixels)
  double cx;                // Principal point x (pixels)
  double cy;                // Principal point y (pixels)
  double skew;              // Pixel skew

  double matx[12];          // Aggregate 3x4 camera projection matrix
  double inv_matx[9];       // Inverse of 3x3 submatrix of matx

  warp_func_t undist_func;  // Undistortion function
  warp_func_t dist_func;    // Distortion function
  void* dist_data;          // Distortion/undistortion data
};

CamTrans * 
camtrans_new_spherical (double width, double height, 
        double fx, double fy, 
        double cx, double cy, double skew,
        const double position[3], const double orientation_quat[4],
        const double distortion_cx, const double distortion_cy,
        double distortion_param)
{
    CamTrans *self = malloc (sizeof (CamTrans));
    self->width = width;
    self->height = height;
    self->fx = fx;
    self->fy = fy;
    self->cx = cx;
    self->cy = cy;
    self->skew = skew;
    self->position[0] = position[0];
    self->position[1] = position[1];
    self->position[2] = position[2];
    self->orientation[0] = orientation_quat[0];
    self->orientation[1] = orientation_quat[1];
    self->orientation[2] = orientation_quat[2];
    self->orientation[3] = orientation_quat[3];

    SphericalDistortion* dist_data;
    dist_data = spherical_distortion_create(self->width, self->height,
                                          distortion_cx, distortion_cy,
                                          distortion_param);

    // Create distortion/undistortion objects
    self->dist_data = (void*)dist_data;
    self->undist_func = spherical_undistort_func;
    self->dist_func = spherical_distort_func;

    // Compute internal matrix representations
    camtrans_compute_matrices (self);
    return self;
}

// Destructor
void camtrans_destroy (CamTrans* t)
{
    if (NULL != t) {
        spherical_distortion_destroy(t->dist_data);
        free(t);
    }
}


void 
camtrans_compute_matrices (CamTrans *t) 
{
    double rot[9];
    double rot_trans[9];
    rot_quat_to_matrix(t->orientation, rot);
    matrix_transpose_3x3d (rot, rot_trans);

    double tmp_34[] = {
        1, 0, 0, -t->position[0],
        0, 1, 0, -t->position[1],
        0, 0, 1, -t->position[2],
    };
    double pinhole[] = {
        t->fx, t->skew, t->cx,
        0,     t->fy,   t->cy,
        0,     0,       1 
    };

    double tmp_33[9];
    matrix_multiply_3x3_3x3 (pinhole, rot_trans, tmp_33);
    matrix_multiply (tmp_33, 3, 3, tmp_34, 3, 4, t->matx);
    double m[9] = {
        t->matx[0], t->matx[1], t->matx[2], 
        t->matx[4], t->matx[5], t->matx[6], 
        t->matx[8], t->matx[9], t->matx[10]
    };
    int status = matrix_inverse_3x3d (m, t->inv_matx);
    if (0 != status) {
        fprintf (stderr, "WARNING: camera matrix is singular (%s:%d)\n",
                __FILE__, __LINE__);
    }
}

// Gets camera position in sensor coordinates
void camtrans_get_position (const CamTrans* t,
                                    double pos[3]) {
  pos[0] = t->position[0];
  pos[1] = t->position[1];
  pos[2] = t->position[2];
}

void camtrans_get_orientation (const CamTrans *t,
                                       double orientation[4]) 
{
    orientation[0] = t->orientation[0];
    orientation[1] = t->orientation[1];
    orientation[2] = t->orientation[2];
    orientation[3] = t->orientation[3];
}

void camtrans_get_world_to_cam_matrix (const CamTrans *t, double matrix[12])
{
    for (int i = 0; i < 12; ++i)
        matrix[i] = t->matx[i];
}

void camtrans_get_cam_to_world_matrix (const CamTrans *t, double matrix[9])
{
    for (int i = 0; i < 9; ++i)
        matrix[i] = t->inv_matx[i];
}

double 
camtrans_get_focal_length_x (const CamTrans *t)
{
    return t->fx;
}
double 
camtrans_get_focal_length_y (const CamTrans *t)
{
    return t->fy;
}
double 
camtrans_get_image_width (const CamTrans *t)
{
    return t->width;
}
double 
camtrans_get_image_height (const CamTrans *t)
{
    return t->height;
}
double 
camtrans_get_principal_x (const CamTrans *t)
{
    return t->cx;
}
double 
camtrans_get_principal_y (const CamTrans *t)
{
    return t->cy;
}

double
camtrans_get_width (const CamTrans *t)
{
    return t->width;
}

double
camtrans_get_height (const CamTrans *t)
{
    return t->height;
}

void
camtrans_get_distortion_center (const CamTrans *t,
                                double *x,
                                double *y)
{
    SphericalDistortion* dist_data = (SphericalDistortion*)t->dist_data;
    *x = dist_data->cx;
    *y = dist_data->cy;
}



void
camtrans_scale_image (CamTrans *t,
                              const double scale_factor)
{
    // Image size
    t->width *= scale_factor;
    t->height *= scale_factor;

    // Pinhole parameters
    t->cx *= scale_factor;
    t->cy *= scale_factor;
    t->fx *= scale_factor;
    t->fy *= scale_factor;
    t->skew *= scale_factor;

    // Projection matrix
    int i;
    for (i = 0; i < 8; ++i)
      t->matx[i] *= scale_factor;

    // Inverse projection matrix
    double inv_scale_factor = 1/scale_factor;
    for (i = 0; i < 3; ++i) {
      t->inv_matx[3*i+0] *= inv_scale_factor;
      t->inv_matx[3*i+1] *= inv_scale_factor;
    }

    // Distortion data
    SphericalDistortion* dist_data = (SphericalDistortion*)t->dist_data;
    dist_data->w *= scale_factor;
    dist_data->h *= scale_factor;
    dist_data->inv_w *= inv_scale_factor;
}

void
camtrans_rotate_camera (CamTrans *t,
                        const double q[4]) {
    // Update orientation quaternion
    double tmp_q[4];
    rot_quat_mult (tmp_q, t->orientation, q);
    t->orientation[0] = tmp_q[0];
    t->orientation[1] = tmp_q[1];
    t->orientation[2] = tmp_q[2];
    t->orientation[3] = tmp_q[3];

    // Update all internal matrices
    camtrans_compute_matrices (t);
}

void
camtrans_set_distortion_center (CamTrans *t,
                                const double cx, const double cy)
{
    SphericalDistortion* dist_data = (SphericalDistortion*)t->dist_data;
    dist_data->cx = cx;
    dist_data->cy = cy;
    dist_data->x_trans = 0.5 + cx;
    dist_data->y_trans = 0.5*t->height/t->width + cy;  
}





int 
camtrans_undistort_pixel (const CamTrans* cam,
                                  const double x, const double y,
                                  double* ox, double* oy)
{
    cam->undist_func(x,y,cam->dist_data,ox,oy);
    return 0;
}



int 
camtrans_distort_pixel (const CamTrans* cam,
                                const double x, const double y,
                                double* ox, double* oy)
{
    cam->dist_func(x,y,cam->dist_data,ox,oy);
    return 0;
}

int camtrans_pixel_to_ray (const CamTrans* cam,
                                  const double x, const double y,
                                  double ray[3]) {
    double u, v;
    int ret;

    ret = camtrans_undistort_pixel(cam, x, y, &u, &v);
    if (ret < 0)
        return ret;

    const double* inv = cam->inv_matx;

    ray[0] = u*inv[0] + v*inv[1] + inv[2];
    ray[1] = u*inv[3] + v*inv[4] + inv[5];
    ray[2] = u*inv[6] + v*inv[7] + inv[8];

    return 0;
}

int 
camtrans_project_point (const CamTrans* cam,
                                const double * p_world,
                                const int distort,
                                double* ox, double* oy, double *oz)
{
    double tx, ty, tz;

    double x = p_world[0];
    double y = p_world[1];
    double z = p_world[2];

    const double* m = cam->matx;
    tx = m[0]*x + m[1]*y + m[2]*z + m[3];
    ty = m[4]*x + m[5]*y + m[6]*z + m[7];
    tz = m[8]*x + m[9]*y + m[10]*z + m[11];

    if (fabs (tz) < CAMERA_EPSILON) return -1;

    double inv_z = 1/tz;
    tx *= inv_z;
    ty *= inv_z;
    if (oz) *oz = tz;

    if (distort != 0)
        return camtrans_distort_pixel(cam,tx,ty,ox,oy);
    else {
        if (ox) *ox = tx;
        if (oy) *oy = ty;
        return 0;
    }
}



int camtrans_project_line (const CamTrans *cam,
                           const double *l_world,
                           double *ox, double *oy, double *oz)
{
    double tx, ty, tz;

    double x = l_world[0];
    double y = l_world[1];
    double z = l_world[2];

    // Multiply by inverse transpose of 3x3 projection matrix
    const double* m = cam->inv_matx;
    tx = m[0]*x + m[3]*y + m[6]*z;
    ty = m[1]*x + m[4]*y + m[7]*z;
    tz = m[2]*x + m[5]*y + m[8]*z;

    double inv_mag = sqrt(tx*tx + ty*ty);
    if (inv_mag < CAMERA_EPSILON)
      return -1;

    inv_mag = 1/inv_mag;
    *ox = tx*inv_mag;
    *oy = ty*inv_mag;
    *oz = tz*inv_mag;
    return 0;
}
