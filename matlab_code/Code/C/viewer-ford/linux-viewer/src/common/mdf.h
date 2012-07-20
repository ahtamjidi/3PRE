#ifndef MISSION_PARSE_H
#define MISSION_PARSE_H

#include "rndf.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

RndfRouteNetwork* rndf_new ();
RndfRouteNetwork* rndf_new_from_file (const char *fname);
int rndf_load_file (FILE * f, RndfRouteNetwork *route);
void rndf_destroy (RndfRouteNetwork *route);

void rndf_printf (RndfRouteNetwork *route);
int rndf_add_checkpoint (RndfRouteNetwork * route, int id, 
        RndfWaypoint * waypoint);
int rndf_print (RndfRouteNetwork *route, char *filename);
void rndf_get_waypoint_id (RndfWaypoint * waypoint,
        int * segzone, int * lanespot, int * waypoint_id);
char * rndf_get_waypoint_str (RndfWaypoint * waypoint);
RndfSegment * rndf_find_segment_by_id (RndfRouteNetwork * route, int id);
RndfZone * rndf_find_zone_by_id (RndfRouteNetwork * route, int id);
RndfWaypoint * rndf_find_waypoint_by_id (RndfRouteNetwork * route, int id1, 
        int id2, int id3);
RndfCheckpoint * rndf_find_checkpoint_by_id (RndfRouteNetwork *route, 
        int id);
int rndf_validate (RndfRouteNetwork *route, char *log);
int rndf_reset_speed_limits ( RndfRouteNetwork *route );


RndfMission *mdf_new (void);
int mdf_load_file( FILE *f, RndfMission *mission);
int mdf_load_memory (uint8_t * data, int length, RndfMission * mission);
void mdf_destroy(RndfMission * mission);
int mdf_insert_checkpoint (RndfMission * mission, int id);

/**
 * mdf_link_to_rndf:
 *
 * "Links" an MDF to an existing RNDF.  This does two things:
 * 1. It fills in the mission->checkpoints[] array so that it points directly
 *     to the checkpoints stored inside route.
 * 2. It modifies the speed limits stored inside route to reflect the
 *     desired speed limits from the MDF.
 */
int mdf_link_to_rndf ( RndfMission *mission, RndfRouteNetwork *route );

void mdf_printf ( RndfMission *mission );
int mdf_print ( RndfMission *mission, char *filename );

int mdf_validate ( RndfRouteNetwork *route, RndfMission *mission, char *log );
    


#ifdef __cplusplus
};
#endif

#endif
