#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

typedef struct _PolylineShiftTester {
    GeometryTester parent;

    GQueue * poly1;
    GRand *rng;
} PolylineShiftTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
polyline_shift_tester_new ()
{
    PolylineShiftTester * self = 
        (PolylineShiftTester*) calloc (1, 
                sizeof (PolylineShiftTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polyline shift";

    self->poly1 = g_queue_new ();

//    g_queue_push_tail (self->poly1, point2d_new (150, 100));
    g_queue_push_tail (self->poly1, point2d_new (200, 102));
    g_queue_push_tail (self->poly1, point2d_new (250, 100));
    g_queue_push_tail (self->poly1, point2d_new (300, 101));
    g_queue_push_tail (self->poly1, point2d_new (350, 101));
    g_queue_push_tail (self->poly1, point2d_new (400, 101));

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolylineShiftTester *self = 
        (PolylineShiftTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
        point2d_free ( (point2d_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    free (self);
}

typedef struct {
    double r;
    double g;
    double b;
} color_t;

color_t COLORS[] = {
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
draw_polygon (pointlist2d_t * poly, int *associations)
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
    PolylineShiftTester *self = 
        (PolylineShiftTester*) super;
    glColor3f (1, 1, 0);

    pointlist2d_t *poly1 = pointlist2d_new_from_gqueue (self->poly1);
    if (!poly1) return;

    draw_polygon (poly1, NULL);

    int *associations = NULL;
    pointlist2d_t * poly2 = geom_polyline_shift_sideways_labeled_2d (poly1, 
            -20, &associations);
    draw_polygon (poly2, associations);
    free (associations);
    associations = NULL;
    pointlist2d_free (poly2);

    pointlist2d_t * poly3 = geom_polyline_shift_sideways_labeled_2d (poly1, 
            30, &associations);
    draw_polygon (poly3, associations);
    free (associations);
    pointlist2d_free (poly3);

    pointlist2d_free (poly1);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolylineShiftTester *self = 
        (PolylineShiftTester*) super;

    if (event->button == 1) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        while (! g_queue_is_empty (self->poly1)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->poly1));
        }
    }
}
