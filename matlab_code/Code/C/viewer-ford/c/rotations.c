#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "rotations.h"

static inline int
feq (double a, double b) {
    return fabs (a - b) < 1e-9;
}

static inline int
qeq (double *q, double a, double b, double c, double d)
{
    return feq(q[0],a) && feq(q[1],b) && feq(q[2],c) && feq(q[3],d);
}

static inline int
rpyeq (double roll, double pitch, double yaw, double r, double p, double y) 
{
    return feq(roll,r) && feq(pitch,p) && feq(yaw,y);
}

/**
 * rot_quat_mult:
 * @c: Array where the result of the multiplication is stored.  Must
 *     be at least 4 elements long.
 * @a: Input quaternion to be multiplied (left-side).
 * @b: Input quaternion to be multiplied (right-side).
 *
 * Multiplies quaternion @a times quaternion @b and stores the result in @c.
 * @a and @b must be double-precision floating point arrays of length 4.
 */
void
rot_quat_mult (double c[4], const double a[4], const double b[4])
{
    c[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
    c[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
    c[2] = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1];
    c[3] = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0];
}

/**
 * rot_quat_rotate:
 * @rot: Unit quaternion that specifies the rotation.
 * @v:   3-vector that is rotated according to the quaternion @rot
 *       and modified in place with the result.
 *
 * Rotates a vector @v from one coordinate system to another as
 * specified by a unit quaternion @rot.
 */
void
rot_quat_rotate (const double rot[4], double v[3])
{
    double a[4], b[4], c[4];

    b[0] = 0;
    memcpy (b+1, v, 3 * sizeof (double));

    rot_quat_mult (a, rot, b);
    b[0] = rot[0];
    b[1] = -rot[1];
    b[2] = -rot[2];
    b[3] = -rot[3];
    rot_quat_mult (c, a, b);

    memcpy (v, c+1, 3 * sizeof (double));
}

/**
 * rot_quat_rotate_rev:
 * @rot: Unit quaternion that specifies the rotation.
 * @v:   3-vector that is rotated according to the quaternion @rot
 *       and modified in place with the result.
 *
 * Rotates a vector @v from one coordinate system to another as
 * specified by a unit quaternion @rot, but performs the rotation
 * in the reverse direction of rot_quat_rotate().
 */
void
rot_quat_rotate_rev (const double rot[4], double v[3])
{
    double a[4], b[4], c[4];

    b[0] = 0;
    memcpy (b+1, v, 3 * sizeof (double));

    rot_quat_mult (a, b, rot);
    b[0] = rot[0];
    b[1] = -rot[1];
    b[2] = -rot[2];
    b[3] = -rot[3];
    rot_quat_mult (c, b, a);

    memcpy (v, c+1, 3 * sizeof (double));
}

void
rot_angle_axis_to_quat (double theta, const double axis[3], double q[4])
{
    double x = axis[0], y = axis[1], z = axis[2];
    double norm = sqrt (x*x + y*y + z*z);
    if (0 == norm) {
        q[0] = 1;
        q[1] = q[2] = q[3] = 0;
        return;
    }

    double t = sin(theta/2) / norm;

    q[0] = cos(theta / 2);
    q[1] = x * t;
    q[2] = y * t;
    q[3] = z * t;
}

void
rot_quat_to_angle_axis (const double q[4], double *theta, double axis[3])
{
    double halftheta = acos (q[0]);
    *theta = halftheta * 2;
    double sinhalftheta = sin (halftheta);
    if (feq (halftheta, 0)) {
        axis[0] = 0;
        axis[0] = 0;
        axis[1] = 1;
        *theta = 0;
    } else {
        axis[0] = q[1] / sinhalftheta;
        axis[1] = q[2] / sinhalftheta;
        axis[2] = q[3] / sinhalftheta;
    }
}

void 
rot_roll_pitch_yaw_to_quat (const double rpy[3], double q[4])
{
    double roll = rpy[0], pitch = rpy[1], yaw = rpy[2];

    double halfroll = roll / 2;
    double halfpitch = pitch / 2;
    double halfyaw = yaw / 2;

    double sin_r2 = sin (halfroll);
    double sin_p2 = sin (halfpitch);
    double sin_y2 = sin (halfyaw);

    double cos_r2 = cos (halfroll);
    double cos_p2 = cos (halfpitch);
    double cos_y2 = cos (halfyaw);

    q[0] = cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2;
    q[1] = sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2;
    q[2] = cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2;
    q[3] = cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2;
}

void 
rot_quat_to_roll_pitch_yaw (const double q[4], double rpy[3]) 
{
    double roll_a = 2 * (q[0]*q[1] + q[2]*q[3]);
    double roll_b = 1 - 2 * (q[1]*q[1] + q[2]*q[2]);
    rpy[0] = atan2 (roll_a, roll_b);

    double pitch_sin = 2 * (q[0]*q[2] - q[3]*q[1]);
    rpy[1] = asin (pitch_sin);

    double yaw_a = 2 * (q[0]*q[3] + q[1]*q[2]);
    double yaw_b = 1 - 2 * (q[2]*q[2] + q[3]*q[3]);
    rpy[2] = atan2 (yaw_a, yaw_b);
}

void 
rot_roll_pitch_yaw_to_angle_axis (const double rpy[3], double *theta,
        double axis[3])
{
    double q[4];
    rot_roll_pitch_yaw_to_quat(rpy, q);
    rot_quat_to_angle_axis (q, theta, axis);
}

void 
rot_angle_axis_to_roll_pitch_yaw (double theta, const double axis[3],
        double rpy[3])
{
    double q[4];
    rot_angle_axis_to_quat (theta, axis, q);
    rot_quat_to_roll_pitch_yaw (q, rpy);
}


int 
rot_quat_to_matrix(const double quat[4], double rot[9])
{
  double norm = quat[0]*quat[0] + quat[1]*quat[1] + quat[2]*quat[2] +
    quat[3]*quat[3];
  if (fabs(norm) < 1e-10)
    return -1;

  norm = 1/norm;
  double x = quat[1]*norm;
  double y = quat[2]*norm;
  double z = quat[3]*norm;
  double w = quat[0]*norm;

  double x2 = x*x;
  double y2 = y*y;
  double z2 = z*z;
  double w2 = w*w;
  double xy = 2*x*y;
  double xz = 2*x*z;
  double yz = 2*y*z;
  double wx = 2*w*x;
  double wy = 2*w*y;
  double wz = 2*w*z;

  rot[0] = w2+x2-y2-z2;  rot[1] = xy-wz;  rot[2] = xz+wy;
  rot[3] = xy+wz;  rot[4] = w2-x2+y2-z2;  rot[5] = yz-wx;
  rot[6] = xz-wy;  rot[7] = yz+wx;  rot[8] = w2-x2-y2+z2;

  return 0;
}

int 
rot_quat_pos_to_matrix(const double quat[4], const double pos[3], double mat[16])
{
    double rot[9];
    rot_quat_to_matrix(quat, rot);

    mat[0] = rot[0];
    mat[1] = rot[1];
    mat[2] = rot[2];
    mat[3] = pos[0];

    mat[4] = rot[3];
    mat[5] = rot[4];
    mat[6] = rot[5];
    mat[7] = pos[1];

    mat[8] = rot[6];
    mat[9] = rot[7];
    mat[10] = rot[8];
    mat[11] = pos[2];

    mat[12] = 0;
    mat[13] = 0;
    mat[14] = 0;
    mat[15] = 1;

    return 0;
}

int 
rot_matrix_to_quat(const double rot[9], double quat[4])
{
  quat[0] = 0.5*sqrt(rot[0]+rot[4]+rot[8]+1);

  if (fabs(quat[0]) > 1e-8) {
    double w4 = 1/4/quat[0];
    quat[1] = (rot[7]-rot[5]) * w4;
    quat[2] = (rot[2]-rot[6]) * w4;
    quat[3] = (rot[3]-rot[1]) * w4;
  }
  else {
    quat[1] = sqrt(fabs(-0.5*(rot[4]+rot[8])));
    quat[2] = sqrt(fabs(-0.5*(rot[0]+rot[8])));
    quat[3] = sqrt(fabs(-0.5*(rot[0]+rot[4])));
  }

  double norm = quat[0]*quat[0] + quat[1]*quat[1] + quat[2]*quat[2] +
    quat[3]*quat[3];
  if (fabs(norm) < 1e-10)
    return -1;

  norm = 1/norm;
  quat[0] *= norm;
  quat[1] *= norm;
  quat[2] *= norm;
  quat[3] *= norm;

  return 0;
}

int
rot_quaternion_test()
{
#define FAIL_TEST { fprintf(stderr, "rot_quaternion_test failed at line %d\n", \
        __LINE__); return 0; }

    fprintf(stderr, "running quaternion test\n");
    double theta = 0;
    double rvec[] = { 0, 0, 1 };
    double q[4];
    double rpy[3];
    double roll, pitch, yaw;

    rot_angle_axis_to_quat(theta, rvec, q);

    if (! qeq (q, 1, 0, 0, 0)) FAIL_TEST;

    rot_quat_to_roll_pitch_yaw (q, rpy);
    roll = rpy[0]; pitch = rpy[1]; yaw = rpy[2];

    if (! rpyeq (roll,pitch,yaw, 0,0,0)) FAIL_TEST;

    // quat_from_angle_axis
    theta = M_PI;
    rot_angle_axis_to_quat(theta, rvec, q);

    fprintf(stderr,"<%.3f, %.3f, %.3f, %.3f>\n", q[0], q[1], q[2], q[3]);
    if (! qeq (q, 0, 0, 0, 1)) FAIL_TEST;

    // rot_quat_to_angle_axis
    rot_quat_to_angle_axis (q, &theta, rvec);
    if (!feq (theta, M_PI)) FAIL_TEST;
    if (!feq(rvec[0], 0) || !feq(rvec[1], 0) || !feq(rvec[2], 1)) FAIL_TEST;

    rot_quat_to_roll_pitch_yaw (q, rpy);

    if (! rpyeq (roll,pitch,yaw, 0,0,M_PI)) FAIL_TEST;

    double q2[4];
    double q3[4];
    double axis1[] = { 0, 1, 0 };
    double axis2[] = { 0, 0, 1 };
    rot_angle_axis_to_quat (M_PI/2, axis1, q);
    rot_angle_axis_to_quat (M_PI/2, axis2, q);
    rot_quat_mult (q3, q, q2);
    rvec[0] = 0; rvec[1] = 0; rvec[2] = 1;
    rot_quat_rotate (q, rvec);
    fprintf(stderr, "by q: [ %.2f, %.2f, %.2f ]\n", rvec[0], rvec[1], rvec[2]);
    rvec[0] = 0; rvec[1] = 0; rvec[2] = 1;
    rot_quat_rotate (q2, rvec);
    fprintf(stderr, "by q2: [ %.2f, %.2f, %.2f ]\n", rvec[0], rvec[1], rvec[2]);
    rvec[0] = 0; rvec[1] = 0; rvec[2] = 1;
    rot_quat_rotate (q3, rvec);
    fprintf(stderr, "by q*q2: [ %.2f, %.2f, %.2f ]\n", 
            rvec[0], rvec[1], rvec[2]);
    rvec[0] = 0; rvec[1] = 0; rvec[2] = 1;
    rot_quat_mult (q3, q2, q);
    rot_quat_rotate (q3, rvec);
    fprintf(stderr, "by q2*q: [ %.2f, %.2f, %.2f ]\n", 
            rvec[0], rvec[1], rvec[2]);

    // TODO

#undef FAIL_TEST

    fprintf(stderr, "rot_quaternion_test complete\n");
    return 1;
}
