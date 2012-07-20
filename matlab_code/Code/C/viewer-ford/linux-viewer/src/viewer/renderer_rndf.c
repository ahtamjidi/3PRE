#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/timestamp.h>
#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/geometry.h>
#include <common/rndf_overlay.h>
#include <common/mdf.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_adu_status_t.h>
#include <lcmtypes/lcmtypes_car_summary_t.h>
#include <lcmtypes/lcmtypes_emc_feedback_t.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"

#define PARAM_NAME_OPACITY "Opacity"
#define PARAM_NAME_DRAW_WAYPOINTS "Draw Waypoints"
#define PARAM_NAME_DRAW_LANE_BOUNDARIES "Draw Lane Boundaries"
#define PARAM_NAME_DRAW_LANE_CENTERS "Draw Lane Centers"
#define PARAM_NAME_IDS "IDs"

#ifndef CONFIG_DIR
#error "CONFIG_DIR not defined!"
#endif

#ifndef METERS_PER_FOOT
#define METERS_PER_FOOT 0.3048
#endif

#define RERENDER_INTERVAL 200000

#define RENDERER_NAME "RNDF"

typedef struct _RendererRNDF RendererRNDF;

struct _RendererRNDF {
    Renderer renderer;
    EventHandler ehandler;

    Config * config;
    CTrans *ctrans;
    GtkuParamWidget    *pw;
    RndfRouteNetwork *rndf;
    RndfOverlay *rndf_overlay;
    double alpha;
    GLUquadricObj * q;
    Viewer *viewer;
    GLuint display_list;
    int show_text;

    int64_t next_rerender_time;

    GList * waypoint_cache;
    RndfOverlayWaypoint * pick_waypoint;
    RndfOverlayWaypoint * moving_waypoint;
    double last_xy[2];
};

static void renderer_rndf_draw (Viewer *viewer, Renderer *super);
static void renderer_rndf_free (Renderer *super);

typedef enum {
    SHOW_TEXT_NONE=0,
    SHOW_TEXT_WAYPOINT,
    SHOW_TEXT_CHECKPOINT,
    SHOW_TEXT_MAX
} show_text_enum_t;



static void draw_segment (RendererRNDF *self, RndfOverlaySegment *s);
static void draw_lane (RendererRNDF *self, RndfOverlayLane *la);
static void draw_zone (RendererRNDF *self, RndfOverlayZone *z);
static void draw_checkpoint (RendererRNDF *self, RndfOverlayCheckpoint *c);

#if 0
static void draw_road_line (RendererRNDF *self, 
        double *xyz_1, double *xyz_2, double lane_width, 
        RndfBoundaryType left_b, RndfBoundaryType right_b);
//static void draw_name_tag (RendererRNDF *self, RndfWaypoint *w, const char *name);
#endif
static void draw_disk (RendererRNDF *self, 
        double xyz[3], double r_in, double r_out);
static void draw_waypoint_xyz (RendererRNDF *self, double *xyz, 
        RndfPointType type, double r);
static void draw_stop_xyz (RendererRNDF *self, double *xyz, double r);
static void draw_waypoint (RendererRNDF *self, RndfOverlayWaypoint *w, double r);
static void draw_exit (RendererRNDF *self, RndfOverlayWaypoint *w1, 
        RndfOverlayWaypoint *w2, GList * yields);
static void draw_arrowhead (double *mid, double *dir, double size);
static void draw_perimeter_exit_xyz (RendererRNDF *self, double *xyz);
static void draw_obstacle (RendererRNDF *self, RndfObstacle *obstacle);
static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        void *user_data);
static void _do_draw (RendererRNDF *self);
static double pick_query(Viewer *viewer, EventHandler *ehandler,
        const double ray_start[3], const double ray_dir[3]);
static void free_waypoint_cache (RendererRNDF * self);

static int mouse_press (Viewer *viewer, EventHandler *ehandler,
        const double ray_start[3], const double ray_dir[3], 
        const GdkEventButton *event);
static int mouse_release(Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventButton *event);
static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventMotion *event);

static void
on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererRNDF *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void
on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererRNDF *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

RendererRNDF *
renderer_rndf_new (Viewer *viewer)
{
    RendererRNDF *self = 
        (RendererRNDF*) calloc(1, sizeof(RendererRNDF));
    self->viewer = viewer;

    self->renderer.draw = renderer_rndf_draw;
    self->renderer.destroy = renderer_rndf_free;
    self->renderer.name = RENDERER_NAME;
    self->renderer.user = self;
    self->renderer.enabled = 1;
    self->renderer.widget = gtk_alignment_new (0, 0.5, 1.0, 0);


    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    gtk_container_add (GTK_CONTAINER (self->renderer.widget), 
            GTK_WIDGET(self->pw));
    gtk_widget_show (GTK_WIDGET (self->pw));

    gtku_param_widget_add_double (self->pw, 
            PARAM_NAME_OPACITY, 
            GTKU_PARAM_WIDGET_SLIDER, 0, 1, 0.05, 0.3);
    gtku_param_widget_add_booleans (self->pw, 0, 
            PARAM_NAME_DRAW_WAYPOINTS, 1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, 
            PARAM_NAME_DRAW_LANE_BOUNDARIES, 0, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, 
            PARAM_NAME_DRAW_LANE_CENTERS, 0, NULL);
    gtku_param_widget_add_enum (self->pw, PARAM_NAME_IDS, 0, SHOW_TEXT_NONE,
                               "None", SHOW_TEXT_NONE,
                               "Waypoints", SHOW_TEXT_WAYPOINT,
                               "Checkpoints", SHOW_TEXT_CHECKPOINT, 
                               NULL);
 
    g_signal_connect (G_OBJECT (self->pw), "changed",
            G_CALLBACK (on_param_widget_changed), self);

    self->config = globals_get_config ();
    self->ctrans = globals_get_ctrans ();
    self->rndf   = globals_get_rndf ();
    if (!self->rndf) {
        printf("renderer_rndf is crippled: no RNDF!\n");
        return NULL;
    }

    self->rndf_overlay = rndf_overlay_new (self->rndf);
    self->q = gluNewQuadric();
    self->display_list = glGenLists (1);
    self->next_rerender_time = 0;
    self->show_text=SHOW_TEXT_NONE;

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

    self->ehandler.name = "Waypoints";
    self->ehandler.enabled = 0;
    self->ehandler.mouse_press = mouse_press;
    self->ehandler.mouse_motion = mouse_motion;
    self->ehandler.mouse_release = mouse_release;
    self->ehandler.pick_query = pick_query;
    self->ehandler.hover_query = pick_query;
    self->ehandler.user = self;
    
    return self;
}

static void
renderer_rndf_free (Renderer *super) 
{
    RendererRNDF *self = (RendererRNDF*) super;
    globals_release_config (self->config);
    globals_release_ctrans (self->ctrans);
    rndf_overlay_free (self->rndf_overlay);
    globals_release_rndf (self->rndf);
    gluDeleteQuadric (self->q);
    glDeleteLists (self->display_list, 1);
    free_waypoint_cache (self);
    free (super);
}

static void
renderer_rndf_draw (Viewer *viewer, Renderer *super)
{
    RendererRNDF *self = (RendererRNDF*) super;

    if (!ctrans_have_gps_to_local (self->ctrans)) return;

    double carpos[3];
    ctrans_local_pos (self->ctrans, carpos);
    glPushMatrix ();
    glTranslatef (0, 0, carpos[2]);
#if 0
    int64_t now = timestamp_now ();
    if (now > self->next_rerender_time) {
        glNewList (self->display_list, GL_COMPILE_AND_EXECUTE);
#endif
        _do_draw (self);
#if 0
        glEndList ();
        self->next_rerender_time = now + RERENDER_INTERVAL;
    } else {
        glCallList (self->display_list);
    }
#endif
    glPopMatrix ();
}

static void
_waypoint_local_pos (RendererRNDF *self, RndfOverlayWaypoint *waypoint, 
        double p[3])
{
    double gps[3] = { waypoint->waypoint->lat, waypoint->waypoint->lon, 0 };
    ctrans_gps_to_local (self->ctrans, gps, p, NULL);
    p[2] = 0;
}

static void
_latlon_local_pos (RendererRNDF *self, double lat, double lon, double p[3])
{
    double gps[3] = { lat, lon, 0 };
    ctrans_gps_to_local (self->ctrans, gps, p, NULL);
}

typedef struct {
    RndfOverlayWaypoint * waypoint;
    double xy[2];
    double radius;
} WaypointCache;

static void
free_waypoint_cache (RendererRNDF * self)
{
    GList * iter;
    for (iter = self->waypoint_cache; iter; iter = iter->next)
        g_slice_free (WaypointCache, iter->data);
    g_list_free (self->waypoint_cache);
    self->waypoint_cache = NULL;
}

static void
add_to_waypoint_cache (RendererRNDF * self, RndfOverlayWaypoint * w,
        double xy[2], double radius)
{
    WaypointCache * wc = g_slice_new (WaypointCache);
    wc->waypoint = w;
    wc->xy[0] = xy[0];
    wc->xy[1] = xy[1];
    wc->radius = radius;
    self->waypoint_cache = g_list_prepend (self->waypoint_cache, wc);
}

static void 
_do_draw (RendererRNDF *self)
{
    self->alpha = gtku_param_widget_get_double (self->pw, PARAM_NAME_OPACITY);
    RndfOverlay *overlay = self->rndf_overlay;

    if (!overlay) return;

    free_waypoint_cache (self);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_DEPTH_TEST);

    glClearStencil (0);
    glClear (GL_STENCIL_BUFFER_BIT);

    glEnable (GL_STENCIL_TEST);
    glStencilFunc (GL_EQUAL, 0, 0xffffffff);
    glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);

    // draw checkpoints
    for (int i=0;i<overlay->num_checkpoints;i++)
        draw_checkpoint (self, &overlay->checkpoints[i]);

    // draw segments
    for (int i=0;i<overlay->num_segments;i++)
        draw_segment (self, &overlay->segments[i]);

    // draw zones
    for (int i=0;i<overlay->num_zones;i++) 
        draw_zone (self, &overlay->zones[i]);

    // draw obstacles
    for (int i=0; i<self->rndf->num_obstacles; i++) 
        draw_obstacle (self, &self->rndf->obstacles[i]);

    // draw intersections

    // draw reference points
    //    draw_ref_points();

    glDisable (GL_STENCIL_TEST);

    glDisable (GL_DEPTH_TEST);
    glDisable (GL_BLEND);
}

static void 
draw_segment (RendererRNDF *self, RndfOverlaySegment *s)
{
    if (!s) return;

    // draw the lane
    for (int i=0;i<s->num_lanes;i++)
        draw_lane (self, &s->lanes[i]);

    // draw the name tag (near the middle lane)
//    if (s->name) {
//        int lane_id = MIN(s->num_lanes-1, s->num_lanes/2+1);
//        if (s->num_lanes > 0 && s->lanes[lane_id].num_waypoints > 0) {
//            int waypoint_id = MIN (s->lanes[lane_id].num_waypoints-1, s->lanes[lane_id].num_waypoints/2+1);
//            draw_name_tag (&s->lanes[lane_id].waypoints[waypoint_id], s->name, self);
//        }
//    }
}

static void
draw_arrowed_line (RendererRNDF * self, const double xyz_1[3],
        const double xyz_2[3])
{
    double eps = 0.005;
    // draw a line between the two waypoints
    glLineWidth(1);
    glBegin(GL_LINES);
    glVertex3f(xyz_1[0], xyz_1[1], xyz_1[2] + eps);
    glVertex3f(xyz_2[0], xyz_2[1], xyz_2[2] + eps);
    glEnd();

    // draw an arrowhead along the way
    double mid[3], dir[3];
    mid[0] = (xyz_1[0] + xyz_2[0])/2.0;
    mid[1] = (xyz_1[1] + xyz_2[1])/2.0;
    mid[2] = (xyz_1[2] + xyz_2[2])/2.0 + eps;
    dir[0] = xyz_2[0] - xyz_1[0];
    dir[1] = xyz_2[1] - xyz_1[1];
    dir[2] = xyz_2[2] - xyz_1[2] + eps;
    draw_arrowhead (mid, dir, 2.0);
}

//static void
//draw_name_tag (RendererRNDF *self, RndfWaypoint *w, const char *name) 
//{
//  if (!w || !name) 
//    return;
//
//  // linearize the waypoint position
//  double xyz[3];
//  double gps[3];
//  gps[0] = w->lat;
//  gps[1] = w->lon;
//  gps[2] = RNDF_DEFAULT_GPS_ELEV;
//  
//  linearize_gps (gps, xyz);
//
//  // determine where that point falls on the screen
//  double x,y;
//
//  draw_project (xyz, &x, &y, self);
//
//  glColor4f(1,0,1); // pink
//
//  // write the name tag
//  display_msg_2 (x+20, y+20, name, self->map_width, self->map_height);
//
//  // draw a 2D line between the name and the object
//  draw_2d_line (x, y, x+20, y+20, self->map_width, self->map_height);
//}

static void draw_spot (RendererRNDF *self, RndfOverlaySpot *s )
{
    if (!s) return;

    double xyz_1[3], xyz_2[3];

    _waypoint_local_pos (self, &s->waypoints[0], xyz_1);
    _waypoint_local_pos (self, &s->waypoints[1], xyz_2);

    glColor4f (1.0, 1.0, 0.2, self->alpha);

#if 0
    draw_arrowed_line (self, xyz_1, xyz_2);
#endif

#if 1
    // determine the unit vector along the spot
    double dx,dy;
    dx = xyz_2[0] - xyz_1[0];
    dy = xyz_2[1] - xyz_1[1];
    double ld = sqrt(dx*dx + dy*dy);

    if ( ld < 1E-6 ) return;

    if ( !s->spot ) return;

    dx /= ld;
    dy /= ld;

    // compute the unit vector perpendicular to the road segment
    double px = dy;
    double py = -dx;

    // compute the road width in meters
    double width_m = s->spot->spot_width * 0.3048;

    // draw a box on the map in yellow  glColor3f(1,1,0); 
    glLineWidth(1);
    glColor4f(1,1,1, self->alpha);
    glBegin(GL_LINES);
    glVertex3f(xyz_1[0]+px*width_m/2,xyz_1[1]+py*width_m/2,xyz_1[2]);
    glVertex3f(xyz_2[0]+px*width_m/2,xyz_2[1]+py*width_m/2,xyz_2[2]);

    glVertex3f(xyz_2[0]+px*width_m/2,xyz_2[1]+py*width_m/2,xyz_2[2]);
    glVertex3f(xyz_2[0]-px*width_m/2,xyz_2[1]-py*width_m/2,xyz_2[2]);

    glVertex3f(xyz_2[0]-px*width_m/2,xyz_2[1]-py*width_m/2,xyz_2[2]);
    glVertex3f(xyz_1[0]-px*width_m/2,xyz_1[1]-py*width_m/2,xyz_1[2]);

    glVertex3f(xyz_1[0]-px*width_m/2,xyz_1[1]-py*width_m/2,xyz_1[2]);
    glVertex3f(xyz_1[0]+px*width_m/2,xyz_1[1]+py*width_m/2,xyz_1[2]);
    glEnd();
#endif
}

static void draw_zone (RendererRNDF *self, RndfOverlayZone *z)
{
    int i=0;

    if ( !z )
        return;

    // draw parking spots
    for (i=0;i<z->num_spots;i++)
        draw_spot (self, &z->spots[i]);

    // draw peripoints
    for (i=0;i<z->num_peripoints;i++) {
        RndfOverlayWaypoint *w = &z->peripoints[i];
        draw_waypoint (self, w, 2);
    }

    // draw perimeters
    glLineWidth(1);
    glColor4f(.2,.2,1.0, self->alpha);
    glBegin(GL_LINE_LOOP);
    for (i=0; i<z->num_peripoints; i++) {
        RndfOverlayWaypoint *w = &z->peripoints[i];
        double xyz[3];
        _waypoint_local_pos (self, w, xyz);
        glVertex3f (xyz[0], xyz[1], xyz[2]);
    }
    glEnd();

//    if ( z->name && get_checkbox_entry_value("checkbutton_show_road_names")) {
//        if ( z->num_peripoints > 0 ) {
//            double x,y;
//            double xyz[3];
//            linearize_gps_wp( &z->peripoints[0], xyz);
//
//            draw_project( xyz, &x, &y, self );
//
//            // draw the name tag
//            if ( self->texture_done ) {
//                glColor3f(0,0,0); // black
//            } else {
//                glColor3f(1,0,1); // pink
//            }
//
//            // write the name tag including the speed limits
//            char str[256];
//            sprintf(str,"%s (%d/%d)", z->name, z->min_speed, z->max_speed);
//
//            display_msg_box (x, y, str, self->map_width, self->map_height,
//                    1,1,1);
//
//            //draw_name_tag ( &z->peripoints[0], z->name, self );
//        }
//    }
}

static void
draw_lane_boundary (RendererRNDF *self, RndfOverlayLane *la, 
        pointlist2d_t *points,
        RndfBoundaryType type)
{
    glLineWidth (3.0);
    if (type == RNDF_BOUND_DBL_YELLOW) {
#if 1
        glColor4f (1, 1, 0, self->alpha);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<points->npoints; i++) {
            glVertex2f (points->points[i].x, points->points[i].y);
        }
        glEnd ();
#else
        glColor4f (1, 1, 0, self->alpha);
        pointlist2d_t *left = geom_polyline_shift_sideways_2d (points, 0.1);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<left->npoints; i++) {
            glVertex2f (left->points[i].x, left->points[i].y);
        }
        glEnd ();
        pointlist2d_free (left);

        pointlist2d_t *right = geom_polyline_shift_sideways_2d (points, -0.1);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<right->npoints; i++) {
            glVertex2f (right->points[i].x, right->points[i].y);
        }
        glEnd ();
        pointlist2d_free (right);
#endif
    } else if (type == RNDF_BOUND_BROKEN_WHITE) {
        glColor4f (1, 1, 1, self->alpha);
        glLineStipple (5, 0xff);
        pointlist2d_t *s = 
            geom_polyline_resample_at_regular_intervals (points, 10);
        glBegin (GL_LINES);
        for (int i=0; i<s->npoints - 1; i+= 2) {
            glVertex2f (s->points[i].x, s->points[i].y);
            glVertex2f (s->points[i+1].x, s->points[i+1].y);
        }
        glEnd ();
        pointlist2d_free (s);
    } else {
        switch (type) {
            case RNDF_BOUND_SOLID_WHITE:
                glColor4f (1, 1, 1, self->alpha);
                break;
            case RNDF_BOUND_SOLID_YELLOW:
                glColor4f (1, 1, 0, self->alpha);
                break;
            default:
            case RNDF_BOUND_UNSPECIFIED:
                glColor4f (0, 1, 1, self->alpha);
                break;
        }

        glBegin (GL_LINE_STRIP);
        for (int i=0; i<points->npoints; i++) {
            glVertex2f (points->points[i].x, points->points[i].y);
        }
        glEnd ();
    }
}

static void
_draw_lane_boundary_section (RendererRNDF *self, RndfOverlayLane *la,
        const pointlist2d_t *centerline, double shift, RndfBoundaryType type,
        int startind, int endind)
{
    if (endind <= startind) return;
    pointlist2d_t *section = 
        pointlist2d_new_copy_subsection (centerline, 
                startind, 0, endind - startind, 0);
    if (!section) return;
    pointlist2d_t *shifted = 
        geom_polyline_shift_sideways_2d (section, shift);
    draw_lane_boundary (self, la, shifted, type);
    pointlist2d_free (shifted);
    pointlist2d_free (section);
//    glColor3f (0, 1, 0);
//    double xyz[3];
//    _waypoint_local_pos (self, owp, xyz);
//    double xyzleft[3];
//    glDisable (GL_STENCIL_TEST);
//    _waypoint_local_pos (self, owp->left_boundary_wp, xyzleft);
//    glEnable (GL_STENCIL_TEST);
//    draw_arrowed_line (self, xyz, xyzleft);
}

static void
draw_yield (RendererRNDF * self, double xyz_1[3], double xyz_2[3])
{
    glPushMatrix ();
    glTranslatef ((xyz_2[0] - xyz_1[0])*0.25 + xyz_1[0],
            (xyz_2[1] - xyz_1[1])*0.25 + xyz_1[1],
            (xyz_2[2] - xyz_1[2])*0.25 + xyz_1[2]);
    glColor3f (1, 0.8, 0);
    glBegin (GL_TRIANGLES);
    glVertex2f (-sqrt(3)/2, 0.5);
    glVertex2f (sqrt(3)/2, 0.5);
    glVertex2f (0, -1);
    glEnd ();
    glPopMatrix ();
}

static void 
draw_lane (RendererRNDF *self, RndfOverlayLane *la)
{
    if (!la) return;
    if (la->num_waypoints == 0) return;

    double xyz[3];
    pointlist2d_t *centerline = pointlist2d_new (la->num_waypoints);
    for (int i=0; i<la->num_waypoints; i++) {
        RndfOverlayWaypoint *wp = &la->waypoints[i];
        double xyz0[3];
        memcpy (xyz0, xyz, 3 * sizeof (double));
        _waypoint_local_pos (self, wp, xyz);

        centerline->points[i].x = xyz[0];
        centerline->points[i].y = xyz[1];

        if (i > 0 && gtku_param_widget_get_bool (self->pw, 
                    PARAM_NAME_DRAW_LANE_CENTERS)) {
            glColor4f (0.1, 1.0, 0.1, self->alpha);
            draw_arrowed_line (self, xyz0, xyz);
            if (wp->prev && wp->prev->next_yields)
                draw_yield (self, xyz0, xyz);
        }
        draw_waypoint (self, wp, la->lane->lane_width * 0.3048 / 2);
    }

    if (gtku_param_widget_get_bool (self->pw, 
                PARAM_NAME_DRAW_LANE_BOUNDARIES)) {

        int left_startind = 0;
        int right_startind = 0;
        for (int i=0; i<la->num_waypoints; i++) {
            RndfOverlayWaypoint *owp = &la->waypoints[i];
            if (!owp->expect_left_boundary_next ||
                (owp->next && !owp->next->expect_left_boundary_prev)) {
                _draw_lane_boundary_section (self, la, centerline,
                        la->lane->lane_width * METERS_PER_FOOT / 2.0,
                        la->lane->left_boundary, left_startind, i+1);
                left_startind = i + 1;
            }
            if (!owp->expect_right_boundary_next ||
                (owp->next && !owp->next->expect_right_boundary_prev)) {
                _draw_lane_boundary_section (self, la, centerline,
                        - la->lane->lane_width * METERS_PER_FOOT / 2.0,
                        la->lane->right_boundary, right_startind, i+1);
                right_startind = i + 1;
            }

            glPushMatrix ();
            glTranslatef (centerline->points[i].x, centerline->points[i].y, 0);
            if (owp->expect_left_boundary_prev) {
                glColor3f (1, 0, 0);
                glutil_draw_circle (0.2);
            }
            if (owp->expect_right_boundary_prev) {
                glColor3f (0, 0, 1);
                glutil_draw_circle (0.3);
            }
            if (owp->expect_left_boundary_next) {
                glColor3f (0, 1, 0);
                glutil_draw_circle (0.4);
            }
            if (owp->expect_right_boundary_next) {
                glColor3f (1, 1, 0);
                glutil_draw_circle (0.5);
            }
            glPopMatrix ();
        }
//        pointlist2d_t *right = geom_polyline_shift_sideways_2d (centerline, 
//                -la->lane->lane_width * METERS_PER_FOOT / 2.0);
//        draw_lane_boundary (self, la, right, la->lane->right_boundary);
//        pointlist2d_free (right);
    }
    pointlist2d_free (centerline);
}

static void 
draw_disk (RendererRNDF *self, double xyz[3], double r_in, double r_out)
{
    glPushMatrix();
    glTranslatef(xyz[0], xyz[1], xyz[2]);
    gluDisk(self->q, r_in, r_out, 10, 1);
    glPopMatrix();
}

static void 
draw_waypoint_xyz (RendererRNDF *self, double *xyz, RndfPointType type,
        double r) 
{
    double radius = r;

    switch (type) {
        case RNDF_POINT_WAYPOINT:
            glColor4f(0,1,0, self->alpha);
            break;
        case RNDF_POINT_PERIMETER:
            glColor4f(.2,.2,1.0, self->alpha);
            break;
        case RNDF_POINT_SPOT:
            glColor4f(1,1,0, self->alpha);
            break;
        default:
            fprintf(stderr,"undefined point type for waypoint!\n");
            break;
    }

    draw_disk (self, xyz, 0, radius);

    // for user, draw a point of fixed size on the screen as well
    glPointSize(6);
    glBegin(GL_POINTS);
    glVertex3f(xyz[0],xyz[1],xyz[2]);
    glEnd();
}

/* draw a stop line if needed */
static void
draw_stop_xyz (RendererRNDF *self, double *xyz, double r)
{
    // draw a red circle on a white circle
    glColor4f(1,1,1, self->alpha);
    double radius = r;
    double inner = r * 2.0 / 3;
    draw_disk (self, xyz, inner, radius);

    glColor4f(1,0,0, self->alpha);
    draw_disk (self, xyz, 0, inner);

}
static void 
draw_waypoint (RendererRNDF *self, RndfOverlayWaypoint *w, double radius)
{
    if (!w) return;
    double xyz[3];
    _waypoint_local_pos (self, w, xyz);

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_DRAW_LANE_CENTERS)) {
        for (int exit_num=0; exit_num<w->num_exits; exit_num++) {
            draw_exit (self, w, w->exits[exit_num], w->exit_yields[exit_num]);
        }
    }


    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_DRAW_WAYPOINTS)) {
        add_to_waypoint_cache (self, w, xyz, radius);

        if (self->ehandler.hovering && self->pick_waypoint == w) {
            glColor4f (1, 1, 1, 1);
            draw_disk (self, xyz, radius, radius + 0.5);
        }

        if (w->is_stop) draw_stop_xyz (self, xyz, 2.5);
        else draw_waypoint_xyz (self, xyz, w->type, radius);
    }

    if (self->show_text==SHOW_TEXT_WAYPOINT) {
        char buf[256];
        switch(w->type){
        case RNDF_POINT_WAYPOINT:
            sprintf(buf, "wp: %d.%d.%d",
                    w->id[0], w->id[1], w->id[2]);
            break;
        case RNDF_POINT_PERIMETER:
            sprintf(buf, "wp: z.%d.%d",
                    w->id[0], w->id[2]);
            break;
        case RNDF_POINT_SPOT:
            sprintf(buf, "wp: %d.%d.%d",
                    w->id[0], w->id[1], w->id[2]);
            break;
        }
        glColor3f(0.3,1,0.3);
        glDisable (GL_STENCIL_TEST);
        glutil_draw_text(xyz, GLUT_BITMAP_HELVETICA_12, buf,
                         GLUTIL_DRAW_TEXT_DROP_SHADOW);
        glEnable (GL_STENCIL_TEST);
    }
}

static void
draw_exit_xyz (RendererRNDF *self, double *xyz_1, double *xyz_2)
{
    glColor4f(1,0,0, self->alpha);
    draw_arrowed_line (self, xyz_1, xyz_2);
}

static void 
draw_exit (RendererRNDF *self, RndfOverlayWaypoint *w1, RndfOverlayWaypoint *w2,
        GList * yields)
{

    if (!w1 || !w2)
        return;

    double xyz_1[3], xyz_2[3];
    _waypoint_local_pos (self, w1, xyz_1);
    _waypoint_local_pos (self, w2, xyz_2);

    // draw perimeter exit box if the exit belongs to a perimeter
    if (w1->type == RNDF_POINT_PERIMETER) draw_perimeter_exit_xyz (self, xyz_1);
    if (w2->type == RNDF_POINT_PERIMETER) draw_perimeter_exit_xyz (self, xyz_2);

    // draw the exit
    draw_exit_xyz (self, xyz_1, xyz_2);

    if (yields)
        draw_yield (self, xyz_1, xyz_2);
}

static void
draw_arrowhead (double *mid, double *dir, double size)
{
    // normalize the direction vector
    double ld = sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
    if (ld < 1E-6)
        return;
    dir[0] /= ld;
    dir[1] /= ld;
    dir[2] /= ld;

    // compute the vector perpendicular to the arrow
    double p[3];
    p[0] = dir[1];
    p[1] = -dir[0];
    p[2] = dir[2];

    // compute the left and right endpoint of the arrow
    double left[3];
    double right[3];
    left[0] = mid[0] - p[0] * size/2 - dir[0] * size/2;
    left[1] = mid[1] - p[1] * size/2 - dir[1] * size/2;
    left[2] = mid[2] - p[2] * size/2 - dir[2] * size/2;

    right[0] = mid[0] + p[0] * size/2 - dir[0] * size/2;
    right[1] = mid[1] + p[1] * size/2 - dir[1] * size/2;
    right[2] = mid[2] + p[2] * size/2 - dir[2] * size/2;

    // draw the arrowhead
    glBegin(GL_LINES);
    glVertex3f(mid[0],mid[1],mid[2]);
    glVertex3f(left[0],left[1],left[2]);

    glVertex3f(mid[0],mid[1],mid[2]);
    glVertex3f(right[0],right[1],right[2]);
    glEnd();
}

static void
draw_perimeter_exit_xyz (RendererRNDF *self, double *xyz)
{
    double pew = 3.0; // perimeter exit box, in meters

    glColor4f(1,0,0, self->alpha);
    glLineWidth(1);
    glBegin(GL_LINES);
    glVertex3f(xyz[0]+pew,xyz[1]+pew,xyz[2]);
    glVertex3f(xyz[0]-pew,xyz[1]+pew,xyz[2]);

    glVertex3f(xyz[0]-pew,xyz[1]+pew,xyz[2]);
    glVertex3f(xyz[0]-pew,xyz[1]-pew,xyz[2]);

    glVertex3f(xyz[0]-pew,xyz[1]-pew,xyz[2]);
    glVertex3f(xyz[0]+pew,xyz[1]-pew,xyz[2]);

    glVertex3f(xyz[0]+pew,xyz[1]-pew,xyz[2]);
    glVertex3f(xyz[0]+pew,xyz[1]+pew,xyz[2]);
    glEnd();
}

static void
draw_checkpoint (RendererRNDF *self, RndfOverlayCheckpoint *c)
{
    if (!c) return;
    RndfOverlayWaypoint *w = c->waypoint;
    double xyz[3];
    _waypoint_local_pos (self, w, xyz);

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_DRAW_WAYPOINTS)) {
        double radius = 1.0;
        glColor4f(1.0,0.5,0.0, self->alpha); // orange

        draw_disk (self, xyz, 0, radius);

        // draw an orange point on top of that
        glPointSize(5);
        glBegin (GL_POINTS);
        glVertex3dv (xyz);
        glEnd();
    }

    // TODO show checkpoint ID
    if (self->show_text==SHOW_TEXT_CHECKPOINT) {
        char buf[256];
        sprintf(buf, "cp: %d",c->checkpoint->id);
        glColor3f(1,0.5,0);
        glDisable (GL_STENCIL_TEST);
        glutil_draw_text(xyz, GLUT_BITMAP_HELVETICA_12, buf,
                         GLUTIL_DRAW_TEXT_DROP_SHADOW);
        glEnable (GL_STENCIL_TEST);
    }
}

static void 
draw_obstacle (RendererRNDF *self, RndfObstacle *o)
{
    if (!o) return;

    glColor4f(1,0,0, self->alpha);
    
    // linearize lat-lon to xyz
    double xyz[3];
    _latlon_local_pos (self, o->lat, o->lon, xyz);

    double cosa = cos(o->orient);
    double sina = sin(o->orient);
    double w1 = o->w1 / 2;
    double w2 = o->w2 / 2;

    glBegin(GL_QUADS);
    glVertex3f (xyz[0] + w1*cosa - w2*sina, xyz[1] + w1*sina + w2*cosa, xyz[2]);
    glVertex3f (xyz[0] - w1*cosa - w2*sina, xyz[1] - w1*sina + w2*cosa, xyz[2]);
    glVertex3f (xyz[0] - w1*cosa + w2*sina, xyz[1] - w1*sina - w2*cosa, xyz[2]);
    glVertex3f (xyz[0] + w1*cosa + w2*sina, xyz[1] + w1*sina - w2*cosa, xyz[2]);
    glEnd();
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        void *user_data)
{
    RendererRNDF *self = (RendererRNDF*) user_data;
    self->show_text = gtku_param_widget_get_enum (pw, PARAM_NAME_IDS);

    viewer_request_redraw (self->viewer);
}

static double
pick_query(Viewer *viewer, EventHandler *ehandler, const double ray_start[3],
        const double ray_dir[3])
{
    RendererRNDF * self = ehandler->user;

    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), POINT3D(ray_dir),
            carpos[2], POINT2D(xy));

    double best_dist = 0;
    RndfOverlayWaypoint * best_waypoint = NULL;
    GList * iter;
    for (iter = self->waypoint_cache; iter; iter = iter->next) {
        WaypointCache * wc = iter->data;
        double r = MAX (wc->radius, 1);
        double distvec[2] = { xy[0] - wc->xy[0], xy[1] - wc->xy[1] };
        if (fabs (distvec[0]) > r || fabs (distvec[1]) > r)
            continue;
        double d = vector_magnitude_2d (distvec);
        if (d > r)
            continue;
        if (!best_waypoint || d < best_dist) {
            best_dist = d;
            best_waypoint = wc->waypoint;
        }
    }

    if (!best_waypoint)
        return -1;

    self->pick_waypoint = best_waypoint;
    return best_dist;
}

static int
mouse_press (Viewer *viewer, EventHandler *ehandler,
        const double ray_start[3], const double ray_dir[3], 
        const GdkEventButton *event)
{
    RendererRNDF *self = ehandler->user;

    if (!self->ehandler.picking || !self->pick_waypoint ||
            event->button != 1) {
        self->ehandler.picking = 0;
        return 0;
    }

    self->moving_waypoint = self->pick_waypoint;

    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), 
            POINT3D(ray_dir), carpos[2], POINT2D(self->last_xy));

    return 1;
}

static void
save_rndf (RendererRNDF * self)
{
    char filename[256];
    if (config_util_get_rndf_absolute_path (self->config, filename,
                sizeof (filename)) < 0) {
        fprintf (stderr, "Error: failed to get RNDF filename, cannot save\n");
        return;
    }
    rndf_print (self->rndf, filename);
}

static int
mouse_release(Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventButton *event)
{
    RendererRNDF *self = ehandler->user;
    if (self->moving_waypoint) {
        self->moving_waypoint = NULL;
        ehandler->picking = 0;
        save_rndf (self);
        return 1;
    }
    return 0;
}

static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventMotion *event)
{
    RendererRNDF *self = ehandler->user;

    if (!self->ehandler.picking || !self->moving_waypoint)
        return 0;

    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), 
            POINT3D(ray_dir), carpos[2], POINT2D(xy));
    
    double wpos[3];
    _waypoint_local_pos (self, self->moving_waypoint, wpos);

    wpos[0] += xy[0] - self->last_xy[0];
    wpos[1] += xy[1] - self->last_xy[1];
    self->last_xy[0] = xy[0];
    self->last_xy[1] = xy[1];
    double gps[3];
    ctrans_local_to_gps (self->ctrans, wpos, gps, NULL);

    self->moving_waypoint->waypoint->lat = gps[0];
    self->moving_waypoint->waypoint->lon = gps[1];

    viewer_request_redraw(self->viewer);
    return 1;
}

void setup_renderer_rndf (Viewer *viewer, int render_priority)
{
    RendererRNDF *self = renderer_rndf_new (viewer);
    if (self) {
        viewer_add_renderer(viewer, &self->renderer, render_priority);
        viewer_add_event_handler(viewer, &self->ehandler, render_priority);
    }
}


