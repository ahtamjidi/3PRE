#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

//#define CHECK_INT

typedef struct _PolygonEdgePointsTester {
    GeometryTester parent;

    GQueue * poly1;
} PolygonEdgePointsTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
polygon_edge_points_tester_new ()
{
    PolygonEdgePointsTester * self = 
        (PolygonEdgePointsTester*) calloc (1, 
                sizeof (PolygonEdgePointsTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polygon edge points";

    self->poly1 = g_queue_new ();
    g_queue_push_tail (self->poly1, point2i_new (100, 100));
    g_queue_push_tail (self->poly1, point2i_new (250, 100));
    g_queue_push_tail (self->poly1, point2i_new (300, 200));
    g_queue_push_tail (self->poly1, point2i_new (100, 220));

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolygonEdgePointsTester *self = 
        (PolygonEdgePointsTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
        point2i_free ( (point2i_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    free (self);
}

static void 
draw (GeometryTester *super)
{
    PolygonEdgePointsTester *self = 
        (PolygonEdgePointsTester*) super;
    glColor3f (1, 1, 0);

    pointlist2i_t *poly1 = pointlist2i_new_from_gqueue (self->poly1);
    if (!poly1) return;

    polygon2i_t p = {
        .nlists = 1,
        .pointlists = poly1,
    };
    int vals[4];
    glGetIntegerv (GL_VIEWPORT, vals);

    pointlist2i_t *edge_points = geom_compute_polygon_edge_points_2i (&p);
    if (edge_points) {
        printf ("%d points\n", edge_points->npoints);
        glColor3f (1, 1, 1);
        glBegin (GL_POINTS);
        for (int i=0; i<edge_points->npoints; i++) {
            point2i_t pt = edge_points->points[i];
            glVertex2i (pt.x, pt.y);
//            printf ("%d %d\n", pt.x, pt.y);
        }
        glEnd ();
        pointlist2i_free (edge_points);
    }

    pointlist2i_free (poly1);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolygonEdgePointsTester *self = 
        (PolygonEdgePointsTester*) super;

    if (event->button == 1) {
        point2i_t * newpoint = point2i_new (event->x, event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        while (! g_queue_is_empty (self->poly1)) {
            point2i_free ((point2i_t*) g_queue_pop_head (self->poly1));
        }
    }
}
