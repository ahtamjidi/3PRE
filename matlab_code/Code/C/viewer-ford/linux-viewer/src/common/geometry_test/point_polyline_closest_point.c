#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

typedef struct _PointPolylineClosestPointTester {
    GeometryTester parent;

    GQueue * poly1;
    GRand *rng;

    point2d_t last_mouse_pt;
    GLUquadricObj *quadric;
} PointPolylineClosestPointTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *super, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
point_polyline_closest_point_tester_new ()
{
    PointPolylineClosestPointTester * self = 
        (PointPolylineClosestPointTester*) calloc (1, 
                sizeof (PointPolylineClosestPointTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "point polyline closest point";

    self->poly1 = g_queue_new ();

//    g_queue_push_tail (self->poly1, point2d_new (150, 100));
    g_queue_push_tail (self->poly1, point2d_new (200, 102));
    g_queue_push_tail (self->poly1, point2d_new (250, 100));
    g_queue_push_tail (self->poly1, point2d_new (300, 101));
    g_queue_push_tail (self->poly1, point2d_new (350, 101));
    g_queue_push_tail (self->poly1, point2d_new (400, 101));

    self->quadric = gluNewQuadric ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PointPolylineClosestPointTester *self = 
        (PointPolylineClosestPointTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
        point2d_free ( (point2d_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    gluDeleteQuadric (self->quadric);
    free (self);
}

typedef struct {
    double r;
    double g;
    double b;
} color_t;

static color_t COLORS[] = {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 1, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 1 },
    { 1, 1, 1 },
    { 0.5, 0, 1 },
    { 0, 0.5, 1 },
    { 0.5, 1, 0 },
    { 0, 1, 0.5 },
    { 1, 0.5, 0 },
    { 1, 0, 0.5 }
};

static int NUM_COLORS = sizeof (COLORS) / (3 * sizeof (double));

static void
draw_disk (PointPolylineClosestPointTester *self, double x, double y, double r)
{
    glPushMatrix ();
    glTranslatef (x, y, 0);
    gluDisk (self->quadric, 0, r, 50, 1);
    glPopMatrix ();
}

static void
draw_polyline (pointlist2d_t * poly, int *associations)
{
    glBegin (GL_LINES);
    int i;
    for (i = 0; i < poly->npoints - 1; i++) {
        color_t c;
        if (associations) {
            c = COLORS[associations[i] % NUM_COLORS], sizeof (c);
        } else {
            c = COLORS[i % NUM_COLORS], sizeof (c);
        }
        glColor3f (c.r, c.g, c.b);
        glVertex2d (poly->points[i].x, poly->points[i].y);
        glVertex2d (poly->points[i+1].x, poly->points[i+1].y);
    }
    glEnd ();
}

static void 
draw (GeometryTester *super)
{
    PointPolylineClosestPointTester *self = 
        (PointPolylineClosestPointTester*) super;
    glColor3f (1, 1, 0);

    pointlist2d_t *poly1 = pointlist2d_new_from_gqueue (self->poly1);
    if (!poly1) return;

    glColor3f (0, 1, 0);
    draw_disk (self, self->last_mouse_pt.x, self->last_mouse_pt.y, 5);

#if 1
    if (poly1->npoints > 1) {
        int vals[4];
        glGetIntegerv (GL_VIEWPORT, vals);
        glBegin (GL_POINTS);
        glColor3f (1, 1, 1);
        for (int i = 0; i < vals[2]; i++) {
            int j;
            for (j = 0; j < vals[3]; j++) {
                point2d_t pt = { .x = i, .y = j };

                int line_ind;
                double line_alpha = 0;
                geom_point_polyline_closest_point_2d (&pt, poly1, &line_ind, 
                        &line_alpha, NULL);
                color_t c = COLORS[line_ind % NUM_COLORS];

//                if (line_ind == 0 && line_alpha <= 0) {
//                    c.r = c.g = c.b = 0;
//                } else if (line_ind == poly1->npoints - 2 && line_alpha >= 1) {
//                    c.r = c.g = c.b = 0;
//                }
                glColor3f (0.3 * c.r, 0.3 * c.g, 0.3 * c.b);
                glVertex2d (pt.x, pt.y);
            }
        }
        glEnd ();
    }
#endif

    draw_polyline (poly1, NULL);

    point2d_t cp;
    int cp_ind;
    double cp_alpha;
    geom_point_polyline_closest_point_2d (&self->last_mouse_pt, poly1, 
            &cp_ind, &cp_alpha, &cp);

    color_t c = COLORS[cp_ind % NUM_COLORS];
    glColor3f (c.r, c.g, c.b);
    draw_disk (self, cp.x, cp.y, 5);

    pointlist2d_free (poly1);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PointPolylineClosestPointTester *self = 
        (PointPolylineClosestPointTester*) super;

    if (event->button == 1) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        while (! g_queue_is_empty (self->poly1)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->poly1));
        }
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    PointPolylineClosestPointTester *self = 
        (PointPolylineClosestPointTester*) super;

    if (motion->state & GDK_BUTTON2_MASK) {
        self->last_mouse_pt.x = motion->x;
        self->last_mouse_pt.y = motion->y;
    }
}
