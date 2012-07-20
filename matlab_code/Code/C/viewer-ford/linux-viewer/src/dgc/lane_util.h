#ifndef __LANE_UTIL_H__
#define __LANE_UTIL_H__

#include <lcm/lcm.h>
#include <common/rndf_overlay.h>
#include <common/geometry.h>
#include <dgc/ctrans.h>
#include <dgc/lcgl.h>

#include <lcmtypes/lcmtypes_lane_list_t.h>
#include <lcmtypes/lcmtypes_navigator_status_t.h>
#include <lcmtypes/lcmtypes_curb_polylines_t.h>

struct _LaneUtil {
    /* The most recent list of lanes received from the lane tracker. */
    lcmtypes_lane_list_t * lanes;

    lcmtypes_navigator_status_t *nav_status;
    lcmtypes_curb_polylines_t  *curb_lines;

    /* This mutex must be held when reading from lanes, above */
    GStaticRecMutex lanes_mutex;

    /* Private */
    int  do_fixup;
    lcm_t * lc;
    RndfOverlay * rndf;
    CTrans * ctrans;

    lcgl_t *lcgl;
    lcmtypes_lane_list_t_subscription_t * lane_sub;
};
typedef struct _LaneUtil LaneUtil;

/**
 * lane_util_new:
 *
 * Creates a new lane_util class.  By default, the class will subscribe
 * to message updates from the lane tracker so that it will always have
 * access to the latest lane information.  If @lc is NULL, LaneUtil will
 * _not_ subscribe to lane updates.  In this case, you must manually
 * provide them via the function lane_util_set_lane_list().
 *
 * Returns: A LaneUtil pointer, or NULL on failure.
 */
LaneUtil *
lane_util_new (lcm_t * lc, RndfOverlay * rndf, CTrans * c);

LaneUtil *
lane_util_new_ex (lcm_t * lc, RndfOverlay * rndf, CTrans * ctrans, int do_fixup);

/**
 * lane_util_free:
 *
 * Destroys a lane_util class and any associated state.
 */
void
lane_util_free (LaneUtil * lu);

/**
 * lane_util_set_lane_list:
 *
 * Use this function only when LaneUtil was created without an lc handle.
 * This supplies a new set of lanes to be used internally for computation.
 * A deep copy is made of the lane list before returning.
 */
void
lane_util_set_lane_list (LaneUtil * lu, const lcmtypes_lane_list_t * lanes);


/**
 * lane_util_find_lane_by_id:
 *
 * Given a segment id and lane id find the lane structure that matches.
 * If you intend to use this function in a multithreaded app, you should
 * acquire the lanes_mutex before calling this function and hold it until
 * you are done using the result of this function.
 * NB: multiple lanes can be returned.
 *
 * Returns: The number of GList of lanes or NULL if none found.
 */
GList *
lane_util_find_lane_by_id (LaneUtil * lu, const int segid, const int laneid);

lcmtypes_lane_t *
lane_util_find_lane_by_id_and_pos (LaneUtil * lu, int segid, int laneid,
        const double pos[2]);

/**
 * lane_util_find_lane:
 *
 * Given a waypoint, finds the lane structure that contains that waypoint.
 * If you intend to use this function in a multithreaded app, you should
 * acquire the lanes_mutex before calling this function and hold it until
 * you are done using the result of this function.
 *
 * Returns: The lane, or NULL if not found.
 */
lcmtypes_lane_t *
lane_util_find_lane (LaneUtil * lu, RndfOverlayWaypoint * w);

/**
 * lane_util_get_waypoint_pos:
 *
 * Gets the position and orientation of an RNDF waypoint in the local frame.
 * If the waypoint was recently received from the lane tracker, that
 * information will be used.  Otherwise, the pure RNDF data is used.  @theta
 * refers to the nominal direction of travel when crossing a waypoint.  In
 * lanes or parking spaces, this is the direction along consecutive waypoints.
 * In zone perimeters, this is the direction of the entrance or exits.
 *
 * @w:  Waypoint to locate
 * @xy: Returned position of the waypoint in the local frame.
 * @theta: Returned orietation of the waypoint in the local frame.  This
 * may be NULL.
 *
 * Returns: 0 on success, -1 on error.
 */
int
lane_util_get_waypoint_pos (LaneUtil * lu,
        RndfOverlayWaypoint * w, double xy[2], double * theta);

/**
 * lane_util_get_waypoint_width:
 *
 * Returns the width of a lane/spot/zone waypoint in meters.  The width
 * is the left-to-right space that the car has available when crossing
 * that waypoint.
 */
int
lane_util_get_waypoint_width (LaneUtil * lu, RndfOverlayWaypoint * w,
        double * width);

/**
 * lane_util_get_dist_to_waypoint:
 *
 * Computes the Euclidean distance to a waypoint in the local frame.  This
 * is a convenience wrapper around lane_util_get_waypoint_pos(), and follows
 * the same rules.
 */
double
lane_util_get_dist_to_waypoint (LaneUtil * lu, RndfOverlayWaypoint * w,
        const double pos[2]);

/**
 * lane_util_get_zone_polygon:
 *
 * Returns a polygon in the local frame that represents the boundary of
 * an RNDF zone.  The boundary is transformed into the local frame using
 * GPS.  The resulting polygon must be freed with polygon2d_free().
 */
polygon2d_t *
lane_util_get_zone_polygon (LaneUtil * lu, RndfOverlayZone * z);

/**
 * lane_util_get_lane_polygon:
 *
 * Returns a polygon in the local frame that represents the boundary of
 * a single RNDF lane.  Note that only the portion of the lane received
 * by the lane tracker is computed.  The resulting polygon must be
 * freed with polygon2d_free().
 */
polygon2d_t *
lane_util_get_lane_polygon (LaneUtil * lu, const lcmtypes_lane_t * lane);

/**
 * lane_util_is_3pt_turn:
 *
 * Returns 1 if driving from waypoint @w1 to waypoint @w2 represents
 * a 3-point turn.  Returns 0 otherwise.
 */
int
lane_util_is_3pt_turn (LaneUtil * lu, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2);

/**
 * lane_util_must_stop:
 * 
 * Returns 1 if driving from waypoint @w1 to waypoint @w2 requires a
 * stop at @w1.  Reasons for stopping include a stop sign, yielding
 * to moving traffic, or a 3-point turn.  If @yield_ptr is non-NULL
 * a list of oncoming waypoints that must be yielded to will be stored
 * there.
 */
int
lane_util_must_stop (LaneUtil * lu, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2, GList ** yield_ptr);

/**
 * lane_util_get_intersection_polygon:
 *
 * Returns a polygon in the local frame that represents the boundary of
 * the intersection @i.  If @i is NULL, NULL is returned.  The resulting polygon
 * the intersection.  The resulting polygon
 * will be "dilated" in meters by the amount in @dilation.  A dilation of
 * 0 will cause the polygon to cross exactly over the waypoints that
 * participate in the intersection.  The resulting polygon must be
 * freed with polygon2d_free().
 */
polygon2d_t *
lane_util_get_intersection_polygon (LaneUtil * lu,
                                    RndfOverlayIntersection * intersection, double dilation);


/**
 * lane_util_is_in_zone:
 *
 * Returns 1 if @pos is inside zone @z, 0 otherwise.  GPS is used for
 * the global-to-local transformation.
 */
int
lane_util_is_in_zone (LaneUtil * lu, RndfOverlayZone * z,
        const double pos[2]);

/**
 * lane_util_is_in_lane:
 *
 * Returns 1 if @pos is inside lane @lane, 0 otherwise.  Only the portion
 * of the lane visible to the lane tracker is used for this computation.
 */
int
lane_util_is_in_lane (LaneUtil * lu, const lcmtypes_lane_t * lane,
        const double pos[2]);

/**
 * lane_util_dist_to_lane:
 *
 * Computes the distance from @pose to the center of @lane, along with
 * other optional quantities.  Only the portion of the lane visible to
 * the lane tracker is used for this computation.  @pose need not be
 * inside the lane.
 *
 * @waypoint_id:  If non-NULL, the next RNDF waypoint ID after @pose in
 * the direction of travel is returned.
 * @dist:  If non-NULL, the Euclidean distance between @pose and the
 * center of the lane is returned.
 * @theta:  If non-NULL, the difference in angle (radians) between
 * the orientation of @pose and the direction of travel in the lane
 * is returned.
 * @at_end:  If non-NULL, set to 1 if @pose is off the end of the lane
 * in the direction of travel.
 * @center_idx:  The index into lane->centerline of the point on the
 * lane center line closest to @pose.
 */
int
lane_util_dist_to_lane (LaneUtil * lu, const lcmtypes_lane_t * lane,
        const lcmtypes_pose_t * pose, int * waypoint_id, double * dist,
        double * theta, int * at_end, int * center_idx);

/**
 * lane_util_dist_to_zone:
 *
 * Computes the distance from @pose to the perimeter of @zone.
 */
int
lane_util_dist_to_zone (LaneUtil * lu, RndfOverlayZone * zone,
        const lcmtypes_pose_t * pose, double * dist);

/**
 * lane_util_find_closest_waypoint
 *
 * Find the closest waypoint to a given pose.
 */
RndfOverlayWaypoint *
lane_util_find_closest_waypoint (LaneUtil *lu, lcmtypes_pose_t *pose, int ignore_orientation);


/**
 * lane_util_get_speed_limits:
 *
 * Get the MDF-derived speed limits for a waypoint in m/s.
 */
int
lane_util_get_speed_limits (RndfOverlayWaypoint * w, float * min, float * max);


int lane_util_find_closest_lane(LaneUtil * lu, const lcmtypes_pose_t * pose, lcmtypes_lane_t ** lane, 
                                       double *dist, int *center_idx);



// used internally.
void lane_util_fixup(LaneUtil *lu);


#endif
