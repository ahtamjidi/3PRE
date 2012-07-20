#ifndef __RNDF_OVERLAY_H__
#define __RNDF_OVERLAY_H__

#include <inttypes.h>
#include <common/rndf.h>
#include <glib.h>

typedef struct _RndfOverlay RndfOverlay;
typedef struct _RndfOverlaySegment RndfOverlaySegment;
typedef struct _RndfOverlayLane RndfOverlayLane;
typedef struct _RndfOverlayWaypoint RndfOverlayWaypoint;
typedef struct _RndfOverlayZone RndfOverlayZone;
typedef struct _RndfOverlaySpot RndfOverlaySpot;
typedef struct _RndfOverlayCheckpoint RndfOverlayCheckpoint;
typedef struct _RndfOverlayIntersection RndfOverlayIntersection;
typedef struct _RndfOverlayYield RndfOverlayYield;

struct _RndfOverlay {
    RndfRouteNetwork * rndf;
    int own_rndf_ptr;

    int num_segments;
    RndfOverlaySegment * segments;
    int num_zones;
    RndfOverlayZone * zones;
    int num_checkpoints;
    RndfOverlayCheckpoint * checkpoints;

    // data: RndfOverlayIntersection*
    GList * intersections;

    // key: int[3] (seg, lane, wp)
    // val: RndfOverlayWaypoint* 
    GHashTable * waypoint_hash;

    // key: int[2] (seg, lane)
    // val: RndfOverlayLane*
    GHashTable * lane_hash;

};

struct _RndfOverlaySegment {
    RndfOverlay * parent;
    int num_lanes;
    RndfOverlayLane * lanes;
    RndfSegment * segment;
};

typedef enum {
    RNDF_OVERLAY_RPOS_LEFT = 0,
    RNDF_OVERLAY_RPOS_RIGHT,
    RNDF_OVERLAY_RPOS_UNKNOWN
} RndfOverlayRelativePosition;

typedef enum {
    RNDF_OVERLAY_RDIR_SAME = 0,
    RNDF_OVERLAY_RDIR_OPPOSITE,
    RNDF_OVERLAY_RDIR_UNKNOWN
} RndfOverlayRelativeDirection;

typedef struct {
    RndfOverlayRelativePosition rpos;
    RndfOverlayRelativeDirection rdir;
} RndfOverlayLaneRelativePlacement;

struct _RndfOverlayLane {
    RndfOverlaySegment * parent;
    int num_waypoints;
    RndfOverlayWaypoint * waypoints;
    RndfLane * lane;

    /* Neighboring lanes in the same direction that can be used for
     * lane changes. */
    RndfOverlayLane * change_left;
    RndfOverlayLane * change_right;

    /* Adjacent lanes not necessarily in the same direction  */
    RndfOverlayLane * adjacent_left;
    RndfOverlayLane * adjacent_right;
    RndfOverlayRelativeDirection adjacent_left_rdir;
    RndfOverlayRelativeDirection adjacent_right_rdir;

    // key: RndfOverlayLane* 
    // val: RndfOverlayLaneRelativePlacement*.  The relationship between key
    //      and self is that key is val of self
    GHashTable * relative_positions;
};

typedef union {
    RndfOverlayLane * lane;
    RndfOverlayZone * zone;
    RndfOverlaySpot * spot;
} RndfOverlayParent;

struct _RndfOverlayYield {
    RndfOverlayWaypoint * w;
    RndfOverlayWaypoint * w_next;
    double d;
};

struct _RndfOverlayWaypoint {
    RndfPointType type;
    RndfOverlayParent parent;
    int32_t id[3];
    int is_stop;

    int num_entrances;
    RndfOverlayWaypoint ** entrances;
    int num_exits;
    RndfOverlayWaypoint ** exits;

    // For each exit, a list of oncoming lanes that should be yielded to.
    // data: RndfOverlayYield *
    GList ** exit_yields;
    // When traveling to the next waypoint, a list of any oncoming lanes that
    // should be yielded to.
    // data: RndfOverlayYield *
    GList * next_yields;

    RndfOverlayWaypoint * next;
    RndfOverlayWaypoint * prev;

    /* Next waypoint to use when changing lanes left or right */
    RndfOverlayWaypoint * change_left;
    RndfOverlayWaypoint * change_right;

    RndfOverlayIntersection * intersection;

    RndfWaypoint * waypoint;
    
    int expect_left_boundary_next;
    int expect_right_boundary_next;
    int expect_left_boundary_prev;
    int expect_right_boundary_prev;

    char id_str[20];
};

struct _RndfOverlayZone {
    RndfOverlay * parent;
    int num_spots;
    RndfOverlaySpot * spots;
    int num_peripoints;
    RndfOverlayWaypoint * peripoints;
    RndfZone * zone;
};

struct _RndfOverlaySpot {
    RndfOverlayZone * parent;
    RndfOverlayWaypoint waypoints[2];
    RndfSpot * spot;
};

struct _RndfOverlayCheckpoint {
    RndfOverlayWaypoint * waypoint;
    RndfCheckpoint * checkpoint;
};

struct _RndfOverlayIntersection {
    // data: RndfOverlayWaypoint
    GList * waypoints;
};

RndfOverlay * rndf_overlay_new (RndfRouteNetwork * rndf);
RndfOverlay * rndf_overlay_new_from_file (const char * file);
void rndf_overlay_free (RndfOverlay * r);

RndfOverlayWaypoint * rndf_overlay_find_waypoint_by_id (RndfOverlay * r,
        const int32_t id[3]);

RndfOverlayLane * rndf_overlay_find_lane_by_id (RndfOverlay *r, const int32_t id[2]);

// not implemented??
RndfOverlaySegment * rndf_overlay_segment_by_id(RndfOverlay *r, const int32_t id[2]);

#endif
