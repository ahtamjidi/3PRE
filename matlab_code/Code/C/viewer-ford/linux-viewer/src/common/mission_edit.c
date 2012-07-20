/**
 * mission_edit.c
 *
 * Code for editing a Route Network Definition (RNDF) and a Mission Definition
 * (MDF) and placing the result inside the structure defined by
 * route-definition.h.
 *
 * The structure are read/written using the mission_parse.c methods.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "mission_edit.h"

/* check the consistency of a route */
int
route_check_consistency ( RndfRouteNetwork *route ) 
{
    int i,j,k;

    printf("checking consistency...\n");

    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = route->segments + i;

        if ( s->id != i+1 ) {
            printf("Error: segment %d has ID %d.\n", i, s->id );
        }

        for (j=0;j<s->num_lanes;j++) {
            
            RndfLane *l = s->lanes + j;

            if ( l->id != j+1 ) {
                printf("Error: lane %d has ID %d.%d.\n", j, s->id, l->id );
            }

            for (k=0;k<l->num_waypoints;k++) {
                
                RndfWaypoint *w = l->waypoints + k;

                if ( w->id != k+1 ) {
                    printf("Error: waypoint %d has ID %d.%d.%d\n", k, s->id, l->id, w->id);
                }
            }
        }
    }

    printf("done.\n");

    return 0;
}


/* return 1 if two waypoints are the same (ids) */
int
same_waypoint (RndfWaypoint *w1, RndfWaypoint *w2 )
{
    if ( !w1 || !w2 )
        return 0;

    if ( w1->type != w2->type )
        return 0;
    
    if ( w1->id != w2->id )
        return 0;

    int w1_parent_parent_id = -1;
    int w2_parent_parent_id = -1;
    int w1_parent_id=-1;
    int w2_parent_id=-1;
    
    switch ( w1->type ) {
    case RNDF_POINT_WAYPOINT:
        w1_parent_id = w1->parent.lane->id;
        w1_parent_parent_id = w1->parent.lane->parent->id;
        break;
    case RNDF_POINT_PERIMETER:
        w1_parent_id = 0;
        w1_parent_parent_id = w1->parent.zone->id;
        break;
    case RNDF_POINT_SPOT:
        w1_parent_id = w1->parent.spot->id;
        w1_parent_parent_id = w1->parent.spot->parent->id;
        break;
    default:
        fprintf(stderr, "unknown waypoint type!\n");
    }
    switch ( w2->type ) {
    case RNDF_POINT_WAYPOINT:
        w2_parent_id = w2->parent.lane->id;
        w2_parent_parent_id = w2->parent.lane->parent->id;
        break;
    case RNDF_POINT_PERIMETER:
        w2_parent_id = 0;
        w2_parent_parent_id = w2->parent.zone->id;
        break;
    case RNDF_POINT_SPOT:
        w2_parent_id = w2->parent.spot->id;
        w2_parent_parent_id = w2->parent.spot->parent->id;
        break;
    default:
        fprintf(stderr, "unknown waypoint type!\n");
    }

    if ( w1_parent_parent_id == w2_parent_parent_id && w1_parent_id == w2_parent_id ) 
        return 1;

    return 0;
}

/* remove pointers to a given waypoint (exits, checkpoints) */
void
route_remove_pointers( RndfRouteNetwork *route, int segment_id, int lane_id, int wp_id  )
{
    if ( !route )
        return;

    //printf("removing pointers %d.%d.%d\n", segment_id, lane_id, wp_id);

    // remove exits in the lanes
    int i,j,k,m,p;
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = &route->segments[i];
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = &s->lanes[j];
            for (k=0;k<l->num_waypoints;k++) {
                RndfWaypoint *w = &l->waypoints[k];
                int done = 0;
                while ( done != 1 ) {
                    done = 1;
                    
                    for (m=0;m<w->num_exits;m++) {
                        RndfWaypoint *exit = w->exits[m];
                        
                        if ( exit->type == RNDF_POINT_WAYPOINT && ( exit->parent.lane->parent->id != segment_id || 
                                                            exit->parent.lane->id != lane_id || exit->id != wp_id ) )
                            continue;
                        if ( exit->type == RNDF_POINT_PERIMETER && ( exit->parent.zone->id != segment_id || 
                                                                lane_id != 0 || exit->id != wp_id ) )
                            continue;

                        if ( exit->type == RNDF_POINT_SPOT )
                            continue;
                        //printf("removing exit (%d exits)...\n", w->num_exits);
                        
                        // remove the waypoint from the list of exits
                        
                        RndfWaypoint **nexits = malloc((w->num_exits-1)*sizeof(RndfWaypoint*));
                        int counter=0;
                        for (p=0;p<w->num_exits;p++) {
                            if ( p != m ) {
                                nexits[counter] = w->exits[p];
                                counter++;
                            }
                        }
                        
                        free( w->exits );
                        w->exits = nexits;
                        w->num_exits--;
                        done = 0;
                        //printf("removed exit %d.%d.%d (%d exits) (%d,%d,%d,%d,%d)\n", segment_id, lane_id, exit->id, w->num_exits+1, i, j, k, m, p );
                        break;
                    }
                }
            }
        }
    }
    
    printf(" # zones: %d\n", route->num_zones);
    // removing exits in the zones
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = &route->zones[i];
        for (j=0;j<z->num_peripoints;j++) {
            RndfWaypoint *w = &z->peripoints[j];
            int done = 0;
            while ( done != 1 ) {
                done = 1;
                
                //printf("%d.%d.%d has %d exits\n", w->parent.zone->id, 0, w->id, w->num_exits);

                for (m=0;m<w->num_exits;m++) {
                    RndfWaypoint *exit = w->exits[m];
                   
                    if ( exit->type == RNDF_POINT_WAYPOINT && ( exit->parent.lane->parent->id != segment_id || \
                                                           exit->parent.lane->id != lane_id || exit->id != wp_id ) )
                        continue;
                    
                    if ( exit->type == RNDF_POINT_PERIMETER && ( exit->parent.zone->id != segment_id || 
                                                            lane_id != 0 || exit->id != wp_id ) )
                        continue;

                    if ( exit->type == RNDF_POINT_SPOT )
                        continue;
                    // remove the waypoint from the list of exits
                    
                    RndfWaypoint **nexits = malloc((w->num_exits-1)*sizeof(RndfWaypoint*));
                    int counter=0;
                    for (p=0;p<w->num_exits;p++) {
                        if ( p != m ) {
                            nexits[counter] = w->exits[p];
                            counter++;
                        }
                    }
                    
                    free( w->exits );
                    w->exits = nexits;
                    w->num_exits--;
                    done = 0;
                    //printf("removed exit %d.%d.%d (%d exits) (%d,%d,%d,%d,%d)\n", segment_id, lane_id, exit->id, w->num_exits+1, i, j, k, m, p );
                    break;
                }
            }
        }
    }


    printf("removing checkpoint pointers %d.%d.%d\n", segment_id, lane_id, wp_id);

    // remove the checkpoint pointers
    for (i=0;i<route->num_checkpoints;i++) {
        //printf("\t%d/%d\n",i,route->num_checkpoints);
        RndfWaypoint *w = route->checkpoints[i].waypoint;
        
        if ( ( w->type == RNDF_POINT_WAYPOINT && w->id == wp_id &&
               w->parent.lane->id == lane_id && w->parent.lane->parent->id == segment_id ) || 
             ( w->type == RNDF_POINT_SPOT && w->id == wp_id &&
               w->parent.spot->id == lane_id && w->parent.spot->parent->id == segment_id ) )
            
            {
            
            int c_id = i;

            //printf("removing checkpoint %d.%d.%d\n",w->parent.lane->parent->id, w->parent.lane->id, w->id);
            printf("removing checkpoint %d\n", i);

            // remove the checkpoint
            RndfCheckpoint * nc = malloc( (route->num_checkpoints-1)*sizeof(RndfCheckpoint));
            int counter = 0;
            for (p=0;p<route->num_checkpoints;p++) {
                RndfCheckpoint c = route->checkpoints[p];
                if ( p != c_id ) {
                    nc[counter] = c;
                    counter++;
                }
            }
            if ( counter != route->num_checkpoints-1 ) {
                fprintf(stderr, "error rewriting route checkpoints. wrote %d instead of %d checkpoints.\n",
                        counter, route->num_checkpoints-1 );
            }
            free(route->checkpoints);
            route->checkpoints = nc;
            route->num_checkpoints--;
            //printf("removed checkpoint %d.%d.%d\n", segment_id, lane_id, wp_id);
            break;
        }
    }              
//printf("done.\n");
}

/* update the exit and checkpoint pointers when data has been shifted */
void
route_update_pointers ( RndfRouteNetwork *route, int segment_id, int lane_id, int wp_id, RndfWaypoint *wn )
{
    //printf("update pointers: %d\n", wn->id);
    if ( !route )
        return;

    int i,j,k,m;

    // update exits in the lanes
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = &route->segments[i];
        
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = &s->lanes[j];
            
            for (k=0;k<l->num_waypoints;k++) {
                RndfWaypoint *w = &l->waypoints[k];
                                
                for (m=0;m<w->num_exits;m++) {
                    RndfWaypoint *exit = w->exits[m];
                     if ( exit->type == RNDF_POINT_WAYPOINT && ( exit->parent.lane->parent->id != segment_id || \
                                                            exit->parent.lane->id != lane_id || exit->id != wp_id ) )
                        continue;
                    
                    if ( exit->type == RNDF_POINT_PERIMETER && ( exit->parent.zone->id != segment_id || 
                                                            lane_id != 0 || exit->id != wp_id ) )
                        continue;
                    
                    if ( exit->type == RNDF_POINT_SPOT )
                        continue;
                    
                    //if ( exit->type == RNDF_POINT_WAYPOINT && exit->id == wp_id &&
                    //   exit->parent.lane->id == lane_id && exit->parent.lane->parent->id == segment_id ) {
                    
                    w->exits[m] = wn;
                    printf("exit: replaced pointer (%d) to %d.%d.%d\n", exit->type, segment_id, lane_id, wp_id);
                       
                }
            }
        }
    }

    // update exits in the zones
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = &route->zones[i];
        for (j=0;j<z->num_peripoints;j++) {
            RndfWaypoint *w = &z->peripoints[j];
            for (m=0;m<w->num_exits;m++) {
                RndfWaypoint *exit = w->exits[m];
                if ( exit->type == RNDF_POINT_WAYPOINT && ( exit->parent.lane->parent->id != segment_id || \
                                                       exit->parent.lane->id != lane_id || exit->id != wp_id ) )
                    continue;
                
                if ( exit->type == RNDF_POINT_PERIMETER && ( exit->parent.zone->id != segment_id || 
                                                        lane_id != 0 || exit->id != wp_id ) )
                    continue;
                
                if ( exit->type == RNDF_POINT_SPOT )
                    continue;
                
                //if ( exit->type == RNDF_POINT_WAYPOINT && exit->id == wp_id &&
                // exit->parent.lane->id == lane_id && exit->parent.lane->parent->id == segment_id ) {
                
                w->exits[m] = wn;
                printf("exit: replaced pointer to %d.%d.%d\n", segment_id, lane_id, wp_id);
            }
        }
    }

    // update route checkpoints
    for (i=0;i<route->num_checkpoints;i++) {
        RndfWaypoint *w = route->checkpoints[i].waypoint;
        if ( w->type == RNDF_POINT_WAYPOINT && w->id == wp_id &&
             w->parent.lane->id == lane_id && w->parent.lane->parent->id == segment_id ) {
            
            route->checkpoints[i].waypoint = wn;
            printf("checkpoint: replaced pointer %d.%d.%d\n", segment_id, lane_id, wp_id);
        }
        if ( w->type == RNDF_POINT_SPOT && w->id == wp_id &&
             w->parent.spot->id == lane_id && w->parent.spot->parent->id == segment_id ) {
            
            route->checkpoints[i].waypoint = wn;
            printf("checkpoint: replaced spot pointer %d.%d.%d\n", segment_id, lane_id, wp_id);
        }
    }
}

/* insert a waypoint in a zone given the pointer to the zone and the two neighbor ids */
int
zone_insert_waypoint( RndfZone *zone, int wp1id, int wp2id, double lat, double lon)
{
   if ( !zone )
        return -1;

    RndfRouteNetwork *route = zone->parent;

    if ( !route )
        return -1;
     
    int i;

    RndfWaypoint w;
    w.lat = lat;
    w.lon = lon;
    w.type = RNDF_POINT_PERIMETER;
    w.is_stop = 0;
    w.num_exits = 0;
    w.exits = NULL;
    w.parent.zone = zone;

    RndfWaypoint *nl = malloc( (zone->num_peripoints+1) * sizeof(RndfWaypoint));

    if ( wp2id == 0 ) {
        for (i=0;i<zone->num_peripoints;i++) 
            nl[i] = zone->peripoints[i];
        w.id = zone->num_peripoints+1;
        nl[zone->num_peripoints] = w;
    } else {
        for (i=zone->num_peripoints;i>=0;i--) {
            RndfWaypoint w2;
            
            if ( i < wp2id ) {
                w2 = zone->peripoints[i];
                route_update_pointers( route, zone->id, 0, w2.id, nl+i);
            } else if ( i == wp2id ) {
                w.id = i+1;
                w2 = w; 
            } else {
                w2 = zone->peripoints[i-1];
                route_update_pointers( route, zone->id, 0, w2.id, nl+i);
                w2.id = i+1;
            }
        
            nl[i] = w2;
        }
    }
    
    // free memory
    free( zone->peripoints );

    // set pointer to the new allocated space
    zone->peripoints = nl;
    
    zone->num_peripoints++;

    return 0;

 
    
}

/* insert a waypoint in the route given the segment id, lane id and two neighbor waypoint ids */
int
route_insert_waypoint ( RndfRouteNetwork *route, int sid, int lid, int wp1id, int wp2id, double lat, \
                        double lon)
{
    if ( !route ) 
        return -1;

    // first check consistency
    if ( sid >= route->num_segments ) {
        fprintf(stderr,"invalid segment position %d\n", sid);
        return -1;
    }

    RndfSegment *s = route->segments + sid;

    if ( lid >= s->num_lanes ) {
        fprintf(stderr, "invalid lane position %d (num_lanes = %d)\n", lid, s->num_lanes );
        return -1;
    }

    RndfLane *l = s->lanes + lid;

    if ( wp1id >= l->num_waypoints || wp2id >= l->num_waypoints ) {
        fprintf(stderr, "invalid waypoint positions %d %d\n", wp1id, wp2id);
        return -1;
    }

    // then realloc memory
    RndfWaypoint *nl = malloc( (l->num_waypoints+1) * sizeof(RndfWaypoint));

    RndfWaypoint w;
    w.id = wp2id+1;
    w.lat = lat;
    w.lon = lon;
    w.type = RNDF_POINT_WAYPOINT;
    w.is_stop = 0;
    w.num_exits = 0;
    w.exits = NULL;
    w.parent.lane = l;

    int i;
 
    for (i=l->num_waypoints;i>=0;i--) {
        RndfWaypoint w2;
        
        if ( i < wp2id ) {
            w2 = l->waypoints[i];
            route_update_pointers( route, s->id, l->id, w2.id, nl+i);
        } else if ( i == wp2id ) {
            w2 = w; 
        } else {
            w2 = l->waypoints[i-1];
            route_update_pointers( route, s->id, l->id, w2.id, nl+i);
            w2.id = i+1;
        }
        
        nl[i] = w2;
    }
    
    // free memory
    free( l->waypoints );

    // set pointer to the new allocated space
    l->waypoints = nl;
    
    l->num_waypoints++;

    return 0;

}

/* remove a waypoint in the zone given the segment id, lane id and two neighbor waypoint ids */
int
zone_remove_waypoint ( RndfZone *zone, int wpid )
{

    if ( !zone )
        return -1;

    RndfRouteNetwork *route = zone->parent;

    if ( !route )
        return -1;
     
    int i;

    if ( !route ) 
        return -1;

    int sid = zone->id;

    // first check consistency
    if ( sid - route->num_segments > route->num_zones ) {
        fprintf(stderr,"invalid zone position %d\n", sid);
        return -1;
    }

    // if the zone has only one waypoints left, don't do anything
    // we do not want to end up with zones having less than one waypoint
    // use remove_lane instead
    if ( zone->num_peripoints == 1 ) {
        printf("zone %d has only one waypoints. skipping.\n", zone->id);
        return 0;
    }

    if ( wpid >= zone->num_peripoints  ) {
        fprintf(stderr, "invalid waypoint position %d for zone %d\n", wpid, zone->id);
        return -1;
    }

    RndfWaypoint w = zone->peripoints[wpid];

   // remove the pointers to the removed waypoint
    printf("removing pointers to %d.0.%d\n", zone->id, w.id);
    route_remove_pointers( route, zone->id, 0, w.id) ;

    // realloc memory
    RndfWaypoint *nl = malloc( (zone->num_peripoints-1) * sizeof(RndfWaypoint));

    for (i=0;i<zone->num_peripoints;i++) {
           //for (i=l->num_waypoints-1;i>=0;i--) {
        RndfWaypoint w2;
        if ( i < wpid ) {
            w2 = zone->peripoints[i];
            nl[i] = w2;
            printf("[%d] processing %d\n", i, w2.id);
            route_update_pointers( route, zone->id, 0, w2.id, nl+i);
        } else if ( i == wpid ) {
            // skip it
        } else {
            w2 = zone->peripoints[i];
            nl[i-1] = w2;
            printf("processing %d\n", w2.id);
            route_update_pointers( route, zone->id, 0, w2.id, nl+i-1);
            //w2.id = i;
        }
    }
        
    // free memory
    free( zone->peripoints );

    // set pointer to the new allocated space
    zone->peripoints = nl;
    
    zone->num_peripoints--;

    // renumber the waypoints on the parent lane
    for (i=0;i<zone->num_peripoints;i++) {
        zone->peripoints[i].id = i+1;
    }

    return 0;

}

/* remove a waypoint in the route given the segment id, lane id and two neighbor waypoint ids */
int
route_remove_waypoint ( RndfRouteNetwork *route, int sid, int lid, int wpid )
{
    int i;

    if ( !route ) 
        return -1;

    // first check consistency
    if ( sid >= route->num_segments ) {
        fprintf(stderr,"invalid segment position %d\n", sid);
        return -1;
    }

    RndfSegment *s = route->segments + sid;

    if ( lid >= s->num_lanes ) {
        fprintf(stderr, "invalid lane position %d (num_lanes = %d)\n", lid, s->num_lanes );
        return -1;
    }

    RndfLane *l = s->lanes + lid;

    // if the lane has only one waypoints left, don't do anything
    // we do not want to end up with lanes having less than one waypoint
    // use remove_lane instead
    if ( l->num_waypoints == 1 ) {
        printf("lane %d.%d has only one waypoints. skipping.\n", s->id, l->id);
        return 0;
    }

    RndfWaypoint w = l->waypoints[wpid];

    if ( wpid >= l->num_waypoints  ) {
        fprintf(stderr, "invalid waypoint positions %d \n", wpid);
        return -1;
    }

   // remove the pointers to the removed waypoint
    printf("removing pointers to %d.%d.%d\n", s->id, l->id, w.id);
    route_remove_pointers( route, s->id, l->id, w.id) ;

    // realloc memory
    RndfWaypoint *nl = malloc( (l->num_waypoints-1) * sizeof(RndfWaypoint));

    for (i=0;i<l->num_waypoints;i++) {
           //for (i=l->num_waypoints-1;i>=0;i--) {
        RndfWaypoint w2;
        if ( i < wpid ) {
            w2 = l->waypoints[i];
            nl[i] = w2;
             printf("[%d] processing %d\n", i, w2.id);
             route_update_pointers( route, s->id, l->id, w2.id, nl+i);
        } else if ( i == wpid ) {
            // skip it
        } else {
            w2 = l->waypoints[i];
            nl[i-1] = w2;
            printf("processing %d\n", w2.id);
            route_update_pointers( route, s->id, l->id, w2.id, nl+i-1);
            //w2.id = i;
        }
    }
        
    // free memory
    free( l->waypoints );

    // set pointer to the new allocated space
    l->waypoints = nl;
    
    l->num_waypoints--;

    // renumber the waypoints on the parent lane
    for (i=0;i<l->num_waypoints;i++) {
        l->waypoints[i].id = i+1;
    }

    return 0;

}

/* move a waypoint */
int
route_move_waypoint( RndfRouteNetwork *route, int sid, int lid, int wpid, double lat, double lon )
{
    if ( !route )
        return -1;
    
        // first check consistency
    if ( sid >= route->num_segments ) {
        fprintf(stderr,"invalid segment position %d\n", sid);
        return -1;
    }
    
    RndfSegment *s = route->segments + sid;
    
    if ( lid >= s->num_lanes ) {
        fprintf(stderr, "invalid lane position %d (num_lanes = %d)\n", lid, s->num_lanes );
        return -1;
    }

    RndfLane *l = s->lanes + lid;

    if ( wpid >= l->num_waypoints ) {
        fprintf(stderr, "invalid waypoint position %d\n", wpid);
    }

    RndfWaypoint *w = l->waypoints + wpid;
    
    w->lat = lat;
    w->lon = lon;
    
    return 0;
}

/* remove a lane given a segment position and a lane position */
int
route_remove_lane( RndfRouteNetwork *route, int sid, int lid )
{
    if ( !route )
        return -1;
    
    // first check consistency
    if ( sid >= route->num_segments ) {
        fprintf(stderr,"invalid segment position %d\n", sid);
        return -1;
    }
    
    RndfSegment *s = route->segments + sid;
    
    if ( lid >= s->num_lanes ) {
        fprintf(stderr, "invalid lane position %d (num_lanes = %d)\n", lid, s->num_lanes );
        return -1;
    }

    if ( s->num_lanes == 1 ) {
        // do not remove the last lane of a segment
        // we do not want to end up with empty segments
        // use remove_segment instead
        printf("cannot remove single lane in a segment. RndfRemove whole segment instead.\n");
        return -1;
    }

    RndfLane *l = s->lanes + lid;

    // allocate new space
    RndfLane *new_lanes = (RndfLane*)malloc((s->num_lanes-1)*sizeof(RndfLane));

    int i,j;

    // remove all the pointers to the waypoints of the lane
    //printf("%d waypoints to remove.", l->num_waypoints);

    for (i=0;i<l->num_waypoints;i++) {
        RndfWaypoint *w = &l->waypoints[i];
        printf("removing waypoint %d.%d.%d\n", s->id, l->id, w->id);
        route_remove_pointers( route, s->id, l->id, w->id);
    }
    
    // free memory
    free( l->waypoints );

    // copy data into the new space, skipping one lane
    for (i=0;i<s->num_lanes;i++) {
        if ( i < lid ) {
            // update the lane pointers in the waypoint
            for (j=0;j<s->lanes[i].num_waypoints;j++) {
                s->lanes[i].waypoints[j].parent.lane = new_lanes + i;
            }
            new_lanes[i] = s->lanes[i];
            printf("restoring lane %d\n",i);
        } else if ( i > lid ) {
            // update the lane pointers in the waypoint
            for (j=0;j<s->lanes[i].num_waypoints;j++) {
                s->lanes[i].waypoints[j].parent.lane = new_lanes + i - 1;
            }
            new_lanes[i-1] = s->lanes[i];
        }
    }    

    // free memory
    free( s->lanes );

    // copy new memory
    s->lanes = new_lanes;

    s->num_lanes--;

    // renumber the lanes on the segment
    for (i=0;i<s->num_lanes;i++) {
        s->lanes[i].id = i+1;
    }

    return 0;
}

/* remove a segment given a segment position */
int
route_remove_segment( RndfRouteNetwork *route, int sid )
{
    if ( !route )
        return -1;
    
    // first check consistency
    if ( sid >= route->num_segments ) {
        fprintf(stderr,"invalid segment position %d\n", sid);
        return -1;
    }
    
    // set this flag to 1 if this is the last segment
    int last_segment = 0;
    if ( route->num_segments == 1 ) {
        last_segment = 1;
    }

    RndfSegment *s = route->segments + sid;
    
    // realloc new space in memory
    RndfSegment *new_segments = NULL;

    if ( !last_segment ) {
        new_segments = malloc( (route->num_segments-1) * sizeof(RndfSegment));
    }

    // remove all pointers to this segment
    int j,k;
    for (j=0;j<s->num_lanes;j++) {
        RndfLane *l = &s->lanes[j];
        for (k=0;k<l->num_waypoints;k++) {
            RndfWaypoint *w = &l->waypoints[k];
            printf("removing waypoint %d.%d.%d\n", s->id, l->id, w->id);
            route_remove_pointers( route, s->id, l->id, w->id);
        }
    }

    // free the space occupied by the segment
    if (s->name)
        free(s->name);

    for (j=0;j<s->num_lanes;j++) {
        free( s->lanes[j].waypoints);
    }
    
    free( s->lanes );

    // copy the previous segments in new space
    int i;
    for (i=0;i<route->num_segments;i++) {
        if ( i < sid ) {
            new_segments[i] = route->segments[i];
            // update pointers to this segment in the lanes
            for (j=0;j<route->segments[i].num_lanes;j++) {
                route->segments[i].lanes[j].parent = new_segments + i;
            }
        } else if ( i > sid ) {
            new_segments[i-1] = route->segments[i];
            // update pointers to this segment in the lanes
            for (j=0;j<route->segments[i].num_lanes;j++) {
                route->segments[i].lanes[j].parent = new_segments + i - 1;
            }
        }
    }

    // free memory
    free( route->segments );
    
    // copy new memory
    route->segments = new_segments;

    route->num_segments--;

    // renumber all the segments on the route
    for (i=0;i<route->num_segments;i++) {
        route->segments[i].id = i+1;
    }

    // renumber all the zones on the route
    for (i=0;i<route->num_zones;i++) {
        route->zones[i].id--;
    }
    
    return 0;
}

/* remove a zone */
int
route_remove_zone ( RndfRouteNetwork *route, int idx )
{
    if ( !route || route->num_zones <= idx )
        return -1;

    RndfZone *z = route->zones + idx;

    // remove all the checkpoints
    int i;
    for (i=0;i<route->num_checkpoints;i++) {
        RndfWaypoint *w = route->checkpoints[i].waypoint;
        int j;
        for (j=0;j<z->num_spots;j++) {
            RndfWaypoint *w1 = z->spots[j].waypoints;
            if ( same_waypoint(w,w1) )
                route_toggle_checkpoint( route, w );
            RndfWaypoint *w2 = z->spots[j].waypoints + 1;
            if ( same_waypoint(w,w2) )
                route_toggle_checkpoint( route, w );
        }
    }

    // remove all the spots (spots are renumbered each time)
    for (i=0;i<z->num_spots;i++) {
        printf("removing spot\n");
        route_remove_spot( route, z, 1 ); // spot ID = 1
    }

    // then remove all the exits coming out from the zone
    for (i=0;i<z->num_peripoints;i++) {
        RndfWaypoint *w1 = z->peripoints + i;
        int j;
        for (j=0;j<w1->num_exits;j++) {
            RndfWaypoint *w2 = w1->exits[j];
            if ( route_remove_exit( w1, w2 ) < 0 ) {
                printf( "RndfError removing exit during zone removal [1].");
            } else {
                printf( "Removed exit %s -> %s\n", rndf_get_waypoint_str( w1 ),
                        rndf_get_waypoint_str( w2 ) );
            }
        }
    }

    // then remove all the exits coming in to the zone
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = route->segments + i;
        int j;
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = s->lanes + j;
            int k;
            for (k=0;k<l->num_waypoints;k++) {
                RndfWaypoint *w1 = l->waypoints + k;
                int m;
                for (m=0;m<w1->num_exits;m++) {
                    RndfWaypoint *w2 = w1->exits[m];
                    
                    if ( w2->type != RNDF_POINT_PERIMETER )
                        continue;
                    
                    if ( w2->parent.zone->id != z->id ) 
                        continue;

                    if ( route_remove_exit( w1, w2 ) < 0 ) {
                        printf( "Error removing exit during zone removal [2].");
                    } else {
                        printf( "Removed exit %s -> %s\n", rndf_get_waypoint_str( w1 ),
                                rndf_get_waypoint_str( w2 ) );
                    }
                }
            }
        }
    }

    // finally remove the zone
    for (i=0;i<route->num_zones-1;i++) {
        if ( i >= idx ) {
            route->zones[i] = route->zones[i+1];
        }
    }

    // realloc memory
    if ( route->num_zones == 1 ) {
        free( route->zones );
        route->zones = NULL;
    } else {
        route->zones = realloc( route->zones, (route->num_zones-1)*sizeof(RndfZone));
    }

    route->num_zones--;

    // update pointers
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        int j;
        for (j=0;j<z->num_peripoints;j++)
            z->peripoints[j].parent.zone = z;
        for (j=0;j<z->num_spots;j++)
            z->spots[j].parent = z;
    }    return 0;

    // renumber all the zones
    for (i=0;i<route->num_zones;i++) {
        route->zones[i].id = route->num_segments + i + 1;
    }
    
}

/* add a segment given segment name and <n> waypoints and a lane width in feet */
int
route_insert_segment( RndfRouteNetwork *route, int n, double **gps, const char *name, int lane_width, \
                      RndfBoundaryType left_boundary, RndfBoundaryType right_boundary)
{
    if ( !route )
        return -1;

    RndfSegment *new_segments = malloc( (route->num_segments+1)*sizeof(RndfSegment));
    
    int i,j;
    for (i=0;i<route->num_segments;i++) {
        new_segments[i] = route->segments[i];
        // update pointers to segment in the lanes
        for (j=0;j<route->segments[i].num_lanes;j++) 
            route->segments[i].lanes[j].parent = new_segments + i;
    }

    RndfSegment *s = new_segments + route->num_segments;
    s->id = route->num_segments+1;
    s->parent = route;
    s->num_lanes = 1;
    s->max_speed = s->min_speed = 0;
    s->name = strdup( name );
    s->lanes = malloc(sizeof(RndfLane));

    // create the lane
    RndfLane *lane = s->lanes;
    lane->parent = s;
    lane->id = 1;
    lane->num_waypoints = n;
    lane->waypoints = malloc(n*sizeof(RndfWaypoint));
    for (i=0;i<n;i++) {
        
        RndfWaypoint w;
        w.lat = gps[i][0];
        w.lon = gps[i][1];
        w.type = RNDF_POINT_WAYPOINT;
        w.id = i+1;
        w.is_stop = 0;
        w.num_exits = 0;
        w.exits = NULL;
        w.parent.lane = lane;
        lane->waypoints[i] = w;
    }

    lane->lane_width = lane_width;
    lane->left_boundary = left_boundary;
    lane->right_boundary = right_boundary;

    route->num_segments++;

    // free the memory

    free(route->segments);

    route->segments = new_segments;

    // shift all zone IDs by one
    for (i=0;i<route->num_zones;i++)
        route->zones[i].id++;
    return 0;
}

/* insert a zone */
int
route_insert_zone( RndfRouteNetwork *route, int n, double **gps, const char *name )
{
    if ( !route )
        return -1;

    // determine the ID of the new zone
    int id = 0;
    int i;
    for (i=0;i<route->num_zones;i++) {
        if ( route->zones[i].id )
            id = route->zones[i].id;
    }
    for (i=0;i<route->num_segments;i++) {
        if ( route->segments[i].id > id )
            id = route->segments[i].id;
    }
    id++;

    // realloc the memory
    route->zones = realloc( route->zones, (route->num_zones+1)*sizeof(RndfZone));

    RndfZone *z = route->zones + route->num_zones;

    z->id = id;

    // copy the zone name
    z->name = strdup( name );

    z->parent = route;

    z->num_spots = 0;
    z->spots = NULL;

    z->max_speed = z->min_speed = 0;

    // copy the peripoints
    z->num_peripoints = n;

    z->peripoints = (RndfWaypoint*)malloc( n*sizeof(RndfWaypoint) );

    for (i=0;i<n;i++) {
        
        RndfWaypoint *w = z->peripoints  + i;

        w->exits = NULL;
        w->num_exits = 0;
        w->lat = gps[i][0];
        w->lon = gps[i][1];
        w->is_stop = 0;
        w->parent.zone = z;
        w->id = i;
        w->type = RNDF_POINT_PERIMETER;
    }
    
    route->num_zones++;

    // update pointers
    int j;
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        for (j=0;j<z->num_peripoints;j++)
            z->peripoints[j].parent.zone = z;
        for (j=0;j<z->num_spots;j++)
            z->spots[j].parent = z;
    }

    return 0;
    
}

int
route_insert_lane ( RndfRouteNetwork *route, RndfSegment *s, int lane_id, RndfBoundaryType b_left, \
                    RndfBoundaryType b_right, int lane_width, int same_direction )
{
    if ( !route || !s )
        return -1;

    // check validity of lane ID
    // lane ID must be <= num_lanes+1
    if ( lane_id <= 0 || lane_id > s->num_lanes + 1 ) {
        fprintf(stderr, "Invalid lane id for insertion : %d must be [%d-%d]\n", lane_id, 1, s->num_lanes+1);
        return -1;
    }

    // find another lane to replicate the waypoints
    if ( s->num_lanes == 0 ) {
        fprintf(stderr, "Invalid segment: has 0 lanes!\n");
        return -1;
    } else {
        printf("Segment has %d lane(s).\n", s->num_lanes);
    }

    int i;

    int nlane_id;
    int higher_id = 0; // true if the new lane has a higher ID
    if ( lane_id == 1 ) {
        nlane_id = 0;
    } else {
        nlane_id = lane_id-2;
        higher_id = 1;
    }
        
    // replicate the waypoints
    // and move them eastward-southward if higher_id is true
    // westward-northward otherwise
    double delta = 0.000050;

    int num_waypoints = s->lanes[nlane_id].num_waypoints;
    printf("replicating %d waypoints.\n", num_waypoints);

    RndfWaypoint *waypoints = malloc(num_waypoints*sizeof(RndfWaypoint));
    for (i=0;i<num_waypoints;i++) {
        RndfWaypoint *w1;
        if ( same_direction ) {
            w1 = &s->lanes[nlane_id].waypoints[i];
        } else {
            w1 = &s->lanes[nlane_id].waypoints[num_waypoints-1-i];
        }            
        RndfWaypoint w;
        w.type = RNDF_POINT_WAYPOINT;
        w.id = i+1;
        w.lat = w1->lat + delta * ( higher_id ? -1.0 : 1.0 );
        w.lon = w1->lon + delta * ( higher_id ? 1.0 : -1.0 );
        w.is_stop = 0;
        w.num_exits = 0;
        w.exits = NULL;
        waypoints[i] = w;
    }

    // insert the lane
    RndfLane *lanes = malloc( (s->num_lanes+1)*sizeof(RndfLane));

    // create a new lane
    RndfLane *new_lane = lanes + lane_id-1;
    new_lane->parent = s;
    new_lane->id = lane_id;
    new_lane->num_waypoints = num_waypoints;
    new_lane->waypoints = waypoints;
    new_lane->lane_width = lane_width;
    new_lane->left_boundary = b_left;
    new_lane->right_boundary = b_right;

    int j;
    for (i=0;i<s->num_lanes+1;i++) {
        if ( i+1 < lane_id ) {
            lanes[i] = s->lanes[i];
            // update pointers to the lane
            for (j=0;j<s->lanes[i].num_waypoints;j++) 
                lanes[i].waypoints[j].parent.lane = lanes + i;
            
        } else if ( i+1 == lane_id ) {
            for (j=0;j<num_waypoints;j++)
                lanes[i].waypoints[j].parent.lane = lanes + i;
        } else if (i+1 > lane_id) {
            lanes[i] = s->lanes[i-1];
            lanes[i].id++;
            // update pointers to the lane
            for (j=0;j<s->lanes[i-1].num_waypoints;j++) 
                lanes[i].waypoints[j].parent.lane = lanes + i;
        }
    }

    // free memory

    free( s->lanes );
    
    //return 0;

    s->lanes = lanes;
    
    s->num_lanes++;

    return 0;
    
}

/* remove a spot given zone and spot ID */
int
route_remove_spot( RndfRouteNetwork *route, RndfZone *zone, int spot_id ) 
{
    if ( !route || !zone )
        return -1;

    if ( zone->num_spots < spot_id ) {
        fprintf(stderr, "Invalid spot ID\n");
        return -1;
    }

    //printf("zone name: %s\n", route->zones[0].name);

    // remove checkpoint on second waypoint of the spot
    if ( route_is_checkpoint( route, &(zone->spots[spot_id-1].waypoints[1]) )) {
        route_toggle_checkpoint( route, &(zone->spots[spot_id-1].waypoints[1]) );
    }

    //zone = route->zones;

    // alloc new memory space
    RndfSpot *spots = malloc((zone->num_spots-1)*sizeof(RndfSpot));
    
    int i;
    for (i=0;i<zone->num_spots;i++) {
        if ( i+1 < spot_id ) {
            spots[i] = zone->spots[i];
            // upadte pointers to the spot end point
            route_update_pointers( route, zone->id, i+1 , 1, spots[i].waypoints  );
            route_update_pointers( route, zone->id, i+1 , 2, spots[i].waypoints + 1 );
            // update pointers to the parent spot
            spots[i].waypoints[0].parent.spot = spots + i;
            spots[i].waypoints[1].parent.spot = spots + i;
        } else if ( i+1 > spot_id ) {
            spots[i-1] = zone->spots[i];
            // update pointers
            route_update_pointers( route, zone->id, i+1, 1, spots[i-1].waypoints );
            route_update_pointers( route, zone->id, i+1, 2, spots[i-1].waypoints + 1 );
            // update pointers to the parent spot
            spots[i-1].waypoints[0].parent.spot = spots + i - 1;
            spots[i-1].waypoints[1].parent.spot = spots + i - 1;
        }
    }

    // remove pointers to the deleted spot waypoints
    route_remove_pointers( route, zone->id, spot_id, 1 );
    route_remove_pointers( route, zone->id, spot_id, 2 );

    free(zone->spots);

    zone->spots = spots;
    
    zone->num_spots--;

    // renumber spot IDs
    for (i=0;i<zone->num_spots;i++)
        zone->spots[i].id = i+1;
    
    return 0;
}

/* add a spot */
int
route_insert_spot( RndfRouteNetwork *route, RndfZone *zone, double *gps1, double *gps2, int spot_width )
{
    if ( !route || !zone )
        return -1;

    RndfSpot *spots = malloc( (zone->num_spots+1)*sizeof(RndfSpot));

    int i;
    for (i=0;i<zone->num_spots;i++) {
        spots[i] = zone->spots[i];
        // upadte pointers to the spot end point
        route_update_pointers( route, zone->id, i+1 , 1, spots[i].waypoints  );
        route_update_pointers( route, zone->id, i+1 , 2, spots[i].waypoints + 1 );
        // update pointers to the parent spot
        spots[i].waypoints[0].parent.spot = spots + i;
        spots[i].waypoints[1].parent.spot = spots + i;
    }

    RndfSpot *new_spot = spots + zone->num_spots;

    RndfWaypoint w1;
    w1.lat = gps1[0];
    w1.lon = gps1[1];
    w1.type = RNDF_POINT_SPOT;
    w1.parent.spot = new_spot;
    w1.id = 1;
    w1.is_stop = 0;
    w1.num_exits = 0;
    w1.exits = NULL;

    RndfWaypoint w2;
    w2.lat = gps2[0];
    w2.lon = gps2[1];
    w2.type = RNDF_POINT_SPOT;
    w2.parent.spot = new_spot;
    w2.id = 2;
    w2.is_stop = 0;
    w2.num_exits = 0;
    w2.exits = NULL;
  
    //printf("adding checkpoint %d\n", route->max_checkpoint_id+1);
    new_spot->parent = zone;
    new_spot->id = zone->num_spots+1;
    new_spot->spot_width = spot_width;
    new_spot->waypoints[0] = w1;
    new_spot->waypoints[1] = w2;
    new_spot->checkpoint_id = route->max_checkpoint_id+1;

    // add checkpoint on second waypoint
    rndf_add_checkpoint( route, route->max_checkpoint_id+1, new_spot->waypoints + 1 );

    // free memory
    free( zone->spots );

    zone->spots = spots;

    zone->num_spots++;

    return 0;
}

/* insert an exit given exit point and entry point
*/

int
route_insert_exit (RndfWaypoint *w1, RndfWaypoint *w2 )
{
    if ( !w1 || !w2 )
        return -1;

    int i;

    // look whether the exit already exists
    for (i=0;i<w1->num_exits;i++) {
        RndfWaypoint *w = w1->exits[i];
        if ( same_waypoint( w, w2 ) ) {
            printf("exit already exists!\n");
            return -1;
        }
    }

    //printf("adding exit between %d and %d (%d exits already)\n", w1->id, w2->id,w1->num_exits );

    if ( w1->exits != NULL && w1->num_exits == 0 )
        printf("problem...\n");

    w1->exits = realloc( w1->exits, (w1->num_exits+1)*sizeof(RndfWaypoint*));

    w1->exits[w1->num_exits] = w2;

    w1->num_exits++;

    return 0;
}

/* remove exit given two waypoints */
int
route_remove_exit ( RndfWaypoint *w1, RndfWaypoint *w2 ) 
{
    if ( !w1 || !w2 )
        return -1;

    int i;
    int index = -1;
    for (i=0;i<w1->num_exits;i++) {
        if ( same_waypoint( w1->exits[i], w2 ) ) {
            index = i;
            break;
        }
    }

    if ( index < 0 ) {
        printf("exit not found!\n");
        return -1;
    }

    if ( w1->num_exits == 1 ) {
        free(w1->exits);
        w1->exits = NULL;
        w1->num_exits = 0;
        return 0;
    }

    // exit exists and will be removed
    RndfWaypoint **new_exits = malloc( (w1->num_exits-1)*sizeof(RndfWaypoint*));
    for (i=0;i<w1->num_exits;i++) {
        if ( i < index ) {
            new_exits[i] = w1->exits[i];
        } else if ( i > index ) {
            new_exits[i-1] = w1->exits[i];
        }
    }

    free( w1->exits );
    w1->exits = new_exits;

    w1->num_exits--;

    return 0;
}

/* add or remove a stop */
int
route_add_rem_stop ( RndfWaypoint *w ) 
{
    if ( !w )
        return -1;
    
    w->is_stop = 1 - w->is_stop;

    return 0;
}

int
route_is_checkpoint ( RndfRouteNetwork *route, RndfWaypoint *w ) 
{
    int i;
    if ( !route || !w )
        return -1;

    for (i=0;i<route->num_checkpoints;i++) {
        RndfWaypoint *w2 = route->checkpoints[i].waypoint;
        if ( same_waypoint(w,w2) ) {
            return route->checkpoints[i].id;
        }
    }
    
    return -1;
}
   
/* toggle a checkpoint */
int
route_toggle_checkpoint( RndfRouteNetwork *route, RndfWaypoint *w )
{
    if ( !w )
        return -1;

    // look whether the point is a checkpoint

    int i;
    int index=-1;
    for (i=0;i<route->num_checkpoints;i++) {
        RndfWaypoint *w2 = route->checkpoints[i].waypoint;
        if ( same_waypoint(w,w2) ) {
            index = i;
            break;
        }
    }

    //printf("ok so far\n");

    // if it is not a checkpoint, make it a checkpoint
    if ( index < 0 ) {
        rndf_add_checkpoint( route, route->max_checkpoint_id+1, w );

        // if on a spot, update spot checkpoint id
        if ( w->type == RNDF_POINT_SPOT) {
            w->parent.spot->checkpoint_id = route->max_checkpoint_id+1;
        }
    
        return 0;
    } else {

        //otherwise, remove the checkpoint
   
        // if on a spot, update spot checkpoint id
        if ( w->type == RNDF_POINT_SPOT) {
            w->parent.spot->checkpoint_id = -1;
        }
        
        if ( route->num_checkpoints == 1 ) {
            free(route->checkpoints);
            route->checkpoints = NULL;
            route->num_checkpoints = 0;
            return 0;
        } else {

            RndfCheckpoint *new_checkpoints = malloc( (route->num_checkpoints-1)*sizeof(RndfCheckpoint));
            for (i=0;i<route->num_checkpoints;i++) {
                if ( i < index ) {
                    new_checkpoints[i] = route->checkpoints[i];
                } else if ( i > index ) {
                    new_checkpoints[i-1] = route->checkpoints[i];
                }
            }

            free( route->checkpoints );
            route->checkpoints = new_checkpoints;
            route->num_checkpoints--;
            return 0;
        }
    }

    return 0;
}

/* return true if the waypoint is at the beginning of the lane */

int
begin_in_lane( RndfWaypoint *w )
{
    if ( !w )
        return 0;

    if ( w->type != RNDF_POINT_WAYPOINT )
        return 0;

    if ( w->id == 1 )
        return 1;

    return 0;
}

/* return true if the waypoint is at the end of the lane */

int
end_in_lane( RndfWaypoint *w )
{
    if ( !w )
        return 0;

    if ( w->type != RNDF_POINT_WAYPOINT )
        return 0;

    if ( w->id == w->parent.lane->num_waypoints )
        return 1;

    return 0;
}

/* return next waypoint in the lane */
RndfWaypoint*
next_in_lane( RndfWaypoint *w ) 
{
    if ( !w || w->type != RNDF_POINT_WAYPOINT )
        return NULL;

    if ( end_in_lane(w) )
        return w;

    return w->parent.lane->waypoints + w->id;
}

/* return prev waypoint in the lane */
RndfWaypoint*
prev_in_lane( RndfWaypoint *w ) 
{
    if ( !w || w->type != RNDF_POINT_WAYPOINT )
        return NULL;

    if ( begin_in_lane(w) )
        return w;

    return w->parent.lane->waypoints + w->id-2;
}

/* add a checkpoint in a mission */
int
mission_rndf_add_checkpoint( RndfMission *mission, int id )
{
    if ( !mission )
        return -1;

    mission->checkpoint_ids = (int*)realloc( mission->checkpoint_ids, \
                                             (mission->num_checkpoints+1) * sizeof(int));


    mission->checkpoint_ids[mission->num_checkpoints] = id;

    mission->num_checkpoints++;

    return 0;
}

/* remove a checkpoint */
int
mission_remove_checkpoint( RndfMission *mission, int idx )
{
    if ( !mission || idx < 0 )
        return -1;

    int n = mission->num_checkpoints;

    if ( idx >= n )
        return -1;

    int *cs = (int*)malloc((n-1)*sizeof(int));

    int i;
    for (i=0;i<n;i++) {
        if ( i < idx ) 
            cs[i] = mission->checkpoint_ids[i];
        else if ( i > idx ) {
            cs[i-1] = mission->checkpoint_ids[i];
        }
    }

    free( mission->checkpoint_ids );
    
    mission->checkpoint_ids = cs;

    mission->num_checkpoints--;

    return 0;
}

/* move up a checkpoint in the mission file */
int
mission_move_up_checkpoint( RndfMission *mission, int idx ) 
{
    if ( !mission || idx < 0 )
        return -1;

    int n = mission->num_checkpoints;

    if ( idx >= n )
        return -1;

    if ( idx == 0 )
        return 0; // nothing to do

    int id = mission->checkpoint_ids[idx-1];
    mission->checkpoint_ids[idx-1] = mission->checkpoint_ids[idx];
    mission->checkpoint_ids[idx] = id;

    return 0;
}

/* move down a checkpoint in the mission file */
int
mission_move_down_checkpoint( RndfMission *mission, int idx ) 
{
    if ( !mission || idx < 0 )
        return -1;

    int n = mission->num_checkpoints;

    if ( idx >= n )
        return -1;

    if ( idx == n-1 )
        return 0; // nothing to do

    int id = mission->checkpoint_ids[idx+1];
    mission->checkpoint_ids[idx+1] = mission->checkpoint_ids[idx];
    mission->checkpoint_ids[idx] = id;

    return 0;
}

int
get_ppid ( RndfWaypoint *w )
{
    int pid = -1;
    if ( w->type == RNDF_POINT_WAYPOINT ) 
        pid = w->parent.lane->parent->id;
    else if ( w->type == RNDF_POINT_SPOT ) 
        pid = w->parent.spot->parent->id;
    else if ( w->type == RNDF_POINT_PERIMETER )
        pid = w->parent.zone->id;

    return pid;
}

void
mission_add_speed_limit( RndfMission *mission, int id, int min_speed, int max_speed )
{
    if ( !mission ) 
        return;

    mission->speed_limits = (RndfSpeedlimit*)realloc(mission->speed_limits, \
                                                 (mission->num_speed_limits+1)*sizeof(RndfSpeedlimit));

    RndfSpeedlimit sl;
    sl.id = id;
    sl.min_speed = min_speed;
    sl.max_speed = max_speed;

    mission->speed_limits[mission->num_speed_limits] = sl;

    mission->num_speed_limits++;
}

/* add an obstacle */
int
route_add_obstacle( RndfRouteNetwork *route, RndfObstacle o)
{
    if ( !route )
        return -1;

    // determine new obstacle id
    int id = 0;
    if ( route->num_obstacles > 0 ) {
        id = route->obstacles[route->num_obstacles-1].id + 1;
    }

    o.id = id;

    // realloc
    route->obstacles = (RndfObstacle*)realloc( route->obstacles, (route->num_obstacles+1)*sizeof(RndfObstacle));

    route->obstacles[route->num_obstacles] = o;

    route->num_obstacles++;
    
    return 0;
}

/* remove an obstacle */
int
route_remove_obstacle( RndfRouteNetwork *route, int index )
{
    if ( !route || index < 0 || index > route->num_obstacles-1)
        return -1;

    // if this is the last obstacle, clear
    if ( route->num_obstacles == 0 ) {
        free( route->obstacles );
        route->obstacles = NULL;
        route->num_obstacles = 0;
        return 0;
    }

    // otherwise, shift obstacles in the list
    int i;
    for (i=index;i<route->num_obstacles-1;i++) 
        route->obstacles[i] = route->obstacles[i+1];

    // realloc
    route->obstacles = realloc( route->obstacles, (route->num_obstacles-1)*sizeof(RndfObstacle));
    
    route->num_obstacles--;

    return 0;
}

/* remove all checkpoints */
int
route_remove_all_checkpoints ( RndfRouteNetwork *route ) 
{
    if ( !route )
        return -1;
    
    free( route->checkpoints );
    
    route->num_checkpoints =  0;

    route->checkpoints = NULL;

    route->max_checkpoint_id = 0;

    return 0;
}

/* add all checkpoints */
int
route_add_all_checkpoints ( RndfRouteNetwork *route )
{
    if ( !route )
        return -1;

    // first remove the existing checkpoints
    route_remove_all_checkpoints( route );

    // add checkpoints everywhere
    int i;
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = route->segments + i;
        int j;
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = s->lanes + j;
            int k;
            for (k=0;k<l->num_waypoints;k++) {
                RndfWaypoint *w = l->waypoints + k;
                
                rndf_add_checkpoint( route, route->max_checkpoint_id+1, w );
            }
        }
    }

    return 0;
}

/* reset all speed limits in the mission */
void
mission_reset_speed_limits ( RndfMission *mission )
{
    if ( !mission )
        return;
    
    free( mission->speed_limits );
    
    mission->speed_limits = NULL;
    
    mission->num_speed_limits = 0;
}

/* set all speed limits in a mission */
void
mission_set_all_speed_limits ( RndfMission *mission, int min_speed, int max_speed, int n )
{
    
    if ( !mission )
        return;
    
    // reset speed limits
    mission_reset_speed_limits ( mission );
    
    // generate new speed limits
    int i;
    
    mission->speed_limits = (RndfSpeedlimit *)malloc(n*sizeof(RndfSpeedlimit));
    
    for (i=1;i<=n;i++) {
        RndfSpeedlimit sl;
        sl.id = i;
        printf("setting %d %d\n", min_speed, max_speed);
        
        sl.min_speed = min_speed;
        sl.max_speed = max_speed;
        mission->speed_limits[i-1] = sl;
    }
    
    mission->num_speed_limits = n;
    
}

/* set a specific speed limit */
void
mission_set_speed_limit ( RndfMission *mission, int min_speed, int max_speed, int id )
{
    if ( !mission )
        return;
    
    // look whether this is the first speed limit
    if ( mission->num_speed_limits == 0 ) {
        
        mission->speed_limits = (RndfSpeedlimit *) malloc(sizeof(RndfSpeedlimit));
        RndfSpeedlimit sl;
        sl.min_speed = min_speed;
        sl.max_speed = max_speed;
        sl.id = id;
        mission->speed_limits[0] = sl;
        mission->num_speed_limits = 1;
        return;
    }
    
    // look whether there is a speed limit for this segment already
    int i;
    
    for (i=0;i<mission->num_speed_limits;i++) {
        if ( mission->speed_limits[i].id == id ) {
            mission->speed_limits[i].min_speed = min_speed;
            mission->speed_limits[i].max_speed = max_speed;
            return;
        }
        
    }
    
    // otherwise, insert the speed limit
    int n = mission->num_speed_limits;
    
    mission->speed_limits = (RndfSpeedlimit *)
        realloc( mission->speed_limits, 
                 (n+1)*sizeof(RndfSpeedlimit));
    
    RndfSpeedlimit sl;
    sl.min_speed = min_speed;
    sl.max_speed = max_speed;
    sl.id = id;
    
    mission->speed_limits[n] = sl;
    
    mission->num_speed_limits = n+1;
}

/* remove a segment in the set of speed limits
 * all counters need to be lowered by one */
void
mission_rm_segment_speed_limit ( RndfMission *mission, int id )
{
    
    if ( !mission ) 
        return;
    
    int n = mission->num_speed_limits;
    

    // remove the speed limit for that segment
    int i;
    int index = -1;
    
    for (i=0;i<n;i++) {
        
        if ( mission->speed_limits[i].id == id ) {
            index = i;
            break;
        }
    }
    
    if ( index != -1 ) {

        printf("removing speed limit segment at position %d\n", index);
        
        if ( n == 1 ) {
            mission_reset_speed_limits( mission );
        } else {
            
            for (i=index;i<n-1;i++) {
                mission->speed_limits[i] = mission->speed_limits[i+1];
            }
            
            mission->speed_limits = (RndfSpeedlimit *)
                realloc( mission->speed_limits, 
                         (n-1)*sizeof(RndfSpeedlimit));
            
            mission->num_speed_limits--;
            
        }
    }
    
    
    // decrement all the segment IDs above <id>
    for (i=0;i<n-1;i++) {
        if ( mission->speed_limits[i].id > id ) {
            printf("renumbering %d into %d\n", mission->speed_limits[i].id, mission->speed_limits[i].id-1);
            
            mission->speed_limits[i].id--;
        }
    }
}

/* insert a segment in the list of speed limits of an MDF */
void
mission_insert_segment_speed_limit ( RndfMission *mission, int id )
{
    if ( !mission )
        return;
    
    int n = mission->num_speed_limits;
    
    mission->speed_limits = ( RndfSpeedlimit *)realloc( mission->speed_limits,
                                                        (n+1)*sizeof(RndfSpeedlimit));
    
    int i;
    
    for (i=0;i<n;i++) {
        if (mission->speed_limits[i].id >= id ) {
            mission->speed_limits[i].id++;
        }
    }
    
    RndfSpeedlimit sl;
    sl.id = id;
    sl.min_speed = 0;
    sl.max_speed = 0;
    mission->speed_limits[n] = sl;
    
    mission->num_speed_limits = n+1;
}

/* applies a GPS offset to all waypoints in the route */
void route_apply_gps_offset( RndfRouteNetwork *route, double lat, double lon) 
{

    int i,j,k;

    // segments
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = route->segments + i;
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = s->lanes + j;
            for (k=0;k<l->num_waypoints;k++) {
               RndfWaypoint *w = l->waypoints + k;
                w->lat += lat;
                w->lon += lon;
            }
        }
    }

    // zones
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        for (j=0;j<z->num_peripoints;j++) {
            RndfWaypoint *w = z->peripoints + j;
            w->lat += lat;
            w->lon += lon;
        }
        for (j=0;j<z->num_spots;j++) {
            RndfSpot *s = z->spots + j;
            s->waypoints[0].lat += lat;
            s->waypoints[0].lon += lon;
            s->waypoints[1].lat += lat;
            s->waypoints[1].lon += lon;
        }
    }
}

            
