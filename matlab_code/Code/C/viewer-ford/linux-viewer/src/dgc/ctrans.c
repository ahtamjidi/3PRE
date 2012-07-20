/* ctrans.c
 *
 * An API for subscribing to the pose messages produced by the position
 * module.  This interface is easier to deal with than a raw LC subscription
 * because it keeps track of recent messages, allows you to look back in
 * time a few seconds, and has convenience functions for converting between
 * the various coordinate frames used in the system.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>

#include <common/glib_util.h>

#include <common/timestamp.h>
#include <common/small_linalg.h>
#include <common/rotations.h>
#include <common/config.h>
#include <common/gps_linearize.h>

#include <lcmtypes/lcmtypes_compat_global_trans_t.h>
#include <lcmtypes/lcmtypes_gps_to_local_t.h>
#include <lcmtypes/lcmtypes_compat_pose_t.h>

#include "ctrans.h"

/* We will never interpolate by more than 50ms between poses. */
#define MAX_INTERPOLATED_DT 50000

struct _CTrans {
    lcm_t *lcm;

    GUCircular * pose_history;
    lcmtypes_gps_to_local_t gps_to_local;
    gps_linearize_t gps_linearize;

    lcm_subscription_t * ph;
    lcmtypes_gps_to_local_t_subscription_t * glhid;
    lcmtypes_compat_global_trans_t_subscription_t  * gthid;

    GList * subscribers;
    Config * config;

    GStaticRecMutex mutex;

    uint8_t have_calibration_to_body;
    double calibration_to_body_quat[4];
    double calibration_to_body_translation[4];
};

typedef struct _CTransSubscriber {
    ctrans_handler_t handler;
    void * user;
} CTransSubscriber;

/* Notify subscribers with their callback.  Lock must already be held
 * when calling this function */
static void
dispatch_subscribers (CTrans * s, ctrans_update_type_t type)
{
    /* Atomic copy of the subscribers list while the mutex is still locked */
    GList * subscribers = g_list_copy (s->subscribers);
    if (subscribers) {
        /* We must fully unlock the mutex. */
        int depth = g_static_rec_mutex_unlock_full (&s->mutex);

        for (GList * iter = subscribers; iter; iter = iter->next) {
            CTransSubscriber * sub = (CTransSubscriber *) iter->data;
            sub->handler (s, type, sub->user);
        }
        g_list_free (subscribers);

        g_static_rec_mutex_lock_full (&s->mutex, depth);
    }
}

// ================= LC handlers ================
static void 
on_pose (const lcm_recv_buf_t *rbuf, const char *channel, void * user)
{
    CTrans * s = (CTrans *) user;
    lcmtypes_pose_t pose;
    /* Decode using the regular LCM decoder.  If that fails try the older
     * version of the lcmtypes_pose_t message and convert it to the new format with
     * acceleration. */
    if (lcmtypes_pose_t_decode (rbuf->data, 0, rbuf->data_size, &pose) < 0) {
        lcmtypes_compat_pose_t cpose;
        if (lcmtypes_compat_pose_t_decode (rbuf->data, 0, rbuf->data_size, &cpose) < 0) {
            fprintf (stderr, "[%s:%d] Error decoding channel %s\n", 
                    __FILE__, __LINE__, channel);
            return;
        }
        pose.utime = cpose.utime;
        int i;
        for (i = 0; i < 3; i++) {
            pose.pos[i] = cpose.pos[i];
            pose.vel[i] = cpose.vel[i];
            pose.rotation_rate[i] = cpose.rotation_rate[i];
            pose.accel[i] = 0;
        }
        for (i = 0; i < 4; i++)
            pose.orientation[i] = cpose.orientation[i];
    }
    g_static_rec_mutex_lock (&s->mutex);

    gu_circular_push_head (s->pose_history, &pose);

    dispatch_subscribers (s, CTRANS_POSE_UPDATE);
    g_static_rec_mutex_unlock (&s->mutex);
}

static void
on_gps_to_local (const lcm_recv_buf_t *rbuf, const char * channel, 
        const lcmtypes_gps_to_local_t * gl, void * user)
{
    CTrans * s = (CTrans *) user;
    g_static_rec_mutex_lock (&s->mutex);

    memcpy (&s->gps_to_local, gl, sizeof (lcmtypes_gps_to_local_t));
    gps_linearize_init (&s->gps_linearize, gl->lat_lon_el_theta);

    dispatch_subscribers (s, CTRANS_GPS_TO_LOCAL_UPDATE);

    g_static_rec_mutex_unlock (&s->mutex);
}

/* Callback for legacy compatability with GLOBAL_TRANS */
static void
on_compat_global_trans (const lcm_recv_buf_t *rbuf, const char * channel, 
        const lcmtypes_compat_global_trans_t *gt,
        void * user)
{
    lcmtypes_gps_to_local_t gl;

    memset (&gl, 0, sizeof (lcmtypes_gps_to_local_t));
    gl.utime = gt->utime;
    memcpy (gl.local, gt->local_gps_origin, 3 * sizeof (double));

    gl.lat_lon_el_theta[0] = gt->gps_origin_lat;
    gl.lat_lon_el_theta[1] = gt->gps_origin_lon;
    gl.lat_lon_el_theta[2] = gt->gps_origin_elev;
    gl.lat_lon_el_theta[3] = gt->heading_diff;

    gl.gps_cov[3][3] = gt->heading_diff_var;

    on_gps_to_local (rbuf, channel, &gl, user);
}

static int
load_calibration_body_transformations (CTrans *s)
{
    if (!s->config)
        return -1;

    if (config_get_double_array (s->config,
                "calibration.calibration_to_body.orientation",
                s->calibration_to_body_quat, 4) < 0) {
        fprintf (stderr, "%s:%d Warning: Missing "
                "calibration_to_body.orientation in config file\n",
                __FILE__, __LINE__);
        return -1;
    }

    if (config_get_double_array (s->config,
                "calibration.calibration_to_body.position",
                s->calibration_to_body_translation, 3) < 0) {
        fprintf (stderr, "%s:%d Warning: Missing "
                "calibration_to_body.position in config file\n",
                __FILE__, __LINE__);
        return -1;
    }
    s->have_calibration_to_body = 1;
    return 0;
}

CTrans *
ctrans_create (lcm_t *lcm, Config * config, int history_len)
{
    CTrans *s = (CTrans *) calloc(1, sizeof(CTrans));

    assert (lcm);

    s->lcm = lcm;
    s->config = config;
    s->subscribers = NULL;

    if (history_len < 1)
        history_len = 1;
    s->pose_history = gu_circular_new (history_len, sizeof (lcmtypes_pose_t));

    load_calibration_body_transformations (s);

    s->ph = lcm_subscribe (lcm, "(POSE|POSE_STATE)", on_pose, s);
    s->glhid = lcmtypes_gps_to_local_t_subscribe (lcm, "GPS_TO_LOCAL",
            on_gps_to_local, s);
    s->gthid = lcmtypes_compat_global_trans_t_subscribe (lcm, "GLOBAL_TRANS",
            on_compat_global_trans, s);

    g_static_rec_mutex_init (&s->mutex);

    return s;
}

void 
ctrans_destroy (CTrans *s)
{
    g_static_rec_mutex_lock (&s->mutex);

    lcm_unsubscribe (s->lcm, s->ph);
    lcmtypes_gps_to_local_t_unsubscribe (s->lcm, s->glhid);
    lcmtypes_compat_global_trans_t_unsubscribe (s->lcm, s->gthid);

    for (GList *iter = s->subscribers; iter; iter = iter->next) {
        free (iter->data);
    }
    g_list_free (s->subscribers);

    g_static_rec_mutex_free (&s->mutex);

    if (s->pose_history)
        gu_circular_free (s->pose_history);
    memset (s, 0, sizeof(CTrans));
    free (s);
}

ctrans_handler_id_t
ctrans_subscribe (CTrans * s, ctrans_handler_t handler, void * user)
{
    g_static_rec_mutex_lock (&s->mutex);
    CTransSubscriber * sub = 
        (CTransSubscriber *) malloc (sizeof(CTransSubscriber));
    sub->handler = handler;
    sub->user = user;
    s->subscribers = g_list_append (s->subscribers, sub);
    g_static_rec_mutex_unlock (&s->mutex);
    return (ctrans_handler_id_t) sub;
}

int
ctrans_unsubscribe (CTrans * s, ctrans_handler_id_t id)
{
    int retval = -1;
    CTransSubscriber * sub = (CTransSubscriber *) id;
    g_static_rec_mutex_lock (&s->mutex);
    for (GList * iter = s->subscribers; iter; iter = iter->next) {
        if (iter->data == sub) {
            free (sub);
            s->subscribers = g_list_delete_link (s->subscribers, iter);
            retval = 0;
            break;
        }
    }
    g_static_rec_mutex_unlock (&s->mutex);
    return retval;
}

int
ctrans_have_gps_to_local (CTrans *s)
{
    g_static_rec_mutex_lock (&s->mutex);
    int result = s->gps_to_local.utime ? 1 : 0;
    g_static_rec_mutex_unlock (&s->mutex);
    return result;
}

int
ctrans_gps_to_local_raw (CTrans *s, lcmtypes_gps_to_local_t * gps_to_local)
{
    if (!ctrans_have_gps_to_local (s))
        return -1;

    g_static_rec_mutex_lock (&s->mutex);
    memcpy (gps_to_local, &s->gps_to_local, sizeof (lcmtypes_gps_to_local_t));
    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_gps_to_local_dir (CTrans *s, double gps_dir[3], double local_dir[3]) {
    if (!ctrans_have_gps_to_local (s))
        return -1;

    g_static_rec_mutex_lock (&s->mutex);

    double theta = s->gps_to_local.lat_lon_el_theta[3];
    double sine, cosine;
    sincos (theta, &sine, &cosine);

    local_dir[0] = gps_dir[0] * cosine - gps_dir[1] * sine;
    local_dir[1] = gps_dir[0] * sine   + gps_dir[1] * cosine;
    local_dir[2] = gps_dir[2];

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

/* gps is (lat, lon, elevation) */
int
ctrans_gps_to_local (CTrans *s, 
        const double gps[3], double xyz[3], double cov[3][3])
{
    if (!ctrans_have_gps_to_local (s))
        return -1;

    g_static_rec_mutex_lock (&s->mutex);
    double p_g[2];
    gps_linearize_to_xy (&s->gps_linearize, gps, p_g);

    double theta = s->gps_to_local.lat_lon_el_theta[3];
    double sine, cosine;
    sincos (theta, &sine, &cosine);

    xyz[0] = p_g[0] * cosine - p_g[1] * sine;
    xyz[1] = p_g[0] * sine   + p_g[1] * cosine;
    xyz[2] = gps[2];

    // now add the local frame offset
    xyz[0] += s->gps_to_local.local[0];
    xyz[1] += s->gps_to_local.local[1];
    xyz[2] += s->gps_to_local.local[2];
    
    if (cov) {
        fprintf (stderr, "TODO: gps_to_local covariance computation\n");
    }

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_local_to_gps (CTrans *s, 
        const double xyz[3], double gps[3], double cov[3][3])
{
    if (!ctrans_have_gps_to_local (s))
        return -1;

    g_static_rec_mutex_lock (&s->mutex);
    double xyz2[3];
    xyz2[0] = xyz[0] - s->gps_to_local.local[0];
    xyz2[1] = xyz[1] - s->gps_to_local.local[1];
    xyz2[2] = xyz[2] - s->gps_to_local.local[2];

    double theta = s->gps_to_local.lat_lon_el_theta[3];
    double sine, cosine;
    sincos (theta, &sine, &cosine);

    double p_g[2];
    p_g[0] =  xyz2[0] * cosine + xyz2[1] * sine;
    p_g[1] = -xyz2[0] * sine   + xyz2[1] * cosine;

    gps_linearize_to_lat_lon (&s->gps_linearize, p_g, gps);
    gps[2] = xyz2[2];

    if (cov) {
        fprintf (stderr, "TODO: local_to_gps covariance computation\n");
    }

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int 
ctrans_have_pose (CTrans * s)
{
    g_static_rec_mutex_lock (&s->mutex);
    int result = !gu_circular_is_empty (s->pose_history);
    g_static_rec_mutex_unlock (&s->mutex);
    return result;
}

int
ctrans_gps_pose (CTrans * s, double lat_lon_el[3], double q[4])
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history) || !s->gps_to_local.utime) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    lcmtypes_pose_t * pose = gu_circular_peek_nth (s->pose_history, 0);
    if (lat_lon_el) {
        ctrans_local_to_gps (s, pose->pos, lat_lon_el, NULL);
    }

    if (q) {
        double theta = s->gps_to_local.lat_lon_el_theta[3];
        double rot[4] = { cos (-theta/2), 0, 0, sin (-theta/2) };
        rot_quat_mult (q, rot, pose->orientation);
    }

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_local_pos (CTrans * s, double p_local[3])
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    lcmtypes_pose_t * pose = gu_circular_peek_nth (s->pose_history, 0);
    p_local[0] = pose->pos[0];
    p_local[1] = pose->pos[1];
    p_local[2] = pose->pos[2];

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_local_pos_heading (CTrans * s, double p_local[3], double *heading)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    lcmtypes_pose_t * pose = gu_circular_peek_nth (s->pose_history, 0);
    p_local[0] = pose->pos[0];
    p_local[1] = pose->pos[1];
    p_local[2] = pose->pos[2];

    double rpy[3];
    rot_quat_to_roll_pitch_yaw(pose->orientation, rpy);
    *heading = rpy[2];

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_local_pose (CTrans * s, lcmtypes_pose_t * pose)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    lcmtypes_pose_t * p = gu_circular_peek_nth (s->pose_history, 0);
    memcpy (pose, p, sizeof (lcmtypes_pose_t));

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

static int ctrans_older_than_complain_count = 0;
static int64_t ctrans_older_than_complain_utime;

int
ctrans_local_pose_at (CTrans * s, lcmtypes_pose_t * pose, uint64_t t)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    int i = 0;
    lcmtypes_pose_t * p = NULL;
    while (i < s->pose_history->len) {
        p = gu_circular_peek_nth (s->pose_history, i);
        if (p->utime <= t)
            break;
        i++;
    }
    if (i == s->pose_history->len) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }
    if (t - p->utime > 50000ULL) {
        int64_t now = timestamp_now();
        double dt = (now - ctrans_older_than_complain_utime)/1000000.0;
        ctrans_older_than_complain_count++;
        if (dt > 1.0) {
            fprintf (stderr, "Warning: closest pose is %.3f s older than "
                     "requested [i=%d, count = %d]\n", (t - p->utime) / 1000000.0, i,
                     ctrans_older_than_complain_count);
            ctrans_older_than_complain_count = 0;
            ctrans_older_than_complain_utime = now;
        }
    }
    memcpy (pose, p, sizeof (lcmtypes_pose_t));

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_body_to_local_matrix (CTrans * s, double m[16])
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    lcmtypes_pose_t * p = gu_circular_peek_nth (s->pose_history, 0);
    rot_quat_pos_to_matrix(p->orientation, p->pos, m);

    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_body_to_local_matrix_with_pose (CTrans * s, double m[16], 
        const lcmtypes_pose_t *pose)
{
    rot_quat_pos_to_matrix(pose->orientation, pose->pos, m);
    return 0;
}

int
ctrans_calibration_to_local_matrix (CTrans * s, double m[16])
{
    g_static_rec_mutex_lock (&s->mutex);
    double body_to_local[16], calibration_to_body[16];
    if (0 != ctrans_body_to_local_matrix (s, body_to_local)) return -1;
    rot_quat_pos_to_matrix (s->calibration_to_body_quat,
                            s->calibration_to_body_translation, 
                            calibration_to_body);
    matrix_multiply_4x4_4x4 (body_to_local, calibration_to_body, m);
    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int 
ctrans_calibration_to_local_matrix_with_pose (CTrans *s, double m[16],
        const lcmtypes_pose_t *pose)
{
    g_static_rec_mutex_lock (&s->mutex);
    double body_to_local[16], calibration_to_body[16];
    if (0 != ctrans_body_to_local_matrix_with_pose (s, body_to_local, pose))
        return -1;
    rot_quat_pos_to_matrix (s->calibration_to_body_quat,
                            s->calibration_to_body_translation, 
                            calibration_to_body);
    matrix_multiply_4x4_4x4 (body_to_local, calibration_to_body, m);
    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

#if 0
int
ctrans_calibration_to_body_dir (CTrans * s,
        const double * p_calibration, double * p_body)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (!s->have_calibration_to_body) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    memcpy (p_body, p_calibration, 3 * sizeof (double));
    rot_quat_rotate (s->calibration_to_body_quat, p_body);
    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}

int
ctrans_calibration_to_body (CTrans * s,
        const double * p_calibration, double * p_body)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (!s->have_calibration_to_body) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }

    memcpy (p_body, p_calibration, 3 * sizeof (double));
    rot_quat_rotate (s->calibration_to_body_quat, p_body);
    p_body[0] += s->calibration_to_body_translation[0];
    p_body[1] += s->calibration_to_body_translation[1];
    p_body[2] += s->calibration_to_body_translation[2];
    g_static_rec_mutex_unlock (&s->mutex);
    return 0;
}
#endif

void
ctrans_lock (CTrans * s)
{
    g_static_rec_mutex_lock (&s->mutex);
}

void
ctrans_unlock (CTrans * s)
{
    g_static_rec_mutex_unlock (&s->mutex);
}

/**
 * interpolate_pos:
 *
 * Computes an interpolated position at time t between indices i and i-1
 * in the circular buffer of historical poses.  t is expected to lie
 * between the utime's of i and i-1.
 *
 * Returns: 0 on success, -1 on failure.
 */
static int
interpolate_pos (CTrans * s, double pos[3], int i, int64_t t)
{
    lcmtypes_pose_t * p0 = gu_circular_peek_nth (s->pose_history, i);
    if (i == 0) {
        int64_t dt = t - p0->utime;
        if (dt < 0 || dt > MAX_INTERPOLATED_DT)
            return -1;
        double dtd = dt / 1000000.0;
        pos[0] = p0->pos[0] + p0->vel[0] * dtd;
        pos[1] = p0->pos[1] + p0->vel[1] * dtd;
        pos[2] = p0->pos[2] + p0->vel[2] * dtd;
        return 0;
    }

    lcmtypes_pose_t * p1 = gu_circular_peek_nth (s->pose_history, i-1);
    int64_t dt = p1->utime - p0->utime;
    if (dt < 0 || dt > MAX_INTERPOLATED_DT)
        return -1;
    if (t > p1->utime || t < p0->utime)
        return -1;

    double scale = (t - p0->utime) / (double) dt;
    pos[0] = p0->pos[0] + (p1->pos[0] - p0->pos[0]) * scale;
    pos[1] = p0->pos[1] + (p1->pos[1] - p0->pos[1]) * scale;
    pos[2] = p0->pos[2] + (p1->pos[2] - p0->pos[2]) * scale;
    return 0;
}

static int
travel_dist_real (CTrans * s, double * dist_out, int64_t t0, int64_t t1)
{
    lcmtypes_pose_t * p;
    double lastpos[3];
    double dist = 0;
    int i = 0;
    while (i < s->pose_history->len) {
        p = gu_circular_peek_nth (s->pose_history, i);
        if (p->utime <= t1)
            break;
        i++;
    }
    if (i == s->pose_history->len)
        return -1;

    if (interpolate_pos (s, lastpos, i, t1) < 0)
        return -1;
    int64_t lastt = t1;

    while (i < s->pose_history->len) {
        p = gu_circular_peek_nth (s->pose_history, i);
        int64_t dt = lastt - p->utime;
        if (dt < 0 || dt > MAX_INTERPOLATED_DT)
            return -1;
        if (p->utime <= t0)
            break;
        dist += vector_dist_3d (lastpos, p->pos);
        lastpos[0] = p->pos[0];
        lastpos[1] = p->pos[1];
        lastpos[2] = p->pos[2];
        lastt = p->utime;
        i++;
    }
    if (i == s->pose_history->len)
        return -1;

    double finalpos[3];
    if (interpolate_pos (s, finalpos, i, t0) < 0)
        return -1;

    dist += vector_dist_3d (lastpos, finalpos);
    *dist_out = dist;
    return 0;
}

int
ctrans_travel_distance (CTrans * s, double * dist_out, int64_t t0, int64_t t1)
{
    g_static_rec_mutex_lock (&s->mutex);
    if (gu_circular_is_empty (s->pose_history)) {
        g_static_rec_mutex_unlock (&s->mutex);
        return -1;
    }
    if (t1 <= 0) {
        lcmtypes_pose_t * p = gu_circular_peek_nth (s->pose_history, 0);
        t1 = p->utime;
    }
    int retval = travel_dist_real (s, dist_out, t0, t1);
    if (retval < 0) {
        lcmtypes_pose_t * p = gu_circular_peek_nth (s->pose_history, 0);
        double dt = (double)(t1 - t0) / 1000000.0;
        *dist_out = vector_magnitude_3d (p->vel) * dt;
        fprintf (stderr, "ctrans Warning: Could not compute travel distance, "
                "estimating from last velocity\n");
        retval = 0;
    }
    g_static_rec_mutex_unlock (&s->mutex);
    return retval;
}
