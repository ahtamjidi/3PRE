#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

//#define CHECK_INT

typedef struct _PolygonCoveredPointsTester {
    GeometryTester parent;

    GQueue * poly1;
} PolygonCoveredPointsTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
polygon_covered_points_tester_new ()
{
    PolygonCoveredPointsTester * self = 
        (PolygonCoveredPointsTester*) calloc (1, 
                sizeof (PolygonCoveredPointsTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polygon covered points";

    self->poly1 = g_queue_new ();
#if 0
    g_queue_push_tail (self->poly1, point2i_new (100, 100));
    g_queue_push_tail (self->poly1, point2i_new (200, 100));
    g_queue_push_tail (self->poly1, point2i_new (300, 100));
    g_queue_push_tail (self->poly1, point2i_new (400, 200));
    g_queue_push_tail (self->poly1, point2i_new (100, 200));
    g_queue_push_tail (self->poly1, point2i_new (90, 180));
    g_queue_push_tail (self->poly1, point2i_new (110, 160));
    g_queue_push_tail (self->poly1, point2i_new (110, 150));
    g_queue_push_tail (self->poly1, point2i_new (110, 140));
    g_queue_push_tail (self->poly1, point2i_new (80, 120));
#else
    g_queue_push_tail (self->poly1, point2i_new (227, 242));
    g_queue_push_tail (self->poly1, point2i_new (262, 300));
    g_queue_push_tail (self->poly1, point2i_new (184, 257));
    g_queue_push_tail (self->poly1, point2i_new (184, 242));
#endif

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolygonCoveredPointsTester *self = 
        (PolygonCoveredPointsTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
        point2i_free ( (point2i_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    free (self);
}

static void 
draw (GeometryTester *super)
{
    PolygonCoveredPointsTester *self = 
        (PolygonCoveredPointsTester*) super;
    glColor3f (1, 1, 0);

    glPushMatrix ();
    glTranslatef (0, -50, 0);

    pointlist2i_t *poly1 = pointlist2i_new_from_gqueue (self->poly1);
    if (!poly1) return;

    polygon2i_t p = {
        .nlists = 1,
        .pointlists = poly1,
    };
    int vals[4];
    glGetIntegerv (GL_VIEWPORT, vals);

    pointlist2i_t *edge_points = geom_compute_polygon_covered_points_2i (&p);
    if (edge_points) {
        glColor3f (1, 1, 1);
        glBegin (GL_POINTS);
        for (int i=0; i<edge_points->npoints; i++) {
            point2i_t pt = edge_points->points[i];
            glVertex2i (pt.x, pt.y);
        }
        glEnd ();
        pointlist2i_free (edge_points);
    }

    glPopMatrix ();

    pointlist2i_free (poly1);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolygonCoveredPointsTester *self = 
        (PolygonCoveredPointsTester*) super;

    if (event->button == 1) {
        point2i_t * newpoint = point2i_new (event->x, event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        while (! g_queue_is_empty (self->poly1)) {
            point2i_free ((point2i_t*) g_queue_pop_head (self->poly1));
        }
    }
}
