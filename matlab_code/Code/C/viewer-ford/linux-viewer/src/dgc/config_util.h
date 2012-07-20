#ifndef _CONFIG_UTIL_H
#define _CONFIG_UTIL_H

#include <common/config.h>
#include <common/geometry.h>
#include <common/camtrans.h>
#include <lcmtypes/lcmtypes_pose_t.h>

#ifdef __cplusplus
extern "C" {
#endif

int config_util_get_quat(const char *name, double quat[4]);
int config_util_get_pos(const char *name, double pos[3]);
int config_util_get_matrix(const char *name, double m[16]);
int config_util_sensor_to_local(const char *name, double m[16]);
int config_util_sensor_to_local_at(const char *name, double m[16], 
        int64_t utime);
int config_util_sensor_to_local_with_pose(const char *name, double m[16], 
        lcmtypes_pose_t *p);

int config_util_get_high_bandwidth_mcaddr_and_port (Config *cfg, char *result, 
        int result_size, uint16_t *port);

/**
 * Retrieves the vehicle footprint.  The resulting polygon will be a rectangle
 * with points in the following order:
 *  front left
 *  front right
 *  rear right
 *  rear left
 *
 * Returns: a newly allocated polygon, or NULL
 */
pointlist2d_t * config_util_get_vehicle_footprint (Config *cfg);

// =================== cameras ========================

CamTrans * config_util_get_new_camtrans (Config *cfg, const char *camera_name);

/**
 * Returns: a newly allocated array of strings that must be freed with
 * g_strfreev or something similar.
 */
char **config_util_get_all_camera_names (Config *cfg);

int config_util_get_camera_calibration_config_prefix (Config *cfg, 
        const char *camera_name, char *result, int result_size);

/**
 * @cfg: the config object to use, or NULL to use the singleton config.
 * @uid: the uid to lookup
 * @result: where to put the result
 *
 * Returns: 0 on success, or -1 on failure or if result is not big enough
 */
int config_util_cam_uid_to_name (Config *cfg, int64_t uid, 
        char *result, int result_size);

/*
 *
 * Returns: 0 on success, or -1 on failure or if result is not big enough
 */
int config_util_get_camera_image_url_channel (Config *cfg, 
        const char *camera_name, char *result, int result_size);

int config_util_get_camera_thumbnail_channel (Config *cfg,
        const char *camera_name, char *result, int result_size);

int config_util_get_camera_full_frame_channel (Config *cfg,
        const char *camera_name, char *result, int result_size);

int config_util_get_camera_short_name (Config * cfg,
        const char * camera_name, char * result, int result_size);

int config_util_cam_uid_to_short_name (Config * cfg, int64_t uid,
        char * result, int result_size);

/**
 * Returns: a newly allocated polygon, or NULL
 */
pointlist2i_t * config_util_get_camera_vehicle_body_polygon (Config *cfg,
        const char *camera_name);

// =================== SICK LMS lidar =================

/**
 * Returns: a newly allocated array of strings that must be freed with
 * g_strfreev or something similar.
 */
char **config_util_get_all_lidar_names (Config *cfg);

int config_util_get_lidar_calibration_config_prefix (Config *cfg, 
        const char *lidar_name, char *result, int result_size);

int config_util_get_lidar_lc_channel (Config *cfg, const char *lidar_name,
        char *result, int result_size);

int config_util_get_lidar_viewer_color (Config *cfg, const char *lidar_name,
        double result[3]);

int config_util_lidar_lc_channel_to_lidar_name (Config *cfg, 
        const char *channel, char *resut, int result_size);

// ====================== Delphi radar =================

/**
 * Returns: a newly allocated array of strings that must be freed with
 * g_strfreev or something similar.
 */
char **config_util_get_all_radar_names (Config *cfg);

int config_util_get_radar_calibration_config_prefix (Config *cfg, 
        const char *radar_name, char *result, int result_size);

int config_util_get_radar_lc_channel (Config *cfg, const char *radar_name,
        char *result, int result_size);

// ===================== Velodyne ===================


// ===================== RNDF =====================
int config_util_get_rndf_absolute_path (Config *cfg, char *buf, int buf_size);

#ifdef __cplusplus
}
#endif

#endif
