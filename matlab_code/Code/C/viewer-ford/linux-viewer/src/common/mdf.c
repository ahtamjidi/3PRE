/**
 * mission_parse.c
 *
 * Code for parsing a Route Network Definition (RNDF) and Mission Definition
 * File (MDF) and placing the result inside the structure defined by
 * route-definition.h.
 *
 * Since the RNDF contains forward references, it is parsed using two passes.
 * The first pass accumulates most of its structure, and the second pass
 * incorporates checkpoints, exits, and stops.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "mdf.h"

#define delimeters " \t\n\r"
#define MAXLINE 256
static char linebuf[MAXLINE];
static int lineno = 0;

void rndf_destroy ( RndfRouteNetwork *route ) 
{  
    int i=0,j=0,k=0;
    
    if ( !route )
        return;
    
    //printf("destroying route...\n");

    if ( route->name )
        free( route->name );

    if ( route->creation_date ) 
        free (route->creation_date );

    if ( route->format_version )
        free ( route->format_version );

    if (route->checkpoints) {
      //for (i=0;i<route->num_checkpoints;i++)
      //free( route->checkpoints[i].waypoint->exits);
      free( route->checkpoints );
    }

    // free segments
    for (i=0;i<route->num_segments;i++) {

        if (route->segments[i].name)
            free(route->segments[i].name);

        for (j=0;j<route->segments[i].num_lanes;j++) {
            for (k=0;k<route->segments[i].lanes[j].num_waypoints;k++) {
                free (route->segments[i].lanes[j].waypoints[k].exits);
            }
            free( route->segments[i].lanes[j].waypoints);
        }

        if (route->segments[i].lanes)
            free( route->segments[i].lanes );

    }

    if (route->segments)
        free(route->segments);

    // free zones
    for (i=0;i<route->num_zones;i++) {
        if (route->zones[i].name)
            free(route->zones[i].name);
        for (j=0;j<route->zones[i].num_peripoints;j++) {
            free (route->zones[i].peripoints[j].exits);
        }
        if (route->zones[i].peripoints)
            free(route->zones[i].peripoints);
        if (route->zones[i].spots)
            free(route->zones[i].spots);
    }

    if (route->obstacles)
        free( route->obstacles );
    route->obstacles = NULL;
    route->num_obstacles = 0;

    if ( route->zones )
        free(route->zones);

    route->name = NULL;
    route->segments = NULL;
    route->zones = NULL;
    route->checkpoints = NULL;

    route->num_segments = 0;
    route->num_zones = 0;
    route->num_checkpoints = 0;

    route->creation_date = NULL;
    route->format_version = NULL;

    route->valid = 0;
    route->max_checkpoint_id = 0;

    free(route);
}

RndfRouteNetwork* rndf_new () 
{

    RndfRouteNetwork *route;

    route = malloc (sizeof (RndfRouteNetwork));
    if ( !route )
        return NULL;

    memset (route, 0, sizeof (RndfRouteNetwork));

    route->name = NULL;
    route->segments = NULL;
    route->zones = NULL;
    route->checkpoints = NULL;

    route->num_segments = 0;
    route->num_zones = 0;
    route->num_checkpoints = 0;

    route->creation_date = NULL;
    route->format_version = NULL;

    route->obstacles = NULL;
    route->num_obstacles = 0;

    route->valid = 0;
    route->max_checkpoint_id = 0;

    return route;
}

RndfRouteNetwork* 
rndf_new_from_file (const char *fname)
{
    RndfRouteNetwork *route = rndf_new ();
    FILE *fp = fopen (fname, "r");
    if (!fp) {
        perror ("fopen");
        fprintf (stderr, "%s:%d Couldn't load RNDF [%s]!!\n", 
                __FILE__, __LINE__, fname);
        goto fail;
    }
    if (0 != rndf_load_file (fp, route)) {
        fprintf (stderr, "couldn't parse route network!\n");
        goto fail;
    }
    fclose (fp);
    return route;
fail:
    if (route) rndf_destroy (route);
    return NULL;
}

RndfMission* mdf_new (void)
{
    RndfMission *mission;

    mission = malloc ( sizeof(RndfMission));
    if ( !mission )
        return NULL;

    memset(mission, 0, sizeof(RndfMission));

    mission->name = NULL;
    mission->route = NULL;
    mission->route_name = NULL;
    mission->format_version = NULL;
    mission->creation_date = NULL;
    mission->num_checkpoints = 0;
    mission->checkpoints = NULL;
    mission->checkpoint_ids = NULL;
    mission->num_speed_limits = 0;
    mission->speed_limits = NULL;
    mission->valid = 0;

    return mission;
}

void 
mdf_destroy ( RndfMission *mission )
{
    if ( !mission )
        return;

    free (mission->name);
    mission->route = NULL;
    free(mission->route_name);
    mission->route_name = NULL;
    if ( mission->format_version )
        free(mission->format_version);
    if (mission->creation_date)
        free(mission->creation_date);
    if (mission->checkpoints)
        free(mission->checkpoints);
    mission->num_checkpoints = 0;
    if ( mission->checkpoint_ids )
        free(mission->checkpoint_ids);
    if (mission->speed_limits)
        free(mission->speed_limits);
    mission->num_speed_limits = 0;
    mission->valid = 0;

    free (mission);
}

static char *get_next_line (FILE * f)
{
    if (fgets (linebuf, MAXLINE, f)) {
        lineno++;
        return linebuf;
    }
    else
        return NULL;
}

static void
parse_error (char * msg, ...)
{
    va_list ap;
    fprintf (stderr, "Error, line %d", lineno);
    if (msg) {
        fprintf (stderr, ": ");
        va_start (ap, msg);
        vfprintf (stderr, msg, ap);
        va_end (ap);
    }
    fprintf (stderr, "\n");
}

static char namestr[64];

static const char *
lane_str (RndfLane * lane)
{
    sprintf (namestr, "%d.%d", lane->parent->id, lane->id);
    return namestr;
}

static const char *
spot_str (RndfSpot * spot)
{
    sprintf (namestr, "%d.%d", spot->parent->id, spot->id);
    return namestr;
}

static int
parse_segment (FILE * f, RndfSegment * segment);
static int
parse_lane (FILE * f, RndfLane * lane);
static int
parse_zone (FILE * f, RndfZone * zone);
static int
parse_spot (FILE * f, RndfSpot * spot);
static int
parse_perimeter (FILE * f, RndfZone * zone);
static int
parse_route_pass2 (FILE * f, RndfRouteNetwork * route);
static int
parse_segment_pass2 (FILE * f, RndfSegment * segment);
static int
parse_lane_pass2 (FILE * f, RndfLane * lane);
static int
parse_zone_pass2 (FILE * f, RndfZone * zone);
static int
parse_spot_pass2 (FILE * f, RndfSpot * spot);
static int
parse_perimeter_pass2 (FILE * f, RndfZone * zone);

/* print out a mission in a MDF file */
int
mdf_print ( RndfMission *mission, char *filename ) 
{
    FILE *f = fopen( filename, "w" );

    if ( !mission || !f )
        return -1;

    fprintf( f, "MDF_name\t%s\n", mission->name);
    fprintf( f, "RNDF\t%s\n", mission->route_name);
    if ( mission->format_version )
        fprintf(f, "format_version\t%s\n", mission->format_version);
    if ( mission->creation_date )
        fprintf(f, "creation_date\t%s\n", mission->creation_date );
    
    // print the checkpoints
    fprintf(f, "checkpoints\n");
    fprintf(f, "num_checkpoints\t%d\n", mission->num_checkpoints);
    int i;
    for (i=0;i<mission->num_checkpoints;i++) 
        fprintf(f, "%d\n", mission->checkpoint_ids[i]);
    fprintf(f, "end_checkpoints\n");

    // print out speed limits
    fprintf(f, "speed_limits\n");
    fprintf(f, "num_speed_limits\t%d\n", mission->num_speed_limits);
    for (i=0;i<mission->num_speed_limits;i++) {
        RndfSpeedlimit *sl = &mission->speed_limits[i];
        fprintf(f, "%d\t%d\t%d\n", sl->id, sl->min_speed, sl->max_speed);
    }
    fprintf(f, "end_speed_limits\n");
    fprintf(f, "end_file\n");

    fclose (f );

    return 0;
}

    
/* print out the content of a mission */
void
mdf_printf ( RndfMission *mission )
{
    if ( !mission )
        return;

    int i=0;

    printf("RndfMission name: %s\n", mission->name);
    printf("Mission route name: %s\n", mission->route_name );
    printf("Mission format version: %s\n", mission->format_version);
    printf("Mission creation date: %s\n", mission->creation_date);
    printf("Mission # checkpoints: %d\n", mission->num_checkpoints);
    printf("Mission # speed limits: %d\n", mission->num_speed_limits);
    for (i=0;i<mission->num_speed_limits;i++) {
        printf("   speed limit %d: point %d min = %d max = %d\n", i, mission->speed_limits[i].id,\
                mission->speed_limits[i].min_speed, mission->speed_limits[i].max_speed);
    }
}

void
rndf_printf ( RndfRouteNetwork *route )
{
    if ( !route )
	return;

    printf ("RndfRoute name: %s\n", route->name);
    printf ("RndfRoute format version: %s\n", route->format_version);
    printf ("Route creation date: %s\n", route->creation_date);
    printf ("Route # segments: %d\n", route->num_segments);
    printf ("Route # waypoints: %d\n", route->num_checkpoints);
    printf ("Route # zones: %d\n", route->num_zones);
}

int
boundary_print ( FILE *f, RndfBoundaryType type )
{
  if ( !f )
    return -1;
  
  switch ( type ) {
  case RNDF_BOUND_DBL_YELLOW:
    fprintf(f, "double_yellow\n");
    break;
  case RNDF_BOUND_SOLID_WHITE:
    fprintf(f, "solid_white\n");
    break;
  case RNDF_BOUND_BROKEN_WHITE:
    fprintf(f, "broken_white\n");
    break;
  case RNDF_BOUND_SOLID_YELLOW:
    fprintf(f, "solid_yellow\n");
    break;
  default:
    fprintf(stderr, "unknown boundary type in boundary_print.\n");
    return -1;
  }
  return 0;
}

/* print a lane */
int
lane_print ( FILE *f, RndfLane *l )
{
  if ( !f )
    return -1;

  int segment_id = l->parent->id;

  fprintf(f, "lane\t%d.%d\n",segment_id,l->id);
  fprintf(f, "num_waypoints\t%d\n", l->num_waypoints);
  if ( l->lane_width != -1 )
    fprintf(f, "lane_width\t%d\n", l->lane_width);
  if ( l->left_boundary != RNDF_BOUND_UNSPECIFIED ) {
  fprintf(f, "left_boundary\t"); boundary_print(f, l->left_boundary);
  }
  if ( l->right_boundary != RNDF_BOUND_UNSPECIFIED ) {
  fprintf(f, "right_boundary\t"); boundary_print( f, l->right_boundary);
  }
  
  // print out checkpoints with same segment and same lane
  int i=0,j=0;
  for (i=0;i<l->parent->parent->num_checkpoints;i++) {
    RndfCheckpoint checkpoint = l->parent->parent->checkpoints[i];
    if ( checkpoint.waypoint->type == RNDF_POINT_WAYPOINT ) {
      if ( checkpoint.waypoint->parent.lane->id == l->id ) {
	if ( checkpoint.waypoint->parent.lane->parent->id == segment_id ) {
	  RndfWaypoint *w = checkpoint.waypoint;
	  fprintf(f, "checkpoint\t%s\t%d\n", rndf_get_waypoint_str(w), checkpoint.id);
	}
      }
    } 
  }
  // print out stops
  for (i=0;i<l->num_waypoints;i++) {
    if ( l->waypoints[i].is_stop )
      fprintf(f, "stop\t%d.%d.%d\n", segment_id, l->id, l->waypoints[i].id);
  }
  // print out exits
  for (i=0;i<l->num_waypoints;i++) {
    RndfWaypoint exit = l->waypoints[i];
    int num_exits = exit.num_exits;
    for (j=0;j<num_exits;j++) {
      RndfWaypoint *entry = exit.exits[j];
      fprintf(f, "exit\t%d.%d.%d\t%s\n", segment_id, l->id, exit.id, rndf_get_waypoint_str(entry));
    }
  }
  // print out waypoints
  for (i=0;i<l->num_waypoints;i++) {
      if ( l->waypoints[i].dense == 1 )
          fprintf(f, "%d.%d.%d\t%.6f\t%.6f dense\n", segment_id, l->id, l->waypoints[i].id, \
                  l->waypoints[i].lat, l->waypoints[i].lon);
      else
          fprintf(f, "%d.%d.%d\t%.6f\t%.6f\n", segment_id, l->id, l->waypoints[i].id, \
                  l->waypoints[i].lat, l->waypoints[i].lon);
          
  }
  fprintf(f, "end_lane\n");
  return 0;
}

/* print out a spot */
int
spot_print ( FILE *f, RndfSpot *spot )
{
  if ( !f || !spot )
    return -1;

  fprintf( f, "spot\t%d.%d\n", spot->parent->id, spot->id);
  if ( spot->spot_width != -1 )
    fprintf( f, "spot_width\t%d\n", spot->spot_width);
  RndfWaypoint w1,w2;
  w1 = spot->waypoints[0];
  w2 = spot->waypoints[1];
  
  // print checkpoint (by convention, it is the second waypoint of the spot)
  if ( spot->checkpoint_id != -1)
  fprintf(f, "checkpoint\t%d.%d.2\t%d\n", spot->parent->id, spot->id, spot->checkpoint_id);

  // print the two waypoints
  fprintf(f, "%d.%d.1\t%.6f\t%.6f\n", spot->parent->id, spot->id, w1.lat, w1.lon);
  fprintf(f, "%d.%d.2\t%.6f\t%.6f\n", spot->parent->id, spot->id, w2.lat, w2.lon);
  
  fprintf(f, "end_spot\n");

  return 0;
}
    
/* print out a zone */
int
zone_print ( FILE *f, RndfZone *zone )
{
  if ( !f || !zone )
    return -1;

  fprintf(f, "zone\t%d\n", zone->id );
  fprintf(f, "num_spots\t%d\n", zone->num_spots);
  if ( zone->name )
    fprintf(f, "zone_name\t%s\n", zone->name);
  int i,j;
  // print the perimeter
  if ( zone->num_peripoints > 0 ) {
    fprintf(f, "perimeter\t%d.0\n", zone->id);
    fprintf(f, "num_perimeterpoints\t%d\n", zone->num_peripoints);
    // first print the exits
    for (i=0;i<zone->num_peripoints;i++) {
      RndfWaypoint exit = zone->peripoints[i];
      for (j=0;j<exit.num_exits;j++) {
	RndfWaypoint *entry = exit.exits[j];
	fprintf(f, "exit\t%d.0.%d\t%s\n", zone->id, exit.id, rndf_get_waypoint_str(entry));
      }
    }
    // then print the peripoints
    for (i=0;i<zone->num_peripoints;i++) {
      RndfWaypoint w = zone->peripoints[i];
      fprintf(f, "%d.0.%d\t%.6f\t%.6f\n", zone->id, w.id, w.lat, w.lon);
    }
    fprintf(f, "end_perimeter\n");
  }

  // print the spots
  for (i=0;i<zone->num_spots;i++) {
    spot_print( f, &zone->spots[i] );
  }

  fprintf(f, "end_zone\n");

  return 0;
}

/* print out an obstacle */
int
obstacle_print( FILE *f, RndfObstacle *o )
{
    if ( !f )
        return -1;

    fprintf( f, "%d %f %f %f %f %f %f\n", o->id, o->lat, o->lon, o->w1, o->w2, o->height, o->orient );

    return 0;
}

/* read in an obstacle */
int
obstacle_read ( FILE *f, RndfObstacle *o )
{
    if ( !f )
        return -1;

    int id;
    double lat, lon, w1, w2, height, orient;
    if ( fscanf( f, "%d%lf%lf%lf%lf%lf%lf\n", &id, &lat, &lon, &w1, &w2, &height, &orient ) != 7 )
        return -1;

    o->id = id;
    o->lat = lat;
    o->lon = lon;
    o->w1 = w1;
    o->w2 = w2;
    o->height = height;
    o->orient = orient;
    
    return 0;
}

/* print out the route in a standard RNDF file */
int
rndf_print ( RndfRouteNetwork *route, char *filename )
{
    FILE *f = fopen( filename, "w" );

    if ( !route || !f )
        return -1;

    // print out the SVN header
    fprintf(f, "/* $LastChangedDate$ */\n");
    fprintf(f, "/* $LastChangedRevision$ */\n");
    fprintf(f, "/* $LastChangedBy$ */\n");
    fprintf(f, "\n");

    // print out the header
    fprintf(f, "RNDF_name\t%s\n", route->name);
    fprintf(f, "num_segments\t%d\n", route->num_segments);
    fprintf(f, "num_zones\t%d\n", route->num_zones);


  if ( route->format_version )
    fprintf(f, "format_version\t%s\n", route->format_version);
  if (route->creation_date)
    fprintf(f, "creation_date\t%s\n", route->creation_date);
  int i=0,j=0;

  // print out the segments
  for (i=0;i<route->num_segments;i++) {
    RndfSegment s = route->segments[i];
    fprintf(f, "segment\t%d\n", s.id);
    fprintf(f, "num_lanes\t%d\n",s.num_lanes);
    if ( s.name )
      fprintf(f, "segment_name\t%s\n", s.name);
    for (j=0;j<s.num_lanes;j++) {
      lane_print ( f, &s.lanes[j] );
    }
    fprintf(f, "end_segment\n");
  }

  // print out the zones
  for (i=0;i<route->num_zones;i++) {
    zone_print( f, &route->zones[i] );
  }

  // print out the obstacles
  if ( route->num_obstacles > 0 ) {
      fprintf( f, "num_obstacles\t%d\n", route->num_obstacles);
      for (i=0;i<route->num_obstacles;i++) {
          obstacle_print( f, &route->obstacles[i] );
      }
  }

  fprintf(f, "end_file\n");

  fclose ( f );

  return 0;
}

typedef enum {
    BYTE_SOURCE_FILE,
    BYTE_SOURCE_MEMORY,
} ByteSourceType;

typedef struct _ByteSource {
    ByteSourceType type;
    union {
        FILE * file;
        struct {
            uint8_t * data;
            int offset, length;
        } mem;
    };
    int lineno;
} ByteSource;

#define MIN(a,b)    ((a) < (b) ? (a) : (b))

static char *source_get_next_line (ByteSource * bs)
{
    if (bs->type == BYTE_SOURCE_FILE) {
        if (fgets (linebuf, MAXLINE, bs->file)) {
            bs->lineno++;
            return linebuf;
        }
        else
            return NULL;
    }
    else if (bs->type == BYTE_SOURCE_MEMORY) {
        if (bs->mem.offset >= bs->mem.length)
            return NULL;
        uint8_t * d = bs->mem.data + bs->mem.offset;
        uint8_t * a = memchr (d, '\n', bs->mem.length - bs->mem.offset);
        int copylen;
        if (!a)
            copylen = bs->mem.length - bs->mem.offset;
        else
            copylen = a - d + 1;
        strncpy (linebuf, (char *) d, MIN (MAXLINE, copylen));
        linebuf[MIN (copylen, MAXLINE-1)] = '\0';
        bs->mem.offset += copylen;
        bs->lineno++;
        return linebuf;
    }
    return NULL;
}

/* Parsing begins with this function */
static int
mdf_load_file_internal (ByteSource * bs, RndfMission *mission)
{
    printf("Loading mission...\n");

    char * l;
    
    int mode = 0; // 1: checkpoints, 2: speed limits
    
    int num_speed = 0;
    int num_check = 0;

    mission->valid = 0;

    while ((l = source_get_next_line (bs))) {
        if ( strlen(l) < 2 ) 
            continue;

        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);

        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "MDF_name") ) {
            if (!arg)
                fprintf (stderr, "Warning: MDF_name is empty\n");
            else {
                mission->name = strdup (arg);
                printf("mission name: %s\n", mission->name);
            }
        }
        else if (!strcmp (cmd, "RNDF") ) {
            if ( !arg )
                fprintf(stderr, "Warning: MDF route name is empty\n");
            else {
                mission->route_name = strdup (arg);
            }
        }
        else if (mode == 1 && !strcmp (cmd, "num_checkpoints") ) {
            char * endptr;
            if (!arg || mission->num_checkpoints) {
                parse_error ("Invalid num_checkpoints");
                break;
            }
            mission->num_checkpoints = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_checkpoints \"%s\"", arg);
                break;
            }
            mission->checkpoints = malloc (mission->num_checkpoints * sizeof (RndfCheckpoint));
            memset (mission->checkpoints, 0, mission->num_checkpoints * sizeof (RndfCheckpoint));
            mission->checkpoint_ids = malloc ( mission->num_checkpoints * sizeof (int) );
            memset (mission->checkpoint_ids, 0, mission->num_checkpoints * sizeof(int));

            //dbgl(DBG_MP, "num checkpoints: %d\n", mission->num_checkpoints);

        }
        else if (mode == 2 && !strcmp (cmd, "num_speed_limits") ) {
            char * endptr;
            if (!arg || mission->num_speed_limits) {
                parse_error ("Invalid num_speed_limits");
                break;
            }
            mission->num_speed_limits = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_speed_limits \"%s\"", arg);
                break;
            }
            mission->speed_limits = malloc (mission->num_speed_limits * sizeof (RndfSpeedlimit));
            memset (mission->speed_limits, 0, mission->num_speed_limits * sizeof (RndfSpeedlimit));

        }
        else if (!strcmp (cmd, "format_version") ) {
            if (!arg)
                fprintf (stderr, "Warning: format_version is empty\n");
            else {
                mission->format_version = strdup (arg);

            }
        }
        else if (!strcmp (cmd, "creation_date") ) {
            if (!arg)
                fprintf (stderr, "Warning: creation_date is empty\n");
            else {
                mission->creation_date = strdup (arg);
            }
        }
        else if (!strcmp(cmd,"checkpoints")) {
            mode = 1;
        }
        else if (!strcmp(cmd,"end_checkpoints")) {
            mode = 0;
        }
        else if (!strcmp(cmd,"speed_limits")) {
            mode = 2;
        }
        else if (!strcmp(cmd,"end_speed_limits")) {
            mode = 0;
        }
        else if (mode == 1) {
            char *endptr;
            int checkpoint_id = strtol(cmd, &endptr, 10);
            if (endptr == cmd || (*endptr && !isspace (*endptr)) || num_check >= mission->num_checkpoints) {
                fprintf(stderr, "Error reading checkpoint ID %d\n", checkpoint_id);
                break;
            }
            
            mission->checkpoint_ids[num_check] = checkpoint_id;
            num_check++;
        }
        else if ( mode == 2) {
            char *endptr;
            int speed_id = strtol(cmd, &endptr, 10);
            if (endptr == cmd || (*endptr && !isspace (*endptr)) || num_speed >= mission->num_speed_limits) {
                parse_error ("Error reading speed ID \"%s\"", cmd);
                break;
            }

            double min_speed = strtol(arg, &endptr, 10);

            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Error reading speed ID \"%s\"", arg);
                break;
            }

            char *arg2 = strsep (&l, delimeters);

            double max_speed = strtol(arg2, &endptr, 10);

            RndfSpeedlimit sl;
            sl.id = speed_id;
            sl.min_speed = (int)min_speed;
            sl.max_speed = (int)max_speed;
            mission->speed_limits[num_speed] = sl;
            num_speed++;
        }
        else if (mode==0  && strncmp (cmd, "end_file",8)==0 ) {

            if ( num_speed != mission->num_speed_limits ) {
                fprintf( stderr, "missing speed limits in MDF file.\n");
            } else {
                printf("... mission ok\n");
                mission->valid = 1;
                return 0;
            }
        }
        else {
            printf ("Unknown command %s. (mode = %d)", cmd, mode);
            break;
        }
    }
    
    fprintf (stderr, "Premature end of mission definition file, aborting...\n");
    return -1;
}

int
mdf_load_file (FILE * f, RndfMission * mission)
{
    ByteSource bs;
    bs.type = BYTE_SOURCE_FILE;
    bs.file = f;
    bs.lineno = 0;
    return mdf_load_file_internal (&bs, mission);
}

int
mdf_load_memory (uint8_t * data, int length, RndfMission * mission)
{
    ByteSource bs;
    bs.type = BYTE_SOURCE_MEMORY;
    bs.mem.data = data;
    bs.mem.offset = 0;
    bs.mem.length = length;
    bs.lineno = 0;
    return mdf_load_file_internal (&bs, mission);
}

static int
parse_segment (FILE * f, RndfSegment * segment)
{
    char * l;
    int lane_num = 0;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);
        
        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "segment_name")) {
            if (!arg)
                fprintf (stderr, "Warning: segment_name is empty\n");
            else
                segment->name = strdup (arg);
        }
        else if (!strcmp (cmd, "num_lanes")) {
            char * endptr;
            if (!arg || segment->num_lanes) {
                parse_error ("Invalid num_lanes in segment %d", segment->id);
                break;
            }
            segment->num_lanes = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_lanes \"%s\" in segment %d",
                        arg, segment->id);
                break;
            }
            segment->lanes = malloc (segment->num_lanes * sizeof (RndfLane));
            memset (segment->lanes, 0, segment->num_lanes * sizeof (RndfLane));
        }
        else if (!strcmp (cmd, "lane")) {
            RndfLane * lane = segment->lanes + lane_num;
            int id, seg_id, n;
            if (!arg || lane_num >= segment->num_lanes) {
                parse_error ("Invalid lane in segment %d", segment->id);
                break;
            }
            n = sscanf (arg, "%d.%d", &seg_id, &id);
            if (n != 2 || seg_id != segment->id || lane_num != id - 1) {
                parse_error ("Invalid lane id \"%s\" in segment %d",
                        arg, segment->id);
                break;
            }
            lane->id = id;
            lane->parent = segment;
            if (parse_lane (f, lane) < 0)
                break;
            lane_num++;
        }
        else if (!strcmp (cmd, "end_segment")) {
            if (lane_num != segment->num_lanes) {
                parse_error ("Segment %d has too few lanes", segment->id);
                break;
            }
            return 0;
        }
        else {
            parse_error ("Unknown command %s", cmd);
            break;
        }
    }

    return -1;
}

static int
parse_lane (FILE * f, RndfLane * lane)
{
    char * l;
    int waypoint_num = 0;
    int seg_id, lane_id, id;

    lane->lane_width = -1;
    lane->left_boundary = RNDF_BOUND_UNSPECIFIED;
    lane->right_boundary = RNDF_BOUND_UNSPECIFIED;

    while ((l = get_next_line (f))) {

        if (l == NULL)
            continue;

        char dense = 0;
        if (strstr(l,"dense") != NULL) { // a special character for dense waypoints
            dense = 1;
        }
        
        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);

        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "num_waypoints")) {
            char * endptr;
            if (!arg || lane->num_waypoints) {
                parse_error ("Invalid num_waypoints in lane %s", lane_str (lane));
                break;
            }
            lane->num_waypoints = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_waypoints \"%s\" in lane %s",
                        arg, lane_str (lane));
                break;
            }
            lane->waypoints = malloc (lane->num_waypoints * sizeof (RndfWaypoint));
            memset (lane->waypoints, 0, lane->num_waypoints * sizeof (RndfWaypoint));
        }
        else if (!strcmp (cmd, "lane_width")) {
            char * endptr;
            if (!arg) {
                parse_error ("Invalid lane_width in lane %s", lane_str (lane));
                break;
            }
            lane->lane_width = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid lane_width \"%s\" in lane %s",
                        arg, lane_str (lane));
                break;
            }
        }
        else if (!strcmp (cmd, "left_boundary") ||
                !strcmp (cmd, "right_boundary")) {
            RndfBoundaryType type = 0;
            if (!arg) {
                parse_error ("Invalid %s in lane %s", cmd, lane_str (lane));
                break;
            }
            if (!strcmp (arg, "double_yellow"))
                type = RNDF_BOUND_DBL_YELLOW;
            else if (!strcmp (arg, "solid_white"))
                type = RNDF_BOUND_SOLID_WHITE;
            else if (!strcmp (arg, "broken_white"))
                type = RNDF_BOUND_BROKEN_WHITE;
            else if (!strcmp (arg, "solid_yellow"))
                type = RNDF_BOUND_SOLID_YELLOW;
            else {
                parse_error ("Invalid boundary \"%s\" in lane %s",
                        arg, lane_str (lane));
                break;
            }
            if (!strcmp (cmd, "left_boundary"))
                lane->left_boundary = type;
            else
                lane->right_boundary = type;
        }
        else if (sscanf (cmd, "%d.%d.%d", &seg_id, &lane_id, &id) == 3) {
            double lat, lon;
            RndfWaypoint * waypoint = lane->waypoints + waypoint_num;
            if (seg_id != lane->parent->id || lane_id != lane->id ||
                    waypoint_num >= lane->num_waypoints ||
                    waypoint_num != id - 1) {
                parse_error ("Invalid waypoint \"%s\" in lane %s",
                        cmd, lane_str (lane));
                break;
            }
            if (sscanf (arg, "%lf", &lat) != 1 || sscanf (l, "%lf", &lon) != 1) {
                parse_error ("Invalid waypoint \"%s\" in lane %s",
                        cmd, lane_str (lane));
                break;
            }
            waypoint->id = id;
            waypoint->type = RNDF_POINT_WAYPOINT;
            waypoint->parent.lane = lane;
            waypoint->lat = lat;
            waypoint->lon = lon;
            waypoint->dense = dense;
            waypoint_num++;
            
            //printf("read waypoint %d.%d.%d %.6f %.6f\n", lane->parent->id, lane->id, id, lat, lon);

        }
        else if (!strcmp (cmd, "checkpoint") || !strcmp (cmd, "stop") ||
                !strcmp (cmd, "exit")) {
            /* skip these commands -- will be processed during 2nd pass */
        }
        else if (!strcmp (cmd, "end_lane")) {
            if (waypoint_num != lane->num_waypoints) {
                parse_error ("Lane %s has too few waypoints", lane_str (lane));
                break;
            }
            return 0;
        }
        else {
            parse_error ("Unknown command %s", cmd);
            break;
        }
    }

    return -1;
}

static int
parse_zone (FILE * f, RndfZone * zone)
{
    char * l;
    int spot_num = 0;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);
        
        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "zone_name")) {
            if (!arg)
                fprintf (stderr, "Warning: zone_name is empty\n");
            else
                zone->name = strdup (arg);
        }
        else if (!strcmp (cmd, "num_spots")) {
            char * endptr;
            if (!arg || zone->num_spots) {
                parse_error ("Invalid num_spots in zone %d", zone->id);
                break;
            }
            zone->num_spots = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_spots \"%s\" in zone %d",
                        arg, zone->id);
                break;
            }
            //printf("ALLOCATING %d spots\n", zone->num_spots);
            zone->spots = malloc (zone->num_spots * sizeof (RndfSpot));
            memset (zone->spots, 0, zone->num_spots * sizeof (RndfSpot));
        }
        else if (!strcmp (cmd, "spot")) {
            RndfSpot * spot = zone->spots + spot_num;
            int id, zone_id, n;
            if (!arg || spot_num >= zone->num_spots) {
                parse_error ("Invalid spot in zone %d", zone->id);
                break;
            }
            n = sscanf (arg, "%d.%d", &zone_id, &id);
            if (n != 2 || zone_id != zone->id) {
                parse_error ("Invalid spot id \"%s\" in zone %d",
                        arg, zone->id);
                break;
            }
            spot->id = id;
            spot->parent = zone;
            if (parse_spot (f, spot) < 0) {
                fprintf(stderr, "failed to parse spot %d\n", id);
                break;
            }
            spot_num++;
        }
        else if (!strcmp (cmd, "perimeter")) {
            int id, zone_id, n;
            if (!arg || zone->num_peripoints) {
                parse_error ("Invalid perimeter in zone %d", zone->id);
                break;
            }
            n = sscanf (arg, "%d.%d", &zone_id, &id);
            if (n != 2 || zone_id != zone->id || id != 0) {
                parse_error ("Invalid perimeter id \"%s\" in zone %d",
                        arg, zone->id);
                break;
            }
            if (parse_perimeter (f, zone) < 0)
                break;
        }
        else if (strncmp (cmd, "end_zone",8)==0) {
            if (spot_num != zone->num_spots) {
                parse_error ("Zone %d has too few spots", zone->id);
                break;
            }
            if (zone->num_peripoints <= 0) {
                parse_error ("Zone %d is missing perimeter", zone->id);
                break;
            }
            return 0;
        }
        else {
            parse_error ("Unknown command %s", cmd);
            break;
        }
    }

    return -1;
}

static int
parse_spot (FILE * f, RndfSpot * spot)
{
    char * l;
    int waypoint_num = 0;
    int num_waypoints = 2;
    int zone_id, spot_id, id;
    spot->spot_width = -1;

    while ((l = get_next_line (f))) {

        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);
        
        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "spot_width")) {
            char * endptr;
            if (!arg) {
                parse_error ("Invalid spot_width in spot %s", spot_str (spot));
                break;
            }
            spot->spot_width = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid spot_width \"%s\" in spot %s",
                        arg, spot_str (spot));
                break;
            }
        }
        else if (sscanf (cmd, "%d.%d.%d", &zone_id, &spot_id, &id) == 3) {
            double lat, lon;
            RndfWaypoint * waypoint = spot->waypoints + waypoint_num;
            if (zone_id != spot->parent->id || spot_id != spot->id ||
                    waypoint_num >= num_waypoints) {
                parse_error ("Invalid waypoint \"%s\" in spot %s",
                        cmd, spot_str (spot));
                break;
            }
            if (sscanf (arg, "%lf", &lat) != 1 || sscanf (l, "%lf", &lon) != 1) {
                parse_error ("Invalid waypoint \"%s\" in spot %s",
                        cmd, spot_str (spot));
                break;
            }
            waypoint->id = id;
            waypoint->type = RNDF_POINT_SPOT;
            waypoint->parent.spot = spot;
            waypoint->lat = lat;
            waypoint->lon = lon;
            waypoint->dense = 0;
            waypoint_num++;
        }
        else if (!strcmp (cmd, "checkpoint")) {
            /* skip these commands -- will be processed during 2nd pass */
        }
        else if (strncmp (cmd, "end_spot",8)==0 ) {
            if (waypoint_num != num_waypoints) {
                parse_error ("Spot %s has too few waypoints", spot_str (spot));
                break;
            }
            return 0;
        }
        else {
            parse_error ("iUnknown command %s", cmd);
            break;
        }
    }

    return -1;
}

static int
parse_perimeter (FILE * f, RndfZone * zone)
{
    char * l;
    int peripoint_num = 0;
    int zone_id, id;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);
        
        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "num_perimeterpoints")) {
            char * endptr;
            if (!arg || zone->num_peripoints) {
                parse_error ("Invalid num_perimeterpoints in zone %d",
                        zone->id);
                break;
            }
            zone->num_peripoints = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_perimeterpoints \"%s\" in zone %d",
                        arg, zone->id);
                break;
            }
            zone->peripoints = malloc (zone->num_peripoints * sizeof (RndfWaypoint));
            memset (zone->peripoints, 0, zone->num_peripoints * sizeof (RndfWaypoint));
        }
        else if (sscanf (cmd, "%d.0.%d", &zone_id, &id) == 2) {
            double lat, lon;
            RndfWaypoint * peripoint = zone->peripoints + peripoint_num;
            if (zone_id != zone->id || peripoint_num >= zone->num_peripoints) {
                parse_error ("Invalid perimeter point \"%s\" in zone %d",
                        cmd, zone->id);
                break;
            }
            if (sscanf (arg, "%lf", &lat) != 1 || sscanf (l, "%lf", &lon) != 1) {
                parse_error ("Invalid perimeter point \"%s\" in zone %d",
                        cmd, zone->id);
                break;
            }
            peripoint->id = id;
            peripoint->type = RNDF_POINT_PERIMETER;
            peripoint->parent.zone = zone;
            peripoint->lat = lat;
            peripoint->lon = lon;
            peripoint->dense = 0;
            peripoint_num++;

            //printf("peripoint %d.0.%d %.6f %.6f\n", zone->id, id, lat, lon);

        }
        else if (!strcmp (cmd, "exit")) {
            /* skip these commands -- will be processed during 2nd pass */
        }
        else if (!strcmp (cmd, "end_perimeter")) {
            if (peripoint_num != zone->num_peripoints) {
                parse_error ("Zone %d has too few perimeter points", zone->id);
                break;
            }
            return 0;
        }
        else {
            parse_error ("Unknown command %s", cmd);
            break;
        }
    }

    return -1;
}

int
parse_route_pass2 (FILE * f, RndfRouteNetwork * route)
{
    char * l;
    int segment_num = 0;
    int zone_num = 0;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        
        if (!cmd || !strlen(cmd))
            continue;

        if (!strcmp (cmd, "segment")) {
            RndfSegment * segment = route->segments + segment_num;
            if (parse_segment_pass2 (f, segment) < 0)
                break;
            segment_num++;
        }
        else if (!strcmp (cmd, "zone")) {
            RndfZone * zone = route->zones + zone_num;
            if (parse_zone_pass2 (f, zone) < 0)
                break;
            zone_num++;
        }
        else if (strncmp (cmd, "end_file",8)==0 ) {
            route->valid = 1;
            printf("ok\n");
            return 0;
        }
    }

    fprintf (stderr, "Premature end of route network definition file, aborting...\n");
    return -1;
}

static int
parse_segment_pass2 (FILE * f, RndfSegment * segment)
{
    char * l;
    int lane_num = 0;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);

        if (!cmd || !strlen (cmd))
            continue;
        
        if (!strcmp (cmd, "lane")) {
            RndfLane * lane = segment->lanes + lane_num;
            if (parse_lane_pass2 (f, lane) < 0)
                break;
            lane_num++;
        }
        else if (!strcmp (cmd, "end_segment")) {
            return 0;
        }
    }

    return -1;
}

static RndfWaypoint *
get_local_waypoint_by_id (RndfWaypoint * waypoints, int num, int id)
{
    int i;
    for (i = 0; i < num; i++) {
        if (waypoints[i].id == id)
            return waypoints + i;
    }
    return NULL;
}

int
rndf_add_checkpoint (RndfRouteNetwork * route, int id, RndfWaypoint * waypoint)
{
    // check that this ID is not in used already
    int i;
    for (i=0;i<route->num_checkpoints;i++) {
        RndfCheckpoint *cp = route->checkpoints + i;
        if ( cp->id == id ) {
            printf("Checkpoint ID %d already exists!\n", id);
            return -1;
        }
    }
                
    int n = route->num_checkpoints;
    RndfCheckpoint * checkpoint;

    route->checkpoints = realloc (route->checkpoints,
            (n + 1) * sizeof (RndfCheckpoint));

    checkpoint = route->checkpoints + n;
    checkpoint->id = id;
    checkpoint->waypoint = waypoint;

    // we do not check that <id> is greater than the current max_checkpoint_id
    // since the checkpoints are added in random order when the RNDF is read
    // (e.g. parking spots are read last and may have a smaller ID than some
    //  segment checkpoints).

    if ( id > route->max_checkpoint_id ) {
      route->max_checkpoint_id = id;
    }

    route->num_checkpoints++;
    return 0;
}

static int
add_exit (RndfWaypoint * exit, RndfWaypoint * entry)
{
    int n = exit->num_exits;
    
    //printf("adding exit between %d and %d\n", exit->id, entry->id);
    exit->exits = realloc (exit->exits, (n + 1) * sizeof (RndfWaypoint *));

    exit->exits[n] = entry;
    exit->num_exits++;
    return 0;
}

static int
parse_lane_pass2 (FILE * f, RndfLane * lane)
{
    char * l;
    int seg_id, lane_id, id;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        
        if (!cmd || !strlen (cmd))
            continue;

        if (!strcmp (cmd, "checkpoint")) {
            int check_id;
            RndfWaypoint * waypoint;
            if (sscanf (l, "%d.%d.%d %d", &seg_id, &lane_id, &id,
                        &check_id) != 4 ||
                    seg_id != lane->parent->id || lane_id != lane->id) {
                parse_error ("Invalid checkpoint in lane %s",
                        lane_str (lane));
                break;
            }
            waypoint = get_local_waypoint_by_id (lane->waypoints,
                    lane->num_waypoints, id);
            if (!waypoint) {
                parse_error ("1: Unknown waypoint %d.%d.%d", seg_id, lane_id, id);
                break;
            }
           if (rndf_add_checkpoint (lane->parent->parent, check_id, waypoint) < 0)
                break;
        }
        else if (!strcmp (cmd, "stop")) {
            RndfWaypoint * waypoint;
            if (sscanf (l, "%d.%d.%d", &seg_id, &lane_id, &id) != 3 ||
                    seg_id != lane->parent->id || lane_id != lane->id) {
                parse_error ("Invalid stop in lane %s",
                        lane_str (lane));
                break;
            }
            waypoint = get_local_waypoint_by_id (lane->waypoints,
                    lane->num_waypoints, id);
            if (!waypoint) {
                parse_error ("2: Unknown waypoint %d.%d.%d", seg_id, lane_id, id);
                break;
            }
            waypoint->is_stop = 1;
        }
        else if (!strcmp (cmd, "exit")) {
            RndfWaypoint * waypoint, * entry_waypoint;
            int entry_seg_id, entry_lane_id, entry_id;
            if (sscanf (l, "%d.%d.%d %d.%d.%d", &seg_id, &lane_id, &id,
                        &entry_seg_id, &entry_lane_id, &entry_id) != 6 ||
                    seg_id != lane->parent->id || lane_id != lane->id) {
                parse_error ("Invalid exit in lane %s",
                        lane_str (lane));
                break;
            }
            waypoint = get_local_waypoint_by_id (lane->waypoints,
                    lane->num_waypoints, id);
            if (!waypoint) {
                parse_error ("3: Unknown waypoint %d.%d.%d", seg_id, lane_id, id);
                break;
            }
            entry_waypoint = rndf_find_waypoint_by_id (lane->parent->parent,
                    entry_seg_id, entry_lane_id, entry_id);
            if (!entry_waypoint) {
                parse_error ("4: Unknown waypoint %d.%d.%d", entry_seg_id,
                        entry_lane_id, id);
                break;
            }
            if (add_exit (waypoint, entry_waypoint) < 0)
                break;
        }
        else if (!strcmp (cmd, "end_lane")) {
            return 0;
        }
    }

    return -1;
}

static int
parse_zone_pass2 (FILE * f, RndfZone * zone)
{
    char * l;
    int spot_num = 0;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        
        if (!cmd || !strlen (cmd))
            continue;

        if (!strcmp (cmd, "spot")) {
            RndfSpot * spot = zone->spots + spot_num;
            if (parse_spot_pass2 (f, spot) < 0) {
                fprintf(stderr, "failed to parse spot %d (pass 2)\n", spot->id);
                break;
            }
            spot_num++;
        }
        else if (!strcmp (cmd, "perimeter")) {
            if (parse_perimeter_pass2 (f, zone) < 0)
                break;
        }
        else if (strncmp (cmd, "end_zone",8)==0) {
            return 0;
        }
    }

    return -1;
}

static int
parse_spot_pass2 (FILE * f, RndfSpot * spot)
{
    char * l;
    int num_waypoints = 2;
    int zone_id, spot_id, id;

    spot->checkpoint_id = -1;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        
        if (!cmd || !strlen (cmd))
            continue;

        if (!strcmp (cmd, "checkpoint")) {
            int check_id;
            RndfWaypoint * waypoint;
            if (sscanf (l, "%d.%d.%d %d", &zone_id, &spot_id, &id,
                        &check_id) != 4 ||
                    zone_id != spot->parent->id || spot_id != spot->id) {
                parse_error ("Invalid checkpoint in spot %s",
                        spot_str (spot));
                break;
            }
            waypoint = get_local_waypoint_by_id (spot->waypoints,
                    num_waypoints, id);
            if (!waypoint) {
                parse_error ("5: Unknown waypoint %d.%d.%d", zone_id, spot_id, id);
                break;
            }
            if (rndf_add_checkpoint (spot->parent->parent, check_id, waypoint) < 0)
                break;

	    spot->checkpoint_id = check_id;

        }
        else if (strncmp (cmd, "end_spot",8)==0) {
            return 0;
        }
    }

    return -1;
}

static int
parse_perimeter_pass2 (FILE * f, RndfZone * zone)
{
    char * l;
    int zone_id, id;

    while ((l = get_next_line (f))) {
        char * cmd = strsep (&l, delimeters);
        
        if (!cmd || !strlen (cmd))
            continue;

        if (!strcmp (cmd, "exit")) {
            RndfWaypoint * waypoint, * entry_waypoint;
            int entry_seg_id, entry_lane_id, entry_id;
            if (sscanf (l, "%d.0.%d %d.%d.%d", &zone_id, &id,
                        &entry_seg_id, &entry_lane_id, &entry_id) != 5 ||
                    zone_id != zone->id) {
                parse_error ("Invalid exit in zone %d", zone->id);
                break;
            }
            waypoint = get_local_waypoint_by_id (zone->peripoints,
                    zone->num_peripoints, id);
            if (!waypoint) {
                parse_error ("Unknown perimeter point %d.0.%d", zone_id, id);
                break;
            }
            entry_waypoint = rndf_find_waypoint_by_id (zone->parent,
                    entry_seg_id, entry_lane_id, entry_id);
            if (!entry_waypoint) {
                parse_error ("6: Unknown waypoint %d.%d.%d", entry_seg_id,
                        entry_lane_id, id);
                break;
            }
            if (add_exit (waypoint, entry_waypoint) < 0)
                break;
        }
        else if (!strcmp (cmd, "end_perimeter")) {
            return 0;
        }
    }

    return -1;
}


  
/* Parsing begins with this function */
int
rndf_load_file (FILE * f, RndfRouteNetwork *route)
{
    printf( "Loading route...\n");

    if ( !f )
        return -1;

    char * l;

    int segment_num = 0;
    int zone_num = 0;

    while ((l = get_next_line (f))) {

        if ( strlen(l) < 2 )
            continue;

        char * cmd = strsep (&l, delimeters);
        char * arg = strsep (&l, delimeters);
        
        /* Skip blank lines or comments */
        if (!cmd || !strlen(cmd) ||
                (strlen(cmd) >= 2 && strncmp (cmd, "/*", 2) ==0))
            continue;

        if (arg && strlen(arg) >= 2 && strncmp(arg, "/*",2) == 0 )
            arg = NULL;

        if (!strcmp (cmd, "RNDF_name")) {
            if (!arg)
                fprintf (stderr, "Warning: RNDF_name is empty\n");
            else {
                route->name = strdup (arg);
		//printf("route name: %s (%d)\n", route->name, strlen(route->name));
	    }
        }
        else if (!strcmp (cmd, "num_segments")) {
            char * endptr;
            if (!arg || route->num_segments) {
                parse_error ("Invalid num_segments");
                break;
            }
            route->num_segments = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_segments \"%s\"", arg);
                break;
            }
            route->segments = malloc (route->num_segments * sizeof (RndfSegment));
            memset (route->segments, 0, route->num_segments * sizeof (RndfSegment));
        }
        else if (!strcmp (cmd, "num_zones")) {
            char * endptr;
            if (!arg || route->num_zones) {
                parse_error ("Invalid num_zones");
                break;
            }
            route->num_zones = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid num_zones \"%s\"", arg);
                break;
            }
            route->zones = malloc (route->num_zones * sizeof (RndfZone));
            memset (route->zones, 0, route->num_zones * sizeof (RndfZone));
        }
        else if (!strcmp (cmd, "format_version")) {
            if (!arg)
                fprintf (stderr, "Warning: format_version is empty\n");
            else
                route->format_version = strdup (arg);
        }
        else if (!strcmp (cmd, "creation_date")) {
            if (!arg)
                fprintf (stderr, "Warning: creation_date is empty\n");
            else
                route->creation_date = strdup (arg);
        }
        else if (!strcmp (cmd, "segment")) {
            RndfSegment * segment = route->segments + segment_num;
            int id;
            char * endptr;
            if (!arg || segment_num >= route->num_segments) {
                parse_error ("Invalid segment (%d >= %d)", segment_num, route->num_segments);
                break;
            }
            id = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid segment id \"%s\"", arg);
                break;
            }
            segment->id = id;
            segment->parent = route;
            if (parse_segment (f, segment) < 0) {
                fprintf(stderr, "failed to parse segment %d\n", id);
                break;
            }
            segment_num++;
        }
        else if (!strcmp (cmd, "zone")) {
            RndfZone * zone = route->zones + zone_num;
            int id;
            char * endptr;
            if (!arg || zone_num >= route->num_zones) {
                parse_error ("Invalid zone");
                break;
            }
            id = strtol (arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid zone id \"%s\"", arg);
                break;
            }

            //printf("reading zone %d\n", id);
            zone->id = id;
            zone->parent = route;
            if (parse_zone (f, zone) < 0) {
                fprintf(stderr, "failed to parse zone %d\n", id );
                break;
            }
            zone_num++;
        }
        else if (!strcmp(cmd, "num_obstacles")) {
            char *endptr;
            route->num_obstacles = strtol( arg, &endptr, 10);
            if (endptr == arg || (*endptr && !isspace (*endptr))) {
                parse_error ("Invalid number of obstacles %s", arg);
                break;
            }
            route->obstacles = (RndfObstacle*)malloc(route->num_obstacles*sizeof(RndfObstacle));
            printf("%d obstacle(s)\n", route->num_obstacles);

            int i;
            for (i=0;i<route->num_obstacles;i++) {
                RndfObstacle o;
                if ( obstacle_read( f, &o ) < 0 ) {
                    fprintf( stderr, "failed to read obstacle %d\n", i );
                    break;
                } else {
                    printf("read one obstacle %f %f\n", o.lat, o.lon);
                }
                route->obstacles[i] = o;
            }
        }   
        else if (strncmp (cmd, "end_file",8)==0 ) {
            if (segment_num != route->num_segments) {
                parse_error ("Route definition has too few segments");
                break;
            }
            if (zone_num != route->num_zones) {
                parse_error ("Route definition has too few zones");
                break;
            }

            /* Pass 1 is now complete, so start pass 2. */
            rewind (f);
            
            return parse_route_pass2 (f, route);
        }
        else {
            parse_error ("Unknown command %s", cmd);
            break;
        }
    }

    fprintf (stderr, "Premature end of route network definition file, aborting...\n");
    return -1;
}

static char outstr[32];

char *
rndf_get_waypoint_str (RndfWaypoint * waypoint)
{
    int id1, id2, id3;
    rndf_get_waypoint_id (waypoint, &id1, &id2, &id3);
    sprintf (outstr, "%d.%d.%d", id1, id2, id3);
    return outstr;
}

void
rndf_get_waypoint_id (RndfWaypoint * waypoint, int * id1, int * id2, int * id3)
{
    switch (waypoint->type) {
        case RNDF_POINT_WAYPOINT:
            *id1 = waypoint->parent.lane->parent->id;
            *id2 = waypoint->parent.lane->id;
            break;
        case RNDF_POINT_PERIMETER:
            *id1 = waypoint->parent.zone->id;
            *id2 = 0;
            break;
        case RNDF_POINT_SPOT:
            *id1 = waypoint->parent.spot->parent->id;
            *id2 = waypoint->parent.spot->id;
            break;
    }
    *id3 = waypoint->id;
}

RndfSegment *
rndf_find_segment_by_id (RndfRouteNetwork * route, int id)
{
    int i;
    for (i = 0; i < route->num_segments; i++) {
        if (route->segments[i].id == id)
            return route->segments + i;
    }
    return NULL;
}

RndfZone *
rndf_find_zone_by_id (RndfRouteNetwork * route, int id)
{
    int i;
    for (i = 0; i < route->num_zones; i++) {

        if (route->zones[i].id == id) {
           
            return route->zones + i;
        }
    }
    return NULL;
}

RndfCheckpoint *
rndf_find_checkpoint_by_id ( RndfRouteNetwork *route, int id )
{
    if ( !route )
        return NULL;

    int i;
    for (i=0;i<route->num_checkpoints;i++) {
        if ( route->checkpoints[i].id == id ) {
            return route->checkpoints + i;
        }
    }

    return NULL;
}

RndfWaypoint *
rndf_find_waypoint_by_id (RndfRouteNetwork * route, int id1, int id2, int id3)
{
    RndfSegment * segment;
    RndfZone * zone;
    int i, j;

    segment = rndf_find_segment_by_id (route, id1);
    if (segment) {
        for (i = 0; i < segment->num_lanes; i++) {
            if (segment->lanes[i].id == id2) {
                RndfLane * lane = segment->lanes + i;
                for (j = 0; j < lane->num_waypoints; j++) {
                    if (lane->waypoints[j].id == id3) {
                        return lane->waypoints + j;
                    }
                }
                fprintf(stderr, "found segment %d and lane %d but not waypoint %d\n", segment->id, lane->id, id3);
                return NULL;
            }
        }
        fprintf(stderr, "found segment %d but not lane %d\n", segment->id, id2);

        return NULL;
    }

    zone = rndf_find_zone_by_id (route, id1);
    if (zone && id2 ==0 ) {
        for (i = 0; i < zone->num_peripoints; i++) {
            if (zone->peripoints[i].id == id3) {
                return zone->peripoints + i;
            }
        }
        fprintf(stderr,"found zone %d but not waypoint %d\n", id1, id3);
        return NULL;
    }
    else if (zone && id2 > 0) {
        for (i = 0; i < zone->num_spots; i++) {
            if (zone->spots[i].id == id2) {
                RndfSpot * spot = zone->spots + i;
                if (spot->waypoints[0].id == id3)
                    return spot->waypoints;
                if (spot->waypoints[1].id == id3)
                    return spot->waypoints + 1;
                return NULL;
            }
        }
        return NULL;
    }

    fprintf(stderr,"did not find segment %d nor zone %d\n", id1, id1);

    return NULL;
}

/* reset the speed limits on a route */
int
rndf_reset_speed_limits ( RndfRouteNetwork *route )
{
    if ( !route )
        return -1;

    int i;

    // reset the speed limits for the route
    for (i=0;i<route->num_segments;i++) {
        route->segments[i].min_speed = 0;
        route->segments[i].max_speed = 0;
    }

    for (i=0;i<route->num_zones;i++) {
        route->zones[i].min_speed = 0;
        route->zones[i].max_speed = 0;
    }

    return 0;
}

int
mdf_insert_checkpoint (RndfMission * mission, int id)
{
    if (!mission)
        return -1;

    int n = mission->num_checkpoints;
    mission->checkpoint_ids = realloc (mission->checkpoint_ids,
            (n + 1) * sizeof (int));
    memmove (mission->checkpoint_ids + 1, mission->checkpoint_ids,
            n * sizeof (int));
    mission->checkpoints = realloc (mission->checkpoints,
            (n + 1) * sizeof (RndfCheckpoint *));
    memmove (mission->checkpoints + 1, mission->checkpoints,
            n * sizeof (RndfCheckpoint *));
    mission->checkpoint_ids[0] = id;
    mission->checkpoints[0] = NULL;
    mission->num_checkpoints = n + 1;
    return 0;
}

/* validate a mission against a route */
int
mdf_link_to_rndf ( RndfMission *mission, RndfRouteNetwork *route )
{
    int i;
    //fprintf(stderr, "linking mission and route...\n");
    int res = 0;

    if ( !mission || !route )
        return -1;

    if ( mission->valid != 1 )
        fprintf(stderr, "mission is not valid.\n");

    if ( route->valid != 1 )
        fprintf( stderr, "route is not valid.\n");

    // compare the route names
    if ( route->name && mission->route_name ) {
        if (strlen(route->name) != strlen(mission->route_name) || \
            strncmp( route->name, mission->route_name, strlen(route->name)) != 0) {
            fprintf(stderr, "Warning: Route name and Mission route name differ: %s, %s\n", route->name, mission->route_name);
        }
    }

    // populate the checkpoints in the mission
    // (realloc the checkpoints in case checkpoints were added or removed)
    mission->checkpoints = (RndfCheckpoint**)realloc( mission->checkpoints,
            mission->num_checkpoints * sizeof( RndfCheckpoint* ) );

    int counter=0;
    for (i=0;i<mission->num_checkpoints;i++) {
        // for each checkpoint, find the equivalent checkpoint in the route network
        RndfCheckpoint *c = rndf_find_checkpoint_by_id ( route, mission->checkpoint_ids[i] );
        if ( !c ) {
            fprintf(stderr, "could not find checkpoint id %d in the route.\n", mission->checkpoint_ids[i] );
            res = -1;
        } else {
            mission->checkpoints[counter] = c;
            counter++;
        }
    }

    mission->checkpoints = (RndfCheckpoint**)realloc(mission->checkpoints,
            counter * sizeof(RndfCheckpoint*));
    mission->num_checkpoints = counter;

    // populate the speed limits
    rndf_reset_speed_limits (route);
    for (i=0;i<mission->num_speed_limits;i++) {
        RndfSpeedlimit sl = mission->speed_limits[i];
        RndfZone * zone = rndf_find_zone_by_id ( route, sl.id );
        RndfSegment * seg = rndf_find_segment_by_id (route, sl.id);
        if ( !zone && !seg ) {
            fprintf(stderr, "Could not find segment or zone with id %d in the route.\n", sl.id );
            res = -1;
            continue;
        }

        if (zone) {
            zone->min_speed = sl.min_speed;
            zone->max_speed = sl.max_speed;
        }
        else {
            seg->min_speed = sl.min_speed;
            seg->max_speed = sl.max_speed;
        }
    }
        
    // attach route to mission
    mission->route = route;

    //fprintf(stderr, "ok.\n");

    return res;
}

/* performs sanity check of the route and write reports in the log 
 * returns: 0 if everything is fine
 *          1 if only warnings were encountered
 *          2 if at least one error was encountered
*/

int
rndf_validate ( RndfRouteNetwork *route, char *log )
{
    
    int code = 0;

    sprintf(log,"\n");

    // route exists
    if ( !route )
        return 2;

    // route has name
    if ( !route->name ) {
        code = 1;
        sprintf(log,"%sWarning: route has no name.\n",log);
    }
    
    // route has version
    if ( !route->format_version ) {
        code = 1;
        sprintf(log,"%sWarning: route has no format version.\n",log);
    }
    
    // route has creation date
    if ( !route->creation_date ) {
        code = 1;
        sprintf(log,"%sWarning: route has no creation date.\n",log);
    }
    
    // segment IDs are consecutive
    // lane IDs are consecutive
    // waypoint IDs are consecutive
    int i,j,k,m,p,q;
    for (i=0;i<route->num_segments;i++) {
        RndfSegment *s = route->segments + i;
        if ( s->id != i+1 ) {
            code = 2;
            sprintf(log,"%sError: %d-th segment has ID = %d\n", log, i+1,s->id);
        }
        for (j=0;j<s->num_lanes;j++) {
            RndfLane *l = s->lanes + j;
            if ( l->id != j+1 ) {
                code = 2;
                sprintf(log,"%sError: %d-th lane in %d-th segment has ID = %d\n", log, j+1,i+1,l->id);
            }
            
            for (k=0;k<l->num_waypoints;k++) {
                RndfWaypoint *w = l->waypoints + k;
                if ( w->type != RNDF_POINT_WAYPOINT ){
                    code = 2;
                    sprintf(log,"%sError: waypoint %d.%d.%d does not have type RNDF_POINT_WAYPOINT.\n", log, s->id, l->id, w->id);
                }
                
                if ( w->id != k+1 ) {
                    code = 2;
                    sprintf(log,"%sError: %d-th waypoint in lane %d.%d has ID = %d\n", log, \
                            k+1,s->id,l->id,w->id);
                }
                
            }
            
        }
    }
    
    // zones have consecutive IDs
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        if ( z->id != route->num_segments + i + 1 ) {
            code = 2;
            sprintf(log,"%sError: %d-th zone has ID = %d (%d segments)\n", log,i+1, z->id, route->num_segments);
        }
    }
    
    // The second waypoint of a parking spot is always a checkpoint 
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        for (j=0;j<z->num_spots;j++) {
            RndfSpot *s = z->spots + j;
            
            if ( s->waypoints[0].type != RNDF_POINT_SPOT || s->waypoints[0].type != RNDF_POINT_SPOT ) {
                code= 2;
                sprintf(log,"%sError: waypoints in spot %d.%d have wrong type!\n",log,z->id, s->id);
            }
            
            int cid = s->checkpoint_id;
            int found = 0;
            for (k=0;k<route->num_checkpoints;k++) {
                if ( route->checkpoints[k].id != cid )
                    continue;
                if ( (route->checkpoints + k)->waypoint != s->waypoints + 1 ) {
                    code = 2;
                    sprintf(log,"%sError: spot %d.%d does not have correct checkpoint.\n", log,z->id, s->id);
                } else {
                    found = 1;
                }
            }
            
            if ( !found ) {
                code = 2;
                sprintf(log,"%sError: spot %d.%d does not have a checkpoint associated to it.\n", log,z->id, s->id);
            }
        }
    }
    
    // each zone must have at least one exit and one entry point
    for (i=0;i<route->num_zones;i++) {
        RndfZone *z = route->zones + i;
        int nexits = 0;
        
        for (j=0;j<z->num_peripoints;j++) {
            
            RndfWaypoint *w = z->peripoints + j;
            if ( w->type != RNDF_POINT_PERIMETER ) {
                code = 2;
                sprintf(log,"%sError: peripoint %d in zone %d does not have type RNDF_POINT_PERIMETER.\n", log,w->id, z->id);
            }
            
            nexits += w->num_exits;
        }
        

        if ( nexits == 0 ) {
            code = 1;
            sprintf(log,"%sWarning: zone %d has no exit point!\n", log,z->id);
        }

        int ok = 0;
        for (j=0;j<route->num_segments;j++) {
            RndfSegment *s = route->segments + j;
            for (k=0;k<s->num_lanes;k++ ) {
                RndfLane *l = s->lanes + k;
                for (m=0;m<l->num_waypoints;m++) {
                    RndfWaypoint *w = l->waypoints + m;
                    for (p=0;p<w->num_exits;p++) {
                        for (q=0;q<z->num_peripoints;q++) {
                            if ( z->peripoints + q == w->exits[p] ) {
                                ok = 1;
                                break;
                            }
                        }
                        if ( ok )
                            break;
                    }
                    if ( ok )
                        break;
                }
                if ( ok )
                    break;
            }
            if ( ok )
                break;
        }
        if ( !ok ) {
            code = 1;
            sprintf(log,"%sWarning: zone %d has no entry point!\n", log,z->id);
        }
    }
    
    if ( code == 0 ) {
        sprintf(log,"%sSanity check OK on RNDF %s!\n", log,route->name);
    } 
    if ( code == 1 ) {
        sprintf(log,"%s==========================================================\n",log);
        sprintf(log,"%sThere are only warnings on this RNDF.\n",log);
    }
    if ( code == 2 ) {
        sprintf(log,"%s==========================================================\n",log);
        sprintf(log,"%sThere are errors in this RNDF!\n",log);
    }

    return code;
        
}


/* sanity check on a mission and a route 
 * returns: 0 if everything is fine
 *          1 if only warnings were encountered
 *          2 if at least one error was encountered
*/
int
mdf_validate ( RndfRouteNetwork *route, RndfMission *mission, char *log )
{
    if ( !route || !mission )
        return 0;
    
    int i,j;
    
    int code = 0;

    sprintf(log,"\n");

    // mission has name
    if ( !mission->name ) {
        code = 1;
        sprintf(log,"%sWarning: mission has no name.\n",log);
    }
    
    // mission has version
    if ( !route->format_version ) {
        code = 1;
        sprintf(log,"%sWarning: mission has no format version.\n",log);
    }
    
    // mission has creation date
    if ( !route->creation_date ) {
        code = 1;
        sprintf(log,"%sWarning: mission has no creation date.\n",log);
    }

    // route name in mission should match actual route name
    if ( strcmp(mission->route_name, route->name) ) {
        code = 1;
        sprintf(log,"%sWarning: route name differ from mission route name.\n",log);
    }
    
    // check that checkpoints are consistent
    for (i=0;i<mission->num_checkpoints;i++) {
        RndfCheckpoint *cm = mission->checkpoints[i];
        RndfWaypoint *wm = cm->waypoint;
    
        int ok = 0;
        
        for (j=0;j<route->num_checkpoints;j++) {
            RndfCheckpoint *cr = route->checkpoints + j;
            RndfWaypoint *wr = cr->waypoint;
            
            if ( cm->id != cr->id )
                continue;
            
            if ( wm != wr ) {
                code = 2;
                sprintf(log,"%sError: Inconsistent checkpoint ID = %d.\n", log, cm->id );
            } else {
                ok = 1;
            }
        }

        if  ( !ok ) {
            code = 2;
            sprintf(log,"%sMissing checkpoint %d in route.\n", log, cm->id);
        }
    }
    
    // check that all segments have speed limits

    int nr = route->num_segments + route->num_zones;
    int nm = mission->num_speed_limits;
    
    if ( nr != nm ) {
        code = 2;
        sprintf(log,"%sError: mission has %d speed limits and route has %d segments/zones.\n", log, nm, nr);
    }

    for (i=1;i<=nr;i++) {
        int ok = 0;
        
        for (j=0;j<nm;j++) {
            RndfSpeedlimit sl = mission->speed_limits[j];
            if ( sl.id == i ) {
                if (sl.min_speed >= 0 && sl.min_speed >= 0 && sl.min_speed <= sl.max_speed ) {
                    ok  = 1;
                }
            }
        }
        
        if ( !ok ) {
            code = 2;
            sprintf(log,"%sError: segment/zone %d has no valid speed limit.\n", log, i);
        }
    }
    
    if ( code == 0 ) {
        sprintf(log,"%sSanity check OK on MDF %s!\n", log,mission->name);
    } 
    if ( code == 1 ) {
        sprintf(log,"%s==========================================================\n",log);
        sprintf(log,"%sThere are only warnings on this MDF.\n",log);
    }
    if ( code == 2 ) {
        sprintf(log,"%s==========================================================\n",log);
        sprintf(log,"%sThere are errors in this MDF!\n",log);
    }

    return code;
        
}
