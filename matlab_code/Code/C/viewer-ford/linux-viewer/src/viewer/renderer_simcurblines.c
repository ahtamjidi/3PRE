#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include <libviewer/viewer.h>
#include <common/math_util.h>
#include <common/glib_util.h>
#include <common/geometry.h>
#include <common/gps_linearize.h>
#include <common/timestamp.h>
#include <common/rotations.h>
#include <dgc/globals.h>

#include <lcmtypes/lcmtypes_curb_polylines_t.h>

#include <libviewer/viewer.h>

#include <lcm/lcm.h>

#define TRANSMIT_CURBLINES_HZ 20

struct point
{
    double ll[2];
    struct point *next;
};

typedef struct _RendererSimCurbLines {
    Renderer     renderer;
    EventHandler ehandler;
    
    Viewer         *viewer;
    lcm_t           *lc;
    CTrans         *ctrans;

    // We maintain a number of poly lines. Each individual polyline is
    // a linked list of points; the points
    // array contains the head of each list of polyline.
    GPtrArray      *points;
    struct point   *edit_point, *hover_point;
    struct point   *last_point;
    double         lastxy[2];

    int            key_pick;
} RendererSimCurbLines;

static void xy_to_ll(RendererSimCurbLines *self, const double xy[2], double ll[2])
{
    double xyz[] = {xy[0], xy[1], 0};
    double lle[3];
    ctrans_local_to_gps(self->ctrans, xyz, lle, NULL);
    memcpy(ll, lle, 2 * sizeof(double));
}

static void ll_to_xy(RendererSimCurbLines *self, const double ll[2], double xy[2])
{
    double lle[3] = {ll[0], ll[1], 0};
    double xyz[3];
    ctrans_gps_to_local(self->ctrans, lle, xyz, NULL);
    memcpy(xy, xyz, 2 * sizeof(double));
}

static struct point *find_point(RendererSimCurbLines *self, const double xy[2])
{
    double best_distance = 1;
    struct point *best_point = NULL;

    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
        for (struct point *p = g_ptr_array_index(self->points, i); p != NULL; p = p->next) {

            double cxy[2];
            ll_to_xy(self, p->ll, cxy);

            double dx = xy[0] - cxy[0];
            double dy = xy[1] - cxy[1];
            double distance = sqrt(sq(dx) + sq(dy));

            if (distance < best_distance) {
                best_distance = distance;
                best_point = p;
            }
        }
    }

    return best_point;
}

gboolean on_transmit_curblines(gpointer _user)
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) _user;
    
    if (!ctrans_have_pose(self->ctrans) ||
        !ctrans_have_gps_to_local (self->ctrans))
        return TRUE;
    
    lcmtypes_curb_polylines_t curb_lines;
    memset(&curb_lines, 0, sizeof(lcmtypes_curb_polylines_t));
    curb_lines.nlines = g_ptr_array_size(self->points); 
    curb_lines.lines = (lcmtypes_pointlist2d_t*) calloc(curb_lines.nlines, sizeof(lcmtypes_pointlist2d_t));
    
    
    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
        
        // count the points
        curb_lines.lines[i].npoints=0;
        for (struct point *p = g_ptr_array_index(self->points, i); p != NULL; p = p->next)
            curb_lines.lines[i].npoints++;
        
        // alloc
        curb_lines.lines[i].points = calloc(curb_lines.lines[i].npoints, sizeof(lcmtypes_point2d_t));
        
        // compute local coord x,y
        int idx = 0;
        for (struct point *p = g_ptr_array_index(self->points, i); p != NULL; p = p->next, idx++) {

            double xyz[3];
            double lle[3] = { p->ll[0], p->ll[1], 0 };

            ctrans_gps_to_local (self->ctrans, lle, xyz, NULL);

            curb_lines.lines[i].points[idx].x = xyz[0];
            curb_lines.lines[i].points[idx].y = xyz[1];
        }
    }

    lcmtypes_curb_polylines_t_publish(self->lc, "CURBLINES", &curb_lines);

    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++)
        free(curb_lines.lines[i].points);
    free(curb_lines.lines);

    return TRUE;
}

static void my_draw( Viewer *viewer, Renderer *super )
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) super->user;
    
    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {

        glColor3f(.7,.7,.7);
        glLineWidth(1);
        glBegin(GL_LINE_STRIP);
        for (struct point *p = g_ptr_array_index(self->points, i); p != NULL; p = p->next) {
            double cxy[2];
            ll_to_xy(self, p->ll, cxy);
            glVertex2dv(cxy);
        }
        glEnd();

        glColor3f(1,1,1);
        glPointSize(4);
        glBegin(GL_POINTS);
        for (struct point *p = g_ptr_array_index(self->points, i); p != NULL; p = p->next) {
            double cxy[2];
            ll_to_xy(self, p->ll, cxy);
            glVertex2dv(cxy);
        }
        glEnd();
    }
/*
    if (self->hover_point) {
        glColor3f(.5, .5, 1);
        glBegin(GL_POINTS);
        double cxy[2];
        ll_to_xy(self, p->ll, cxy);
        glVertex2dv(cxy);
        glEnd();
    }
*/
}

static int mouse_press (Viewer *viewer, EventHandler *ehandler,
                        const double ray_start[3], const double ray_dir[3], const GdkEventButton *event)
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) ehandler->user;

    double xy[2];
    int consumed = 0;

    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), POINT3D(ray_dir), 
                                          0, POINT2D(xy));

    memcpy(self->lastxy, xy, 2 * sizeof(double));
    int control = event->state & GDK_CONTROL_MASK;

    if (event->button != 1) {
        if (self->last_point) {
            self->last_point = NULL;
            ehandler->picking = 0;
            self->key_pick = 0;
            return 1;
        }
        return 0;
    }

    // creating?
    if (ehandler->picking && self->edit_point == NULL && control) {
        struct point *p = (struct point*) calloc(1, sizeof(struct point));

        xy_to_ll(self, xy, p->ll);
       
        if (self->last_point)
            self->last_point->next = p;
        else
            g_ptr_array_add(self->points, p);

        self->last_point = p;

        viewer_request_pick(viewer, ehandler);
        return 1;
    }

    // selecting a previous point
    if (ehandler->picking && self->hover_point) {
        self->edit_point = find_point(self, xy);
        return 1;
    }

    return consumed;
}

static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventMotion *event)
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) ehandler->user;
 
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), POINT3D(ray_dir), 
                                  0, POINT2D(xy));

    if (!self->edit_point)
        return 0;
    
    xy_to_ll(self, xy, self->edit_point->ll);
    return 1;
}

static int mouse_release (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], 
                          const GdkEventButton *event)
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) ehandler->user;

    self->edit_point = NULL;
    if (!self->key_pick)
        ehandler->picking = 0;
    
    viewer_request_redraw(viewer);
    return 1;
}

static double pick_query(Viewer *viewer, EventHandler *ehandler, const double ray_start[3], const double ray_dir[3])
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), 
                                  POINT3D(ray_dir), 0, POINT2D(xy));

    self->hover_point = find_point(self, xy);
    if (!self->hover_point)
        return -1;
    
    double cxy[2];
    ll_to_xy(self, self->hover_point->ll, cxy);

    double dist = sqrt(sq(cxy[0] - xy[0]) + sq(cxy[1] - xy[1]));

    return dist;
}

static int key_press (Viewer *viewer, EventHandler *ehandler, const GdkEventKey *event)
{
    RendererSimCurbLines *self = (RendererSimCurbLines*) ehandler->user;

    if (event->keyval == 'c' || event->keyval == 'C') {
        viewer_request_pick(viewer, ehandler);
        self->key_pick = 1;
        return 1;
    }

    if (self->edit_point && (event->keyval == GDK_Delete || 
                             event->keyval == GDK_BackSpace)) {

        for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {

            for (struct point **pp = (struct point**) &self->points->pdata[i]; *pp != NULL; pp = &((*pp)->next)) {

                if (*pp == self->edit_point) {
                    *pp = self->edit_point->next;
                    free(self->edit_point);
                    self->edit_point = NULL;
                    break;
                }
            }

            // remove any empty poly lines

            g_ptr_array_remove(self->points, NULL);
        }

        viewer_request_redraw(viewer);
        return 1;
    }

    if (event->keyval == GDK_Escape) {
        ehandler->picking = 0;
        self->key_pick = 0;
        self->last_point = NULL;
    }

    return 0;
}

void setup_renderer_simcurblines(Viewer *viewer, int priority)
{
    if (!viewer->simulation_flag)
        return;

    RendererSimCurbLines *self = (RendererSimCurbLines*) calloc(1, sizeof(RendererSimCurbLines));

    self->viewer = viewer;
    self->ctrans = globals_get_ctrans();

    self->renderer.enabled = 1;
    self->renderer.name = "SimCurbLines";
    self->renderer.draw = my_draw;
    self->renderer.user = self;

    self->ehandler.name = "SimCurbLines";
    self->ehandler.enabled = 1;
    self->ehandler.mouse_press = mouse_press;
    self->ehandler.mouse_motion = mouse_motion;
    self->ehandler.mouse_release = mouse_release;
    self->ehandler.pick_query = pick_query;
    self->ehandler.hover_query = pick_query;
    self->ehandler.key_press = key_press;
    self->ehandler.user = self;
    
    self->lc = globals_get_lcm();
    self->points = g_ptr_array_new();

    viewer_add_event_handler(viewer, &self->ehandler, priority);
    viewer_add_renderer(viewer, &self->renderer, priority);

    g_timeout_add(1000 / TRANSMIT_CURBLINES_HZ, on_transmit_curblines, self);

}
