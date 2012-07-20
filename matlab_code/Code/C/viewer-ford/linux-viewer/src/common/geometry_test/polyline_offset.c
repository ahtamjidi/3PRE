#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include <gtk_util/gtk_util.h>
#include "geometry_tester.h"

typedef struct _PolylineOffsetTester {
    GeometryTester parent;
    GtkuParamWidget *pw;

    GQueue * points;

    double width;
    GLUquadricObj *quadric;
} PolylineOffsetTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);
static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        void *user);

GeometryTester * 
polyline_offset_tester_new ()
{
    PolylineOffsetTester * self = 
        (PolylineOffsetTester*) calloc (1, sizeof (PolylineOffsetTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polyline offset";
    self->parent.widget = gtku_param_widget_new ();

    self->pw = GTKU_PARAM_WIDGET (self->parent.widget);
    gtku_param_widget_add_double (self->pw, "Start radius", 0,
            1, 200, 1, 70);
    gtku_param_widget_add_double (self->pw, "End radius", 0,
            1, 200, 1, 20);
    gtku_param_widget_add_int (self->pw, "Circle points", 0, 2, 30, 2, 10);
    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);

    self->quadric = gluNewQuadric ();

    self->points = g_queue_new ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolylineOffsetTester *self = (PolylineOffsetTester*) super;
    gluDeleteQuadric (self->quadric);
    while (! g_queue_is_empty (self->points)) {
        point2d_free ((point2d_t*) g_queue_pop_head (self->points));
    }
    g_queue_free (self->points);

    free (self);
}

static void
draw_polygon (polygon2d_t *polygon)
{
    for (int i=0; i<polygon->nlists; i++) {
        glColor3f (1, 0, 0);
        glBegin (GL_LINE_LOOP);
        const pointlist2d_t *pl = &polygon->pointlists[i];
        for (int j=0; j<pl->npoints; j++) {
            glVertex2f (pl->points[j].x, pl->points[j].y);
        }
        glEnd ();
    }
}

static void 
draw (GeometryTester *super)
{
    PolylineOffsetTester *self = (PolylineOffsetTester*) super;
    glColor3f (1, 1, 0);

    pointlist2d_t *polyline = pointlist2d_new_from_gqueue (self->points);

    glBegin (GL_LINES);
    for (int i=0; i<polyline->npoints-1; i++) {
        glVertex2f (polyline->points[i].x, polyline->points[i].y);
        glVertex2f (polyline->points[i+1].x, polyline->points[i+1].y);
    }
    glEnd ();

    glColor3f (0, 1, 1);
    for (int i=0; i<polyline->npoints; i++) {
        glPushMatrix ();
        glTranslatef (polyline->points[i].x, polyline->points[i].y, 0);
        gluDisk (self->quadric, 0, 5, 20, 1);
        glPopMatrix ();
    }

    double start_radius = gtku_param_widget_get_double (self->pw, 
            "Start radius");
    double end_radius = gtku_param_widget_get_double (self->pw,
            "End radius");
    int ncircle_points = gtku_param_widget_get_int (self->pw, "Circle points");
    polygon2d_t *offset = 
        geom_polyline_compute_polygon_offset_tapered_2d (polyline, 
                start_radius, end_radius, ncircle_points);

    if (offset) {
        draw_polygon (offset);

        polygon2d_free (offset);
    }

    pointlist2d_free (polyline);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolylineOffsetTester *self = (PolylineOffsetTester*) super;

    if (event->button == 1) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->points, newpoint);
    } else if (event->button == 2) {
        while (! g_queue_is_empty (self->points)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->points));
        }
    }
}

static void
on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    geometry_tester_request_redraw (user);
}
