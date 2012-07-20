#include <math.h>
#include <assert.h>

#include <common/rotations.h>
#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/config.h>
#include <dgc/globals.h>

#include "config_util.h"

#ifndef CONFIG_DIR
#error "CONFIG_DIR not defined!"
#endif

#define err(...) fprintf (stderr, __VA_ARGS__)

static int 
_config_util_get_quat (Config *cfg, const char *name, double quat[4])
{
    assert (cfg);

    char key[256];
    sprintf(key, "calibration.%s.orientation", name);
    if (config_has_key(cfg, key)) {
        int sz = config_get_double_array(cfg, key, quat, 4);
        assert(sz==4);
        return 0;
    }

    sprintf(key, "calibration.%s.rpy", name);
    if (config_has_key(cfg, key)) {
        double rpy[3];
        int sz = config_get_double_array(cfg, key, rpy, 3);
        assert(sz == 3);
        for (int i = 0; i < 3; i++)
            rpy[i] = to_radians(rpy[i]);
        rot_roll_pitch_yaw_to_quat(rpy, quat);
        return 0;
    }

    sprintf(key, "calibration.%s.angleaxis", name);
    if (config_has_key(cfg, key)) {
        double aa[4];
        int sz = config_get_double_array(cfg, key, aa, 4);
        assert(sz==4);

        double theta = aa[0];
        double s = sin(theta/2);

        quat[0] = cos(theta/2);
        for (int i = 1; i < 4; i++)
            quat[i] = aa[i] * s;
        return 0;
    }
    return -1;
}

int config_util_get_quat(const char *name, double quat[4])
{
    Config *cfg = globals_get_config();
    int result = _config_util_get_quat (cfg, name, quat);
    globals_release_config (cfg);
    return result;
}

int config_util_get_pos(const char *name, double pos[3])
{
    Config *cfg = globals_get_config();
    char key[256];

    sprintf(key, "calibration.%s.position", name);
    if (config_has_key(cfg, key)) {
        int sz = config_get_double_array(cfg, key, pos, 3);
        assert(sz==3);
        globals_release_config (cfg);
        return 0;
    } 

    globals_release_config (cfg);
    return -1;
}

int config_util_get_matrix(const char *name, double m[16])
{
    double quat[4];
    double pos[3];

    if (config_util_get_quat(name, quat))
        return -1;

    if (config_util_get_pos(name, pos))
        return -1;

    rot_quat_pos_to_matrix(quat, pos, m);

    return 0;
}

int 
config_util_sensor_to_local_with_pose(const char *name, double m[16], 
        lcmtypes_pose_t *p)
{
    double body_to_local[16];

    rot_quat_pos_to_matrix(p->orientation, p->pos, body_to_local);

    double sensor_to_calibration[16];

    if (config_util_get_matrix(name, sensor_to_calibration))
        return -1;

    char key[128];
    sprintf(key,"calibration.%s.relative_to", name);

    char *calib_frame = config_get_str_or_fail(globals_get_config(), key);
    if (!strcmp(calib_frame, "body")) {

        matrix_multiply_4x4_4x4(body_to_local, sensor_to_calibration, m);

    } else {
        double calibration_to_body[16];
        
        if (config_util_get_matrix(calib_frame, calibration_to_body))
            return -1;

        double sensor_to_body[16];
        matrix_multiply_4x4_4x4(calibration_to_body, sensor_to_calibration, 
                                sensor_to_body);
        matrix_multiply_4x4_4x4(body_to_local, sensor_to_body, m);
    }

    return 0;
}

int config_util_sensor_to_local(const char *name, double m[16])
{
    lcmtypes_pose_t p;

    CTrans *ctrans = globals_get_ctrans ();
    int res = ctrans_local_pose(ctrans, &p);
    globals_release_ctrans (ctrans);
    if (res)
        return res;

    return config_util_sensor_to_local_with_pose(name, m, &p);
}

int 
config_util_sensor_to_local_at(const char *name, double m[16], 
        int64_t utime)
{
    lcmtypes_pose_t p;

    CTrans *ctrans = globals_get_ctrans ();
    int res = ctrans_local_pose_at(ctrans, &p, utime);
    globals_release_ctrans (ctrans);
    if (res)
        return res;

    return config_util_sensor_to_local_with_pose(name, m, &p);
}

static int
get_str (Config *cfg, const char *key, char *result, int result_size)
{
    int use_singleton = (cfg == NULL);
    if (use_singleton) cfg = globals_get_config ();
    char *val = NULL;
    int status = config_get_str (cfg, key, &val);
    if (0 == status && result_size > strlen (val)) {
        strcpy (result, val);
        status = 0;
    } else {
        status = -1;
    }
    if (use_singleton) globals_release_config (cfg);
    return status;
}

static int
get_int (Config *cfg, const char *key, int *result)
{
    int use_singleton = (cfg == NULL);
    if (use_singleton) cfg = globals_get_config ();
    int status = config_get_int (cfg, key, result);
    if (use_singleton) globals_release_config (cfg);
    return status;
}

int 
config_util_get_high_bandwidth_mcaddr_and_port (Config *cfg, char *result, 
        int result_size, uint16_t *port)
{
    int status = get_str (cfg, "high_bandwidth_multicast_address", 
            result, result_size);
    if (0 != status) return status;
    int _port;
    status = get_int (cfg, "high_bandwidth_multicast_port", &_port);
    if (0 != status) return status;
    if (_port < 0 || _port > 65535) return -1;
    *port = _port;
    return 0;
}

pointlist2d_t *
config_util_get_vehicle_footprint (Config *cfg)
{
    char *key_fl = "calibration.vehicle_bounds.front_left";
    char *key_fr = "calibration.vehicle_bounds.front_right";
    char *key_bl = "calibration.vehicle_bounds.rear_left";
    char *key_br = "calibration.vehicle_bounds.rear_right";

    int use_singleton = (cfg == NULL);
    if (use_singleton) cfg = globals_get_config ();

    if ((config_get_array_len (cfg, key_fl) != 2) ||
        (config_get_array_len (cfg, key_fr) != 2) ||
        (config_get_array_len (cfg, key_bl) != 2) ||
        (config_get_array_len (cfg, key_br) != 2)) {
        err ("ERROR! invalid calibration.vehicle_bounds!\n");
        return NULL;
    }

    double fl[2], fr[2], bl[2], br[2];
    config_get_double_array (cfg, key_fl, fl, 2);
    config_get_double_array (cfg, key_fr, fr, 2);
    config_get_double_array (cfg, key_bl, bl, 2);
    config_get_double_array (cfg, key_br, br, 2);

    pointlist2d_t *result = pointlist2d_new (4);
    result->points[0].x = fl[0];
    result->points[0].y = fl[1];

    result->points[1].x = fr[0];
    result->points[1].y = fr[1];

    result->points[2].x = br[0];
    result->points[2].y = br[1];

    result->points[3].x = bl[0];
    result->points[3].y = bl[1];

    if (use_singleton) globals_release_config (cfg);
    return result;
}

// ================ cameras ==============
CamTrans * 
config_util_get_new_camtrans (Config *cfg, const char *camera_name)
{
    int global_cfg = 0;
    if (!cfg) {
        cfg = globals_get_config ();
        global_cfg = 1;
    }

    char prefix[1024];
    int status = config_util_get_camera_calibration_config_prefix (cfg, 
            camera_name, prefix, sizeof (prefix));
    if (0 != status) goto fail;

    char key[2048];
    double width;
    sprintf(key, "%s.width", prefix);
    if (0 != config_get_double(cfg, key, &width)) goto fail;

    double height;
    sprintf(key, "%s.height", prefix);
    if (0 != config_get_double(cfg, key, &height)) goto fail;

    double pinhole_params[5];
    snprintf(key, sizeof (key), "%s.pinhole", prefix);
    if (5 != config_get_double_array(cfg, key, pinhole_params, 5)) goto fail;
    double fx = pinhole_params[0];
    double fy = pinhole_params[1];
    double cx = pinhole_params[3];
    double cy = pinhole_params[4];
    double skew = pinhole_params[2];

    double position[3];
    sprintf(key, "%s.position", prefix);
    if (3 != config_get_double_array(cfg, key, position, 3)) goto fail;

    sprintf (key, "cameras.%s", camera_name);
    double orientation[4];
    if (0 != _config_util_get_quat (cfg, key, orientation)) goto fail;

    double dist_center[2];
    sprintf(key, "%s.distortion_center", prefix);
    if (2 != config_get_double_array(cfg, key, dist_center, 2)) goto fail;

    double distortion_param;
    sprintf(key, "%s.distortion_params", prefix);
    if (1 != config_get_double_array(cfg, key, &distortion_param, 1)) goto fail;

    if (global_cfg) globals_release_config (cfg);
    return camtrans_new_spherical (width, height, fx, fy, cx, cy, skew,
            position, orientation, dist_center[0], dist_center[1], 
            distortion_param);
fail:
    if (global_cfg) globals_release_config (cfg);
    return NULL;
}

char **
config_util_get_all_camera_names (Config *cfg)
{
    return config_get_subkeys (cfg, "cameras");
}

int 
config_util_get_camera_calibration_config_prefix (Config *cfg, 
        const char *camera_name, char *result, int result_size)
{
    snprintf (result, result_size, "calibration.cameras.%s", camera_name);
    return config_has_key (cfg, result) ? 0 : -1;
}

int
config_util_cam_uid_to_name (Config *cfg, int64_t uid, char *result, 
        int result_size)
{
    int status = -1;
    int use_singleton = (cfg == NULL);
    if (use_singleton) cfg = globals_get_config ();

    memset (result, 0, result_size);
    char **cam_names = config_get_subkeys (cfg, "cameras");

    for (int i=0; cam_names && cam_names[i]; i++) {
        char *uid_key = 
            g_strdup_printf ("cameras.%s.uid", cam_names[i]);
        char *cam_uid_str = NULL;
        int key_status = config_get_str (cfg, uid_key, &cam_uid_str);
        free (uid_key);

        if (0 == key_status) {
            int64_t cam_uid = strtoll (cam_uid_str, NULL, 16);
            if (cam_uid == uid) {
                if (result_size > strlen (cam_names[i])) {
                    strcpy (result, cam_names[i]);
                    status = 0;
                }
                break;
            }
        }
    }
    g_strfreev (cam_names);

    if (use_singleton) globals_release_config (cfg);

    return status;
}

int 
config_util_get_camera_image_url_channel (Config *cfg, 
        const char *camera_name, char *result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "cameras.%s.image_url_channel", camera_name);
    return get_str (cfg, key, result, result_size);
}

int 
config_util_get_camera_thumbnail_channel (Config *cfg,
        const char *camera_name, char *result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "cameras.%s.thumbnail_channel", camera_name);
    return get_str (cfg, key, result, result_size);
}

int 
config_util_get_camera_full_frame_channel (Config *cfg,
        const char *camera_name, char *result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "cameras.%s.full_frame_channel", camera_name);
    return get_str (cfg, key, result, result_size);
}

int
config_util_get_camera_short_name (Config * cfg,
        const char * camera_name, char * result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "cameras.%s.short_name", camera_name);
    return get_str (cfg, key, result, result_size);
}

int
config_util_cam_uid_to_short_name (Config * cfg, int64_t uid,
        char * result, int result_size)
{
    char name[64];
    if (config_util_cam_uid_to_name (cfg, uid, name, sizeof (name)) < 0)
        return -1;
    return config_util_get_camera_short_name (cfg, name, result, result_size);
}

pointlist2i_t * 
config_util_get_camera_vehicle_body_polygon (Config *cfg,
        const char *camera_name)
{
    char key[1024];
    snprintf (key, sizeof (key), "cameras.%s.vehicle_body", camera_name);

    int use_singleton = (cfg == NULL);
    if (use_singleton) cfg = globals_get_config ();

    int array_len = config_get_array_len (cfg, key);
    printf ("!!%d %s\n", array_len, key);
    if (array_len < 1) return NULL;

    double array_vals[array_len];
    config_get_double_array (cfg, key, array_vals, array_len);
    pointlist2i_t *result = pointlist2i_new (array_len / 2);
    for (int i=0; i<result->npoints; i++) {
        result->points[i].x = (int) round(array_vals[i*2+0]);
        result->points[i].y = (int) round(array_vals[i*2+1]);
        printf ("%d, %d\n", result->points[i].x, result->points[i].y);
    }
    if (use_singleton) globals_release_config (cfg);
    return result;
}

// ============ lidar utilities ============
char **
config_util_get_all_lidar_names (Config *cfg)
{
    return config_get_subkeys (cfg, "lidars");
}

int 
config_util_get_lidar_calibration_config_prefix (Config *cfg, 
        const char *lidar_name, char *result, int result_size)
{
    snprintf (result, result_size, "calibration.lidars.%s", lidar_name);
    return config_has_key (cfg, result) ? 0 : -1;
}

int 
config_util_get_lidar_lc_channel (Config *cfg, const char *lidar_name,
        char *result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "lidars.%s.channel", lidar_name);
    if (! config_has_key (cfg, key)) return -1;
    return get_str (cfg, key, result, result_size);
}

int 
config_util_get_lidar_viewer_color (Config *cfg, const char *lidar_name,
        double result[3])
{
    char key[1024];
    snprintf (key, sizeof (key), "lidars.%s.viewer_color", lidar_name);
    return (config_get_double_array (cfg, key, result, 3) == 3) ? 0 : -1;
}

int 
config_util_lidar_lc_channel_to_lidar_name (Config *cfg, 
        const char *channel, char *result, int result_size)
{
    int status = -1;
    char ** lidar_names = config_util_get_all_lidar_names (cfg);
    for (int i=0; lidar_names[i]; i++) {
        char lchannel[256];
        if (0 == config_util_get_lidar_lc_channel (cfg, lidar_names[i], 
                    lchannel, sizeof (lchannel)) &&
            0 == strcmp (channel, lchannel)) {
            if (result_size > strlen (lidar_names[i])) {
                strcpy (result, lidar_names[i]);
                status = 0;
            }
            break;
        }
    }
    g_strfreev (lidar_names);
    return status;
}

// ===================== radar =====================
//
char **
config_util_get_all_radar_names (Config *cfg)
{
    return config_get_subkeys (cfg, "radars");
}

int 
config_util_get_radar_calibration_config_prefix (Config *cfg, 
        const char *radar_name, char *result, int result_size)
{
    snprintf (result, result_size, "calibration.radars.%s", radar_name);
    return config_has_key (cfg, result) ? 0 : -1;
}

int 
config_util_get_radar_lc_channel (Config *cfg, const char *radar_name,
        char *result, int result_size)
{
    char key[1024];
    snprintf (key, sizeof (key), "radars.%s.channel", radar_name);
    return get_str (cfg, key, result, result_size);
}


// ===================== RNDF =====================

int 
config_util_get_rndf_absolute_path (Config *cfg, char *buf, int buf_size)
{
    char rndf_fname[4096];
    char *rndf_postfix;
    if (config_get_str (cfg, "rndf", &rndf_postfix) < 0)
        return -1;
    snprintf (rndf_fname, sizeof (rndf_fname), CONFIG_DIR"/%s", rndf_postfix);

    if (strlen (rndf_fname) > buf_size) return -1;
    strcpy (buf, rndf_fname);
    return 0;
}
