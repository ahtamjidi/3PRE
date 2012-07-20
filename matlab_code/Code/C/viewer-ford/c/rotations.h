#ifndef _TRANSFORMS_H
#define _TRANSFORMS_H

/**
 * SECTION:transforms
 * @short_description: Transformation utilities
 *
 * This code allows several different rotation representations to be
 * converted.  The representations are:
 *
 * Quaternion           double[4]
 * Angle/Axis           double, double[3]
 * Roll/Pitch/Yaw       double[3]
 * Rotation matrix      double[9]
 * Transform matrix     double[16]
 *
 * Note that these conventions have the property that each
 * representation yields a different function signature.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Multiply quaternions a and b, storing the result in c.
 *
 * When composing rotations, the resulting quaternion represents 
 * first rotating by b, then rotation by a.
 */
void rot_quat_mult (double c[4], const double a[4], const double b[4]);

void rot_quat_rotate (const double quat[4], double v[3]);
void rot_quat_rotate_rev (const double quat[4], double v[3]);

// formerly: rot_quat_to_matrix
int rot_quat_to_matrix(const double quat[4], double rot[9]);

// formerly: rot_matrix_to_quat
int rot_matrix_to_quat(const double rot[9], double quat[4]);

int rot_quat_pos_to_matrix(const double quat[4], const double pose[3], 
        double m[16]);

/** quat_from_angle_axis:
 *
 * populates a quaternion so that it represents a rotation of theta radians
 * about the axis <x,y,z>
 **/
void rot_angle_axis_to_quat (double theta, const double axis[3], double q[4]);

void rot_quat_to_angle_axis (const double q[4], double *theta, double axis[3]);

/**
 * converts a rotation from RPY representation (radians) into unit quaternion
 * representation
 *
 * rpy[0] = roll
 * rpy[1] = pitch
 * rpy[2] = yaw
 */
void rot_roll_pitch_yaw_to_quat(const double rpy[3], double q[4]);

/**
 * converts a rotation from unit quaternion representation to RPY
 * representation.  Resulting values are in radians.
 *
 * If any of roll, pitch, or yaw are NULL, then they are not set.
 *
 * rpy[0] = roll
 * rpy[1] = pitch
 * rpy[2] = yaw
 */
void rot_quat_to_roll_pitch_yaw (const double q[4], double rpy[3]);
  
/* These doesn't truly belong with the quaternion functions, but are useful
 * and sort of fits in here.
 */
void rot_roll_pitch_yaw_to_angle_axis (const double rpy[3], double *angle,
				   double axis[3]);

void rot_angle_axis_to_roll_pitch_yaw (double angle, const double axis[3],
				   double rpy[3]);

// runs some sanity checks
int rot_quaternion_test();

#ifdef __cplusplus
}
#endif

#endif
