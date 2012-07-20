#ifndef MISSION_EDIT_H
#define MISSION_EDIT_H

#include "rndf.h"
#include "mdf.h"

int
route_check_consistency ( RndfRouteNetwork *route );

int
route_insert_waypoint ( RndfRouteNetwork *route, int sid, int lid, int wp1id, int wp2id, double lat, double lon );
int
route_remove_waypoint ( RndfRouteNetwork *route, int sid, int lid, int wpid );
int
route_move_waypoint( RndfRouteNetwork *route, int sid, int lid, int wpid, double lat, double lon );
int
route_remove_lane( RndfRouteNetwork *route, int sid, int lid );
int
route_remove_segment( RndfRouteNetwork *route, int sid );
int
route_insert_segment( RndfRouteNetwork *route, int n, double **gps, const char *name, int lane_width, \
                      RndfBoundaryType left_boundary, RndfBoundaryType right_boundary);
int
route_insert_lane ( RndfRouteNetwork *route, RndfSegment *s, int lane_id, RndfBoundaryType b_left, \
                    RndfBoundaryType b_right, int lane_width, int same_direction );

int
zone_insert_waypoint( RndfZone *zone, int wp1id, int wp2id, double lat, double lon);
int
zone_remove_waypoint ( RndfZone *zone, int wpid );

int
route_remove_spot( RndfRouteNetwork *route, RndfZone *zone, int spot_id ) ;
int
route_insert_spot( RndfRouteNetwork *route, RndfZone *zone, double *gps1, double *gps2, int spot_width );
int
route_insert_exit (RndfWaypoint *w1, RndfWaypoint *w2 );
int
route_remove_exit ( RndfWaypoint *w1, RndfWaypoint *w2 ) ;
int
route_add_rem_stop ( RndfWaypoint *w );
int
route_toggle_checkpoint( RndfRouteNetwork *route, RndfWaypoint *w );
int
same_waypoint (RndfWaypoint *w1, RndfWaypoint *w2 );

int
begin_in_lane( RndfWaypoint *w );
int
end_in_lane( RndfWaypoint *w );
RndfWaypoint*
next_in_lane( RndfWaypoint *w ); 
RndfWaypoint*
prev_in_lane( RndfWaypoint *w );

int
route_insert_zone( RndfRouteNetwork *route, int n, double **gps, const char *name );
int
route_remove_zone ( RndfRouteNetwork *route, int idx );

int
route_is_checkpoint ( RndfRouteNetwork *route, RndfWaypoint *w ) ;

int
mission_move_down_checkpoint( RndfMission *mission, int idx ) ;
int
mission_move_up_checkpoint( RndfMission *mission, int idx ) ;
int
mission_remove_checkpoint( RndfMission *mission, int idx );
int
mission_rndf_add_checkpoint( RndfMission *mission, int id );
void
mission_add_speed_limit( RndfMission *mission, int id, int min_speed, int max_speed );
int
get_ppid ( RndfWaypoint *w );
int
route_remove_obstacle( RndfRouteNetwork *route, int index );
int
route_add_obstacle( RndfRouteNetwork *route, RndfObstacle o);

int
route_remove_all_checkpoints ( RndfRouteNetwork *route );
int
route_add_all_checkpoints ( RndfRouteNetwork *route );
void
mission_set_all_speed_limits ( RndfMission *mission, int min_speed, int max_speed, int n );
void
mission_reset_speed_limits ( RndfMission *mission );
void
mission_set_speed_limit ( RndfMission *mission, int min_speed, int max_speed, int id );
void
mission_rm_segment_speed_limit ( RndfMission *mission, int id );
void
mission_insert_segment_speed_limit ( RndfMission *mission, int id );
void 
route_apply_gps_offset( RndfRouteNetwork *route, double lat, double lon);


#endif
