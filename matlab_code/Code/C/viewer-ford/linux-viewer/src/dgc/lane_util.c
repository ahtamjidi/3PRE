#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <common/small_linalg.h>
#include <common/rotations.h>
#include <common/convexhull.h>

#include "lane_util.h"

/* Make a local copy of lcmtypes_lane_t messages that we receive. */
static void
on_lanes (const lcm_recv_buf_t *rbuf, const char * channel, 
        const lcmtypes_lane_list_t * lanes, void * user)
{
    LaneUtil * lu = user;
    lane_util_set_lane_list (lu, lanes);
}

static void
on_navigator_status (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_navigator_status_t *nav_status, void *user)
{
    LaneUtil *lu = user;
    g_static_rec_mutex_lock (&lu->lanes_mutex);
    if (lu->nav_status)
        lcmtypes_navigator_status_t_destroy(lu->nav_status);
    lu->nav_status = lcmtypes_navigator_status_t_copy(nav_status);
    g_static_rec_mutex_unlock (&lu->lanes_mutex);
}

static void
on_curb_polylines (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_curb_polylines_t *curb_lines, void *user)
{
    LaneUtil *lu = user;

    g_static_rec_mutex_lock (&lu->lanes_mutex);
    if (lu->curb_lines)
        lcmtypes_curb_polylines_t_destroy(lu->curb_lines);
    lu->curb_lines = lcmtypes_curb_polylines_t_copy(curb_lines);
    g_static_rec_mutex_unlock (&lu->lanes_mutex);
}

LaneUtil *
lane_util_new (lcm_t * lc, RndfOverlay * rndf, CTrans * ctrans)
{
    return lane_util_new_ex(lc, rndf, ctrans, 0);
}

LaneUtil *
lane_util_new_ex (lcm_t * lc, RndfOverlay * rndf, CTrans * ctrans, int do_fixup)
{
    LaneUtil * lu = calloc (1, sizeof (LaneUtil));

    assert (rndf);

    lu->lc = lc;
    lu->rndf = rndf;
    lu->ctrans = ctrans;
    lu->do_fixup = do_fixup;
    g_static_rec_mutex_init (&lu->lanes_mutex);

    if (lc)
        lu->lane_sub = lcmtypes_lane_list_t_subscribe (lc, "LANES", on_lanes, lu);

    if (lc && lu->do_fixup) {
        lcmtypes_navigator_status_t_subscribe(lc, "NAVIGATOR_STATUS", on_navigator_status, lu);
        lcmtypes_curb_polylines_t_subscribe(lc, "CURBLINES", on_curb_polylines, lu);
    }

    return lu;
}

void
lane_util_free (LaneUtil * lu)
{
    g_static_rec_mutex_lock (&lu->lanes_mutex);
    if (lu->lanes)
        lcmtypes_lane_list_t_destroy (lu->lanes);
    if (lu->nav_status)
        lcmtypes_navigator_status_t_destroy(lu->nav_status);
    if (lu->lane_sub)
        lcmtypes_lane_list_t_unsubscribe (lu->lc, lu->lane_sub);
    g_static_rec_mutex_free (&lu->lanes_mutex);
    free (lu);
}

void
lane_util_set_lane_list (LaneUtil * lu, const lcmtypes_lane_list_t * lanes)
{
    g_static_rec_mutex_lock (&lu->lanes_mutex);
    if (lu->lanes)
        lcmtypes_lane_list_t_destroy (lu->lanes);
    lu->lanes = lcmtypes_lane_list_t_copy (lanes);

    if (lu->do_fixup)
        lane_util_fixup(lu);

    g_static_rec_mutex_unlock (&lu->lanes_mutex);
}


GList *
lane_util_find_lane_by_id (LaneUtil * lu, const int segid, const int laneid)
{

    if (!lu||!lu->lanes)
        return NULL;

    GList *matches=NULL;
    for (int i = 0; i < lu->lanes->nlanes; i++) {
        lcmtypes_lane_t * lane = lu->lanes->lanes + i;
        if (segid == lane->rndf_segment_id && laneid == lane->rndf_lane_id) {
            matches = g_list_append(matches, lane);
        }
    } 
    return matches;
}

lcmtypes_lane_t *
lane_util_find_lane_by_id_and_pos (LaneUtil * lu, int segid, int laneid,
        const double pos[2])
{
    if (!lu->lanes)
        return NULL;

    lcmtypes_lane_t * best_lane = NULL;
    double dist = 0;
    for (int i = 0; i < lu->lanes->nlanes; i++) {
        lcmtypes_lane_t * lane = lu->lanes->lanes + i;
        if (segid != lane->rndf_segment_id || laneid != lane->rndf_lane_id)
            continue;

        for (int j = 0; j < lane->ncenterpoints; j++) {
            double distvec[2] = {
                pos[0] - lane->centerline[j][0],
                pos[1] - lane->centerline[j][1],
            };
            if (best_lane &&
                    (fabs (distvec[0]) > dist || fabs (distvec[1]) > dist))
                continue;

            double d = vector_magnitude_2d (distvec);
            if (!best_lane || d < dist) {
                best_lane = lane;
                dist = d;
            }
        }
    }
    return best_lane;
}


lcmtypes_lane_t *
lane_util_find_lane (LaneUtil * lu, RndfOverlayWaypoint * w)
{
    if (!lu->lanes || w->type != RNDF_POINT_WAYPOINT)
        return NULL;

    int i;
    g_static_rec_mutex_lock (&lu->lanes_mutex);
    for (i = 0; i < lu->lanes->nlanes; i++) {
        lcmtypes_lane_t * lane = lu->lanes->lanes + i;
        if (w->id[0] != lane->rndf_segment_id ||
                w->id[1] != lane->rndf_lane_id)
            continue;

        if (w->id[2] > lane->prev_waypoint_id &&
                w->id[2] < lane->next_waypoint_id) {
            g_static_rec_mutex_unlock (&lu->lanes_mutex);
            return lane;
        }
    }
    g_static_rec_mutex_unlock (&lu->lanes_mutex);
    return NULL;
}


polygon2d_t *
lane_util_get_zone_polygon (LaneUtil * lu, RndfOverlayZone * z)
{
    pointlist2d_t * boundary = pointlist2d_new (z->num_peripoints);
    int i;
    for (i = 0; i < z->num_peripoints; i++) {
        double xy[2];
        lane_util_get_waypoint_pos (lu, z->peripoints + i, xy, NULL);
        boundary->points[i].x = xy[0];
        boundary->points[i].y = xy[1];
    }
    polygon2d_t * boundary_poly = polygon2d_new ();
    polygon2d_add_pointlist (boundary_poly, boundary);
    pointlist2d_free (boundary);
    return boundary_poly;
}

int
lane_util_is_in_zone (LaneUtil * lu, RndfOverlayZone * z,
        const double pos[2])
{
    polygon2d_t * boundary_poly = lane_util_get_zone_polygon (lu, z);
    int retval = geom_point_inside_polygon_2d (POINT2D (pos), boundary_poly);
    polygon2d_free (boundary_poly);

    return retval;
}

static void
extend_pair (point2d_t * p1, point2d_t * p2, double dist)
{
    double dp[2] = { p2->x - p1->x, p2->y - p1->y };
    double mag = vector_magnitude_2d (dp);
    if (mag < 1e-10)
        return;
    p2->x = p1->x + dp[0] * (mag + dist) / mag;
    p2->y = p1->y + dp[1] * (mag + dist) / mag;
}

#if 0
polygon2d_t *
lane_util_get_multi_lane_polygon (LaneUtil * lu, const lcmtypes_lane_t * lane1,
        const lcmtypes_lane_t * lane2)
{
    assert (lane1->rndf_segment_id == lane2->rndf_segment_id);
    return NULL;
}
#endif

polygon2d_t *
lane_util_get_lane_polygon (LaneUtil * lu, const lcmtypes_lane_t * lane)
{
    pointlist2d_t * boundary = pointlist2d_new (lane->left_boundary.npoints +
            lane->right_boundary.npoints);
    int j = 0;
    int i;

    /* Convert the two lane boundaries into a closed polygon */
    for (i = 0; i < lane->left_boundary.npoints; i++) {
        boundary->points[j].x = lane->left_boundary.points[i][0];
        boundary->points[j].y = lane->left_boundary.points[i][1];
        j++;
    }
    /* Extend the ends of the lane slightly */
    if (lane->left_boundary.npoints > 1) {
        extend_pair (boundary->points + 1, boundary->points, 2.5);
        extend_pair (boundary->points + j - 2, boundary->points + j - 1, 2.5);
    }
    int n = j;
    for (i = lane->right_boundary.npoints - 1; i >= 0; i--) {
        boundary->points[j].x = lane->right_boundary.points[i][0];
        boundary->points[j].y = lane->right_boundary.points[i][1];
        j++;
    }
    /* Extend the ends of the lane slightly */
    if (lane->right_boundary.npoints > 1) {
        extend_pair (boundary->points + n + 1, boundary->points + n, 2.5);
        extend_pair (boundary->points + j - 2, boundary->points + j - 1, 2.5);
    }
    polygon2d_t * boundary_poly = polygon2d_new ();
    polygon2d_add_pointlist (boundary_poly, boundary);
    pointlist2d_free (boundary);
    return boundary_poly;
}

int
lane_util_is_in_lane (LaneUtil * lu, const lcmtypes_lane_t * lane,
        const double pos[2])
{
    polygon2d_t * boundary_poly = lane_util_get_lane_polygon (lu, lane);

    int retval = geom_point_inside_polygon_2d (POINT2D (pos), boundary_poly);
    polygon2d_free (boundary_poly);
    return retval;
}

int
lane_util_dist_to_zone (LaneUtil * lu, RndfOverlayZone * zone,
        const lcmtypes_pose_t * pose, double * dist)
{
    assert (zone->num_peripoints);

    *dist = INFINITY;
    double xy2[2];
    lane_util_get_waypoint_pos (lu, zone->peripoints, xy2, NULL);
    for (int i = 1; i < zone->num_peripoints; i++) {
        double xy1[2];
        xy1[0] = xy2[0];
        xy1[1] = xy2[1];
        lane_util_get_waypoint_pos (lu, zone->peripoints + i, xy2, NULL);
        double d = geom_point_line_seg_distance_2d (POINT2D (pose->pos),
                POINT2D (xy1), POINT2D (xy2));
        if (d < *dist)
            *dist = d;
    }
    return 0;
}

int
lane_util_dist_to_lane (LaneUtil * lu, const lcmtypes_lane_t * lane,
        const lcmtypes_pose_t * pose, int * waypoint_id, double * dist,
        double * theta, int * at_end, int * center_idx)
{
    assert (lane->ncenterpoints > 1);

    point2d_t pos = { .x = pose->pos[0], .y = pose->pos[1] };
    double distbest = INFINITY;
    int distidx = 0;
    int i;
    double u = 0, bestu = 0;
    /* Consider each line segment that makes up the center line and
     * find the point we are closest to. */
    for (i = 0; i < lane->ncenterpoints-1; i++) {
        double * p1 = lane->centerline[i];
        double * p2 = lane->centerline[i+1];
        double v[2];
        vector_sub_nd (p2, p1, 2, v);
        double magsq = vector_magnitude_squared_2d (v);
        u = 1;
        if (magsq > 1e-9) {
            u = ((pos.x - p1[0]) * v[0] + (pos.y - p1[1]) * v[1]) / magsq;
            u = CLAMP (u, 0, 1);
        }
        double p[2] = { p1[0] + u * v[0], p1[1] + u * v[1] };
        double dist = vector_dist_2d (p, pose->pos);
        if (dist <= distbest) {
            distbest = dist;
            distidx = i;
            bestu = u;
        }
    }

    if (at_end) {
        if (distidx == lane->ncenterpoints-2 && u > 0.9999)
            *at_end = 1;
        else
            *at_end = 0;
    }

    if (theta) {
        double * p1 = lane->centerline[distidx];
        double * p2 = lane->centerline[distidx+1];
        double v[3] = { 0, 0, 0 };
        vector_sub_nd (p2, p1, 2, v);
        rot_quat_rotate_rev (pose->orientation, v);
        *theta = atan2 (v[1], v[0]);
    }

    if (dist)
        *dist = distbest;

    if (bestu == 0)
        distidx -= 1;

    if (center_idx)
        *center_idx = distidx + 1;

    if (waypoint_id) {
        for (i = 0; i < lane->nwaypoints; i++) {
            if (lane->waypoint_indices[i] > distidx) {
                *waypoint_id = lane->prev_waypoint_id + i + 1;
                return 0;
            }
        }
        *waypoint_id = lane->next_waypoint_id;
        int idv[3] = { lane->rndf_segment_id, lane->rndf_lane_id,
            *waypoint_id };
        if (!rndf_overlay_find_waypoint_by_id (lu->rndf, idv))
            *waypoint_id = lane->prev_waypoint_id + lane->nwaypoints;
    }
    return 0;
}

static void
add_unit_direction_vector (double v[2], const double xy1[2],
        const double xy2[2], double scale)
{
    double xy[2];
    vector_sub_nd (xy2, xy1, 2, xy);
    vector_normalize_2d (xy);
    v[0] += scale * xy[0];
    v[1] += scale * xy[1];
}

static void
add_rndf_unit_direction_vector (CTrans * c, double v[2], const double xy[2],
        RndfOverlayWaypoint * w, double scale)
{
    double gps[3] = { w->waypoint->lat, w->waypoint->lon, 0 };
    double xyz[3];
    ctrans_gps_to_local (c, gps, xyz, NULL);
    add_unit_direction_vector (v, xy, xyz, scale);
}

int
lane_util_get_waypoint_width (LaneUtil * lu, RndfOverlayWaypoint * w,
        double * width)
{
    *width = 15; // feet

    if (w->type == RNDF_POINT_WAYPOINT) {
        RndfLane * lane = w->parent.lane->lane;
        *width = lane->lane_width; // feet
    }
    else if (w->type == RNDF_POINT_SPOT) {
        RndfSpot * spot = w->parent.spot->spot;
        *width = spot->spot_width; // feet
    }

    /* Width will be negative when the RNDF doesn't specify it */
    if (*width <= 0)
        *width = 12;

    *width *= 0.3048;  // convert to meters
    return 0;
}

int
lane_util_get_waypoint_pos (LaneUtil * lu,
        RndfOverlayWaypoint * w, double xy[2], double * theta)
{
    double v[2] = { 0, 0 };
    /* First check the lane list for this waypoint */
    g_static_rec_mutex_lock (&lu->lanes_mutex);
    lcmtypes_lane_t * lane = lane_util_find_lane (lu, w);
    if (lane && lane->ncenterpoints > 1) {
        int wpidx = w->id[2] - lane->prev_waypoint_id - 1;
        assert (wpidx < lane->nwaypoints);

        int idx = lane->waypoint_indices[wpidx];
        assert (idx < lane->ncenterpoints);

        xy[0] = lane->centerline[idx][0];
        xy[1] = lane->centerline[idx][1];

        /* Compute the orientation */
        if (theta) {
            if (idx > 0) {
                int j = 1;
                while (vector_dist_2d (xy, lane->centerline[idx-j]) < 2 &&
                        idx - j > 0)
                    j++;
                add_unit_direction_vector (v, xy, lane->centerline[idx-j], -1);
            }
            if (idx < lane->ncenterpoints - 1) {
                int j = 1;
                while (vector_dist_2d (xy, lane->centerline[idx+j]) < 2 &&
                        idx + j < lane->ncenterpoints - 1)
                    j++;
                add_unit_direction_vector (v, xy, lane->centerline[idx+j], 1);
            }
            *theta = atan2 (v[1], v[0]);
        }
        g_static_rec_mutex_unlock (&lu->lanes_mutex);
        return 0;
    }
    g_static_rec_mutex_unlock (&lu->lanes_mutex);

    /* Fall back to RNDF */
    double gps[3] = { w->waypoint->lat, w->waypoint->lon, 0 };
    double xyz[3];
    if (ctrans_gps_to_local (lu->ctrans, gps, xyz, NULL) < 0)
        return -1;
    xy[0] = xyz[0];
    xy[1] = xyz[1];

    /* Compute the orientation */
    if (theta) {
        if (w->type == RNDF_POINT_PERIMETER) {
            add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                    w->next, 1);
            add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                    w->prev, -1);
            double tmp = v[1];
            v[1] = v[0];
            v[0] = -tmp;
            double xy2[2] = { xy[0] + v[0], xy[1] + v[1] };
            int is_in = lane_util_is_in_zone (lu, w->parent.zone, xy2);
            if ((w->num_exits && is_in) || (w->num_entrances && !is_in)) {
                v[0] = -v[0];
                v[1] = -v[1];
            }
        }
        else if (!w->prev && !w->next) {
            /* If this is a single waypoint, we have to use the exits
             * and entrances for orientation. */
            assert (w->num_exits || w->num_entrances);
            if (w->num_exits)
                add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                        w->exits[0], 1);
            if (w->num_entrances)
                add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                        w->entrances[0], -1);
        }
        else {
            /* For a regular lane waypoint we use the neighboring waypoints. */
            if (w->next)
                add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                        w->next, 1);
            if (w->prev)
                add_rndf_unit_direction_vector (lu->ctrans, v, xy,
                        w->prev, -1);
        }

        *theta = atan2 (v[1], v[0]);
    }
    return 0;
}

double
lane_util_get_dist_to_waypoint (LaneUtil * lu, RndfOverlayWaypoint * w,
        const double pos[2])
{
    double xy[2];
    lane_util_get_waypoint_pos (lu, w, xy, NULL);
    return vector_dist_2d (xy, pos);
}

int
lane_util_is_3pt_turn (LaneUtil * lu, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2)
{
    if (!w1 || !w2)
        return 0;
    if (w1->type != RNDF_POINT_WAYPOINT || w2->type != RNDF_POINT_WAYPOINT)
        return 0;

    RndfOverlayLane * lane1 = w1->parent.lane;
    RndfOverlayLane * lane2 = w2->parent.lane;
    if (lane1->parent != lane2->parent)
        return 0;

    RndfOverlayLaneRelativePlacement * rp =
        g_hash_table_lookup (lane1->relative_positions, lane2);
    if (rp && rp->rdir == RNDF_OVERLAY_RDIR_OPPOSITE)
        return 1;

    return 0;
}

int
lane_util_must_stop (LaneUtil * lu, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2, GList ** yield_ptr)
{
    GList * yields = NULL;
    for (int i = 0; i < w1->num_exits; i++) {
        if (w1->exits[i] == w2) {
            yields = w1->exit_yields[i];
            break;
        }
    }
    if (w1->next == w2)
        yields = w1->next_yields;

    if (w1->is_stop || yields || lane_util_is_3pt_turn (lu, w1, w2)) {
        if (yield_ptr)
            *yield_ptr = yields;
        return 1;
    }

    return 0;
}

polygon2d_t *
lane_util_get_intersection_polygon (LaneUtil * lu,
        RndfOverlayIntersection * intersection, double dilation)
{
    if (!intersection)
        return NULL;
    GList * waypoints = intersection->waypoints;

    int len = g_list_length (waypoints);
    point2d_t pts[4*len];
    double width_offset = dilation;

    int j = 0;
    for (GList * iter = waypoints; iter; iter = iter->next) {
        RndfOverlayWaypoint * wp = iter->data;
        double pos[2], theta, width;
        lane_util_get_waypoint_pos (lu, wp, pos, &theta);
        lane_util_get_waypoint_width (lu, wp, &width);
        double si, co;
        sincos (theta, &si, &co);
        double l = dilation;
        double w = width/2 + width_offset;
        int i, is_3pt = 0;
        for (i = 0; i < wp->num_exits; i++) {
            if (lane_util_is_3pt_turn (lu, wp, wp->exits[i])) {
                l += 8;
                w += 2;
                is_3pt = 1;
                break;
            }
        }
        for (i = 0; !is_3pt && i < wp->num_entrances; i++) {
            if (lane_util_is_3pt_turn (lu, wp, wp->entrances[i])) {
                l += 8;
                w += 2;
                break;
            }
        }
        point2d_t pt1 = { .x = pos[0] + l * co, .y = pos[1] + l * si };
        point2d_t pt2 = { .x = pos[0] - l * co, .y = pos[1] - l * si };
        pts[j].x = pt1.x - w * si;
        pts[j].y = pt1.y + w * co;
        j++;
        pts[j].x = pt1.x + w * si;
        pts[j].y = pt1.y - w * co;
        j++;
        pts[j].x = pt2.x - w * si;
        pts[j].y = pt2.y + w * co;
        j++;
        pts[j].x = pt2.x + w * si;
        pts[j].y = pt2.y - w * co;
        j++;
    }  
    pointlist2d_t * input = pointlist2d_new_from_array (pts, j);
    pointlist2d_t * output = convexhull_graham_scan_2d (input);
    pointlist2d_free (input);

    polygon2d_t * boundary_poly = polygon2d_new ();
    polygon2d_add_pointlist (boundary_poly, output);
    pointlist2d_free (output);
    return boundary_poly;
}

int
lane_util_get_speed_limits (RndfOverlayWaypoint * w, float * min, float * max)
{
    if (w->type == RNDF_POINT_WAYPOINT) {
        RndfSegment * segment = w->parent.lane->parent->segment;

        *min = segment->min_speed;
        *max = segment->max_speed;
    }
    else {
        RndfZone * zone;
        if (w->type == RNDF_POINT_PERIMETER)
            zone = w->parent.zone->zone;
        else
            zone = w->parent.spot->parent->zone;

        *min = zone->min_speed;
        *max = zone->max_speed;
    }

    if (*max == 0)
        *max = 30;

    /* Convert from mph to m/s */
    *min *= 0.447;
    *max *= 0.447;
    return 0;
}


RndfOverlayWaypoint *
lane_util_find_closest_waypoint (LaneUtil *lu, lcmtypes_pose_t *pose, int ignore_orientation)
{
    if (!lu->lanes) {
        printf ("Waiting for lane information...\n");
        return NULL;
    }

    RndfOverlayWaypoint * waypoint_best = NULL;
    double dist_best=HUGE;

    g_static_rec_mutex_lock (&lu->lanes_mutex);
    
    // search for closest waypoint in lanes
    /* If we are in a lane, return the next waypoint in that lane.  Note that to be "in a lane"
     * we have to be driving the correct direction in it. */
    for (int i = 0; i < lu->lanes->nlanes; i++) {
        lcmtypes_lane_t * lane = lu->lanes->lanes + i;
        if (lane_util_is_in_lane (lu, lane, pose->pos)) {
            int id;
            double theta;
            lane_util_dist_to_lane (lu, lane, pose, &id, NULL, &theta,
                                    NULL, NULL);
            /* If our heading is greater than 60 degrees different than
             * the lane heading, we don't count as being inside the lane. */
            if (!ignore_orientation && (theta > M_PI/3 || theta < -M_PI/3))
                continue;
            int idv[3] = { lane->rndf_segment_id, lane->rndf_lane_id, id };
            
            RndfOverlayWaypoint *wp = rndf_overlay_find_waypoint_by_id (lu->rndf, idv);
            double dist =lane_util_get_dist_to_waypoint (lu, wp, pose->pos);
            if (dist<dist_best) {
                dist_best=dist;
                waypoint_best=wp;
            }
        }
    }
    // search for closest waypoint in zones
    for (int i = 0; i < lu->rndf->num_zones; i++) {
        RndfOverlayZone * zone = lu->rndf->zones + i;
        if (lane_util_is_in_zone (lu, zone, pose->pos)) {
            waypoint_best = &zone->peripoints[0];
            RndfOverlayWaypoint *wp = &zone->peripoints[0];
            double dist =lane_util_get_dist_to_waypoint (lu, wp, pose->pos);
            if (dist<dist_best) {
                dist_best=dist;
                waypoint_best=wp;
            }
            //break;
        }
    }
    g_static_rec_mutex_unlock (&lu->lanes_mutex);
    return waypoint_best;
}

// returns 0 on success.
int lane_util_find_closest_lane(LaneUtil * lu, const lcmtypes_pose_t * pose, lcmtypes_lane_t ** lane, 
                                       double *dist, int *center_idx) 
{
    lcmtypes_lane_t * best_lane=NULL;
    double best_dist=INFINITY;
    int best_center_idx=0;
    
    for (int i = 0; (lu->lanes)&&(i < lu->lanes->nlanes); i++) {
        lcmtypes_lane_t * tmp_lane = lu->lanes->lanes + i;
        double tmp_dist;
        int tmp_center_idx;
        lane_util_dist_to_lane (lu, tmp_lane, pose, NULL, &tmp_dist, NULL, NULL, &tmp_center_idx);
        if (best_dist>tmp_dist) {
            best_lane=tmp_lane;
            best_dist=tmp_dist;
            best_center_idx=tmp_center_idx;
        }
    } 
    if (lane)
        *lane=best_lane;
    if (dist)
        *dist=best_dist;
    if (center_idx)
        *center_idx=best_center_idx;

    if (best_lane == NULL)
        return -1;
    return 0;
}
