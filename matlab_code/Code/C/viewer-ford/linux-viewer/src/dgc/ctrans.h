#ifndef __CTRANS_H__
#define __CTRANS_H__

#include <lcm/lcm.h>
#include <common/config.h>
//#include <common/glib_util.h>
#include <lcmtypes/lcmtypes_pose_t.h>
#include <lcmtypes/lcmtypes_gps_to_local_t.h>
//#include <common/sensor_rotations.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SECTION:CTrans
 *
 * CTrans is a class that transforms between the multiple
 * coordinate frames available in the system.  It listens to POSE_STATE,
 * NMEA, and GLOBAL_TRANS messages in order to facilitate transformations
 * among the global (GPS), local, body, and calibration coordinate systems.
 *
 * CTrans is thread-safe
 */

typedef struct _CTrans CTrans;

typedef enum {
    CTRANS_POSE_UPDATE,
    CTRANS_GPS_TO_LOCAL_UPDATE,
} ctrans_update_type_t;

typedef void * ctrans_handler_id_t;
typedef int (*ctrans_handler_t)(CTrans *, ctrans_update_type_t, void *);

/**
 * ctrans_create:
 *
 * Initializes the traditional version of ctrans that enables translation
 * between the global, local, body, and calibration frames.
 */
CTrans * ctrans_create( lcm_t *lcm, Config * config, int history_len);

/**
 * ctrans_create_rigid_global:
 *
 * Initializes a "special" version of ctrans that uses a fixed
 * global-to-local transformation for all time.  In other words, when
 * transforming from GPS (lat-lon) coordinates into local (X-Y)
 * coordinates, the transformation will always produce the same X-Y
 * coordinate given the same lat-lon.  It chooses the origin for this X-Y
 * frame using the first GPS coordinate it receives from the GPS
 * receiver after calling ctrans_create_rigid_global().  This function
 * should not be used for navigation (since no other module will agree
 * about the meaning of the X-Y frame here).  It should be used for
 * visualization and debugging where GPS coordinates need to be converted
 * to an X-Y representation.
 *
 * Note that transformations to the local and calibration frames can not be
 * used with the CTrans returned by this function.  Use
 * ctrans_gps_to_local() to convert GPS coordinates to the X-Y frame.
 */
//CTrans * ctrans_create_rigid_global (lc_t * lc);
void ctrans_destroy( CTrans *s );

/**
 * ctrans_subscribe:
 *
 * subscribes a function handler to coordinate transformation updates.  The
 * handler must not unsubscribe itself or any other handler during the
 * callback, as memory corruption could occur.
 */
ctrans_handler_id_t ctrans_subscribe (CTrans * s,
        ctrans_handler_t handler, void * userdata);
int ctrans_unsubscribe (CTrans * s, ctrans_handler_id_t id);

int ctrans_gps_to_local( CTrans *s, 
        const double gps[3], double xyz[3], double cov[3][3]);
int ctrans_gps_to_local_raw (CTrans *s, lcmtypes_gps_to_local_t * gps_to_local);
int ctrans_gps_to_local_dir (CTrans *s, double gps_dir[3],
        double local_dir[3]);
int ctrans_local_to_gps (CTrans *s, 
        const double xyz[3], double gps[3], double cov[3][3]);
int ctrans_have_gps_to_local (CTrans *s);
int ctrans_gps_pose (CTrans * s, double lat_lon_el[3], double q[4]);

int ctrans_have_pose (CTrans * s);
int ctrans_local_pos (CTrans * s, double p_local[3]);
int ctrans_local_pos_heading (CTrans *s, double p_local[3], double *heading);
int ctrans_local_pose (CTrans * s, lcmtypes_pose_t * pose);

int ctrans_local_pose_at (CTrans * s, lcmtypes_pose_t * pose, uint64_t t);

int ctrans_body_to_local_matrix (CTrans * s, double m[16]);
int ctrans_body_to_local_matrix_with_pose (CTrans * s, double m[16], 
        const lcmtypes_pose_t *pose);

int ctrans_calibration_to_local_matrix (CTrans * s, double m[16]);
int ctrans_calibration_to_local_matrix_with_pose (CTrans *s, double m[16],
        const lcmtypes_pose_t *pose);

/**
 * ctrans_travel_distance:
 *
 * Computes the distance traveled by the vehicle between times t0 and t1.
 * It does this by examining the saved path and measuring the length of
 * it.  Endpoints are interpolated if their times don't coincide exactly
 * with a position measurement.  An warning will be printed if the saved
 * path has a gap of 50ms or larger, or if t1 is more than 50ms in the
 * future.  In this case, the current velocity alone is used to compute
 * the path length, scaled according to the time difference between t0
 * and t1.  Note that depending on the arguments to ctrans_create(), the
 * saved path may only be a few seconds in length, and thus this function
 * will only work on the recent history of the vehicle.
 *
 * @t0: Starting time for measuring the path length
 * @t1: Ending time for measuring the path length.  Set to 0 or -1 to
 * indicate "now".
 * @dist_out: Computed path length in meters is stored here.
 *
 * Returns: 0 on success, -1 on failure.  Failure occurs when no poses
 * are available.
 */
int ctrans_travel_distance (CTrans * s, double * dist_out,
        int64_t t0, int64_t t1);

/**
 * Below two functions are mainly for batch processing of multiple coordinate 
 * transforms using the same transformation for all conversions.
 */
void ctrans_lock (CTrans * s);
void ctrans_unlock (CTrans * s);

#if 0
/**
 * ctrans_enable_roll_pitch:
 *
 * By default, ctrans uses the full roll-pitch-yaw estimate from the IMU.
 * This function can be used to enable or disable the use of roll-pitch
 * estimates.  When disabled, ctrans will compute yaw in the local frame
 * only, and fix pitch and roll at zero.
 */
void ctrans_enable_roll_pitch (CTrans * s, int enable);
#endif

#ifdef __cplusplus
}
#endif

#endif
