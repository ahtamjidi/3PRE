#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <lcm/lcm.h>
#include <lcmtypes/lcmtypes_lane_list_t.h>
#include <lcmtypes/lcmtypes_navigator_status_t.h>
#include <lcmtypes/lcmtypes_pose_t.h>

#include <common/rndf_overlay.h>
#include <common/geometry.h>
#include <common/small_linalg.h>
#include <common/rotations.h>
#include <common/convexhull.h>
#include <common/math_util.h>
#include <common/fasttrig.h>

#include <dgc/ctrans.h>
#include <dgc/lcgl.h>
#include <dgc/globals.h>

#include "lane_util.h"

// if the confidence of the closest control point is > 1, that means
// we're on a densified segment. We do not do any fixup on a dense
// segment, regardless of the ENABLES below.

// lookahead: these are the curb fingers that adjust the lane estimate
// ahead of the vehicle. The curb fingers have a length of up to
// LOOKAHEAD_DISTANCE
#define ENABLE_LOOKAHEAD 1
#define LOOKAHEAD_DISTANCE 15

// stretchy: interpolate the lane through the vehicle's current position
#define ENABLE_STRETCHY  1

// stretchy bias: bias the stretchy lane back towards the linear
// interpolation up to some constant distance (in meters).
#define ENABLE_STRETCHY_BIAS 1
#define STRETCHY_BIAS 1.25

static void linearly_interpolate(const double *xy0, const double *xy1, double *pts[2], int idx0, int idx1)
{
    if (idx0 == idx1)
        return;

    for (int i = idx0; i <= idx1; i++) {
        double alpha = ((double) (i - idx0)) / (idx1 - idx0);

        pts[i][0] = xy0[0]*(1-alpha) + xy1[0]*alpha;
        pts[i][1] = xy0[1]*(1-alpha) + xy1[1]*alpha;
    }
}

static void free_doubles(double *xy[2], int npoints)
{
    for (int i = 0; i < npoints; i++)
        free(xy[i]);
    free(xy);
}

static pointlist2d_t *doubles_to_pointlist(double *xy[2], int npoints)
{
    pointlist2d_t *list = pointlist2d_new(npoints);
    for (int i = 0; i < npoints; i++) {
        list->points[i].x = xy[i][0];
        list->points[i].y = xy[i][1];
    }
    return list;
}

static double** pointlist_to_doubles(pointlist2d_t *list)
{
    double **pts = malloc(list->npoints * sizeof(double*));
    for (int i = 0; i < list->npoints; i++) {
        pts[i] = malloc(2 * sizeof(double));
        pts[i][0] = list->points[i].x;
        pts[i][1] = list->points[i].y;
    }
    return pts;
}

// compute the closest intersection of a ray cast from xy0 with a polyline
double ray_cast_test(point2d_t *xy0, point2d_t *xy1, pointlist2d_t *points)
{
    double closest_dist = HUGE;

    for (int pidx = 0; pidx+1 < points->npoints; pidx++) {
        point2d_t *p0 = &points->points[pidx];
        point2d_t *p1 = &points->points[pidx+1];

        point2d_t res;
        if (geom_line_seg_line_seg_intersect_2d(xy0, xy1, p0, p1, &res)) {
            closest_dist = fmin(closest_dist, sqrt(sq(res.x - xy0->x) +
                                                   sq(res.y - xy0->y)));
        }
    }

    return closest_dist;
}

// return 0 on success
int compute_lookahead_point(LaneUtil *lu, double *goal_pt, double *lookahead_pt, double lookahead_distance)
{
    if (!lu->curb_lines)
        return -1;

    lcmtypes_pose_t pose;
    if (ctrans_local_pose(lu->ctrans, &pose))
        return -1;

    double pos[3], heading;
    if (ctrans_local_pos_heading(lu->ctrans, pos, &heading))
        return -1;

    lcmtypes_curb_polylines_t *curb_lines = lu->curb_lines; // save typing

    double age = (pose.utime - curb_lines->utime) / 1000000.0;

    if (0 && age > 0.5) // XXX curbs wasn't filling in utime until recently
        return -1;

    lcgl_t *lcgl = lu->lcgl;

    // convert curb lines into pointlists
    pointlist2d_t *pointlists[curb_lines->nlines];

    for (int lidx = 0; lidx < curb_lines->nlines; lidx++) {
        lcmtypes_pointlist2d_t *line = &curb_lines->lines[lidx];

        pointlist2d_t *points = pointlist2d_new(line->npoints);
        pointlists[lidx] = points;

        for (int pidx = 0; pidx < line->npoints; pidx++) {
            points->points[pidx].x = line->points[pidx].x;
            points->points[pidx].y = line->points[pidx].y;
        }
    }

    double span = to_radians(30);
    double theta0 = -span;
    double dtheta = to_radians(2.5);
    int nrays = 2*span / dtheta;
    double thetas[nrays];
    double ranges[nrays];
    double xys[nrays][2];
    double max_range = lookahead_distance;
        
    // for each theta, how far is it until we hit one of the curbs?
    double theta_to_goal = fasttrig_atan2(goal_pt[1] - pos[1], goal_pt[0] - pos[0]);

    for (int ridx = 0; ridx < nrays; ridx++) {
        double theta = theta_to_goal + theta0 + ridx * dtheta;
        thetas[ridx] = theta;

        double s, c;
        fasttrig_sincos(theta, &s, &c);
        double dist = max_range;
        
        point2d_t xy0 = {.x = pos[0], 
                         .y = pos[1]};

        point2d_t xy1 = {.x = pos[0] + c * max_range,
                         .y = pos[1] + s * max_range};

        for (int lidx = 0; lidx < curb_lines->nlines; lidx++) 
            dist = fmin(dist, ray_cast_test(&xy0, &xy1, pointlists[lidx]));

        ranges[ridx] = dist;
        xys[ridx][0] = pos[0] + c * dist;
        xys[ridx][1] = pos[1] + s * dist;
    }

    // free the pointlists
    for (int lidx = 0; lidx < curb_lines->nlines; lidx++) {
        pointlist2d_free(pointlists[lidx]);
        pointlists[lidx] = NULL;
    }
            
    // filter the rays: don't let tiny gaps confuse us
    if (1) {
        double filter_range = to_radians(5);
        int filter_span = filter_range / dtheta / 2.0 + 1;
        double filtered_ranges[nrays];
        memcpy(filtered_ranges, ranges, sizeof(ranges));
        
        for (int ridx = 0; ridx < nrays; ridx++) {
            int lo = imax(0, ridx - filter_span);
            int hi = imin(nrays-1, ridx + filter_span);
            for (int i = lo; i <= hi; i++)
                filtered_ranges[i] = fmin(filtered_ranges[i], ranges[ridx]);
        }
        memcpy(ranges, filtered_ranges, sizeof(ranges));
    }

    // find the best ray
    double best_range = 0;
    double best_dtheta = HUGE;
    double best_dist_to_goal_sq = HUGE;
    int    best_ridx = -1;

    // select the best ray:
    // A) of the rays that reach maximum range, the one that is closest to the goal
    // B) otherwise, the ray that has maximum range.
    for (int ridx = 0; ridx < nrays; ridx++) {
        best_range = fmax(best_range, ranges[ridx]);
        double dtheta = thetas[ridx] - heading;
        if (ranges[ridx] == max_range) {

            double dist_sq = sq(xys[ridx][0] - goal_pt[0]) + 
                sq(xys[ridx][1] - goal_pt[1]);

//            if (fabs(dtheta) < fabs(best_dtheta)) {
            if (dist_sq < best_dist_to_goal_sq) {
                best_dtheta = dtheta;
                best_range = ranges[ridx];
                best_ridx = ridx;
                best_dist_to_goal_sq = dist_sq;
            }
        } else if (ranges[ridx] > best_range) {
            best_range = ranges[ridx];
            best_dtheta = dtheta;
            best_ridx = ridx;
        }
    }
 
    if (best_ridx < 0)
        return -1; // shouldn't happen

    // write the result
    lookahead_pt[0] = xys[best_ridx][0];
    lookahead_pt[1] = xys[best_ridx][1];
 
    // draw the rays
    if (1) {
        lcglLineWidth(4);
        lcglPointSize(6);
        lcglColor3f(1, 0, 0);

        for (int lidx = 0; lidx < curb_lines->nlines; lidx++) {
            lcmtypes_pointlist2d_t *line = &curb_lines->lines[lidx];
            lcglBegin(GL_LINE_STRIP);
            for (int pidx = 0; pidx < line->npoints; pidx++) {
                lcglVertex3d(line->points[pidx].x, line->points[pidx].y, pos[2]);
            }
            lcglEnd();
        }
        
        lcglLineWidth(1);

        for (int ridx = 0; ridx < nrays; ridx++) {
            
            if (ridx == best_ridx)
                lcglColor3f(1, .5, .5);
            else
                lcglColor3f(.3, .3, 1);
            lcglBegin(GL_LINES);
            lcglVertex3d(pos[0], pos[1], pos[2]);
            lcglVertex3d(xys[ridx][0], xys[ridx][1], pos[2]);
            lcglEnd();
        }
        lcglColor3f(1, .5, .5);
        lcglPointSize(8);
        lcglBegin(GL_POINTS);
        lcglVertex3d(xys[best_ridx][0], xys[best_ridx][1], pos[2]);
        lcglEnd();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////

void lane_util_fixup(LaneUtil *lu)
{
    lcmtypes_pose_t pose;
    if (ctrans_local_pose(lu->ctrans, &pose))
        return;

    if (lu->lcgl == NULL) {
        lu->lcgl = lcgl_init(lu->lc, "LaneUtilFixup:E pt");
    }
    lcgl_t *lcgl = lu->lcgl;

    if (!ENABLE_STRETCHY)
        return;

    // save typing
    lcmtypes_lane_list_t *lanes = lu->lanes;
    lcmtypes_navigator_status_t *nav_status = lu->nav_status;

    if (!nav_status || nav_status->inside_first_intersection || nav_status->num_intersections==0)
        return;

    if (nav_status->need_kpoint_turn)
        return;

    ///////////////////////////////////////////////////////////////////////////
    // what lane are we in?
    lcmtypes_lane_t *closest_lane = NULL;
    int         closest_lane_idx = -1;
    double      closest_lane_dist = HUGE;
    
    int *cid = nav_status->intersections[0];
    for (int lidx = 0; lidx < lanes->nlanes; lidx++) {
        lcmtypes_lane_t *lane = &lanes->lanes[lidx];
        if (cid[0] == lane->rndf_segment_id && cid[1] == lane->rndf_lane_id) {
            double this_dist;
            int this_idx;
            lane_util_dist_to_lane(lu, lane, &pose, NULL, &this_dist, NULL, NULL, &this_idx);
            if (this_dist < closest_lane_dist) {
                closest_lane = lane;
                closest_lane_idx = this_idx;
                closest_lane_dist = this_dist;
            }
        }
    }

    if (!closest_lane)
        return;

    // if we're on a densified segment, do nothing.
/*    if (closest_lane->centerline_confidence[closest_lane_idx] > 1) {
        return;
    }
*/

// I used to do the below instead of the above, but the "closest" lane
// ignores direction, and the "target" lane of the navigator is more
// useful.

//    if (lane_util_find_closest_lane(lu, &pose, &closest_lane, &closest_lane_dist, &closest_lane_idx))
//        return;

    ///////////////////////////////////////////////////////////////////////////
    // find the previous and next waypoints on this segment that are confident.
    int previous_confident_idx = closest_lane_idx;
    double previous_confident_dist = 0;

    while (closest_lane->centerline_confidence[previous_confident_idx] < closest_lane->centerline_confidence_thresh && 
           previous_confident_idx > 0) {

        previous_confident_dist += sqrt(sq(closest_lane->centerline[previous_confident_idx][0] -
                                           closest_lane->centerline[previous_confident_idx-1][0]) +
                                        sq(closest_lane->centerline[previous_confident_idx][1] -
                                           closest_lane->centerline[previous_confident_idx-1][1]));

        previous_confident_idx--;
    }

    int next_confident_idx = closest_lane_idx;
    double next_confident_dist = 0;

    while (closest_lane->centerline_confidence[next_confident_idx] < closest_lane->centerline_confidence_thresh && 
           next_confident_idx + 1 < closest_lane->ncenterpoints) {

        next_confident_dist += sqrt(sq(closest_lane->centerline[next_confident_idx][0] -
                                       closest_lane->centerline[next_confident_idx+1][0]) +
                                    sq(closest_lane->centerline[next_confident_idx][1] -
                                       closest_lane->centerline[next_confident_idx+1][1]));
        
        next_confident_idx++;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // if the enclosing waypoints are very close together,
    // don't do anything. (Currently disabled.)

    if (fmin(previous_confident_dist, next_confident_dist) < 0)
        return;

    ///////////////////////////////////////////////////////////////////////////
    // linearly interpolate the centerlines such that the car's
    // current position is in the lane.
    
    double elastic_point[] = { pose.pos[0], pose.pos[1] };

    if (ENABLE_STRETCHY_BIAS) {
        // bias the elastic point towards the linear interpolation point
        double linear_bias = STRETCHY_BIAS;

        double vec[2] = { closest_lane->centerline[closest_lane_idx][0] - elastic_point[0],
                          closest_lane->centerline[closest_lane_idx][1] - elastic_point[1]};
        double dist = sqrt(sq(vec[0]) + sq(vec[1]));
        if (dist > linear_bias) {
            vec[0] *= linear_bias / dist;
            vec[1] *= linear_bias / dist;
        }

        elastic_point[0] += vec[0];
        elastic_point[1] += vec[1];
    }

    linearly_interpolate(closest_lane->centerline[previous_confident_idx],
                         elastic_point,
                         closest_lane->centerline,
                         previous_confident_idx,
                         closest_lane_idx);

    double lookahead_point[2];
    double lookahead_distance = sqrt(sq(closest_lane->centerline[closest_lane_idx][0] - closest_lane->centerline[next_confident_idx][0]) +
                                     sq(closest_lane->centerline[closest_lane_idx][1] - closest_lane->centerline[next_confident_idx][1])) / 2.0;
    lookahead_distance = fmin(lookahead_distance, 15);

    // use lookahead?
    if (ENABLE_LOOKAHEAD && (next_confident_idx - closest_lane_idx >= 2) &&
//        lookahead_distance > 5 &&
        !compute_lookahead_point(lu, closest_lane->centerline[next_confident_idx], 
                                 lookahead_point, lookahead_distance)) {

        // 2 meters per control point, ~10m lookahead = 5 control points
        int lookahead_idx = closest_lane_idx + LOOKAHEAD_DISTANCE / 2.0;
        if (lookahead_idx >= next_confident_idx)
            lookahead_idx = (closest_lane_idx + next_confident_idx) / 2;
        
        linearly_interpolate(elastic_point,
                             lookahead_point,
                             closest_lane->centerline,
                             closest_lane_idx,
                             lookahead_idx);

        linearly_interpolate(lookahead_point,
                             closest_lane->centerline[next_confident_idx],
                             closest_lane->centerline,
                             lookahead_idx,
                             next_confident_idx);
    } else {
        linearly_interpolate(elastic_point,
                             closest_lane->centerline[next_confident_idx],
                             closest_lane->centerline,
                             closest_lane_idx,
                             next_confident_idx);
    }

    ///////////////////////////////////////////////////////////////////////////
    // recompute all affected lane boundaries (everything on the same segment)
    for (int lidx = 0; lidx < lanes->nlanes; lidx++) {
        lcmtypes_lane_t *lane = &lanes->lanes[lidx];

        if (lane->rndf_segment_id == cid[0]) {
            pointlist2d_t *center = doubles_to_pointlist(lane->centerline, lane->ncenterpoints);

            pointlist2d_t *left_shifted = geom_polyline_shift_sideways_2d(center, lane->lane_width / 2);
            free_doubles(lane->left_boundary.points, lane->left_boundary.npoints);
            lane->left_boundary.points = pointlist_to_doubles(left_shifted);
            lane->left_boundary.npoints = left_shifted->npoints;
            pointlist2d_free(left_shifted);

            pointlist2d_t *right_shifted = geom_polyline_shift_sideways_2d(center, -lane->lane_width / 2);
            free_doubles(lane->right_boundary.points, lane->right_boundary.npoints);
            lane->right_boundary.points = pointlist_to_doubles(right_shifted);
            lane->right_boundary.npoints = right_shifted->npoints;
            pointlist2d_free(right_shifted);

            pointlist2d_free(center);
        }
    }

    lcgl_switch_buffer(lcgl);
}
